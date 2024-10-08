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

#include <stdlib.h>
#include <string.h>

#include "Complex.h"
#include "Defines.h"
#include "Error.h"
#include "internal/Macros.h"
#include "internal/Simd.h"
#include "internal/Util.h"
#include "Math.h"
#include "Mem.h"
#include "Vector.h"

/*
==============================================================================
   3. LOCAL DEFINITIONS
==============================================================================
*/

const ifx_Float_t clipping_value_for_db = 1e-6f;

/*
==============================================================================
   5. LOCAL FUNCTION PROTOTYPES
==============================================================================
*/

/**
 * @brief Computes number of higher and lower elements in array for given position.
 *        It takes into consideration element on index pick_pos_offsetted+offset
 *        as a base for comparison.
 *
 * Special cases in counting elements:<br>
 * 1) If value is equal and is before pick_pos_offsetted -1 is added.<br/>
 * 2) If value is equal and is after pick_pos_offsetted -1 is added.<br/>
 * Important is that this function returns 0 if is median and number of elements is odd
 * -1 or 1 if number of elements is even for values that directly between.
 *
 * /code
 * Example
 *   input = {3, 2, 1, 7}
 * for pick_pos_offsetted = 1 (value = 2); offset = 0; number_of_el = 4
 *   3, 7 > 2 so +2
 *   1 < 2 so -1
 * Result is +2-1=1
 * /endcode
 *
 * @param [in]     input            buffer that is used to compute rank
 * @param [in]     offset           start position of computation
 * @param [in]     number_of_el     number of elements to compute rank
 *
 * @param [in]     pick_pos_offsetted       on this position (offset+pick_pos_offsetted)
 *                                          is element to compute rank (it is excluded
 *                                          from rank)
 * @retval a rank value
 */
static int median_rank(
    const ifx_Vector_R_t* input, uint32_t offset,
    uint32_t number_of_el, uint32_t pick_pos_offsetted);

/*
==============================================================================
   6. LOCAL FUNCTIONS
==============================================================================
*/

#ifdef IFX_SSE2
/**
 * @brief Compute dot product between x and y
 *
 * Compute the dot product between the vectors x and y of length len. Use SSE2
 * to speed up the computation as much as possible.
 *
 * @param [in]  x       array of len elements
 * @param [in]  y       array of len elements
 * @param [in]  len     length of arrays x and y
 * @retval dot product between x and y
 */
static float dot_sse2(const float* x, const float* y, size_t len)
{
    vf32x4 vv;

    // len rounded down to the next integer divisable by 4
    const size_t len_truncated4 = len & ~3;

    // len rounded down to the next integer divisable by 16
    const size_t len_truncated16 = len & ~15;

    // Compute the dot product of the first len_truncated16 elements
    {
        vf32x4 vv0 = vf32x4_setzero();  // vv0 = 0
        vf32x4 vv1 = vf32x4_setzero();  // vv1 = 0
        vf32x4 vv2 = vf32x4_setzero();  // vv2 = 0
        vf32x4 vv3 = vf32x4_setzero();  // vv3 = 0

        // Compute the dot product using SSE2 instructions. Use four SSE2
        // vectors with 4 elements each to utilize the CPU pipeline as much as
        // possible.
        for (size_t i = 0; i < len_truncated16; i += 16)
        {
            vf32x4 x0 = vf32x4_loadu(&x[i + 0]);
            vf32x4 y0 = vf32x4_loadu(&y[i + 0]);

            vf32x4 x1 = vf32x4_loadu(&x[i + 4]);
            vf32x4 y1 = vf32x4_loadu(&y[i + 4]);

            vf32x4 x2 = vf32x4_loadu(&x[i + 8]);
            vf32x4 y2 = vf32x4_loadu(&y[i + 8]);

            vf32x4 x3 = vf32x4_loadu(&x[i + 12]);
            vf32x4 y3 = vf32x4_loadu(&y[i + 12]);

            vf32x4 m0 = vf32x4_mul(x0, y0);  // x0*y0
            vf32x4 m1 = vf32x4_mul(x1, y1);  // x1*y1
            vf32x4 m2 = vf32x4_mul(x2, y2);  // x2*y2
            vf32x4 m3 = vf32x4_mul(x3, y3);  // x3*y3

            vv0 = vf32x4_add(vv0, m0);       // vv0 += x0*y0
            vv1 = vf32x4_add(vv1, m1);       // vv1 += x1*y1
            vv2 = vf32x4_add(vv2, m2);       // vv2 += x2*y2
            vv3 = vf32x4_add(vv3, m3);       // vv3 += x3*y3
        }

        vf32x4 vv01 = vf32x4_add(vv0, vv1);  // vv01 = vv0 + vv1
        vf32x4 vv23 = vf32x4_add(vv2, vv3);  // vv23 = vv2 + vv3

        vv = vf32x4_add(vv01, vv23);         // vv = vv01 + vv23
    }

    for (size_t i = len_truncated16; i < len_truncated4; i += 4)
    {
        vf32x4 x0 = vf32x4_loadu(&x[i]);
        vf32x4 y0 = vf32x4_loadu(&y[i]);

        vv = vf32x4_mla(vv, x0, y0);  // vv += x0 + y0
    }

    // compute the sum of the vector vv
    float v = vf32x4_extract1(vv, 0);
    v += vf32x4_extract1(vv, 1);
    v += vf32x4_extract1(vv, 2);
    v += vf32x4_extract1(vv, 3);

    // compute the dot element for the remaining elements
    for (size_t i = len_truncated4; i < len; i++)
    {
        v += x[i] * y[i];
    }

    return v;
}
#endif

//----------------------------------------------------------------------------

/*
==============================================================================
   7. EXPORTED FUNCTIONS
==============================================================================
*/

void ifx_vec_rawview_r(ifx_Vector_R_t* vector,
                       ifx_Float_t* d,
                       uint32_t length,
                       uint32_t stride)
{
    ifx_mda_rawview_r(vector, d, 1, &length, (const size_t[]) {stride}, 0);
}

//----------------------------------------------------------------------------

void ifx_vec_rawview_c(ifx_Vector_C_t* vector,
                       ifx_Complex_t* d,
                       uint32_t length,
                       uint32_t stride)
{
    ifx_mda_rawview_c(vector, d, 1, &length, (const size_t[]) {stride}, 0);
}

//----------------------------------------------------------------------------

void ifx_vec_view_r(ifx_Vector_R_t* vector,
                    ifx_Vector_R_t* source,
                    uint32_t offset,
                    uint32_t length,
                    uint32_t spacing)
{
    IFX_ERR_BRK_NULL(vector);

    memset(vector, 0, sizeof(ifx_Vector_R_t));

    IFX_VEC_BRK_VALID(source);
    IFX_VEC_BRK_VEC_BOUNDS(source, offset);
    IFX_ERR_BRK_COND(spacing < 1, IFX_ERROR_ARGUMENT_INVALID);
    IFX_ERR_BRK_COND(length == 0, IFX_ERROR_ARGUMENT_INVALID);

    // source must have length of at least min_length
    const uint32_t min_length = offset + (length - 1) * spacing + 1;
    IFX_ERR_BRK_COND(vLen(source) < min_length, IFX_ERROR_ARGUMENT_OUT_OF_BOUNDS);

    const uint32_t shape[] = {length};
    const size_t stride[] = {spacing};
    ifx_mda_rawview_r(vector, &vAt(source, offset), 1, shape, stride, 0);
}

//----------------------------------------------------------------------------

void ifx_vec_view_c(ifx_Vector_C_t* vector,
                    ifx_Vector_C_t* source,
                    uint32_t offset,
                    uint32_t length,
                    uint32_t spacing)
{
    IFX_ERR_BRK_NULL(vector);

    memset(vector, 0, sizeof(ifx_Vector_R_t));

    IFX_VEC_BRK_VALID(source);
    IFX_VEC_BRK_VEC_BOUNDS(source, offset);
    IFX_ERR_BRK_COND(spacing < 1, IFX_ERROR_ARGUMENT_INVALID);
    IFX_ERR_BRK_COND(length == 0, IFX_ERROR_ARGUMENT_INVALID);

    // source must have length of at least min_length
    const uint32_t min_length = offset + (length - 1) * spacing + 1;
    IFX_ERR_BRK_COND(vLen(source) < min_length, IFX_ERROR_ARGUMENT_OUT_OF_BOUNDS);

    const uint32_t shape[] = {length};
    const size_t stride[] = {spacing};
    ifx_mda_rawview_c(vector, &vAt(source, offset), 1, shape, stride, 0);
}

//----------------------------------------------------------------------------

ifx_Vector_R_t* ifx_vec_create_r(uint32_t length)
{
    ifx_Vector_R_t* v = IFX_MDA_CREATE_R(length);
    if (v)
        ifx_mda_clear_r(v);
    return v;
}

//----------------------------------------------------------------------------

ifx_Vector_C_t* ifx_vec_create_c(uint32_t length)
{
    ifx_Vector_C_t* v = IFX_MDA_CREATE_C(length);
    if (v)
        ifx_mda_clear_c(v);
    return v;
}

//----------------------------------------------------------------------------

ifx_Vector_R_t* ifx_vec_clone_r(const ifx_Vector_R_t* vector)
{
    IFX_VEC_BRV_VALID(vector, NULL);

    return ifx_mda_clone_r(vector);
}

//----------------------------------------------------------------------------

ifx_Vector_C_t* ifx_vec_clone_c(const ifx_Vector_C_t* vector)
{
    IFX_VEC_BRV_VALID(vector, NULL);

    return ifx_mda_clone_c(vector);
}

//----------------------------------------------------------------------------

void ifx_vec_destroy_r(ifx_Vector_R_t* vector)
{
    ifx_mda_destroy_r(vector);
}

//----------------------------------------------------------------------------

void ifx_vec_destroy_c(ifx_Vector_C_t* vector)
{
    ifx_mda_destroy_c(vector);
}

//----------------------------------------------------------------------------

void ifx_vec_blit_r(const ifx_Vector_R_t* vector,
                    uint32_t offset,
                    uint32_t length,
                    uint32_t target_offset,
                    ifx_Vector_R_t* target)
{
    IFX_VEC_BRK_VALID(vector);
    IFX_VEC_BRK_VALID(target);
    IFX_ERR_BRK_COND(length + offset > vLen(vector), IFX_ERROR_ARGUMENT_OUT_OF_BOUNDS);
    IFX_ERR_BRK_COND(length + target_offset > vLen(target), IFX_ERROR_ARGUMENT_OUT_OF_BOUNDS);

    const size_t vstride = vStride(vector);
    const size_t tstride = vStride(target);

    const ifx_Float_t* vdata = &IFX_MDA_AT(vector, offset);
    ifx_Float_t* tdata = &IFX_MDA_AT(target, target_offset);

    for (size_t i = 0; i < length; i++)
    {
        *tdata = *vdata;
        vdata += vstride;
        tdata += tstride;
    }
}

//----------------------------------------------------------------------------

void ifx_vec_blit_c(const ifx_Vector_C_t* vector,
                    uint32_t offset,
                    uint32_t length,
                    uint32_t target_offset,
                    ifx_Vector_C_t* target)
{
    IFX_VEC_BRK_VALID(vector);
    IFX_VEC_BRK_VALID(target);
    IFX_ERR_BRK_COND(length + offset > vLen(vector), IFX_ERROR_ARGUMENT_OUT_OF_BOUNDS);
    IFX_ERR_BRK_COND(length + target_offset > vLen(target), IFX_ERROR_ARGUMENT_OUT_OF_BOUNDS);

    const size_t vstride = vStride(vector);
    const size_t tstride = vStride(target);
    const ifx_Complex_t* vdata = IFX_MDA_DATA(vector) + IFX_MDA_OFFSET(vector, offset);
    ifx_Complex_t* tdata = IFX_MDA_DATA(target) + IFX_MDA_OFFSET(target, target_offset);

    for (size_t i = 0; i < length; i++)
    {
        *tdata = *vdata;
        vdata += vstride;
        tdata += tstride;
    }
}

//----------------------------------------------------------------------------

void ifx_vec_copy_r(const ifx_Vector_R_t* vector,
                    ifx_Vector_R_t* target)
{
    IFX_VEC_BRK_VALID(vector);

    ifx_mda_copy_r(vector, target);
}

//----------------------------------------------------------------------------

void ifx_vec_copy_c(const ifx_Vector_C_t* vector,
                    ifx_Vector_C_t* target)
{
    IFX_VEC_BRK_VALID(vector);

    ifx_mda_copy_c(vector, target);
}

//----------------------------------------------------------------------------

void ifx_vec_copyshift_r(const ifx_Vector_R_t* vector,
                         uint32_t shift,
                         ifx_Vector_R_t* target)
{
    IFX_VEC_BRK_VALID(vector);
    IFX_VEC_BRK_VALID(target);
    IFX_VEC_BRK_DIM(vector, target);

    const uint32_t length = vLen(vector);

    if (vector != target)  // simple copying of segments from source to target vectors
    {
        const uint32_t n2 = length - shift;
        ifx_vec_blit_r(vector, 0, n2, shift, target);
        ifx_vec_blit_r(vector, n2, shift, 0, target);
    }
    else  // in-place swapping operation within a single vector using just one temp variable
          // algo reference: http://www.eis.mdx.ac.uk/staffpages/r_bornat/oldteaching/I2A/slides%209%20circshift.pdf
    {
        uint32_t m = 0;
        uint32_t count = 0;

        for (m = 0, count = 0; count != length; m++)
        {
            ifx_Float_t t = vAt(vector, m);
            uint32_t i;
            uint32_t j;

            for (i = m, j = m + shift;
                 j != m;
                 i = j, j = (j + shift < length) ? (j + shift) : (j + shift - length), count++)
            {
                vAt(vector, i) = vAt(vector, j);
            }

            vAt(vector, i) = t;
            count++;
        }
    }
}

//----------------------------------------------------------------------------

void ifx_vec_copyshift_c(const ifx_Vector_C_t* vector,
                         uint32_t shift,
                         ifx_Vector_C_t* target)
{
    IFX_VEC_BRK_VALID(vector);
    IFX_VEC_BRK_VALID(target);
    IFX_VEC_BRK_DIM(vector, target);

    const uint32_t length = vLen(vector);
    const uint32_t n2 = length - shift;

    if (vector != target)  // simple copying of segments from source to target vectors
    {
        ifx_vec_blit_c(vector, 0, n2, shift, target);
        ifx_vec_blit_c(vector, n2, shift, 0, target);
    }
    else  // in-place swapping operation within a single vector using just one temp variable
          // algo reference: http://www.eis.mdx.ac.uk/staffpages/r_bornat/oldteaching/I2A/slides%209%20circshift.pdf
    {
        uint32_t m = 0;
        uint32_t count = 0;

        for (m = 0, count = 0; count != length; m++)
        {
            ifx_Complex_t t;
            t = vAt(vector, m);
            uint32_t i;
            uint32_t j;

            for (i = m, j = m + shift;
                 j != m;
                 i = j, j = (j + shift < length) ? (j + shift) : (j + shift - length), count++)
            {
                vAt(vector, i) = vAt(vector, j);
            }

            vAt(vector, i) = t;
            count++;
        }
    }
}

//----------------------------------------------------------------------------

void ifx_vec_shift_r(ifx_Vector_R_t* vector,
                     uint32_t shift)
{
    IFX_VEC_BRK_VALID(vector);

    if (shift == 0)
    {
        return;
    }

    ifx_vec_copyshift_r(vector, shift, vector);
}

//----------------------------------------------------------------------------

void ifx_vec_shift_c(ifx_Vector_C_t* vector,
                     uint32_t shift)
{
    IFX_VEC_BRK_VALID(vector);

    if (shift == 0)
    {
        return;
    }

    ifx_vec_copyshift_c(vector, shift, vector);
}

//----------------------------------------------------------------------------

void ifx_vec_setall_r(ifx_Vector_R_t* vector,
                      ifx_Float_t value)
{
    IFX_VEC_BRK_VALID(vector);

    ifx_mda_setall_r(vector, value);
}

//----------------------------------------------------------------------------

void ifx_vec_setall_c(ifx_Vector_C_t* vector,
                      ifx_Complex_t value)
{
    IFX_VEC_BRK_VALID(vector);

    ifx_mda_setall_c(vector, value);
}

//----------------------------------------------------------------------------

void ifx_vec_complex_c(const ifx_Vector_R_t* input_real,
                       const ifx_Vector_R_t* input_imag,
                       ifx_Vector_C_t* output)
{
    IFX_VEC_BRK_VALID(input_real);
    IFX_VEC_BRK_VALID(input_imag);
    IFX_VEC_BRK_VALID(output);

    IFX_VEC_BRK_DIM(input_real, input_imag);
    IFX_VEC_BRK_DIM(input_real, output);

    const uint32_t len = vLen(input_real);
    for (uint32_t i = 0; i < len; i++)
    {
        IFX_COMPLEX_SET(vAt(output, i), vAt(input_real, i), vAt(input_imag, i));
    }
}

//----------------------------------------------------------------------------

void ifx_vec_set_range_r(ifx_Vector_R_t* vector,
                         uint32_t offset,
                         uint32_t length,
                         ifx_Float_t value)
{
    IFX_VEC_BRK_VALID(vector);
    IFX_VEC_BRK_VEC_BOUNDS(vector, offset);
    IFX_ERR_BRK_COND(length + offset > vLen(vector), IFX_ERROR_ARGUMENT_OUT_OF_BOUNDS);

    for (uint32_t i = offset; i < (offset + length); i++)
    {
        vAt(vector, i) = value;
    }
}

//----------------------------------------------------------------------------

void ifx_vec_set_range_c(ifx_Vector_C_t* vector,
                         uint32_t offset,
                         uint32_t length,
                         ifx_Complex_t value)
{
    IFX_VEC_BRK_VALID(vector);
    IFX_VEC_BRK_VEC_BOUNDS(vector, offset);
    IFX_ERR_BRK_COND(length + offset > vLen(vector), IFX_ERROR_ARGUMENT_OUT_OF_BOUNDS);

    for (uint32_t i = offset; i < (offset + length); i++)
    {
        vAt(vector, i) = value;
    }
}

//----------------------------------------------------------------------------

void ifx_vec_setat_r(ifx_Vector_R_t* vector,
                     uint32_t idx,
                     ifx_Float_t value)
{
    IFX_VEC_BRK_VALID(vector);
    IFX_VEC_BRK_VEC_BOUNDS(vector, idx);

    vAt(vector, idx) = value;
}

//----------------------------------------------------------------------------

void ifx_vec_setat_c(ifx_Vector_C_t* vector,
                     uint32_t idx,
                     ifx_Complex_t value)
{
    IFX_VEC_BRK_VALID(vector);
    IFX_VEC_BRK_VEC_BOUNDS(vector, idx);

    vAt(vector, idx) = value;
}

//----------------------------------------------------------------------------

ifx_Float_t ifx_vec_sum_r(const ifx_Vector_R_t* vector)
{
    IFX_VEC_BRV_VALID(vector, 0);

    /* Use the Kahan summation algorithm to reduce numerical error.
     *
     * The algorithm is taken from Wikipedia, see
     * https://en.wikipedia.org/wiki/Kahan_summation_algorithm.
     */
    ifx_Float_t sum = 0;
    ifx_Float_t c = 0; /* running compensation for lost low-order bits */

    for (uint32_t i = 0; i < vLen(vector); i++)
    {
        const ifx_Float_t y = vAt(vector, i) - c;
        const ifx_Float_t t = sum + y;
        c = (t - sum) - y;
        sum = t;
    }

    return sum;
}

//----------------------------------------------------------------------------

ifx_Complex_t ifx_vec_sum_c(const ifx_Vector_C_t* vector)
{
    ifx_Complex_t sum;
    IFX_COMPLEX_SET(sum, 0, 0);

    IFX_VEC_BRV_VALID(vector, sum);

    const uint32_t length = vLen(vector);

    for (uint32_t i = 0; i < length; i++)
    {
        sum = ifx_complex_add(sum, vAt(vector, i));
    }

    return sum;
}

//----------------------------------------------------------------------------

ifx_Float_t ifx_vec_sqsum_r(const ifx_Vector_R_t* vector)
{
    IFX_VEC_BRV_VALID(vector, 0);

    ifx_Float_t result = 0.f;
    const uint32_t length = vLen(vector);

    for (uint32_t i = 0; i < length; i++)
    {
        const ifx_Float_t val = vAt(vector, i);
        result += val * val;
    }

    return result;
}

//----------------------------------------------------------------------------

ifx_Float_t ifx_vec_sqsum_c(const ifx_Vector_C_t* vector)
{
    IFX_VEC_BRV_VALID(vector, 0);

    ifx_Float_t result = 0.f;
    const uint32_t length = vLen(vector);

    for (uint32_t i = 0; i < length; i++)
    {
        result = result + ifx_complex_sqnorm(vAt(vector, i));
    }

    return result;
}

//----------------------------------------------------------------------------

ifx_Float_t ifx_vec_maxabs_r(const ifx_Vector_R_t* vector)
{
    IFX_VEC_BRV_VALID(vector, 0);
    IFX_ERR_BRV_COND(vLen(vector) == 0, IFX_ERROR_ARGUMENT_INVALID, 0);

    const uint32_t length = vLen(vector);
    ifx_Float_t result = FABS(vAt(vector, 0));

    for (uint32_t i = 1; i < length; i++)
    {
        const ifx_Float_t val = FABS(vAt(vector, i));

        if (val > result)
        {
            result = val;
        }
    }

    return result;
}

//----------------------------------------------------------------------------

ifx_Float_t ifx_vec_minabs_r(const ifx_Vector_R_t* vector)
{
    IFX_VEC_BRV_VALID(vector, 0);
    IFX_ERR_BRV_COND(vLen(vector) == 0, IFX_ERROR_ARGUMENT_INVALID, 0);

    const uint32_t length = vLen(vector);
    ifx_Float_t result = FABS(vAt(vector, 0));

    for (uint32_t i = 1; i < length; i++)
    {
        const ifx_Float_t val = FABS(vAt(vector, i));

        if (val < result)
        {
            result = val;
        }
    }

    return result;
}

//----------------------------------------------------------------------------

ifx_Float_t ifx_vec_maxabs_c(const ifx_Vector_C_t* vector)
{
    IFX_VEC_BRV_VALID(vector, 0);
    IFX_ERR_BRV_COND(vLen(vector) == 0, IFX_ERROR_ARGUMENT_INVALID, 0);

    const uint32_t length = vLen(vector);
    ifx_Float_t max = ifx_complex_sqnorm(vAt(vector, 0));

    for (uint32_t i = 1; i < length; i++)
    {
        const ifx_Float_t val = ifx_complex_sqnorm(vAt(vector, i));

        if (val > max)
        {
            max = val;
        }
    }

    ifx_Float_t result = SQRT(max);

    return result;
}

//----------------------------------------------------------------------------

uint32_t ifx_vec_max_idx_r(const ifx_Vector_R_t* vector)
{
    IFX_VEC_BRV_VALID(vector, 0);

    uint32_t index = 0;
    ifx_Float_t max = (vAt(vector, 0));

    for (uint32_t i = 1; i < vLen(vector); i++)
    {
        const ifx_Float_t val = (vAt(vector, i));

        if (val > max)
        {
            max = val;
            index = i;
        }
    }

    return index;
}
//----------------------------------------------------------------------------

uint32_t ifx_vec_min_idx_r(const ifx_Vector_R_t* vector)
{
    IFX_VEC_BRV_VALID(vector, 0);

    uint32_t index = 0;
    ifx_Float_t min = vAt(vector, 0);

    for (uint32_t i = 1; i < vLen(vector); i++)
    {
        const ifx_Float_t val = vAt(vector, i);

        if (val < min)
        {
            min = val;
            index = i;
        }
    }

    return index;
}
//----------------------------------------------------------------------------

uint32_t ifx_vec_max_idx_c(const ifx_Vector_C_t* vector)
{
    IFX_VEC_BRV_VALID(vector, 0);

    uint32_t index = 0;
    ifx_Float_t max = ifx_complex_sqnorm(vAt(vector, 0));

    for (uint32_t i = 1; i < vLen(vector); i++)
    {
        const ifx_Float_t val = ifx_complex_sqnorm(vAt(vector, i));

        if (val > max)
        {
            max = val;
            index = i;
        }
    }

    return index;
}

//----------------------------------------------------------------------------

void ifx_vec_add_r(const ifx_Vector_R_t* v1,
                   const ifx_Vector_R_t* v2,
                   ifx_Vector_R_t* result)
{
    IFX_VEC_BRK_VALID(v1);
    IFX_VEC_BRK_VALID(v2);
    IFX_VEC_BRK_VALID(result);
    IFX_VEC_BRK_DIM(v1, v2);
    IFX_VEC_BRK_DIM(v1, result);

    for (uint32_t i = 0; i < vLen(v1); ++i)
    {
        vAt(result, i) = vAt(v1, i) + vAt(v2, i);
    }
}

//----------------------------------------------------------------------------

void ifx_vec_add_c(const ifx_Vector_C_t* v1,
                   const ifx_Vector_C_t* v2,
                   ifx_Vector_C_t* result)
{
    IFX_VEC_BRK_VALID(v1);
    IFX_VEC_BRK_VALID(v2);
    IFX_VEC_BRK_VALID(result);
    IFX_VEC_BRK_DIM(v1, v2);
    IFX_VEC_BRK_DIM(v1, result);

    for (uint32_t i = 0; i < vLen(v1); ++i)
    {
        vAt(result, i) = ifx_complex_add(vAt(v1, i), vAt(v2, i));
    }
}

//----------------------------------------------------------------------------

void ifx_vec_sub_r(const ifx_Vector_R_t* v1,
                   const ifx_Vector_R_t* v2,
                   ifx_Vector_R_t* result)
{
    IFX_VEC_BRK_VALID(v1);
    IFX_VEC_BRK_VALID(v2);
    IFX_VEC_BRK_VALID(result);
    IFX_VEC_BRK_DIM(v1, v2);
    IFX_VEC_BRK_DIM(v1, result);

    for (uint32_t i = 0; i < vLen(v1); ++i)
    {
        vAt(result, i) = vAt(v1, i) - vAt(v2, i);
    }
}

//----------------------------------------------------------------------------

void ifx_vec_sub_c(const ifx_Vector_C_t* v1,
                   const ifx_Vector_C_t* v2,
                   ifx_Vector_C_t* result)
{
    IFX_VEC_BRK_VALID(v1);
    IFX_VEC_BRK_VALID(v2);
    IFX_VEC_BRK_VALID(result);
    IFX_VEC_BRK_DIM(v1, v2);
    IFX_VEC_BRK_DIM(v1, result);

    for (uint32_t i = 0; i < vLen(v1); ++i)
    {
        vAt(result, i) = ifx_complex_sub(vAt(v1, i), vAt(v2, i));
    }
}

//----------------------------------------------------------------------------

void ifx_vec_mul_r(const ifx_Vector_R_t* v1,
                   const ifx_Vector_R_t* v2,
                   ifx_Vector_R_t* result)
{
    IFX_VEC_BRK_VALID(v1);
    IFX_VEC_BRK_VALID(v2);
    IFX_VEC_BRK_VALID(result);
    IFX_VEC_BRK_DIM(v1, v2);
    IFX_VEC_BRK_DIM(v1, result);

    for (uint32_t i = 0; i < vLen(v1); ++i)
    {
        vAt(result, i) = vAt(v1, i) * vAt(v2, i);
    }
}

//----------------------------------------------------------------------------

void ifx_vec_mul_c(const ifx_Vector_C_t* v1,
                   const ifx_Vector_C_t* v2,
                   ifx_Vector_C_t* result)
{
    IFX_VEC_BRK_VALID(v1);
    IFX_VEC_BRK_VALID(v2);
    IFX_VEC_BRK_VALID(result);
    IFX_VEC_BRK_DIM(v1, v2);
    IFX_VEC_BRK_DIM(v1, result);

    for (uint32_t i = 0; i < vLen(v1); ++i)
    {
        vAt(result, i) = ifx_complex_mul(vAt(v1, i), vAt(v2, i));
    }
}

//----------------------------------------------------------------------------

void ifx_vec_mul_cr(const ifx_Vector_C_t* v1,
                    const ifx_Vector_R_t* v2,
                    ifx_Vector_C_t* result)
{
    IFX_VEC_BRK_VALID(v1);
    IFX_VEC_BRK_VALID(v2);
    IFX_VEC_BRK_VALID(result);
    IFX_VEC_BRK_DIM(v1, v2);
    IFX_VEC_BRK_DIM(v1, result);

    for (uint32_t i = 0; i < vLen(v1); ++i)
    {
        vAt(result, i) = ifx_complex_mul_real(vAt(v1, i), vAt(v2, i));
    }
}

//----------------------------------------------------------------------------

void ifx_vec_abs_r(const ifx_Vector_R_t* input,
                   ifx_Vector_R_t* output)
{
    IFX_VEC_BRK_VALID(input);
    IFX_VEC_BRK_VALID(output);
    IFX_VEC_BRK_DIM(input, output);

    for (uint32_t i = 0; i < vLen(input); ++i)
    {
        vAt(output, i) = FABS(vAt(input, i));
    }
}

//----------------------------------------------------------------------------

void ifx_vec_abs_c(const ifx_Vector_C_t* input,
                   ifx_Vector_R_t* output)
{
    IFX_VEC_BRK_VALID(input);
    IFX_VEC_BRK_VALID(output);
    IFX_VEC_BRK_DIM(input, output);

    for (uint32_t i = 0; i < vLen(input); ++i)
    {
        vAt(output, i) = ifx_complex_abs(vAt(input, i));
    }
}

//----------------------------------------------------------------------------

void ifx_vec_flip_r(const ifx_Vector_R_t* input,
                    ifx_Vector_R_t* output)
{
    IFX_VEC_BRK_VALID(input);
    IFX_VEC_BRK_VALID(output);
    IFX_VEC_BRK_DIM(input, output);
    uint32_t start_read = vLen(input) - 1;
    for (uint32_t i = 0; i < vLen(input); ++i)
    {
        vAt(output, i) = vAt(input, start_read--);
    }
}

//----------------------------------------------------------------------------

void ifx_vec_sub_rs(const ifx_Vector_R_t* input,
                    ifx_Float_t scalar_value,
                    ifx_Vector_R_t* output)
{
    IFX_VEC_BRK_VALID(input);
    IFX_VEC_BRK_VALID(output);
    IFX_VEC_BRK_DIM(input, output);

    for (uint32_t i = 0; i < vLen(input); ++i)
    {
        vAt(output, i) = vAt(input, i) - scalar_value;
    }
}

//----------------------------------------------------------------------------

void ifx_vec_sub_cs(const ifx_Vector_C_t* input,
                    ifx_Complex_t scalar_value,
                    ifx_Vector_C_t* output)
{
    IFX_VEC_BRK_VALID(input);
    IFX_VEC_BRK_VALID(output);
    IFX_VEC_BRK_DIM(input, output);

    for (uint32_t i = 0; i < vLen(input); ++i)
    {
        vAt(output, i) = ifx_complex_sub(vAt(input, i), scalar_value);
    }
}

//----------------------------------------------------------------------------

void ifx_vec_scale_r(const ifx_Vector_R_t* input,
                     ifx_Float_t scale,
                     ifx_Vector_R_t* output)
{
    IFX_VEC_BRK_VALID(input);
    IFX_VEC_BRK_VALID(output);
    IFX_VEC_BRK_DIM(input, output);

    for (uint32_t i = 0; i < vLen(input); ++i)
    {
        vAt(output, i) = vAt(input, i) * scale;
    }
}

//----------------------------------------------------------------------------

void ifx_vec_scale_rc(const ifx_Vector_R_t* input,
                      ifx_Complex_t scale,
                      ifx_Vector_C_t* output)
{
    IFX_VEC_BRK_VALID(input);
    IFX_VEC_BRK_VALID(output);
    IFX_VEC_BRK_DIM(input, output);

    for (uint32_t i = 0; i < vLen(input); ++i)
    {
        vAt(output, i) = ifx_complex_mul_real(scale, vAt(input, i));
    }
}

//----------------------------------------------------------------------------

void ifx_vec_scale_c(const ifx_Vector_C_t* input,
                     ifx_Complex_t scale,
                     ifx_Vector_C_t* output)
{
    IFX_VEC_BRK_VALID(input);
    IFX_VEC_BRK_VALID(output);
    IFX_VEC_BRK_DIM(input, output);

    for (uint32_t i = 0; i < vLen(input); ++i)
    {
        vAt(output, i) = ifx_complex_mul(vAt(input, i), scale);
    }
}

//----------------------------------------------------------------------------

void ifx_vec_scale_cr(const ifx_Vector_C_t* input,
                      ifx_Float_t scale,
                      ifx_Vector_C_t* output)
{
    IFX_VEC_BRK_VALID(input);
    IFX_VEC_BRK_VALID(output);
    IFX_VEC_BRK_DIM(input, output);

    for (uint32_t i = 0; i < vLen(input); ++i)
    {
        vAt(output, i) = ifx_complex_mul_real(vAt(input, i), scale);
    }
}

//----------------------------------------------------------------------------

ifx_Float_t ifx_vec_distance_r(const ifx_Vector_R_t* v1,
                               const ifx_Vector_R_t* v2)
{
    IFX_VEC_BRV_VALID(v1, 0.0f);
    IFX_VEC_BRV_VALID(v2, 0.0f);
    IFX_VEC_BRV_DIM(v1, v2, 0.0f);

    ifx_Float_t acc = 0.0f;
    ifx_Float_t result = 0.0f;

    for (uint32_t i = 0; i < vLen(v1); ++i)
    {
        acc += (vAt(v1, i) - vAt(v2, i)) * (vAt(v1, i) - vAt(v2, i));
    }

    result = SQRT(acc);

    return result;
}

//----------------------------------------------------------------------------

void ifx_vec_mac_r(const ifx_Vector_R_t* v1,
                   const ifx_Vector_R_t* v2,
                   ifx_Float_t scale,
                   ifx_Vector_R_t* result)
{
    IFX_VEC_BRK_VALID(v1);
    IFX_VEC_BRK_VALID(v2);
    IFX_VEC_BRK_VALID(result);
    IFX_VEC_BRK_DIM(v1, v2);
    IFX_VEC_BRK_DIM(v1, result);

    for (uint32_t i = 0; i < vLen(v1); ++i)
    {
        vAt(result, i) = vAt(v1, i) + (scale * vAt(v2, i));
    }
}

//----------------------------------------------------------------------------

void ifx_vec_mac_c(const ifx_Vector_C_t* v1,
                   const ifx_Vector_C_t* v2,
                   ifx_Complex_t scale,
                   ifx_Vector_C_t* result)
{
    IFX_VEC_BRK_VALID(v1);
    IFX_VEC_BRK_VALID(v2);
    IFX_VEC_BRK_VALID(result);
    IFX_VEC_BRK_DIM(v1, v2);
    IFX_VEC_BRK_DIM(v1, result);

    for (uint32_t i = 0; i < vLen(v1); ++i)
    {
        vAt(result, i) = ifx_complex_add(vAt(v1, i), ifx_complex_mul(vAt(v2, i), scale));
    }
}

//----------------------------------------------------------------------------

ifx_Float_t ifx_vec_mean_r(const ifx_Vector_R_t* vector)
{
    IFX_VEC_BRV_VALID(vector, 0);

    const float len = (float)(vLen(vector) ? vLen(vector) : 1);
    return ifx_vec_sum_r(vector) / len;
}

//----------------------------------------------------------------------------

ifx_Complex_t ifx_vec_mean_c(const ifx_Vector_C_t* vector)
{
    const ifx_Complex_t zero = IFX_COMPLEX_DEF(0, 0);

    IFX_VEC_BRV_VALID(vector, zero);

    const float len = (float)(vLen(vector) ? vLen(vector) : 1);
    return ifx_complex_div_real(ifx_vec_sum_c(vector), len);
}

//----------------------------------------------------------------------------

ifx_Float_t ifx_vec_max_r(const ifx_Vector_R_t* vector)
{
    IFX_VEC_BRV_VALID(vector, 0);

    ifx_Float_t result = vAt(vector, 0);

    for (uint32_t i = 0; i < vLen(vector); ++i)
    {
        const ifx_Float_t val = vAt(vector, i);

        if (val > result)
        {
            result = val;
        }
    }

    return result;
}

//----------------------------------------------------------------------------

ifx_Float_t ifx_vec_var_r(const ifx_Vector_R_t* vector)
{
    IFX_VEC_BRV_VALID(vector, 0);

    const ifx_Float_t mean = ifx_vec_mean_r(vector);
    ifx_Float_t acc = 0;
    ifx_Float_t result;

    for (uint32_t i = 0; i < vLen(vector); ++i)
    {
        acc += (vAt(vector, i) - mean) * (vAt(vector, i) - mean);
    }
    result = acc / vLen(vector);

    return result;
}

//----------------------------------------------------------------------------

uint32_t ifx_vec_local_maxima(const ifx_Vector_R_t* vector,
                              ifx_Float_t threshold,
                              uint32_t num_maxima,
                              uint32_t* maxima_idxs)
{
    IFX_VEC_BRV_VALID(vector, 0);
    IFX_ERR_BRV_NULL(maxima_idxs, 0);
    uint32_t count = 0;

    for (uint32_t i = 1; i < vLen(vector) - 1; ++i)
    {
        if (vAt(vector, i) > vAt(vector, (size_t)i - 1) && vAt(vector, i) >= vAt(vector, (size_t)i + 1))
        {
            if (vAt(vector, i) > threshold)
            {
                maxima_idxs[count] = i;
                count++;

                if (count >= num_maxima)
                {
                    break;
                }
            }
        }
    }

    return count;
}

//----------------------------------------------------------------------------

void ifx_vec_clear_r(ifx_Vector_R_t* vector)
{
    IFX_VEC_BRK_VALID(vector);

    ifx_mda_clear_r(vector);
}

//----------------------------------------------------------------------------

void ifx_vec_clear_c(ifx_Vector_C_t* vector)
{
    IFX_VEC_BRK_VALID(vector);

    ifx_mda_clear_c(vector);
}

//----------------------------------------------------------------------------

void ifx_vec_linspace_r(ifx_Float_t start,
                        ifx_Float_t end,
                        ifx_Vector_R_t* output)
{
    IFX_VEC_BRK_VALID(output);

    // compute increment
    ifx_Float_t delta = (end - start) / vLen(output);

    // populate linspace values
    vAt(output, 0) = start;
    for (uint32_t i = 1; i < vLen(output); i++)
    {
        vAt(output, i) = vAt(output, i - 1) + delta;
    }
}

//----------------------------------------------------------------------------

static int median_rank(
    const ifx_Vector_R_t* input, uint32_t offset,
    uint32_t number_of_el, uint32_t pick_pos_offsetted)
{
    int res = 0;
    ifx_Float_t val = vAt(input, pick_pos_offsetted + offset);
    for (uint32_t i = 0; i < pick_pos_offsetted; i++)
    {
        if (val >= vAt(input, i + offset))  // There is difference if you are before or after index (==)
        {
            res++;
        }
        else
        {
            res--;
        }
    }
    for (uint32_t i = pick_pos_offsetted + 1; i < number_of_el; i++)
    {
        if (val > vAt(input, i + offset))  // There is difference if you are before or after index (==)
        {
            res++;
        }
        else
        {
            res--;
        }
    }
    return res;
}

//----------------------------------------------------------------------------

ifx_Float_t ifx_vec_median_range_r(const ifx_Vector_R_t* input, uint32_t offset, uint32_t length)
{
    IFX_VEC_BRV_VALID(input, IFX_NAN);
    IFX_ERR_BRV_ARGUMENT(vLen(input) < length + offset, IFX_NAN);

    ifx_Float_t upper_limit = IFX_INF_POS;
    ifx_Float_t lower_limit = IFX_INF_NEG;

    const int32_t undef_idx = -1;
    int32_t prev_idx = undef_idx;

    for (int32_t idx = 0; idx < (int32_t)length; idx++)
    {
        ifx_Float_t val = vAt(input, idx + offset);
        if (val > upper_limit || val < lower_limit)
        {
            continue;
        }
        int rank = median_rank(input, offset, length, idx);
        if (rank == 0)
        {
            return val;
        }
        else if (rank == 1 || rank == -1)  // Only happens for length n%2==0
        {
            if (prev_idx != undef_idx)
            {
                ifx_Float_t val_prev = vAt(input, prev_idx + offset);
                return (val + val_prev) / 2;
            }
            prev_idx = idx;
        }

        if (rank >= 0)
        {
            upper_limit = val;
        }
        else
        {
            lower_limit = val;
        }
    }
    return IFX_NAN;
}

//----------------------------------------------------------------------------

ifx_Float_t ifx_vec_median_r(const ifx_Vector_R_t* input)
{
    return ifx_vec_median_range_r(input, 0, vLen(input));
}

//----------------------------------------------------------------------------

ifx_Float_t ifx_vec_dot_r(const ifx_Vector_R_t* v1, const ifx_Vector_R_t* v2)
{
    IFX_VEC_BRV_VALID(v1, IFX_NAN);
    IFX_VEC_BRV_VALID(v2, IFX_NAN);
    IFX_ERR_BRV_COND(vLen(v1) != vLen(v2), IFX_ERROR_DIMENSION_MISMATCH, IFX_NAN);

    return ifx_vec_dot2_r(v1, v2, 0, 0, vLen(v1));
}

//----------------------------------------------------------------------------

ifx_Float_t ifx_vec_dot2_r(const ifx_Vector_R_t* v1, const ifx_Vector_R_t* v2, uint32_t offset_v1, uint32_t offset_v2, uint32_t len)
{
    IFX_VEC_BRV_VALID(v1, IFX_NAN);
    IFX_VEC_BRV_VALID(v2, IFX_NAN);
    IFX_ERR_BRV_COND(offset_v1 + len > vLen(v1), IFX_ERROR_DIMENSION_MISMATCH, IFX_NAN);
    IFX_ERR_BRV_COND(offset_v2 + len > vLen(v2), IFX_ERROR_DIMENSION_MISMATCH, IFX_NAN);

#ifdef IFX_SSE2
    // We can use the SSE2 version of the dot product if:
    //  - SSE2 is available
    //  - the strides of v1 and v2 are 1
    //  - ifx_Float_t corresponds to float (and not double)
    if (vStride(v1) == 1 && vStride(v2) == 1 && sizeof(ifx_Float_t) == sizeof(float))
    {
        const ifx_Float_t* x_ptr = IFX_MDA_DATA(v1) + offset_v1;
        const ifx_Float_t* y_ptr = IFX_MDA_DATA(v2) + offset_v2;

        return dot_sse2(x_ptr, y_ptr, len);
    }
#endif

    // naive implementation
    ifx_Float_t s = 0;
    for (uint32_t j = 0; j < len; j++)
    {
        s += vAt(v1, offset_v1 + j) * vAt(v2, offset_v2 + j);
    }

    return s;
}

//----------------------------------------------------------------------------

void ifx_vec_log10_r(const ifx_Vector_R_t* input,
                     ifx_Vector_R_t* output)
{
    IFX_VEC_BRK_VALID(input);
    IFX_VEC_BRK_VALID(output);
    IFX_ERR_BRK_ARGUMENT(vLen(input) < 1);
    IFX_ERR_BRK_ARGUMENT(vLen(output) < 1);

    uint32_t N = vLen(output);

    const ifx_Float_t clipped_log_value = LOG10(clipping_value_for_db);

    if (vLen(input) < N)
    {
        N = vLen(input);
    }

    for (uint32_t i = 0; i < N; i++)
    {
        vAt(output, i) = (vAt(input, i) > clipping_value_for_db) ? LOG10(vAt(input, i)) : clipped_log_value;
    }
}

//----------------------------------------------------------------------------

void ifx_vec_log10_c(const ifx_Vector_C_t* input,
                     ifx_Vector_C_t* output)
{
    IFX_VEC_BRK_VALID(input);
    IFX_VEC_BRK_VALID(output);
    IFX_ERR_BRK_ARGUMENT(vLen(input) < 1);
    IFX_ERR_BRK_ARGUMENT(vLen(output) < 1);

    uint32_t N = vLen(output);

    if (vLen(input) < N)
    {
        N = vLen(input);
    }

    for (uint32_t i = 0; i < N; ++i)
    {
        vAt(output, i) = ifx_complex_log10(vAt(input, i));
    }
}

//----------------------------------------------------------------------------

void ifx_vec_linear_to_dB(const ifx_Vector_R_t* input,
                          ifx_Float_t scale,
                          ifx_Vector_R_t* output)
{
    IFX_VEC_BRK_VALID(input);
    IFX_VEC_BRK_VALID(output);
    IFX_ERR_BRK_ARGUMENT(vLen(input) < 1);
    IFX_ERR_BRK_ARGUMENT(vLen(output) < 1);
    IFX_ERR_BRK_ARGUMENT(scale == 0);

    ifx_vec_log10_r(input, output);

    ifx_vec_scale_r(output, scale, output);
}

//----------------------------------------------------------------------------

void ifx_vec_dB_to_linear(const ifx_Vector_R_t* input,
                          ifx_Float_t scale,
                          ifx_Vector_R_t* output)
{
    IFX_VEC_BRK_VALID(input);
    IFX_VEC_BRK_VALID(output);
    IFX_ERR_BRK_ARGUMENT(vLen(input) < 1);
    IFX_ERR_BRK_ARGUMENT(vLen(output) < 1);
    IFX_ERR_BRK_ARGUMENT(scale == 0);

    uint32_t N = vLen(output);

    if (vLen(input) < N)
    {
        N = vLen(input);
    }

    for (uint32_t i = 0; i < N; ++i)
    {
        vAt(output, i) = POW(10.0, (vAt(input, i) / scale));  // always operate on real values
    }
}

//----------------------------------------------------------------------------

ifx_Float_t ifx_vec_difference_deviation_r(const ifx_Vector_R_t* v1, const ifx_Vector_R_t* v2)
{
    IFX_ERR_BRV_NULL(v1, 0);
    IFX_ERR_BRV_NULL(v2, 0);
    IFX_ERR_BRV_COND(vLen(v1) < 2 || vLen(v2) < 2, IFX_ERROR_ARGUMENT_INVALID, 0);
    IFX_ERR_BRV_COND(vLen(v1) != vLen(v2), IFX_ERROR_ARGUMENT_INVALID, 0);

    uint32_t N = vLen(v1);

    // calculate mean of the difference vector
    ifx_Float_t mean = 0;
    for (uint32_t i = 0; i < N; ++i)
    {
        mean += (vAt(v2, i) - vAt(v1, i));
    }
    mean /= N;

    // calculate the standard deviation
    ifx_Float_t sum = 0;
    for (uint32_t i = 0; i < N; ++i)
    {
        sum += POW(((vAt(v2, i) - vAt(v1, i)) - mean), 2);
    }
    return SQRT(sum / (N - 1));
}

//----------------------------------------------------------------------------

void ifx_vec_squared_norm_c(const ifx_Vector_C_t* input, ifx_Vector_R_t* output)
{
    IFX_VEC_BRK_DIM(input, output);

    for (uint32_t i = 0; i < vLen(input); i++)
    {
        const ifx_Complex_t z = vAt(input, i);
        const ifx_Float_t real = IFX_COMPLEX_REAL(z);
        const ifx_Float_t imag = IFX_COMPLEX_IMAG(z);

        vAt(output, i) = real * real + imag * imag;
    }
}

//----------------------------------------------------------------------------

void ifx_vec_spectrum2_to_db(ifx_Vector_R_t* vec, ifx_Float_t scale, ifx_Float_t threshold)
{
    /* Computing square roots and logarithms is computational
     * expensive, so we avoid computing the square root directly.
     *
     * Perform the clipping on the squared absolute (hence threshold2 is
     * the square of threshold). If the value is not clipped compute the
     * square root and convert to dB. The factor of 1/2 when converting to dB
     * corresponds to taking the square root using the identity:
     *      log(sqrt(a)) = log(a**0.5) = 0.5*log(a)
     */

    const ifx_Float_t threshold2 = threshold * threshold;
    const ifx_Float_t clip_value = ifx_math_linear_to_db(clipping_value_for_db, scale);

    for (uint32_t i = 0; i < vLen(vec); i++)
    {
        if (vAt(vec, i) < threshold2)
            vAt(vec, i) = clip_value;
        else
            vAt(vec, i) = ifx_math_linear_to_db(vAt(vec, i), scale / 2);
    }
}
