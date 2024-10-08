/* ===========================================================================
** Copyright (C) 2021 Infineon Technologies AG
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

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include "ifxAlgo/Window.h"

#include "ifxBase/Defines.h"
#include "ifxBase/Error.h"
#include "ifxBase/internal/Macros.h"
#include "ifxBase/Vector.h"

/*
==============================================================================
   2. DEFINITIONS
==============================================================================
*/

/*
==============================================================================
   3. LOCAL TYPES
==============================================================================
*/

/*
==============================================================================
   4. LOCAL DATA
==============================================================================
*/

/*
==============================================================================
   5. LOCAL FUNCTION PROTOTYPES
==============================================================================
*/

/**
 * @brief Generates the coefficients of a Hamming window for a given length.
 *
 * @param [in,out] win       Pointer to an allocated and populated, but empty
 *                           vector instance defined by \ref ifx_Vector_R_t
 *                           to be filled with Hamming Window coefficients.
 */
static void init_hamming(ifx_Vector_R_t* win);

/**
 * @brief Generates the coefficients of a Hanning window for a given length.
 *
 * @param [in,out] win       Pointer to an allocated and populated, but empty
 *                           vector instance defined by \ref ifx_Vector_R_t
 *                           to be filled with Hanning Window coefficients.
 *
 */
static void init_hann(ifx_Vector_R_t* win);

/**
 * @brief Generates the coefficients of a Blackman harris window for a given length.
 *
 * @param [in,out] win       Pointer to an allocated and populated, but empty
 *                           vector instance defined by \ref ifx_Vector_R_t
 *                           to be filled with Blackman Window coefficients.
 *
 */
static void init_blackmanharris(ifx_Vector_R_t* win);

/**
 * @brief  Generates the coefficients of a Chebyshev window for a given length.
 *
 * @param [in,out] win       Pointer to an allocated and populated, but empty
 *                           vector instance defined by \ref ifx_Vector_R_t
 *                           to be filled with Chebyshev Window coefficients.
 *
 * @param [in]     at_dB     Defines the attenuation in dBs required to generate
 *                           the pass band ripple for a Chebyshev window.
 *                           This must be a positive number.
 *
 * Reference: Dolph Chebyshev window according to
 * http://practicalcryptography.com/miscellaneous/machine-learning/implementing-dolph-chebyshev-window/
 *
 */
static void init_chebyshev(ifx_Vector_R_t* win,
                           ifx_Float_t at_dB);

/**
 * @brief Generates the coefficients of a Blackman window
 *        for a given length.
 *
 * @param [in,out] win       Pointer to an allocated and populated, but empty
 *                           vector instance defined by \ref ifx_Vector_R_t
 *                           to be filled with Blackman Window coefficients.
 *
 */
static void init_blackman(ifx_Vector_R_t* win);

/**
 * @brief Computes acos(1-x) for 0<=x<=2
 *
 * The function avoids a loss of significance if the argument of the arccos
 * function is close to 1.
 *
 * For x < 0.01 a Taylor series of acos(1-x) at x=0 is used. For x >= 0.01 the
 * acos function of the C library is used. For x<0 and x>2 NAN is returned.
 *
 * @param [in]     x         argument
 * @retval         y         y=acos(1-x) for x
 * @retval         NAN       if x < 0 or x > 2
 */
static ifx_Float_t acos1mx(ifx_Float_t x);

/**
 * @brief Computes acosh(1+x) for x>=0
 *
 * The function avoids a loss of significance if the argument of the arccosh
 * function is close to 1. The function uses the identity
 *      acosh(1+x) = log(1 + x + sqrt(x*(x+2)) ).
 * To evaluate the logarithm the function log1p is used.
 *
 * @param [in]     x       argument
 * @retval         y       y=acosh(x)
 * @retval         NAN     if x < 0
 */
static ifx_Float_t acosh1px(ifx_Float_t x);

/**
 * @brief Computes cos(x)-1
 *
 * This function avoids a loss of significance if the argument of the cosine
 * function is small. The function uses the identity
 *      cos(x)-1 = -2*sin^2(x/2).
 *
 * @param [in]  x            argument
 * @retval      y            y=cos(x)-1
 */
static ifx_Float_t cosm1(ifx_Float_t x);

/**
 * @brief Computes cosh(x)-1
 *
 * This function avoids a loss of significance if the argument of the cosh
 * function is small. The function uses the identity
 *      cosh(x)-1 = 2*sinh^2(x/2).
 *
 * @param [in]     x         argument
 * @retval         y         y=cos(x)-1
 */
static ifx_Float_t coshm1(ifx_Float_t x);

/**
 * @brief Evaluates Chebyshev polynomial of the first kind
 *
 * Evaluate the Chebyshev polynomial T_n(1+x).
 *
 * @param [in]     n         order
 * @param [in]     x         argument
 * @retval         y         y=T_n(1+x)
 */
static ifx_Float_t chebyxp1(int n,
                            ifx_Float_t x);

/*
==============================================================================
   6. LOCAL FUNCTIONS
==============================================================================
*/

static void init_hamming(ifx_Vector_R_t* win)
{
    if (vLen(win) == 1)
    {
        vAt(win, 0) = 1;
    }
    else
    {
        ifx_Float_t N = 1.0f / (ifx_Float_t)(vLen(win) - 1);

        for (uint32_t i = 0; i < vLen(win); ++i)
        {
            vAt(win, i) = 0.54f - 0.46f * COS(2.0f * IFX_PI * i * N);
        }
    }
}

//----------------------------------------------------------------------------

static void init_hann(ifx_Vector_R_t* win)
{
    if (vLen(win) == 1)
    {
        vAt(win, 0) = 1;
    }
    else
    {
        ifx_Float_t N = 1.0f / (ifx_Float_t)(vLen(win) - 1);

        for (uint32_t i = 0; i < vLen(win); ++i)
        {
            vAt(win, i) = 0.5f * (1.0f - COS(2.0f * IFX_PI * i * N));
        }
    }
}

//----------------------------------------------------------------------------

static void init_blackmanharris(ifx_Vector_R_t* win)
{
    if (vLen(win) == 1)
    {
        vAt(win, 0) = 1;
    }
    else
    {
        const ifx_Float_t a0 = (ifx_Float_t)0.35875;
        const ifx_Float_t a1 = (ifx_Float_t)0.48829;
        const ifx_Float_t a2 = (ifx_Float_t)0.14128;
        const ifx_Float_t a3 = (ifx_Float_t)0.01168;

        const ifx_Float_t scale = IFX_PI / (vLen(win) - 1);

        for (uint32_t i = 0; i < vLen(win); ++i)
        {
            const ifx_Float_t phi = scale * (ifx_Float_t)i;
            vAt(win, i) = a0
                          + a2 * COS(4 * phi)
                          - a1 * COS(2 * phi)
                          - a3 * COS(6 * phi);
        }
    }
}

//----------------------------------------------------------------------------

static void init_blackman(ifx_Vector_R_t* win)
{
    if (vLen(win) == 1)
    {
        vAt(win, 0) = 1;
    }
    else
    {
        const ifx_Float_t a0 = (ifx_Float_t)0.42;
        const ifx_Float_t a1 = (ifx_Float_t)0.5;
        const ifx_Float_t a2 = (ifx_Float_t)0.08;

        const ifx_Float_t scale = IFX_PI / (vLen(win) - 1);

        for (uint32_t i = 0; i < vLen(win); ++i)
        {
            const ifx_Float_t phi = scale * (ifx_Float_t)i;
            vAt(win, i) = a0
                          + a2 * COS(4 * phi)
                          - a1 * COS(2 * phi);
        }
    }
}

//----------------------------------------------------------------------------

static ifx_Float_t acos1mx(ifx_Float_t x)
{
    if (x < (ifx_Float_t)0.01)
    {
        /* Use Taylor series of acos(1-x) for x=~0. The maximum relative error
         * for double is ~7e-14 at x=0.01.
         */
        const ifx_Float_t x2 = x * x;    // x^2
        const ifx_Float_t x3 = x * x2;   // x^3
        const ifx_Float_t x4 = x2 * x2;  // x^4
        return SQRT(2 * x) * (1 + x / 12 + 3 * x2 / 160 + 5 * x3 / 896 + 35 * x4 / 18432);
    }
    else
        return ACOS(1 - x);
}

//----------------------------------------------------------------------------

static ifx_Float_t acosh1px(ifx_Float_t x)
{
    return LOG1P(x + SQRT(x * (x + 2)));
}

//----------------------------------------------------------------------------

static ifx_Float_t cosm1(ifx_Float_t x)
{
    ifx_Float_t s = SIN(x / 2);
    return -2 * s * s;
}

//----------------------------------------------------------------------------

static ifx_Float_t coshm1(ifx_Float_t x)
{
    ifx_Float_t s = SINH(x / 2);
    return 2 * s * s;
}

//----------------------------------------------------------------------------

static ifx_Float_t chebyxp1(int n,
                            ifx_Float_t x)
{
    if (x < 0 && x >= -2)
        return COS(n * acos1mx(-x));
    else
        return COSH(n * acosh1px(x));
}

//----------------------------------------------------------------------------

static void init_chebyshev(ifx_Vector_R_t* win,
                           ifx_Float_t at_dB)
{
    const uint32_t N = vLen(win);

    if (N == 1)
    {
        vAt(win, 0) = (ifx_Float_t)1;
        return;
    }

    const ifx_Float_t pssbnd_ripple = POW(10, at_dB / 20);

    /* We evaluate the formulas from Ref. [1] but we do some things differently
     * to avoid numerical problems
     *
     * Please note that this algorithm can be optimized: chebyxp1 is often
     * called with the same arguments, and one can compute cos(i*pi/N) for
     * i=1,2,3... more effectively (see Numerical Recipes in C). However, as
     * the code runs only once at startup, so it's probably not worth to put
     * effort into it.
     *
     * Moreover, one can reduce the complexity from O(N^2) to O(N*log(N)) by
     * computing the function in the frequency domain and then applying a FFT.
     *
     * References:
     *   [1] http://practicalcryptography.com/miscellaneous/machine-learning/implementing-dolph-chebyshev-window/
     */

    // x0m1 = x0-1 where x0 = cosh(acosh(1/r)/(N-1))
    const ifx_Float_t x0m1 = coshm1(ACOSH(pssbnd_ripple) / (N - 1));

    ifx_Float_t max_val = 0;
    for (uint32_t j = 0; j < N / 2 + 1; j++)
    {
        const ifx_Float_t n = j + (ifx_Float_t)0.5;

        // inside the loop: sign = (-1)^i
        int sign = -1;

        ifx_Float_t sum = 0;
        for (uint32_t i = 1; i < (N - 1) / 2 + 1; i++)
        {
            // cm1 = cos(i*pi/N)-1
            ifx_Float_t cm1 = cosm1(i * IFX_PI / N);

            // arg = x0*cos(i*pi/N) - 1
            ifx_Float_t arg = cm1 + x0m1 * (1 + cm1);
            sum += sign * chebyxp1(N - 1, arg) * COS((2 * IFX_PI * n * i) / N);
            sign = -sign;
        }

        // here a loss of significance occurs
        ifx_Float_t value = pssbnd_ripple + 2 * sum;
        if (value > max_val)
            max_val = value;

        vAt(win, j) = vAt(win, N - j - 1) = value;
    }

    ifx_vec_scale_r(win, 1 / max_val, win);
}

/*
==============================================================================
   7. EXPORTED FUNCTIONS
==============================================================================
*/

void ifx_window_init(const ifx_Window_Config_t* config,
                     ifx_Vector_R_t* win)
{
    IFX_VEC_BRK_VALID(win);
    IFX_ERR_BRK_ARGUMENT(config->size != vLen(win));

    switch (config->type)
    {
        case IFX_WINDOW_HAMM:
            init_hamming(win);
            break;

        case IFX_WINDOW_HANN:
            init_hann(win);
            break;

        case IFX_WINDOW_BLACKMANHARRIS:
            init_blackmanharris(win);
            break;

        case IFX_WINDOW_CHEBYSHEV:
            init_chebyshev(win, config->at_dB);
            break;

        case IFX_WINDOW_BLACKMAN:
            init_blackman(win);
            break;

        default:
            IFX_ERR_BRK_ARGUMENT(1);
            break;
    }
}
