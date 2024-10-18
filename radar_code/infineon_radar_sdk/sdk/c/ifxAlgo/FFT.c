/* ===========================================================================
** Copyright (C) 2019-2021 Infineon Technologies AG. All rights reserved.
** ===========================================================================
**
** ===========================================================================
** Infineon Technologies AG (INFINEON) is supplying this file for use
** exclusively with Infineon's sensor products. This file can be freely
** distributed within development tools and software supporting such
** products.
**
** THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
** OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
** MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
** INFINEON SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES, FOR ANY REASON
** WHATSOEVER.
** ===========================================================================
*/

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <mufft.h>

#include "ifxAlgo/FFT.h"

#include "ifxBase/Complex.h"
#include "ifxBase/Error.h"
#include "ifxBase/internal/Macros.h"
#include "ifxBase/Math.h"
#include "ifxBase/Mem.h"
#include "ifxBase/Vector.h"

/*
==============================================================================
   2. LOCAL DEFINITIONS
==============================================================================
*/

// Maximum supported FFT size
#define FFT_MAX_SIZE (65536U)

// For muFFT the data must be aligned to 32bytes boundary
#define MUFFT_REQUIRED_ALIGNMENT (32U)

/*
==============================================================================
   3. LOCAL TYPES
==============================================================================
*/

/**
 * @brief Defines the structure for FFT module.
 *        Use type ifx_FFT_t for this struct.
 */
struct ifx_FFT_s
{
    uint32_t fft_size;                   /**< FFT Size, must be power of 2 and not greater than \ref FFT_MAX_SIZE.*/
    ifx_FFT_Type_t fft_type;             /**< FFT type defined by \ref ifx_FFT_Type_t.*/
    ifx_Complex_t* zero_pad_fft_input_c; /**< Container to store complex zero padded FFT input
                                            in case fft_type is \ref IFX_FFT_TYPE_C2C. Otherwise ignored.*/
    ifx_Complex_t* fft_output_c;         /**< Container to store complex input FFT with half output use case.*/
    mufft_plan_1d* plan_r2c;             /**< muFFT plan .*/
    mufft_plan_1d* plan_c2c;             /**< muFFT plan .*/
};

/*
==============================================================================
   6. LOCAL FUNCTIONS
==============================================================================
*/

/** @brief Copy vector to zero padded buffer
 *
 * Copy at most fft_size elements of the vector input to buffer. If the length
 * of input is smaller than fft_size, the buffer is zero padded. Exactly
 * fft_size elements are copied to buffer.
 */
static void copy_to_buffer_zeropadded_c(const ifx_Vector_C_t* input, ifx_Complex_t* buffer, uint32_t fft_size)
{
    // length of FFT input
    const uint32_t len = MIN(fft_size, vLen(input));

    // Do not use memcpy because of potential stride != 1
    for (uint32_t i = 0; i < len; i++)
        buffer[i] = vAt(input, i);

    // zero padding
    const ifx_Complex_t complex_zero = IFX_COMPLEX_DEF(0, 0);
    for (uint32_t i = len; i < fft_size; i++)
        buffer[i] = complex_zero;
}

static void copy_to_buffer_zeropadded_r(const ifx_Vector_R_t* input, ifx_Float_t* buffer, uint32_t fft_size)
{
    // length of FFT input
    const uint32_t len = MIN(fft_size, vLen(input));

    // Do not use memcpy because of potential stride != 1
    for (uint32_t i = 0; i < len; i++)
        buffer[i] = vAt(input, i);

    // zero padding
    for (uint32_t i = len; i < fft_size; i++)
        buffer[i] = 0;
}

static void fill_negative_half(ifx_Complex_t* output, uint32_t output_size, uint32_t fft_size)
{
    if (output_size >= fft_size)  // Needs to fill negative half
    {
        for (uint32_t i = (fft_size / 2 + 1); i < fft_size; i++)
        {
            uint32_t i_pos = fft_size - i;
            output[i] = ifx_complex_conj(output[i_pos]);
        }
    }
}

/*
==============================================================================
   7. EXPORTED FUNCTIONS
==============================================================================
*/

ifx_FFT_t* ifx_fft_create(ifx_FFT_Type_t fft_type,
                          uint32_t fft_size)
{
    IFX_ERR_BRN_ARGUMENT((fft_type != IFX_FFT_TYPE_R2C) && (fft_type != IFX_FFT_TYPE_C2C));

    int fft_size_error = !ifx_math_ispower_of_2(fft_size);
    IFX_ERR_BRN_ARGUMENT(fft_size_error || (fft_size < 4) || (fft_size > FFT_MAX_SIZE));

    //------------------------- handle creation ------------------------------

    ifx_FFT_t* h = ifx_mem_calloc(1, sizeof(struct ifx_FFT_s));
    IFX_ERR_BRN_MEMALLOC(h);

    h->fft_size = fft_size;
    h->fft_type = fft_type;

    //--------------------------- plan creation -------------------------

    h->fft_output_c = ifx_mem_aligned_alloc(fft_size * sizeof(ifx_Complex_t), MUFFT_REQUIRED_ALIGNMENT);
    IFX_ERR_BRF_MEMALLOC(h->fft_output_c);

    h->zero_pad_fft_input_c = ifx_mem_aligned_alloc(fft_size * sizeof(ifx_Complex_t), MUFFT_REQUIRED_ALIGNMENT);
    IFX_ERR_BRF_MEMALLOC(h->zero_pad_fft_input_c);

    // TODO: Info: "NO_AVX seems to be faster for small transforms, maybe even NO_SSE3" - to be aligned with smart-tv
    const unsigned int flags = MUFFT_FLAG_CPU_NO_AVX;
    h->plan_c2c = mufft_create_plan_1d_c2c(fft_size, MUFFT_FORWARD, flags);
    IFX_ERR_BRF_MEMALLOC(h->plan_c2c);

    h->plan_r2c = mufft_create_plan_1d_r2c(fft_size, flags);
    IFX_ERR_BRF_MEMALLOC(h->plan_r2c);

    return h;

fail:
    ifx_fft_destroy(h);
    return NULL;
}

//----------------------------------------------------------------------------

void ifx_fft_destroy(ifx_FFT_t* handle)
{
    if (handle == NULL)
        return;

    ifx_mem_aligned_free(handle->fft_output_c);
    ifx_mem_aligned_free(handle->zero_pad_fft_input_c);

    mufft_free_plan_1d(handle->plan_c2c);
    mufft_free_plan_1d(handle->plan_r2c);

    ifx_mem_free(handle);
}

//----------------------------------------------------------------------------

void ifx_fft_raw_c(ifx_FFT_t* handle, const ifx_Complex_t* in, ifx_Complex_t* out)
{
    ifx_Vector_C_t input = {0};
    ifx_Vector_C_t output = {0};
    ifx_vec_rawview_c(&input, (ifx_Complex_t*)in, handle->fft_size, 1);
    ifx_vec_rawview_c(&output, out, handle->fft_size, 1);

    ifx_fft_run_c(handle, &input, &output);
}

//----------------------------------------------------------------------------

void ifx_fft_raw_rc(ifx_FFT_t* handle, const ifx_Float_t* in, ifx_Complex_t* out)
{
    mufft_execute_plan_1d(handle->plan_r2c, out, in);
}

//----------------------------------------------------------------------------

void ifx_fft_run_rc(ifx_FFT_t* handle, const ifx_Vector_R_t* input, ifx_Vector_C_t* output)
{
    IFX_ERR_BRK_NULL(handle);
    IFX_VEC_BRK_VALID(input);
    IFX_VEC_BRK_VALID(output);
    IFX_VEC_BRK_MINSIZE(output, handle->fft_size / 2);  // Half spectrum output supported
    IFX_ERR_BRK_COND(handle->fft_type != IFX_FFT_TYPE_R2C, IFX_ERROR_ARGUMENT_INVALID_EXPECTED_REAL);

    // FFT size
    const uint32_t N = handle->fft_size;

    // see comment in ifx_fft_run_c
    bool copy_input = vLen(input) < N || !IFX_IS_ALIGNED(vDat(input), MUFFT_REQUIRED_ALIGNMENT) || vStride(input) != 1;

    /* The output vector has to be copied into an internal buffer if
     *   - length of output vector is smaller than fft_size/2 + 1 because muFFT
     *     needs at least an output vector of fft_size/2
     *   - the output vector is not aligned (muFFT requires aligned input and due
     *     views a the data of a ifx_Vector_C_t vector is not necessarily
     *     aligned),
     *   - the stride is not 1 (might happen due to views).
     */
    bool copy_output = vLen(output) < (N / 2 + 1) || !IFX_IS_ALIGNED(vDat(output), MUFFT_REQUIRED_ALIGNMENT) || vStride(output) != 1;

    const ifx_Float_t* in = vDat(input);
    if (copy_input)
    {
        copy_to_buffer_zeropadded_r(input, (ifx_Float_t*)handle->zero_pad_fft_input_c, N);
        in = (ifx_Float_t*)handle->zero_pad_fft_input_c;
    }

    ifx_Complex_t* out = copy_output
                             ? handle->fft_output_c
                             : vDat(output);

    // compute FFT
    mufft_execute_plan_1d(handle->plan_r2c, out, in);

    // fill negative half if required
    fill_negative_half(out, vLen(output), N);

    if (copy_output)
    {
        // See documentation of this function why len is chosen like this
        uint32_t len;
        if (vLen(output) >= N)
            len = handle->fft_size;
        else if (vLen(output) >= (N / 2 + 1))
            len = N / 2 + 1;
        else
            len = N / 2;

        // Do not use memcpy here because of a potential stride != 1
        for (uint32_t i = 0; i < len; i++)
            vAt(output, i) = out[i];
    }
}

//----------------------------------------------------------------------------

void ifx_fft_run_c(ifx_FFT_t* handle, const ifx_Vector_C_t* input, ifx_Vector_C_t* output)
{
    IFX_ERR_BRK_NULL(handle);
    IFX_VEC_BRK_VALID(input);
    IFX_VEC_BRK_VALID(output);
    IFX_VEC_BRK_MINSIZE(output, handle->fft_size);
    IFX_ERR_BRK_COND(handle->fft_type != IFX_FFT_TYPE_C2C, IFX_ERROR_ARGUMENT_INVALID_EXPECTED_REAL);

    // FFT size
    const uint32_t N = handle->fft_size;

    /* The input vector has to be copied into an internal buffer if
     *   - the input length is smaller than the FFT size (and hence zero
     *     padding is required),
     *   - the input vector is not aligned (muFFT requires aligned input and due
     *     views a the data of a ifx_Vector_C_t vector is not necessarily
     *     aligned),
     *   - the stride is not 1 (might happen due to views).
     */
    bool copy_input = vLen(input) < N || !IFX_IS_ALIGNED(vDat(input), MUFFT_REQUIRED_ALIGNMENT) || vStride(input) != 1;

    /* We need to use an internal buffer for the output if
     *   - the output vector is not aligned (might happen due to views),
     *   - the stride of the output vector is not 1 (might happen due to views).
     */
    bool copy_output = !IFX_IS_ALIGNED(vDat(output), MUFFT_REQUIRED_ALIGNMENT) || vStride(output) != 1;

    const ifx_Complex_t* in = vDat(input);
    if (copy_input)
    {
        copy_to_buffer_zeropadded_c(input, handle->zero_pad_fft_input_c, N);
        in = handle->zero_pad_fft_input_c;
    }

    if (copy_output)
    {
        mufft_execute_plan_1d(handle->plan_c2c, handle->fft_output_c, in);

        // Do not use memcpy here because of a potential stride != 1
        for (uint32_t i = 0; i < N; i++)
            vAt(output, i) = handle->fft_output_c[i];
    }
    else
        mufft_execute_plan_1d(handle->plan_c2c, vDat(output), in);
}

//----------------------------------------------------------------------------

uint32_t ifx_fft_get_fft_size(const ifx_FFT_t* handle)
{
    IFX_ERR_BRV_NULL(handle, 0);

    return handle->fft_size;
}

//----------------------------------------------------------------------------

ifx_FFT_Type_t ifx_fft_get_fft_type(const ifx_FFT_t* handle)
{
    IFX_ERR_BRV_NULL(handle, 255U);

    return handle->fft_type;
}

//----------------------------------------------------------------------------

void ifx_fft_shift_r(const ifx_Vector_R_t* input,
                     ifx_Vector_R_t* output)
{
    IFX_VEC_BRK_VALID(input);
    IFX_VEC_BRK_VALID(output);

    ifx_vec_copyshift_r(input, vLen(input) / 2, output);
}

//----------------------------------------------------------------------------

void ifx_fft_shift_c(const ifx_Vector_C_t* input,
                     ifx_Vector_C_t* output)
{
    IFX_VEC_BRK_VALID(input);
    IFX_VEC_BRK_VALID(output);

    ifx_vec_copyshift_c(input, vLen(input) / 2, output);
}
