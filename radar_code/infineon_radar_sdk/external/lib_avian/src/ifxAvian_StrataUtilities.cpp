/**
 * \file ifxAvian_Utilities.cpp
 *
 * This file implements auxiliary functions that support usage of Avian
 * devices.
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

#define USE_SADC_FOR_CLOCK_DETECTION

// ---------------------------------------------------------------------------- includes
#include "ifxAvian_StrataUtilities.hpp"

#include "common/Finally.hpp"
#include "Driver/registers_BGT60TRxxC.h"
#include "Driver/registers_BGT60TRxxE.h"
#include "ifxAvian_DeviceTraits.hpp"
#include "ifxAvian_Utilities.hpp"
#include "platform/exception/EProtocolFunction.hpp"
#include "ports/ifxAvian_StrataControlPort.hpp"
#include "register_maps/BGT120TR24E_ID000A.hpp"
#include "register_maps/BGT60TRxxC_ID0003.hpp"
#include "register_maps/BGT60TRxxC_ID0005.hpp"
#include "register_maps/BGT60TRxxD_ID0006.hpp"
#include "register_maps/BGT60TRxxD_ID0007.hpp"
#include "register_maps/BGT60TRxxE_ID0008.hpp"
#include "register_maps/BGT60TRxxE_ID0009.hpp"
#include "universal/spi_definitions.h"
#include "universal/types/DataSettingsBgtRadar.h"

#include <array>
#include <functional>
#include <stdexcept>


#ifndef USE_SADC_FOR_CLOCK_DETECTION
#include <chrono>
#include <thread>
#endif

// ---------------------------------------------------------------------------- namespaces
namespace Infineon {
namespace Avian {

// ---------------------------------------------------------------------------- generate_oscillator_measurement_signal
/**
 * \brief This function configures the chip to output a signal on the IRQ pin
 *        for detection of the oscillator frequency.
 *
 * Internally an ADC is configured to continuously generate a busy signal with
 * an as long as possible period.
 * Currently this works only with SADC, because when using the MADC the
 * observed frequency depends only on the SPI speed instead of the reference
 * clock.
 * \todo To make the measurement work with BGT60UTR11AIP too, MADC should be used.
 */
void generate_oscillator_measurement_signal(HW::IControlPort& port,
                                            size_t command_repetitions)
{
    /*
     * These register reset values are used below to overwrite the bit fields
     * DFT0:DEBUG_DIG and PACR1:OSCCLEKEN. This can only work because it's
     * known that these bit fields are reset to 0.
     */
    constexpr uint32_t BGT60TRxxC_DFT0_Reset = 0x000200;
    constexpr uint32_t BGT60TRxxC_PACR1_Reset = 0x196524;

#ifdef USE_SADC_FOR_CLOCK_DETECTION
    /*
     * The SADC module is enabled in Deep Sleep mode and configured for a long
     * conversion time, using 32x oversampling. The SADC busy signal is routed
     * to the IRQ pin and the system clock is enabled via the PACR1 register.
     * It is assumed that this code is never run for Avian C devices, so the
     * register and bit field layout introduced with BGT60UTR13D is assumed.
     */
    std::array<HW::Spi_Command_t, 4> setup_sadc;
    setup_sadc[0] = BGT60TRxxC_SET(CSCDS, ABB_ISOPD, 1)
                    | BGT60TRxxC_SET(CSCDS, RF_ISOPD, 1)
                    | BGT60TRxxC_SET(CSCDS, BG_EN, 0)
                    | BGT60TRxxC_SET(CSCDS, MADC_ISOPD, 1)
                    | BGT60TRxxC_SET(CSCDS, SADC_ISOPD, 0)
                    | BGT60TRxxC_SET(CSCDS, BG_TMRF_EN, 0)
                    | BGT60TRxxC_SET(CSCDS, PLL_ISOPD, 1);
    setup_sadc[1] = BGT60TRxxC_SET(SADC_CTRL, SADC_CHSEL, 0)
                    | BGT60TRxxC_SET(SADC_CTRL, START_SADC, 0)
                    | BGT60TRxxE_SET(SADC_CTRL, SADC_CLK_DIV, 3)
                    | BGT60TRxxC_SET(SADC_CTRL, SD_EN, 0)
                    | BGT60TRxxC_SET(SADC_CTRL, OVERS_CFG, 3)
                    | BGT60TRxxC_SET(SADC_CTRL, SESP, 0)
                    | BGT60TRxxC_SET(SADC_CTRL, LVGAIN, 0)
                    | BGT60TRxxC_SET(SADC_CTRL, DSCAL, 1)
                    | BGT60TRxxC_SET(SADC_CTRL, TC_TRIM, 0);
    setup_sadc[2] = BGT60TRxxC_DFT0_Reset
                    | BGT60TRxxC_SET(DFT0, DEBUG_DIG, 3);
    setup_sadc[3] = BGT60TRxxC_PACR1_Reset
                    | BGT60TRxxC_SET(PACR1, OSCCLKEN, 1);
    port.send_commands(setup_sadc.data(), setup_sadc.size());

    /*
     * A long  SPI sequence is generated that keeps on triggering the SADC.
     * While the SADC is busy the trigger words are ignored, but when the SADC
     * has finished it is immediately started again. This results in a toggling
     * SADC busy signal which is then measured.
     */
    std::vector<HW::Spi_Command_t> trigger_sadc(command_repetitions);
    for (auto& command : trigger_sadc)
        command = setup_sadc[1] | BGT60TRxxC_SET(SADC_CTRL, START_SADC, 1);
    port.send_commands(trigger_sadc.data(), trigger_sadc.size());

#else
    // USE_MADC

    /*
     * The MADC module is enabled in Deep Sleep mode and configured for a long
     * conversion time, using 8x oversampling. The MADC busy signal is routed
     * to the IRQ pin and the system clock is enabled via the PACR1 register.
     * It is assumed that this code is never run for Avian C devices, so the
     * register and bit field layout introduced with BGT60UTR13D is assumed.
     *
     * Note: The MADC is enabled twice, because on newer devices the MADC_EN
     *       bit was removed and internally derived from BBCH_SEL.
     */
    std::array<HW::Spi_Command_t, 5> setup_madc;
    setup_madc[0] = BGT60TRxxC_SET(CSP_D_1, MADC_EN, 1)
                    | BGT60TRxxC_SET(CSP_D_1, BBCH_SEL, 1);
    setup_madc[1] = BGT60TRxxC_SET(CSCDS, ABB_ISOPD, 1)
                    | BGT60TRxxC_SET(CSCDS, RF_ISOPD, 1)
                    | BGT60TRxxC_SET(CSCDS, BG_EN, 1)
                    | BGT60TRxxC_SET(CSCDS, MADC_ISOPD, 0)
                    | BGT60TRxxC_SET(CSCDS, SADC_ISOPD, 1)
                    | BGT60TRxxC_SET(CSCDS, BG_TMRF_EN, 0)
                    | BGT60TRxxC_SET(CSCDS, PLL_ISOPD, 1);
    setup_madc[2] = BGT60TRxxC_SET(ADC0, ADC_OVERS_CFG, 3)
                    | BGT60TRxxC_SET(ADC0, BG_TC_TRIM, 0)
                    | BGT60TRxxC_SET(ADC0, BG_CHOP_EN, 0)
                    | BGT60TRxxC_SET(ADC0, STC, 3)
                    | BGT60TRxxC_SET(ADC0, DSCAL, 1)
                    | BGT60TRxxC_SET(ADC0, TRACK_CFG, 3)
                    | BGT60TRxxC_SET(ADC0, TRIG_MADC, 0)
                    | BGT60TRxxC_SET(ADC0, ADC_DIV, 40);
    setup_madc[3] = BGT60TRxxC_DFT0_Reset | BGT60TRxxC_SET(DFT0, DEBUG_DIG, 4);
    setup_madc[4] = BGT60TRxxC_PACR1_Reset | BGT60TRxxC_SET(PACR1, OSCCLKEN, 1);
    port.send_commands(setup_madc.data(), setup_madc.size());

    /*
     * Wait a little to let band gap startup.
     * \Todo: Check if this is really necessary. Bandgap should not be required
     * because ADC shall not capture signals, but only generate a clock signal.
     */
    const auto madc_bg_settling_time = std::chrono::milliseconds(1);
    std::this_thread::sleep_for(madc_bg_settling_time);

    /*
     * A long SPI sequence is generated that keeps on triggering the MADC.
     * While the MADC is busy the trigger words are ignored, but when the MADC
     * has finished it is immediately started again. This results in a toggling
     * MADC busy signal which is then measured.
     */
    std::vector<HW::Spi_Command_t> trigger_madc(command_repetitions);
    for (auto& command : trigger_madc)
        command = setup_madc[2] | BGT60TRxxC_SET(ADC0, TRIG_MADC, 1);
    port.send_commands(trigger_madc.data(), trigger_madc.size());
#endif

    port.generate_reset_sequence();
}

// ---------------------------------------------------------------------------- detect_reference_clock
bool detect_reference_clock(BoardInstance& board, Driver& driver)
{
    /*
     * Avian C devices work only with external 80Mhz oscillator. There is no
     * frequency doubler and no internal oscillator.
     *
     * Measurement does not work with BGT60UTR11AIP, because that device does not
     * have an SADC which is needed to generate the signal to measure.
     */
    auto device_type = driver.get_device_type();
    auto& traits = Device_Traits::get(device_type);
    if (!traits.has_ref_frequency_doubler || !traits.has_sadc)
        return true;

    /*
     * Before starting the measurement the Avian device is put into a defined
     * state.
     *
     * Note: After a reset it is usually required to configure the high-speed,
     *       but here this is not required, because there is no register read
     *       back.
     */
    StrataControlPort avian_port(&board);
    avian_port.generate_reset_sequence();

    /*
     * For oscillator frequency measurement, a special firmware feature is
     * required. The data path is configured in a way that data frames are
     * generated that contain just timestamps of IRQ events. To reduce the
     * system load, not each IRQ event is counted, but only each Nth (defined
     * through scaler) is considered. Frame rate is reduced by putting
     * multiple timestamps (defined through numbe_of_edges) into one frame.
     * The firmware may not support this time stamp recording mode. In this
     * case Strata throws an exception which is caught here.
     */
    uint8_t data_index = 0;
    auto* data = board.getIBridge()->getIBridgeControl()->getIData();
    auto* bridge_data = board.getIBridge()->getIBridgeData();

    const auto pool_size = 2;
    const uint16_t scaler = 32;
    const uint16_t number_of_edges = 4;
    const uint8_t settings[] =
        {
            DATA_SETTINGS_BGT_RADAR_ENTRY(scaler - 1, number_of_edges)};

    IDataProperties_t properties = {};
    properties.format = DataFormat_U64;
    try
    {
        data->configure(data_index, &properties, &settings);
    }
    catch (const EProtocolFunction& e)
    {
        if (e.code() == E_INVALID_PARAMETER)
            return false;
        else
            throw e;
    }

    const uint32_t buffer_size = number_of_edges * 8;
    bridge_data->setFrameBufferSize(buffer_size);
    bridge_data->setFrameQueueSize(pool_size);

    /*
     * For the measurement the SPI clock is programmed to the minimum value
     * supported by the Hatvan firmware. This is because the Avian device keeps
     * system clock disabled in Deep Sleep mode, but during SPI transfer the
     * system clock is temporarily enabled. The low SPI speed shall keep system
     * clock enabled for a longer time.
     * After the measurement has completed (both successful, or when an error
     * occurs), the SPI configuration must be reset to the regular speed.
     */
    const uint8_t dev_id = 0;
    const uint8_t spi_flags = SPI_MODE_0;
    const uint8_t spi_word_size = 8;
    const uint32_t normal_spi_speed = 50'000'000;
    const uint32_t measuring_spi_speed = 800'000;

    auto* spi = board.getISpi();
    spi->configure(dev_id, spi_flags, spi_word_size, measuring_spi_speed);

    auto cleanup = strata::finally([&]() {
        spi->configure(dev_id, spi_flags, spi_word_size, normal_spi_speed);
    });

    /*
     * The measurement is done by enabling the Strata data path and configuring
     * the Avian device to route the clock signal to the IRQ pin.
     */
    auto measure = [&]() -> IFrame* {
        /*
         * This is the maximum number of commands that can be transmitted in
         * one command packet over USB Serial.
         * TODO: A clean solution should query the maximum number of commands
         * first, because it may be different for different communication
         * channels, e.g. UDP for Aurix.
         */
        const unsigned int trigger_command_count = 1021;
        const uint16_t timeout = 100;

        data->start(data_index);
        bridge_data->startStreaming();

        generate_oscillator_measurement_signal(avian_port, trigger_command_count);
        auto* frame = bridge_data->getFrame(timeout);

        data->stop(data_index);
        bridge_data->stopStreaming();

        return frame;
    };

    if (traits.has_internal_oscillator)
    {
        /*
         * If the device type has an internal oscillator, that's the preferred
         * option. In this case there is no need for frequency detection,
         * because the internal oscillator support only 40MHz or 38.4Mhz but
         * this detection code is not precise enough to distinguish between
         * these two options.
         * To check if the internal oscillator is working the device is
         * configured for internal 40MHz clock before the measurement is done.
         */
        Oscillator_Configuration osc_config;
        driver.get_oscillator_configuration(&osc_config);
        osc_config.clock_source = Clock_Source::Internal;
        driver.set_oscillator_configuration(&osc_config);
        driver.set_reference_clock_frequency(Reference_Clock_Frequency::_40MHz);

        /*
         * The assumption is that the internal oscillator is either running at
         * the right frequency or not at all. If the oscillator is running, the
         * measure routine returns a valid frame. In that case there is nothing
         * more to do, because the driver is already configured correctly.
         */
        initialize_reference_clock(avian_port, driver.get_clock_config_command());
        if (measure() != nullptr)
            return true;

        /*
         * If there was no frame, the internal clock is not working. The
         * configuration is switched back to external 80MHz oscillator to run
         * the frequency detection.
         */
        osc_config.clock_source = Clock_Source::External;
        driver.set_oscillator_configuration(&osc_config);
        driver.set_reference_clock_frequency(Reference_Clock_Frequency::_80MHz);
        initialize_reference_clock(avian_port, driver.get_clock_config_command());
    }
    auto* frame = measure();

    /*
     * If the measurement was successful, a valid frame has been received. The
     * average period between the timestamps (in micro seconds) is calculated
     * and used to check if system clock is 40MHz and 80MHz.
     * The threshold is derived from the SPI speed of 800kHz and the SADC
     * conversion time according to the SADC configuration, because the
     * measured clock signal is the SADC busy signal. The expected values are
     * either 10kHz for 40MHz system clock and 20kHz for 80MHz system clock.
     */
    const float frequency_threshold = 15e3f;
    float measured_frequency = 0.f;
    if (frame)
    {
        if (frame->getStatusCode() == 0)
        {
            auto* timestamps = reinterpret_cast<uint64_t*>(frame->getBuffer());
            const auto count = frame->getDataSize() / sizeof(*timestamps) - 1;

            for (auto i = count; i--;)
            {
                const auto t1 = *timestamps++;
                const auto t2 = *timestamps;
                const auto delta = t2 - t1;
                const float f = 1.0e6f * scaler / delta;
                measured_frequency += f;
            }
            measured_frequency /= count;
            if (measured_frequency < frequency_threshold)
            {
                driver.set_reference_clock_frequency(Reference_Clock_Frequency::_40MHz);
            }
        }
        frame->release();
    }

    return true;
}

std::map<std::string, NamedMemory<uint16_t, uint32_t>::Address> get_register_map(Device_Type device_type)
{
    switch (device_type)
    {
        case Device_Type::BGT60TR13C:    // chip_id_digital == 3
            return Infineon::BGT60TRxxC_ID0003::registers;
        case Device_Type::BGT60ATR24C:   // chip_id_digital == 5
        case Device_Type::BGT24LTR24:    // chip_id_digital == 5
            return Infineon::BGT60TRxxC_ID0005::registers;
        case Device_Type::BGT60UTR13D:   // chip_id_digital == 6
            return Infineon::BGT60TRxxD_ID0006::registers;
        case Device_Type::BGT60UTR11AIP:    // chip_id_digital == 7
            return Infineon::BGT60TRxxD_ID0007::registers;
        case Device_Type::BGT60TR12E:    // chip_id_digital == 8
        case Device_Type::BGT120UTR13E:  // chip_id_digital == 8
            return Infineon::BGT60TRxxE_ID0008::registers;
        case Device_Type::BGT60ATR24E:
        case Device_Type::BGT24LTR13E:
            return Infineon::BGT60TRxxE_ID0009::registers;
        case Device_Type::BGT120UTR24:  // chip_id_digital == 10
            return Infineon::BGT120TR24E_ID000A::registers;
        default:
            throw std::invalid_argument("No register map available for the given device type!");
    }
}

/* ------------------------------------------------------------------------ */
}  // namespace Avian
}  // namespace Infineon

/* --- End of File -------------------------------------------------------- */
