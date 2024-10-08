/**
 * \file ifxAvian_TimingConversion.cpp
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
#include "value_conversion/ifxAvian_TimingConversion.hpp"

// ---------------------------------------------------------------------------- namespaces
namespace Infineon {
namespace Avian {
namespace Value_Conversion {

// ---------------------------------------------------------------------------- divide64
/**
 * \internal
 * \brief This function calculates the quotient of two unsigned 64 bit integer
 *        values.
 *
 * Even though this is built into the C language and the compiler knows how to
 * divide this function is an explicit implementation. It's needed to have
 * this implementation to make this driver platform independent. On 32bit
 * platform the CPU cannot do 64 bit division natively and if this driver runs
 * in kernel context, user space math libraries that contain 64 bit division
 * implementation is not available.
 *
 * \param[in,out] dividend  The value to be divided. After the division the
 *                          remainder is returned through this parameter.
 * \param[in]     divisor   The dividend is divided by this value.
 *
 * \note A divide-by-0 check is not done. The caller must take care that the
 *       divisor is not 0.
 */
static uint64_t divide64(uint64_t* dividend, uint64_t divisor)
{
    uint64_t quotient = 0;
    uint64_t bitmask = 1;

    /*
     * The division is done bit by bit. To find the highest bit position of
     * the quotient, the divisor is shifted until it's larger than the
     * dividend. To avoid an overflow, the shifting also stops when bit 63 of
     * the divisor is set.
     */
    while ((divisor < *dividend) && ((divisor & 0x8000000000000000ull) == 0))
    {
        bitmask <<= 1;
        divisor *= 2;
    }

    /*
     * Now that the divisor is scaled accordingly, the division is done bit by
     * bit. This works like we all have learned it at school but with binary
     * digits rather than decimal digits. There should be no need to describe
     * this loop in detail.
     */
    while ((bitmask != 0) && (*dividend != 0))
    {
        if (*dividend >= divisor)
        {
            quotient += bitmask;
            *dividend -= divisor;
        }
        bitmask >>= 1;
        divisor /= 2;
    }
    return quotient;
}

// ---------------------------------------------------------------------------- cycles_to_100ps
uint64_t cycles_to_100ps(uint64_t num_cycles, uint32_t reference_clock_Hz)
{
    /*
     * Mathematically the conversion is just a division of num_cycles by the
     * reference clock frequency. The result must be multiplied by 10^10 to
     * scale it to 100ps steps.
     *
     * Due to integer arithmetic in such a case the scaling is usually be done
     * before the division. That order does not work in this because
     * the multiplication might overflow even with 64bit arithmetic. The
     * maximum number of cycles to be expected is 255 * 2^30 * 8 + 33
     * (= maximum shape end delay), which requires 41 bit. The constant 10^10
     * requires 34bit. Nevertheless the reference clock frequency can be
     * represented with 27 bit (which is sufficient for a frequency up to
     * 134.2MHz) and so the final result requires 48 bit and fits well into a
     * 64 bit variable.
     * Addendum: Actually the result can be a 49 bit, because the clock
     * frequency is a 27 bit number, but it's not 2^27!
     *
     * To do the calculation at full precision the constant 10^10 is split into
     * 2^10 * 5^10.
     */

    /* Step 1: Multiplication by 2^10 (41bit + 10bit = 51bit) */
    uint64_t dividend = num_cycles << 10;

    /* Step 2: Division by reference clock frequency (51bit - 27bit = 24bit) */
    uint64_t quotient = divide64(&dividend, reference_clock_Hz);

    /* Step 3: Multiplication by 5^10 (24bit + 24bit = 48bit) */
    dividend *= 9765625;
    quotient *= 9765625;

    /* Step 4: Division of remainder */
    quotient += divide64(&dividend, reference_clock_Hz);

    /* Step 5: Round to nearest */
    if ((dividend << 1) >= reference_clock_Hz)
        ++quotient;

    return quotient;
}

// ---------------------------------------------------------------------------- cycles_from_100ps
uint64_t cycles_from_100ps(uint64_t period_100ps, uint32_t reference_clock_Hz)
{
    /*
     * Mathematically the conversion is just a multiplication of period_100ps
     * by the reference clock frequency. The result must be divided by 10^10 to
     * compensate scaling to 100ps steps.
     *
     * The maximum period to be expected is a 49 bit value representing the
     * maximum shape end delay of 255 * 2^30 * 8 + 33 clock cycles, which is
     * between 7 and 8 hours. The clock_frequency is a 27bit number, so the
     * product does not fit into a 64bit variable.
     *
     * To do the calculation at full precision the division by th 34 bit
     * constant 10^10 is done first. The result is a 15bit quotient and a
     * 34 bit remainder. Both can be multiplied by the 27 bit clock frequency
     * without overflow. Finally the multiplied remainder is divided again to
     * get the final result.
     */
    const uint64_t divisor = 10000000000;
    uint64_t quotient = divide64(&period_100ps, divisor);

    period_100ps *= reference_clock_Hz;
    quotient *= reference_clock_Hz;

    quotient += divide64(&period_100ps, divisor);

    /* Finally the result is rounded to the nearest value. */
    if ((period_100ps * 2) >= 10000000000)
        ++quotient;

    return quotient;
}

// ---------------------------------------------------------------------------- Scaled_Timer::Scaled_Timer
Scaled_Timer::Scaled_Timer() :
    m_counter(0),
    m_shift(0)
{}

// ---------------------------------------------------------------------------- Scaled_Timer::Scaled_Timer
Scaled_Timer::Scaled_Timer(uint8_t counter, uint8_t shift) :
    m_counter(counter),
    m_shift(shift)
{}

// ---------------------------------------------------------------------------- Scaled_Timer::Scaled_Timer
Scaled_Timer::Scaled_Timer(uint64_t clock_cycles,
                           uint8_t max_shift, uint8_t max_count) :
    m_counter(0),
    m_shift(0)
{
    /*
     * If a multiplied timer is used (counter > 0) the minimum period is
     * 11 cycles. If the counter is not used (counter = 0) it's only 1 cycle.
     * Check for the special case counter = 0 and bypass the iteration below.
     */
    if (clock_cycles < 7)
        return;

    // The result is initialized with value, that indicates an overflow.
    m_counter = 255;
    m_shift = max_shift + 1;
    uint64_t current_error = clock_cycles;

    for (uint8_t shift = 0; shift <= max_shift; ++shift)
    {
        /*
         * The relation between clock_cycles (x), counter (c) and
         * shift_factor (s) is x = c * 2^(s+3) + s + 3
         * Try with the current shift factor and round to nearest counter
         * value.
         */
        uint64_t counter = clock_cycles - shift - 3;
        counter >>= shift + 2;
        counter += 1;
        counter /= 2;

        /* if counter does not fit into 8 bit, ship this iteration */
        if (counter > max_count)
            continue;

        /* calculate the new rounding error */
        Scaled_Timer new_result((uint8_t)counter, shift);

        uint64_t new_error = new_result.get_clock_cycles();
        new_error = (new_error > clock_cycles) ? (new_error - clock_cycles)
                                               : (clock_cycles - new_error);

        /* if error becomes bigger again, the final result has been found */
        if (new_error > current_error)
            break;

        *this = new_result;
        current_error = new_error;
    }
}

// ---------------------------------------------------------------------------- Scaled_Timer::get_clock_cycles
uint64_t Scaled_Timer::get_clock_cycles() const
{
    /*
     * If the counter value is 0 FSM skips the timer logic. This results in
     * only 1 cycle.
     */
    if (m_counter == 0)
        return 1;

    uint64_t clock_cycles;
    clock_cycles = m_counter;
    clock_cycles <<= m_shift;
    clock_cycles *= 8;
    clock_cycles += m_shift;

    /* add additional offset cycles */
    clock_cycles += 3;

    return clock_cycles;
}

/* ------------------------------------------------------------------------ */
}  // namespace Value_Conversion
}  // namespace Avian
}  // namespace Infineon

/* --- End of File -------------------------------------------------------- */
