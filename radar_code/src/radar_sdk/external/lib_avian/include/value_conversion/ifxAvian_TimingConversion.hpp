/**
 * \file ifxAvian_TimingConversion.hpp
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

#ifndef IFX_AVIAN_TIMING_CONVERSION_H
#define IFX_AVIAN_TIMING_CONVERSION_H

// ---------------------------------------------------------------------------- includes
#include <cstdint>

// ---------------------------------------------------------------------------- namespaces
namespace Infineon {
namespace Avian {
namespace Value_Conversion {

// ---------------------------------------------------------------------------- cycles_to_100ps
/**
 * This function converts number of clock cycles to time.
 *
 * \param[in] num_cycles          The number of clock cycles to be converted.
 * \param[in] reference_clock_Hz  The frequency of the reference clock
 *                                oscillator.
 *
 * \return The time period in 100ps steps.
 */
uint64_t cycles_to_100ps(uint64_t num_cycles, uint32_t reference_clock_Hz);

// ---------------------------------------------------------------------------- cycles_from_100ps
/**
 * This function converts a time period to number of clock cycles.
 *
 * \param[in] period_100ps        The time period in 100ps steps to be
 *                                converted.
 * \param[in] reference_clock_Hz  The frequency of the reference clock
 *                                oscillator.
 *
 * \return The number of clock cycles.
 */
uint64_t cycles_from_100ps(uint64_t period_100ps, uint32_t reference_clock_Hz);

// ---------------------------------------------------------------------------- Scaled_Timer
/**
 * \brief This types holds bit field value to represent long timings.
 *
 * Some timings of the Avian device's state machine are represented as two bit
 * fields, where one value is a counter counting 100ns steps and the other is
 * a shift factor that divides the reference clock by a power of 2 to scale the
 * time base.
 *
 * The counter and shift values can be written to the according bit fields of
 * an Avian device to specify T_FED, T_SED, T_WAKEUP, T_INIT0 or T_INIT1.
 */
class Scaled_Timer
{
public:
    /**
     * This constructor initializes the timer to the minimum value.
     */
    Scaled_Timer();

    /**
     * This constructor initializes the timer with the specified bit field
     * values.
     */
    Scaled_Timer(uint8_t counter, uint8_t shift);

    /**
     * This constructor initializes the timer from the specified total number
     * of clock cycles.
     *
     * The number of clock cycles is rounded to the nearest representable
     * value. This takes the additional clock cycles into account that are
     * added internally by the FSM.
     *
     * The counter and shift values are limited as specified. It the total
     * number of clock cycles exceeds the representable range, the shift
     * value is set to max_shift + 1 to indicate an overflow.
     *
     * \param[in] clock_cycles  The number of clock cycles to wait.
     * \param[in] max_shift     The maximum possible shift factor.
     * \param[in] max_count     The maximum possible counter value.
     */
    Scaled_Timer(uint64_t clock_cycles, uint8_t max_shift, uint8_t max_count);

    /**
     * This function returns the total number of clock cycles, that is
     * represented by the timer's counter and shift values.
     *
     * This takes the also additional clock cycles into account that are
     * added internally by the FSM.
     */
    uint64_t get_clock_cycles() const;

    /**
     * This function returns the counter value bit field value.
     */
    inline uint8_t get_counter() const;

    /**
     * This function returns the multiplier bit field value.
     */
    inline uint8_t get_shift() const;

private:
    uint8_t m_counter;
    uint8_t m_shift;
};

// ---------------------------------------------------------------------------- Scaled_Timer::get_counter
uint8_t Scaled_Timer::get_counter() const
{
    return m_counter;
}

// ---------------------------------------------------------------------------- Scaled_Timer::get_shift
uint8_t Scaled_Timer::get_shift() const
{
    return m_shift;
}

/* ------------------------------------------------------------------------ */
}  // namespace Value_Conversion
}  // namespace Avian
}  // namespace Infineon

#endif /* IFX_AVIAN_TIMING_CONVERSION_H */

/* --- End of File -------------------------------------------------------- */
