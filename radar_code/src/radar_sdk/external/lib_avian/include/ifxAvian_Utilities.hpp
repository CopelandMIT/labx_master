/**
 * \file ifxAvian_Utilities.hpp
 *
 * This file declares auxiliary functions that support usage of Avian devices.
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

#ifndef IFX_AVIAN_UTILITIES_H
#define IFX_AVIAN_UTILITIES_H

// ---------------------------------------------------------------------------- includes
#include "ifxAvian_IPort.hpp"
#include "ifxAvian_Types.hpp"
#include <cstdint>

// ---------------------------------------------------------------------------- namespaces
namespace Infineon {
namespace Avian {

// ---------------------------------------------------------------------------- Lib_Version
/**
 * \brief This structure holds the version of libAvian.
 */
struct Lib_Version
{
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
};

// ---------------------------------------------------------------------------- get_lib_version
/**
 * \brief This function returns the version number of libAvian.
 */
Lib_Version get_lib_version();

// ---------------------------------------------------------------------------- read_device_type
/**
 * This function reads the chip version register of Avian device connected to
 * provided port. The read information is evaluated and the Avian device
 * type is returned.
 */
Device_Type read_device_type(HW::IControlPort& port);

// ---------------------------------------------------------------------------- detect_device_type
/**
 * This function detects the chip version from the value of an Avian device's
 * CHIP_ID register.
 */
Device_Type detect_device_type(uint32_t chip_id_register);

// ---------------------------------------------------------------------------- get_min_sampling_rate
/**
 * This function returns the minimum ADC sampling rate for RX signal
 * acquisition in Hz.
 *
 * \param[in] ref_clock_freq  This is the frequency of the Avian device's
 *                            reference clock oscillator. This parameter is
 *                            optional. If it is omitted the recommended
 *                            default frequency of 80MHz is assumed.
 */
float get_min_sampling_rate(float ref_clock_freq = 80.0e6f);

// ---------------------------------------------------------------------------- get_max_sampling_rate
/**
 * This function returns the maximum ADC sampling rate for RX signal
 * acquisition in Hz.
 *
 * The maximum sampling rate depends on the ADC configuration, so the ADC
 * parameters must be passed to this function.
 *
 * \param[in] sample_time      See \ref Adc_Sample_Time for details.
 * \param[in] tracking         See \ref Adc_Tracking for details.
 * \param[in] double_msb_time  This flags indicates if the ADC uses the double
 *                             amount of clock cycles to device the value of
 *                             the most significant bit (MSB).
 * \param[in] oversampling     See \rif Adc_Oversampling for details.
 * \param[in] ref_clock_freq   This is the frequency of the Avian device's
 *                             reference clock oscillator. This parameter is
 *                             optional. If it is omitted the recommended
 *                             default frequency of 80MHz is assumed.
 */
float get_max_sampling_rate(Adc_Sample_Time sample_time,
                            Adc_Tracking tracking,
                            bool double_msb_time,
                            Adc_Oversampling oversampling,
                            float ref_clock_freq = 80.0e6f);

// ---------------------------------------------------------------------------- initialize_reference_clock
/**
 * This initializes the reference clock of the connection Avian device.
 *
 * This function needs the configuration command word, which can be retrieved
 * from an instance of \ref Driver through the method
 * \ref Driver::get_clock_config_command(). According to that command word
 * this function programs the according register, performs the oscillator
 * startup sequence (if needed), or skips initialization completely if not
 * needed (e.g. for Avian C devices).
 *
 * \param[in] port                  The port the Avian device is connected to.
 * \param[in] clock_config_command  The command word that sets the final
 *                                  reference clock configuration.
 */
void initialize_reference_clock(HW::IControlPort& port,
                                HW::Spi_Command_t clock_config_command);

/* ------------------------------------------------------------------------ */
}  // namespace Avian
}  // namespace Infineon

#endif /* IFX_AVIAN_UTILITIES_H */

/* --- End of File -------------------------------------------------------- */
