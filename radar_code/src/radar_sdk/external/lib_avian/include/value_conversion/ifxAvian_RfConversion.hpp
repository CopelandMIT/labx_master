/**
 * \file ifxAvian_RfConversion.hpp
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

#ifndef IFX_AVIAN_RF_CONVERSION_H
#define IFX_AVIAN_RF_CONVERSION_H

// ---------------------------------------------------------------------------- includes
#include <cstdint>

// ---------------------------------------------------------------------------- namespaces
namespace Infineon {
namespace Avian {
namespace Value_Conversion {

// ---------------------------------------------------------------------------- RF_Converter
/**
 * This class converts RF frequency values between physical frequency value
 * in kHz and the according PLL register bit field FSU.
 */
class RF_Converter
{
public:
    /**
     * The constructor initializes the converter according to the specified
     * PLL parameters.
     *
     * \param[in] reference_clock_Hz  The frequency in Hz of the reference
     *                                clock oscillator of the PLL.
     * \param[in] pll_div_set         The PLL divider offset that is
     *                                configured through bit field
     *                                PARC2:DIV_SET.
     * \param[in] pll_pre_divider     The factor of the PLL's fixed divider.
     *                                The value depends on the type of the
     *                                Avian device.
     */
    RF_Converter(uint32_t reference_clock_Hz,
                 uint8_t pll_div_set, uint8_t pll_pre_divider);

    /**
     * This function converts a frequency value in kHz into the according
     * PLL register bit field value FSU.
     *
     * Even though this function returns a 32 bit signed integer, only the
     * lower 24 bit are relevant for the PLL register.
     */
    int32_t freq_to_pll(uint32_t frequency_kHz);

    /**
     * This function converts a PLL register bit field value FSU into the
     * according frequency value in kHz.
     *
     * This function takes only the lower 24 bit of the FSU value into
     * account, so this function accepts both, signed 32 bit integers and
     * 24 bit field values. For the latter the sign is extended according
     * to bit 23.
     */
    uint32_t pll_to_freq(int32_t fsu);

private:
    const uint32_t m_reference_clock_Hz;
    const uint8_t m_pll_div_set;
    const uint8_t m_pll_pre_divider;
};

/* ------------------------------------------------------------------------ */
}  // namespace Value_Conversion
}  // namespace Avian
}  // namespace Infineon

#endif /* IFX_AVIAN_RF_CONVERSION_H */

/* --- End of File -------------------------------------------------------- */
