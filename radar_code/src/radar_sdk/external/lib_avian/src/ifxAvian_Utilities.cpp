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

// ---------------------------------------------------------------------------- includes
#include "ifxAvian_Utilities.hpp"
#include "Driver/registers_BGT120TR24E.h"
#include "Driver/registers_BGT60TRxxC.h"
#include "Driver/registers_BGT60TRxxE.h"
#include "ifxAvian_IPort.hpp"
#include "Version.h"
#include <array>
#include <chrono>
#include <thread>
#include <vector>

// ---------------------------------------------------------------------------- namespaces
namespace Infineon {
namespace Avian {

// ---------------------------------------------------------------------------- checkError
Lib_Version get_lib_version()
{
    Lib_Version version;
    version.major = VERSION_MAJOR;
    version.minor = VERSION_MINOR;
    version.patch = VERSION_PATCH;
    return version;
}

// ---------------------------------------------------------------------------- read_device_type
Device_Type read_device_type(HW::IControlPort& port)
{
    /* Configure SPI high speed communication before reading from device*/
    auto properties = port.get_properties();
    HW::Spi_Command_t spi_word;
    spi_word = BGT60TRxxE_SET(SFCTL, MISO_HS_READ,
                              properties.high_speed_compensation ? 1 : 0)
               | BGT60TRxxE_SET(SFCTL, QSPI_WT,
                                properties.quad_spi_wait_cycles - 1);
    port.send_commands(&spi_word, 1);

    /* read chip ID register and check */
    spi_word = BGT60TRxxE_REGISTER_READ_CMD(CHIP_ID);
    port.send_commands(&spi_word, 1, &spi_word);
    return detect_device_type(spi_word);
}

// ---------------------------------------------------------------------------- detect_device_type
Device_Type detect_device_type(uint32_t chip_id_register)
{
    uint8_t chip_id_digital = BGT60TRxxE_EXTRACT(CHIP_ID, DIGITAL_ID,
                                                 chip_id_register);
    uint8_t chip_id_rf = BGT60TRxxE_EXTRACT(CHIP_ID, RF_ID, chip_id_register);
    uint8_t step_id = uint8_t(BGT60TRxxE_EXTRACT(CHIP_ID, STEP_ID,
                                                 chip_id_register));
    uint8_t tech_id = uint8_t(BGT60TRxxE_EXTRACT(CHIP_ID, TECH_ID,
                                                 chip_id_register));

    /*
     * Tech ID and Step ID are currently not needed for device detection.
     * They may be used in future.
     */
    (void)tech_id;
    (void)step_id;

    if ((chip_id_digital == 3) && (chip_id_rf == 3))
    {
        return Device_Type::BGT60TR13C;
    }
    else if ((chip_id_digital == 5) && (chip_id_rf == 4))
    {
        return Device_Type::BGT60ATR24C;
    }
    else if ((chip_id_digital == 6)
             && ((chip_id_rf == 6) || (chip_id_rf == 11)))
    {
        return Device_Type::BGT60UTR13D;
    }
    else if ((chip_id_digital == 7)
             && ((chip_id_rf == 7) || (chip_id_rf == 9) || (chip_id_rf == 12)))
    {
        return Device_Type::BGT60UTR11AIP;
    }
    else if ((chip_id_digital == 8) && (chip_id_rf == 12))
    {
        return Device_Type::BGT60UTR11AIP;
    }
    else if ((chip_id_digital == 8) && (chip_id_rf == 8))
    {
        return Device_Type::BGT60TR12E;
    }
    else if ((chip_id_digital == 8) && (chip_id_rf == 10))
    {
        return Device_Type::BGT120UTR13E;
    }
    else if ((chip_id_digital == 5) && (chip_id_rf == 5))
    {
        return Device_Type::BGT24LTR24;
    }
    else if ((chip_id_digital == 10)
             && ((chip_id_rf == 13) || (chip_id_rf == 14)))
    {
        return Device_Type::BGT120UTR24;
    }
    else if ((chip_id_digital == 9) && (chip_id_rf == 15))
    {
        return Device_Type::BGT60ATR24E;
    }
    else if ((chip_id_digital == 9) && (chip_id_rf == 16))
    {
        return Device_Type::BGT24LTR13E;
    }
    else
    {
        return Device_Type::Unknown;
    }
}

// ---------------------------------------------------------------------------- get_min_sampling_rate
float get_min_sampling_rate(float ref_clock_freq)
{
    /*
     * The minimum sampling frequency is limited by the maximum bit field value
     * that control the sample rate divider.
     */
    uint32_t max_num_cycles = 1023;
    return ref_clock_freq / float(max_num_cycles);
}

// ---------------------------------------------------------------------------- get_max_sampling_rate
float get_max_sampling_rate(Adc_Sample_Time sample_time,
                            Adc_Tracking tracking,
                            bool double_msb_time,
                            Adc_Oversampling oversampling,
                            float ref_clock_freq)
{
    /*
     * To calculate the maximum sampling rate the clock cycles of a single
     * conversion are counted.
     */
    uint32_t min_num_cycles = 0;

    switch (sample_time)
    {
        case Adc_Sample_Time::_50ns: min_num_cycles += 4; break;
        case Adc_Sample_Time::_100ns: min_num_cycles += 8; break;
        case Adc_Sample_Time::_200ns: min_num_cycles += 16; break;
        case Adc_Sample_Time::_400ns: min_num_cycles += 32; break;
    }

    switch (tracking)
    {
        case Adc_Tracking::None: min_num_cycles += 0; break;
        case Adc_Tracking::_1_Subconversion: min_num_cycles += 8; break;
        case Adc_Tracking::_3_Subconversions: min_num_cycles += 24; break;
        case Adc_Tracking::_7_Subconversions: min_num_cycles += 56; break;
    }

    min_num_cycles += double_msb_time ? 17 : 16;

    switch (oversampling)
    {
        case Adc_Oversampling::Off: break;
        case Adc_Oversampling::_2x: min_num_cycles *= 2; break;
        case Adc_Oversampling::_4x: min_num_cycles *= 4; break;
        case Adc_Oversampling::_8x: min_num_cycles *= 8; break;
    }

    return ref_clock_freq / float(min_num_cycles);
}

// ---------------------------------------------------------------------------- initialize_reference_clock
void initialize_reference_clock(HW::IControlPort& port,
                                HW::Spi_Command_t clock_config_command)
{
    using std::this_thread::sleep_for;
    using microseconds = std::chrono::microseconds;

    /*
     * If there is no configuration command word(e.g. for Avian C devices),
     * there's nothing to do at all.
     */
    if (clock_config_command == 0)
        return;

    /*
     * If the internal oscillator is used, the startup sequence must be
     * performed as described in BGT120UTR24 product specification document.
     */
    if (clock_config_command & BGT120TR24E_CLK_XOSC_LDO_EN_msk)
    {
        /*
         * First only XOSC_LDO_ENand XOSC_BG_EN are enabled. It is assumed that
         * those bits are set in the current configuration. All other bits are
         * masked for the first programming step.
         */
        auto spi_word = clock_config_command
                        & ~BGT120TR24E_CLK_XOSC_HP_MODE_msk
                        & ~BGT120TR24E_CLK_XOSC_FILT_EN_msk
                        & ~BGT120TR24E_CLK_XOSC_BUF_EN_msk
                        & ~BGT120TR24E_CLK_XOSC_CORE_EN_msk
                        & ~BGT120TR24E_CLK_XOSC_BG_RFILT_SEL_msk;
        port.send_commands(&spi_word, 1);

        sleep_for(microseconds(20));

        spi_word |= BGT120TR24E_CLK_XOSC_BG_RFILT_SEL_msk;
        port.send_commands(&spi_word, 1);

        sleep_for(microseconds(10));

        spi_word |= BGT120TR24E_CLK_XOSC_CORE_EN_msk;
        port.send_commands(&spi_word, 1);

        sleep_for(microseconds(300));

        spi_word |= BGT120TR24E_CLK_XOSC_BUF_EN_msk
                    | BGT120TR24E_CLK_XOSC_HP_MODE_msk;
        port.send_commands(&spi_word, 1);

        sleep_for(microseconds(20));

        spi_word |= BGT120TR24E_CLK_XOSC_FILT_EN_msk;
        port.send_commands(&spi_word, 1);
    }
    else
    {
        /*
         * If the internal oscillator is not used, just sending the
         * configuration command word as is enough to configure the
         * frequency doubler.
         */
        port.send_commands(&clock_config_command, 1);
    }
}

/* ------------------------------------------------------------------------ */
}  // namespace Avian
}  // namespace Infineon

/* --- End of File -------------------------------------------------------- */
