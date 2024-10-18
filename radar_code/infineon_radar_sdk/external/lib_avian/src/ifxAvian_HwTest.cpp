/**
 * \file ifxAvian_HwTest.cpp
 *
 * This file implements some function for hardware of system comprising an
 * Avian device.
 */
/* ===========================================================================
** Copyright (C) 2020-2023 Infineon Technologies AG
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**
** 1. Redistributions of source code must retain the above copyright notice,
**    this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. Neither the name of the copyright holder nor the names of its
**    contributors may be used to endorse or promote products derived from
**    this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
** LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
** CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
** INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
** CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
** ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
** POSSIBILITY OF SUCH DAMAGE.
** ===========================================================================
*/

// ---------------------------------------------------------------------------- includes
#include "ifxAvian_HwTest.hpp"

#include "Driver/registers_BGT60TRxxC.h"
#include "ifxAvian_DeviceTraits.hpp"
#include "ifxAvian_Driver.hpp"
#include "ifxAvian_Utilities.hpp"
#include <array>
#include <condition_variable>
#include <mutex>

// ---------------------------------------------------------------------------- namespaces
namespace Infineon {
namespace Avian {
namespace HwTest {

// ---------------------------------------------------------------------------- test_spi_connection
bool test_spi_connection(HW::IPort<HW::Packed_Raw_Data_t>& port,
                         HW::Spi_Command_t clock_config_command)
{
    constexpr uint16_t cfg_num_samples_per_chirp = 256;

    /*
     * A driver instance is used to generate a basic configuration for the
     * Avian device. The basic configuration uses a single triangle shape
     * and repeated several times. The shape set is run only once. When the
     * state machine has completed the frame, it goes to deep sleep mode.
     */
    port.generate_reset_sequence();
    auto device_type = read_device_type(port);
    if (device_type == Device_Type::Unknown)
        return false;

    Driver driver(port, device_type);

    const uint16_t fifo_size = Device_Traits::get(device_type).fifo_size * 2;
    const uint16_t num_chirps = fifo_size / cfg_num_samples_per_chirp;
    Frame_Definition frame_definition =
        {
            {/*             num_reps,   following_power_mode,   delay */
             /* Shape 1 */ {num_chirps, Power_Mode::Deep_Sleep, 0},
             /* Shape 2 */ {0, Power_Mode::Deep_Sleep, 0},
             /* Shape 3 */ {0, Power_Mode::Deep_Sleep, 0},
             /* Shape 4 */ {0, Power_Mode::Deep_Sleep, 0}},
            /* Shape Set */ {1, Power_Mode::Deep_Sleep, 0},
            /* num_frames */ 1};

    Frame_Format frame_format =
        {
            /* num_samples_per_chirp */ cfg_num_samples_per_chirp,
            /* num_chirps_per_frame  */ 1,
            /* rx_mask               */ 0x01};

    Fmcw_Configuration fmcw_configuration =
        {
            /* lower_frequency_kHz */ 60000000,
            /* upper_frequency_kHz */ 60000000,
            /* shape_type          */ Shape_Type::Saw_Up,
            /* tx_power            */ 0};

    driver.set_frame_definition(&frame_definition);
    driver.select_shape_to_configure(0, false);
    driver.set_frame_format(&frame_format);
    driver.set_fmcw_configuration(&fmcw_configuration);
    driver.set_slice_size(cfg_num_samples_per_chirp);

    /*
     * The driver generates a configuration for normal operation. This test
     * requires additional configuration to enable on device test sequence
     * generator that overwrites the data captured by the ADC.
     */
    auto register_set = driver.get_device_configuration();
    register_set.set(BGT60TRxxC_REG_SFCTL,
                     register_set[BGT60TRxxC_REG_SFCTL]
                         | BGT60TRxxC_SFCTL_LFSR_EN_msk);

    if (clock_config_command)
        register_set.set(clock_config_command);

    /*
     * Here the test sequence generator is defined. The test sequence is
     * generated by a linear feedback shift register (LFSR) with the generator
     * polynomial x^12 + x^11 + x^10 + x^4 + 1 (see Avian device data sheet).
     */
    uint16_t shift_register = 0x001;
    auto get_next_test_word = [&shift_register]() -> uint16_t {
        uint16_t current_value = shift_register;

        uint16_t next_value = (shift_register >> 1)
                              | (((shift_register << 11)
                                  ^ (shift_register << 10)
                                  ^ (shift_register << 9)
                                  ^ (shift_register << 3))
                                 & 0x0800);

        shift_register = next_value;
        return current_value;
    };

    /*
     * During the test multiple data blocks are processed. The test is finished
     * when all blocks have been processed, or when the test fails.
     * The lambda functions defined here help to track the total test state.
     */
    volatile size_t num_tested_blocks = 0;
    volatile bool test_result = true;

    auto update_state = [&](bool result) -> void {
        test_result = result;
        ++num_tested_blocks;
    };

    auto is_finished = [&]() -> bool {
        return !test_result || (num_tested_blocks == num_chirps);
    };

    auto has_succeeded = [&]() -> bool {
        return test_result && (num_tested_blocks == num_chirps);
    };

    /*
     * The processing of the received data is done in a separate thread, that
     * is owned by the port, while the main thread has to wait until the test
     * has finished. For thread synchronization a condition variable is needed,
     * which itself requires a mutex to synchronize access to the state
     * variables. The DataConverter unpacks the raw data returned by the port.
     * The data processing is defined through the lambda function passed to the
     * converter.
     */
    std::mutex state_guard;
    std::condition_variable synchronizer;

    constexpr size_t block_size = cfg_num_samples_per_chirp * 3 / 2;
    std::array<HW::Packed_Raw_Data_t, block_size> test_buffer;

    port.start_reader(driver.get_burst_prefix(), cfg_num_samples_per_chirp,
                      [&](HW::Spi_Command_t) -> void {
                          /*
                           * This is the actual test. Received data is unpacked and
                           * compared against expected data.
                           */
                          bool data_matches = true;
                          auto it = test_buffer.begin();
                          while (it != test_buffer.end())
                          {
                              uint16_t first_received = (uint16_t(*it++) << 4);
                              first_received |= *it >> 4;
                              uint16_t second_received = uint16_t(*it++ & 0x0F) << 8;
                              second_received |= *it++;

                              uint16_t first_expected = get_next_test_word();
                              uint16_t second_expected = get_next_test_word();

                              if ((first_received != first_expected)
                                  || (second_received != second_expected))
                              {
                                  data_matches = false;
                                  break;
                              }
                          }

                          /*
                           * The test state is updated. The lock synchronizes this update with
                           * the waiting main thread. The curly braces keep the locked scope is
                           * small as possible.
                           */
                          {
                              std::unique_lock<std::mutex> lock(state_guard);
                              update_state(data_matches);
                          }

                          /*
                           * If the test has finished the waiting main thread is notified,
                           * otherwise a work buffer is passed again to the data converter to
                           * keep the test thread running.
                           */
                          if (is_finished())
                              synchronizer.notify_one();
                          else
                              port.set_buffer(test_buffer.data());
                      });

    /*
     * Everything is prepared now, the test can start. Providing the work
     * buffer to the converter unblocks the processing thread. To be sure
     * the test is not influenced by any previous state, the Avian device is
     * reset, before the register configuration prepared above is programmed.
     */
    port.set_buffer(test_buffer.data());
    port.generate_reset_sequence();
    initialize_reference_clock(port, clock_config_command);
    register_set.send_to_device(port, true);

    /*
     * Here it's waited until the data processing has finished. The lock used
     * for waiting must be immediately released, because otherwise a dead lock
     * may occur when stop_reader() waits for a callback in progress which
     * itself for the state_guard mutex.
     */
    {
        std::unique_lock<std::mutex> lock(state_guard);
        synchronizer.wait_for(lock, std::chrono::seconds(1), is_finished);
    }

    /*
     * For proper cleanup, Avian device and data fetching are stopped.
     * (In case the test failed, it may still be running.)
     */
    port.stop_reader();
    port.generate_reset_sequence();

    return has_succeeded();
}

// ---------------------------------------------------------------------------- test_fifo_memory
bool test_fifo_memory(HW::IControlPort& port,
                      HW::Spi_Command_t clock_config_command)
{
    bool test_result = true;

    for (uint8_t algo = 0; algo < 4; ++algo)
    {
        // The Avian devices is reset to clear any previous state
        port.generate_reset_sequence();
        initialize_reference_clock(port, clock_config_command);

        /*
         * The memory test is started through the DFT0 register. The MBIST
         * mode must be activated and one of 4 memory test algorithms must be
         * selected.
         */
        HW::Spi_Command_t command = BGT60TRxxC_SET(DFT0, MODE, 1)
                                    | BGT60TRxxC_SET(DFT0, NHOLD, 1)
                                    | BGT60TRxxC_SET(DFT0, CTRL, 1)
                                    | BGT60TRxxC_SET(DFT0, ALG, (1 << algo));

        port.send_commands(&command, 1);

        /*
         * The end of the test is indicated through a bit in the DFT1 register.
         * A polling loop waits for the end of the memory test.
         */
        command = BGT60TRxxC_REGISTER_READ_CMD(DFT1);
        HW::Spi_Command_t test_status = 0;
        while (BGT60TRxxC_EXTRACT(DFT1, DONE, test_status) == 0)
            port.send_commands(&command, 1, &test_status);

        // The DFT1 register also contains a bit that indicates the test result.
        if (BGT60TRxxC_EXTRACT(DFT1, FAIL, test_status) != 0)
        {
            test_result = false;
            break;
        }
    }

    // The chip reset puts the Avian device back to deep sleep state.
    port.generate_reset_sequence();

    return test_result;
}

/* ------------------------------------------------------------------------ */
}  // namespace HwTest
}  // namespace Avian
}  // namespace Infineon

/* --- End of File -------------------------------------------------------- */
