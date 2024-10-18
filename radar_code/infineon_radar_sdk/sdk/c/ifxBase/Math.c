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

#include <float.h>

#include "Complex.h"
#include "Defines.h"
#include "Error.h"
#include "internal/Macros.h"
#include "Math.h"
#include "Vector.h"

/*
==============================================================================
   2. LOCAL DEFINITIONS
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

/*
==============================================================================
   6. LOCAL FUNCTIONS
==============================================================================
*/

/*
==============================================================================
   7. EXPORTED FUNCTIONS
==============================================================================
*/

ifx_Float_t ifx_math_find_max(const ifx_Vector_R_t* input,
                              uint32_t* max_idx)
{
    IFX_VEC_BRV_VALID(input, -FLT_MAX);
    IFX_ERR_BRV_ARGUMENT(vLen(input) < 1, -FLT_MAX);

    uint32_t idx = 0;
    ifx_Float_t max_val = vAt(input, 0);

    for (uint32_t i = 1; i < vLen(input); ++i)
    {
        if (vAt(input, i) > max_val)
        {
            max_val = vAt(input, i);
            idx = i;
        }
    }

    if (max_idx != NULL)
    {
        *max_idx = idx;
    }

    return max_val;
}

//----------------------------------------------------------------------------

void ifx_math_vec_clip_lt_threshold_r(const ifx_Vector_R_t* input,
                                      ifx_Float_t threshold,
                                      ifx_Float_t clip_value,
                                      ifx_Vector_R_t* output)
{
    IFX_VEC_BRK_VALID(input);
    IFX_VEC_BRK_VALID(output);
    IFX_ERR_BRK_ARGUMENT(vLen(input) < 1);
    IFX_ERR_BRK_ARGUMENT(vLen(output) < 1);
    IFX_ERR_BRK_ARGUMENT(threshold < 0);

    uint32_t N = vLen(output);

    if (vLen(input) < N)
    {
        N = vLen(input);
    }

    for (uint32_t i = 0; i < N; i++)
    {
        if (vAt(input, i) < threshold)
        {
            vAt(output, i) = clip_value;
        }
        else
        {
            vAt(output, i) = vAt(input, i);
        }
    }
}

//----------------------------------------------------------------------------

void ifx_math_vec_clip_gt_threshold_r(const ifx_Vector_R_t* input,
                                      ifx_Float_t threshold,
                                      ifx_Float_t clip_value,
                                      ifx_Vector_R_t* output)
{
    IFX_VEC_BRK_VALID(input);
    IFX_VEC_BRK_VALID(output);
    IFX_ERR_BRK_ARGUMENT(vLen(input) < 1);
    IFX_ERR_BRK_ARGUMENT(vLen(output) < 1);
    IFX_ERR_BRK_ARGUMENT(threshold < 0);

    uint32_t N = vLen(output);

    if (vLen(input) < N)
    {
        N = vLen(input);
    }

    for (uint32_t i = 0; i < N; i++)
    {
        if (vAt(input, i) > threshold)
        {
            vAt(output, i) = clip_value;
        }
        else
        {
            vAt(output, i) = vAt(input, i);
        }
    }
}

//----------------------------------------------------------------------------

ifx_Float_t ifx_math_linear_to_db(ifx_Float_t input,
                                  ifx_Float_t scale)
{
    IFX_ERR_BRV_ARGUMENT(scale == 0, -FLT_MAX)

    return scale * LOG10(input);  // always operate on real values
}

//----------------------------------------------------------------------------

ifx_Float_t ifx_math_db_to_linear(ifx_Float_t input,
                                  ifx_Float_t scale)
{
    IFX_ERR_BRV_ARGUMENT(scale == 0, -FLT_MAX)

    return POW(10.0, input / scale);  // always operate on real values
}

//----------------------------------------------------------------------------

uint32_t ifx_math_round_up_power_of_2_uint32(uint32_t x)
{
    // see https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x++;

    return x;
}

//----------------------------------------------------------------------------

int ifx_math_ispower_of_2(uint32_t n)
{
    return (n && !(n & (n - 1)));
}
