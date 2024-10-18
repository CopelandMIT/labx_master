/**
 * \file ifxAvian_RfConversion.cpp
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
#include "value_conversion/ifxAvian_RfConversion.hpp"
#include <cmath>

// ---------------------------------------------------------------------------- namespaces
namespace Infineon {
namespace Avian {
namespace Value_Conversion {

// ---------------------------------------------------------------------------- RF_Converter
RF_Converter::RF_Converter(uint32_t reference_clock_Hz, uint8_t pll_div_set,
                           uint8_t pll_pre_divider) :
    m_reference_clock_Hz(reference_clock_Hz),
    m_pll_div_set(pll_div_set),
    m_pll_pre_divider(pll_pre_divider)
{}

// ---------------------------------------------------------------------------- freq_to_pll
int32_t RF_Converter::freq_to_pll(uint32_t frequency_kHz)
{
    /*
     * Formula to calculate register value from frequency:
     * X = (frequency_kHz * 1000 / (REFERENCE_OSC_FREQ_Hz * PLL_PRE_DIV)
     *     - (PLL_DIVSET + 2) * 4 + 8) * 2^20
     */
    double frequency = double(frequency_kHz) * 1000.0;
    double reference = double(m_reference_clock_Hz) * m_pll_pre_divider;
    int32_t offset = int32_t(m_pll_div_set) * 4 + 16;

    return int32_t(round((frequency / reference - offset) * double(1l << 20)));
}

// ---------------------------------------------------------------------------- pll_to_freq
uint32_t RF_Converter::pll_to_freq(int32_t fsu)
{
    // sign correction
    fsu &= 0x00FFFFFF;
    fsu = (fsu < (1 << 23)) ? fsu : fsu - (1 << 24);

    /*
     * Formula to calculate frequency from register value according to data
     * sheet:
     * frequency_Hz = REFERENCE_OSC_FREQ_Hz * PLL_PRE_DIV
     *                * (PLL_DIVSET * 4 + 16 + X * 2^-20)
     *
     * The range is returned in kHz, so the result of the formula above must
     * be divided by 1000.
     */
    double reference = double(m_reference_clock_Hz) * m_pll_pre_divider;
    int32_t offset = int32_t(m_pll_div_set) * 4 + 16;
    double frequency = (double(fsu) / double(1l << 20) + offset) * reference;

    return uint32_t(round(frequency * 0.001));
}

/* ------------------------------------------------------------------------ */
}  // namespace Value_Conversion
}  // namespace Avian
}  // namespace Infineon

/* --- End of File -------------------------------------------------------- */
