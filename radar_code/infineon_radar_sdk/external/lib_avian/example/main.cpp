/**
 * \file main.cpp
 *
 * This files demonstrates how to use libAvian and Strata to configure an
 * Avian radar sensor connected to a Radar Baseboard MCU7 ("Hatvan") and
 * acquire sensor through that sensor. Acquired data is simply written to
 * console.
 */
/* ===========================================================================
** Copyright (C) 2016-2023 Infineon Technologies AG
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
#include "ifxAvian_DataConverter.hpp"
#include "ifxAvian_Driver.hpp"
#include "ifxAvian_ErrorCodes.hpp"
#include "platform/BoardManager.hpp"
#include "ports/ifxAvian_StrataPort.hpp"
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <vector>

// ---------------------------------------------------------------------------- main
int main()
{
    /*
     * In case of a communication error Strata throws an exception. The method
     * of Infineon::Avian::Driver return status codes, which are turned into
     * exceptions through the function Infineon::Avian::check_error.
     */
    try
    {
        using namespace Infineon::Avian;

        /**********************************************************************/
        /* Connection to Radar Baseboard MCU7 ("Hatvan")                      */
        /**********************************************************************/
        /*
         * While the rest of this example is platform independent and refers
         * only to Avian library, this part is specific for Hatvan sensor
         * board. To adapt this example for a different hardware setup, this
         * section must be replaced by some code that creates a different
         * implementation of Infineon::Avian::IPort according to the desired
         * target platform.
         *
         * The BoardManager class is part of the Strata library and is needed
         * to find the connected Hatvan sensor board. If boards are found, the
         * first one is taken and a board instance representing that board is
         * created. From that board instance a StrataPort is created which
         * implements the interface Infineon::Avian::IPort needed by libAvian.
         * StrataPort does not take ownership of the board instance, so the
         * lifetime of the board instance object must be managed manually and
         * it must be guaranteed that the board instance does not go out of
         * scope before the StrataPort is destroyed.
         */
        BoardManager board_manager;
        board_manager.enumerate();
        if (board_manager.getEnumeratedList().empty())
            throw std::runtime_error("No sensor board found.");

        auto& descriptor = board_manager.getEnumeratedList().front();
        auto board_instance = descriptor->createBoardInstance();
        StrataPort avian_port(board_instance.get());

        /**********************************************************************/
        /* Driver Creation                                                    */
        /**********************************************************************/
        /*
         * The class Infineon::Avian::Driver generates a register configuration
         * for the Avian radar sensor according to the user provided
         * parameters. The static method create_driver uses the provided IPort
         * instance to read the chip ID to detect the type of the Avian device
         * and create the driver instance accordingly. In case of an error the
         * sensor type might not be recognized and no driver instance is
         * created.
         *
         * (If the device type is known upfront, it's also possible to create
         * the driver instance using the constructor. Anyway, there are
         * different RF shields for the Hatvan sensor board, and this example
         * cannot know what RF shield is used.)
         */
        auto driver = Driver::create_driver(avian_port);
        if (!driver)
            throw std::runtime_error("Avian driver instance could not be "
                                     "created.");

        /**********************************************************************/
        /* Avian Sensor Configuration                                         */
        /**********************************************************************/
        /*
         * The following code defines the chirp sequence used for data
         * acquisition. All parameters are passed to the driver instance.
         *
         * First a frame consisting of 16 chirp with 500 micro seconds delay
         * in between is configured. The sensor is configured to acquire
         * 10 frames before it goes automatically to deep sleep mode. Frames
         * are acquired with 500ms delay in between to avoid a FIFO overflows
         * when data is written to the console, which is very slow.
         */
        Frame_Definition frame_definition = {};
        frame_definition.shapes[0].num_repetitions = 1;
        frame_definition.shapes[0].following_power_mode = Power_Mode::Idle;
        frame_definition.shapes[0].post_delay_100ps = 5000000;
        frame_definition.shape_set.num_repetitions = 16;
        frame_definition.shape_set.following_power_mode = Power_Mode::Idle;
        frame_definition.shape_set.post_delay_100ps = 5000000000;
        frame_definition.num_frames = 10;
        check_error(driver->set_frame_definition(&frame_definition));

        /*
         * Next the chirp parameters are configured. In general a frame can
         * contain up to four shapes with two chirps (up and down) each.
         * Before configuring the chirp parameters the driver instance must
         * be informed which of these eight different chirps will be
         * configured next. In this case it's the up-chirp of shape 1.
         */
        check_error(driver->select_shape_to_configure(0, false));

        /*
         * The FMCW configuration refers to the whole shape. In case of a
         * triangle shape RF range and transmission power are applied to
         * both, up and down chirp. Anyway in this example there is just an
         * up chirp, which is also defined through the FMCW configuration.
         */
        Fmcw_Configuration fmcw_configuration;
        fmcw_configuration.lower_frequency_kHz = 58000000;
        fmcw_configuration.upper_frequency_kHz = 63000000;
        fmcw_configuration.shape_type = Shape_Type::Saw_Up;
        fmcw_configuration.tx_power = 31;
        check_error(driver->set_fmcw_configuration(&fmcw_configuration));

        /*
         * The frame format defines number of samples per chirp and which RX
         * antennas are used for data acquisition. The
         * field num_chirps_per_frame must always be one. It has no meaning,
         * because the number of chirp repetitions is set through the method
         * set_frame_definition.
         * (The name "frame format" is misleading, but was kept for historical
         * reasons.)
         *
         * In case of a triangle shape, the "frame format" can be set
         * differently for up and down chirp. The call of set_frame_format
         * always applies to the chirp recently selected through method
         * select_shape_to_configure.
         */
        Frame_Format frame_format;
        frame_format.num_samples_per_chirp = 64;
        frame_format.num_chirps_per_frame = 1;
        frame_format.rx_mask = 0x01;
        check_error(driver->set_frame_format(&frame_format));

        /*
         * The base band configuration defines the parameters of the analog
         * base band chain of filters and amplifiers. The parameters are set
         * individually for each RX channel. The structure always takes
         * parameters for maximum of four RX channels. The parameters for the
         * non-existing RX channels are ignored. Anyhow, Parameters should be
         * set for all four channels, because set_baseband_configuration may
         * fail, if some fields of the structure are undefined.
         *
         * In case of a triangle shape, the "base band configuration can be
         * set differently for up and down chirp. The call of
         * set_baseband_configuration always applies to the chirp recently
         * selected through method select_shape_to_configure.
         */
        Baseband_Configuration baseband_configuration;
        baseband_configuration.hp_gain_1 = Hp_Gain::_18dB;
        baseband_configuration.hp_cutoff_1_Hz = 80000;
        baseband_configuration.vga_gain_1 = Vga_Gain::_5dB;
        baseband_configuration.hp_gain_2 = Hp_Gain::_18dB;
        baseband_configuration.hp_cutoff_2_Hz = 80000;
        baseband_configuration.vga_gain_2 = Vga_Gain::_5dB;
        baseband_configuration.hp_gain_3 = Hp_Gain::_18dB;
        baseband_configuration.hp_cutoff_3_Hz = 80000;
        baseband_configuration.vga_gain_3 = Vga_Gain::_5dB;
        baseband_configuration.hp_gain_4 = Hp_Gain::_18dB;
        baseband_configuration.hp_cutoff_4_Hz = 80000;
        baseband_configuration.vga_gain_4 = Vga_Gain::_5dB;
        baseband_configuration.reset_period_100ps = 5000;
        check_error(driver->set_baseband_configuration(&baseband_configuration));

        /*
         * ADC configuration is global and applies to all chirps. The method
         * select_shape_to_configure has no impact on the behavior of method
         * set_adc_configuration.
         * For sure sampling rate is the most important parameter here, the
         * other parameters are reasonable defaults, but have an impact on the
         * maximum possible sampling rate. The absolute maximum sampling rate
         * of 4MHz can only be achieved with 50ns sampling time, no tracking
         * and single MSB decision time.
         * Oversampling is anyhow just a parameter for the lab and should be
         * set disabled.
         */
        Adc_Configuration adc_config;
        adc_config.samplerate_Hz = 2000000;
        adc_config.sample_time = Adc_Sample_Time::_100ns;
        adc_config.tracking = Adc_Tracking::_1_Subconversion;
        adc_config.double_msb_time = 1;
        adc_config.oversampling = Adc_Oversampling::Off;
        check_error(driver->set_adc_configuration(&adc_config));

        /*
         * The stream of acquired radar data is chopped into slices. The Avian
         * device generates an interrupt signal, when the FIFO contains enough
         * samples for one slice and IPort reads data blocks with size of a
         * slice.
         * It is highly recommended to align the slice size with the frame
         * size. Frame size shall be an integer multiple of the slice size,
         * and slice size shall not exceed half of the FIFO size.
         * In this simple example the slice size is set to the frame size.
         * Usually also the number of enabled RX antennas must also be
         * considered, but this example uses only one RX antenna.
         */
        auto slice_size = uint16_t(frame_format.num_samples_per_chirp
                                   * frame_definition.shape_set.num_repetitions);
        check_error(driver->set_slice_size(slice_size));

        /**********************************************************************/
        /* Data Acquisition                                                   */
        /**********************************************************************/
        /*
         * Typically implementations of IPort provide raw data where two 12bit
         * sample values are packed into 3 bytes. While the packed data format
         * is good for efficient data storage, it is not well suited for data
         * processing. The tedious job of unpacking raw data and separating
         * the sample values is done by Infineon::Avian::DataConverter. That
         * class implements the interface Infineon::Avian::IReadPort, a subset
         * of Infineon::Avian::IPort. For purpose of data acquisition it can
         * be used in the same way as the IPort instance representing the real
         * hardware. DataConverter wraps smoothly around the IPort provided at
         * construction time, and forwards all necessary call to that IPort
         * instance, so no direct usage of IPort is needed.
         */
        DataConverter<uint16_t> data_converter(avian_port);

        /*
         * The memory buffer for data acquisition is owned and managed by the
         * application. For this simple example a single buffer is sufficient
         * and can be re-used for each frame.
         */
        std::vector<uint16_t> data_buffer(slice_size);
        unsigned frame_counter = 0;

        /*
         * An application receives acquired raw data through a callback. Each
         * time a data slice is available the callback is invoked. In an
         * embedded target the callback is typically invoked from an interrupt
         * handler, so the callback should occupy the CPU as short as possible.
         * Nevertheless, this example is not embedded and for simplicity all
         * "data processing" happens right here in the callback.
         *
         * The buffer containing the acquired data is not passed to the
         * callback. The callback must know what buffer has been provided to
         * the IPort.
         */
        auto data_callback = [&](uint32_t status_word) -> void {
            /*
             * Sample values are written to the console.Also the status word
             * received during SPI burst initiation is written, even though it
             * is typically meaningless.
             */
            auto num_samples = frame_format.num_samples_per_chirp;
            auto num_chirps = frame_definition.shape_set.num_repetitions;
            for (unsigned chirp = 1; chirp <= num_chirps; ++chirp)
            {
                std::cout << std::dec << "Frame " << frame_counter
                          << ", Chirp " << chirp << " (status = 0x"
                          << std::hex << std::setw(8) << std::setfill('0')
                          << status_word << "):" << std::dec << std::endl;

                for (unsigned smp = 0; smp < num_samples; ++smp)
                    std::cout << std::setw(4) << data_buffer[smp] << ", ";
                std::cout << std::endl;
            }
            std::cout << std::endl;

            /*
             * All data is processed, so the buffer can be re-used for the
             * next frame. If no new buffer is provided to the data converter
             * here, data reception stops (see below).
             */
            data_converter.set_buffer(data_buffer.data());

            /*
             * The frames are counted, because the main thread waits for the
             * last frame to be acquired.
             */
            ++frame_counter;
        };

        /*
         * Before the data acquisition can be started, the data read path must
         * be prepared. Otherwise a FIFO overflow could occur even before the
         * application receives the first slice of data.
         *
         * An IReadPort instance takes care that interrupt requests from an
         * Avian device is handled and an SPI burst transfer to read a data
         * slice is started. The application just has to provide some
         * information needed by the interrupt handler and of course a data
         * buffer.
         *
         * Parameters for the interrupt handler are passed to the IPort
         * instance through the method start_reader. It is assumed that these
         * parameters don't change during an acquisition session:
         * - SPI burst prefix word (depends on the Avian device type)
         * - SPI bust size (must be same as the slice size told to the driver
         *   instance)
         * - The callback to be invoked, once the SPI burs transfer is over.
         *
         * To avoid unnecessary data copying, an IReadPort instance does not
         * own any memory buffer for data acquisition. This means the
         * application must provide a buffer to the IReadPort instance,
         * otherwise the application cannot receive any data from the
         * IReadPort instance. Once a provided buffer is filled, the IReadPort
         * instance assumes that the buffer is occupied, so a new buffer must
         * be provided for the next data slice. Ideally the new buffer is
         * passed to the IReadPort instance from the callback, just like this
         * example does.
         *
         * Finally it should mentions, that start_reader and set_buffer could
         * also be called on avian_port rather than data_converter. In this
         * case the application would received packed data, rather then
         * separated samples.
         */
        data_converter.start_reader(driver->get_burst_prefix(), data_buffer.size(),
                                    data_callback);
        data_converter.set_buffer(data_buffer.data());

        /*
         * Now that the data read path is prepared, the Avian device can
         * finally start to acquire data. Up to here the device has not been
         * programmed. All parameters have only be passed to the Avian driver
         * instance. The method get_device_configuration returns the register
         * set representing the configuration defined through the user
         * parameters. Sending the register set to the Avian device with the
         * start bit set, starts acquisition.
         * (Note that the register set cannot be sent to data_converter
         * because a DataConverter instance handles only the data path, but
         * the register set is control information.)
         */
        avian_port.generate_reset_sequence();
        driver->get_device_configuration().send_to_device(avian_port, true);

        /*
         * After all data frames have been acquired, acquisition and reading
         * is stopped. Actually resetting the Avian device to stop acquisition
         * is not necessary for this example, because after acquiring a
         * finite number of frames the Avian radar device automatically goes
         * to deep sleep mode. It would be required if endless acquisition
         * had been configured (frame_definition.num_frames = 0). In this case
         * it is recommended to reset the device before stopping data reading
         * to avoid any dangling unread data somewhere in the data path.
         */
        while (frame_counter < frame_definition.num_frames)
            ;
        avian_port.generate_reset_sequence();
        data_converter.stop_reader();

        return 0;
    }
    catch (std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
}
