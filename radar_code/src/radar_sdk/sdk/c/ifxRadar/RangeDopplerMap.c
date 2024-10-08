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

#include "ifxAlgo/FFT.h"
#include "ifxAlgo/Window.h"

#include "ifxBase/Complex.h"
#include "ifxBase/Defines.h"
#include "ifxBase/Error.h"
#include "ifxBase/internal/Macros.h"
#include "ifxBase/Matrix.h"
#include "ifxBase/Mem.h"
#include "ifxBase/Vector.h"

#include "ifxRadar/RangeDopplerMap.h"

/*
==============================================================================
   2. LOCAL DEFINITIONS
==============================================================================
*/

#define CLIPPING_VALUE (1e-6f)  // Corresponds to -120dB

/*
==============================================================================
   3. LOCAL TYPES
==============================================================================
*/

/**
 * @brief Defines the structure for Range Doppler map module processing.
 *        Use type ifx_RDM_t for this struct.
 */
struct ifx_RDM_s
{
    ifx_Math_Scale_Type_t output_scale_type; /**< Linear or dB scale for the output of range spectrum module.*/
    ifx_Float_t spect_threshold;             /**< Threshold is in always linear scale, should be greater than 1-e6.
                                                  Range spectrum output values below this are set to 1-e6 (-120dB).*/
    ifx_PPFFT_t* range_ppfft_handle;         /**< Preprocessed FFT handle for Range FFT defined by \ref ifx_PPFFT_t
                                                  e.g. Mean removal, window settings, FFT settings.*/
    ifx_PPFFT_t* doppler_ppfft_handle;       /**< Preprocessed FFT settings for Doppler FFT defined by \ref ifx_PPFFT_t
                                                  e.g. Mean removal, window settings, FFT settings.*/
    ifx_Vector_C_t* doppler_fft_result;      /**< Container to store the result of Doppler FFT during range doppler spectrum
                                                  calculation.*/
    ifx_Matrix_C_t* rdm_matrix;              /**< Container to store the result of range and doppler FFT.*/
};

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

/**
 * @brief Computes squared norm of complex vector.
 *
 * For each element in the vector input compute the square of the absolute
 * value and save it in output: \f$\mathrm{output}_j = |\mathrm{input}_j|^2\f$
 *
 * @param [in]     input     Complex input vector.
 * @param [out]    output    Square of norm of vector input.
 */
static void ifx_vec_abs2_c(const ifx_Vector_C_t* input, ifx_Vector_R_t* output)
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

/**
 * @brief Convert squared absolute of spectrum to dB
 *
 * Given the squared norm of the spectrum, convert it to dB.
 *
 * The function is equivalent to:
 *   1. Taking the square root of all elements of vec.
 *   2. Clipping all values smaller than threshold to CLIPPING_VALUE.
 *   3. Converting all value to dB using scale.
 *
 * @param [in,out]  vec         squared norm of spectrum
 * @param [in]      scale       scale factor
 * @param [in]      threshold   threshold for clipping
 */
static void spectrum2_to_db(ifx_Vector_R_t* vec, ifx_Float_t scale, ifx_Float_t threshold)
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
    const ifx_Float_t clip_value = ifx_math_linear_to_db(CLIPPING_VALUE, scale);

    for (uint32_t i = 0; i < vLen(vec); i++)
    {
        if (vAt(vec, i) < threshold2)
            vAt(vec, i) = clip_value;
        else
            vAt(vec, i) = ifx_math_linear_to_db(vAt(vec, i), scale / 2);
    }
}

/**
 * @brief Convert squared absolute of spectrum to linear
 *
 * Given the squared norm of the spectrum, convert it to linear.
 *
 * The function is equivalent to:
 *   1. Taking the square root of all elements of vec.
 *   2. Clipping all values smaller than threshold to CLIPPING_VALUE.
 *
 * @param [in,out]  vec         squared norm of spectrum
 * @param [in]      threshold   threshold for clipping
 */
static void spectrum2_to_linear(ifx_Vector_R_t* vec, ifx_Float_t threshold)
{
    const ifx_Float_t threshold2 = threshold * threshold;
    const ifx_Float_t clip_value = CLIPPING_VALUE;

    for (uint32_t i = 0; i < vLen(vec); i++)
    {
        if (vAt(vec, i) < threshold2)
            vAt(vec, i) = clip_value;
        else
            vAt(vec, i) = SQRT(vAt(vec, i));
    }
}

/*
==============================================================================
   7. EXPORTED FUNCTIONS
==============================================================================
*/

ifx_RDM_t* ifx_rdm_create(const ifx_RDM_Config_t* config)
{
    IFX_ERR_BRN_NULL(config)

    ifx_RDM_t* h = ifx_mem_calloc(1, sizeof(struct ifx_RDM_s));
    IFX_ERR_BRN_MEMALLOC(h);

    uint32_t rng_fft_out_size;
    uint32_t doppler_fft_out_size = config->doppler_fft_config.fft_size;

    if (config->range_fft_config.fft_type == IFX_FFT_TYPE_R2C)
    {
        rng_fft_out_size = config->range_fft_config.fft_size / 2;  // for real input use only positive half spectrum
    }
    else
    {
        rng_fft_out_size = config->range_fft_config.fft_size;  // for complex input use full spectrum
    }

    IFX_ERR_HANDLE_N(ifx_rdm_set_output_scale_type(h, config->output_scale_type),
                     ifx_rdm_destroy(h));

    IFX_ERR_HANDLE_N(ifx_rdm_set_threshold(h, config->spect_threshold),
                     ifx_rdm_destroy(h));

    IFX_ERR_HANDLE_N(h->range_ppfft_handle = ifx_ppfft_create(&config->range_fft_config),
                     ifx_rdm_destroy(h));

    IFX_ERR_HANDLE_N(h->doppler_ppfft_handle = ifx_ppfft_create(&config->doppler_fft_config),
                     ifx_rdm_destroy(h));

    h->doppler_fft_result = ifx_vec_create_c(doppler_fft_out_size);

    IFX_ERR_HANDLE_N(h->rdm_matrix = ifx_mat_create_c(rng_fft_out_size, doppler_fft_out_size),
                     ifx_rdm_destroy(h));
    return h;
}

//-----------------------------------------------------------------------------

void ifx_rdm_destroy(ifx_RDM_t* handle)
{
    if (handle == NULL)
    {
        return;
    }

    ifx_vec_destroy_c(handle->doppler_fft_result);

    ifx_mat_destroy_c(handle->rdm_matrix);

    ifx_ppfft_destroy(handle->range_ppfft_handle);

    ifx_ppfft_destroy(handle->doppler_ppfft_handle);

    ifx_mem_free(handle);
}

//-----------------------------------------------------------------------------

void ifx_rdm_run_rc(ifx_RDM_t* handle,
                    const ifx_Matrix_R_t* input,
                    ifx_Matrix_C_t* output)
{
    IFX_ERR_BRK_NULL(handle);
    IFX_MAT_BRK_VALID(input);
    IFX_MAT_BRK_VALID(output);

    uint32_t samples_per_chirp = ifx_ppfft_get_window_size(handle->range_ppfft_handle);
    uint32_t num_of_chirps = ifx_ppfft_get_window_size(handle->doppler_ppfft_handle);

    IFX_ERR_BRK_COND(mCols(input) != samples_per_chirp, IFX_ERROR_DIMENSION_MISMATCH);
    IFX_ERR_BRK_COND(mRows(input) != num_of_chirps, IFX_ERROR_DIMENSION_MISMATCH);
    IFX_MAT_BRK_DIM((handle->rdm_matrix), output);

    uint32_t rng_fft_out_size = mRows(handle->rdm_matrix);
    uint32_t dopp_fft_out_size = mCols(handle->rdm_matrix);

    ifx_Vector_C_t fft_result;
    ifx_Vector_R_t range_fft_inp;

    if (mRows(input) > dopp_fft_out_size)
    {
        num_of_chirps = dopp_fft_out_size;
    }

    for (uint32_t i = 0; i < num_of_chirps; ++i)
    {
        ifx_mat_get_rowview_r(input, i, &range_fft_inp);

        ifx_mat_get_colview_c(handle->rdm_matrix, i, &fft_result);

        ifx_ppfft_run_rc(handle->range_ppfft_handle, &range_fft_inp, &fft_result);
    }

    ifx_Vector_C_t output_vec;
    ifx_Vector_C_t doppler_fft_inp_view;
    uint32_t output_len = vLen(handle->doppler_fft_result);

    for (uint32_t i = 0; i < rng_fft_out_size; ++i)
    {
        ifx_mat_get_rowview_c(handle->rdm_matrix, i, &doppler_fft_inp_view);

        vLen(&doppler_fft_inp_view) = num_of_chirps;

        ifx_mat_get_rowview_c(output, i, &output_vec);

        ifx_ppfft_run_c(handle->doppler_ppfft_handle, &doppler_fft_inp_view, handle->doppler_fft_result);

        // shift the spectrum to bring DC to zero and then rotate around DC to bring approaching
        //  targets on the right side of the spectrum i.e. positive velocity for approaching target
        for (uint32_t j = 0; j < output_len / 2; ++j)
        {
            vAt(&output_vec, j) = vAt(handle->doppler_fft_result, output_len / 2 - 1 - j);
            vAt(&output_vec, output_len / 2 + j) = vAt(handle->doppler_fft_result, output_len - 1 - j);
        }
    }
}

//-----------------------------------------------------------------------------

void ifx_rdm_run_r(ifx_RDM_t* handle,
                   const ifx_Matrix_R_t* input,
                   ifx_Matrix_R_t* output)
{
    IFX_ERR_BRK_NULL(handle);
    IFX_ERR_BRK_NULL(input);
    IFX_ERR_BRK_NULL(output);

    uint32_t samples_per_chirp = ifx_ppfft_get_window_size(handle->range_ppfft_handle);
    uint32_t num_of_chirps = ifx_ppfft_get_window_size(handle->doppler_ppfft_handle);

    IFX_ERR_BRK_COND(mCols(input) != samples_per_chirp, IFX_ERROR_DIMENSION_MISMATCH);
    IFX_ERR_BRK_COND(mRows(input) != num_of_chirps, IFX_ERROR_DIMENSION_MISMATCH);
    IFX_MAT_BRK_DIM((handle->rdm_matrix), output);

    const ifx_Math_Scale_Type_t scale = handle->output_scale_type;

    ifx_rdm_run_rc(handle, input, handle->rdm_matrix);

    for (uint32_t i = 0; i < mRows(output); ++i)
    {
        ifx_Vector_C_t rdm_view;
        ifx_mat_get_rowview_c(handle->rdm_matrix, i, &rdm_view);

        ifx_Vector_R_t output_vec;
        ifx_mat_get_rowview_r(output, i, &output_vec);

        /* compute squared norm of spectrum */
        ifx_vec_abs2_c(&rdm_view, &output_vec);

        /* convert to linear or to dB */
        if (scale == IFX_SCALE_TYPE_LINEAR)
            spectrum2_to_linear(&output_vec, handle->spect_threshold);
        else
            spectrum2_to_db(&output_vec, (ifx_Float_t)scale, handle->spect_threshold);
    }
}

//-----------------------------------------------------------------------------

void ifx_rdm_run_c(ifx_RDM_t* handle,
                   const ifx_Matrix_C_t* input,
                   ifx_Matrix_C_t* output)
{
    IFX_ERR_BRK_NULL(handle);
    IFX_MAT_BRK_VALID(input);
    IFX_MAT_BRK_VALID(output);

    uint32_t samples_per_chirp = ifx_ppfft_get_window_size(handle->range_ppfft_handle);
    uint32_t num_of_chirps = ifx_ppfft_get_window_size(handle->doppler_ppfft_handle);

    IFX_ERR_BRK_COND(mCols(input) != samples_per_chirp, IFX_ERROR_DIMENSION_MISMATCH);
    IFX_ERR_BRK_COND(mRows(input) != num_of_chirps, IFX_ERROR_DIMENSION_MISMATCH);
    IFX_MAT_BRK_DIM((handle->rdm_matrix), output);

    uint32_t rng_fft_out_size = mRows(handle->rdm_matrix);
    uint32_t dopp_fft_out_size = mCols(handle->rdm_matrix);

    ifx_Vector_C_t fft_result;
    ifx_Vector_C_t range_fft_inp;

    if (mRows(input) > dopp_fft_out_size)
    {
        num_of_chirps = dopp_fft_out_size;
    }

    for (uint32_t i = 0; i < num_of_chirps; ++i)
    {
        ifx_mat_get_rowview_c(input, i, &range_fft_inp);

        ifx_mat_get_colview_c(handle->rdm_matrix, i, &fft_result);

        ifx_ppfft_run_c(handle->range_ppfft_handle, &range_fft_inp, &fft_result);
    }

    ifx_Vector_C_t output_vec;
    ifx_Vector_C_t doppler_fft_inp_view;

    for (uint32_t i = 0; i < rng_fft_out_size; ++i)
    {
        ifx_mat_get_rowview_c(handle->rdm_matrix, i, &doppler_fft_inp_view);

        vLen(&doppler_fft_inp_view) = num_of_chirps;

        ifx_mat_get_rowview_c(output, i, &output_vec);

        ifx_ppfft_run_c(handle->doppler_ppfft_handle, &doppler_fft_inp_view, handle->doppler_fft_result);

        // only shift is enough, no rotation required for complex input data based range doppler as
        // in this case approaching target falls on positive side.
        ifx_fft_shift_c(handle->doppler_fft_result, &output_vec);
    }
}

//-----------------------------------------------------------------------------

void ifx_rdm_run_cr(ifx_RDM_t* handle,
                    const ifx_Matrix_C_t* input,
                    ifx_Matrix_R_t* output)
{
    IFX_ERR_BRK_NULL(handle);
    IFX_ERR_BRK_NULL(input);
    IFX_ERR_BRK_NULL(output);

    uint32_t samples_per_chirp = ifx_ppfft_get_window_size(handle->range_ppfft_handle);
    uint32_t num_of_chirps = ifx_ppfft_get_window_size(handle->doppler_ppfft_handle);

    IFX_ERR_BRK_COND(mCols(input) != samples_per_chirp, IFX_ERROR_DIMENSION_MISMATCH);
    IFX_ERR_BRK_COND(mRows(input) != num_of_chirps, IFX_ERROR_DIMENSION_MISMATCH);
    IFX_MAT_BRK_DIM((handle->rdm_matrix), output);

    ifx_Float_t scale = handle->output_scale_type;

    ifx_rdm_run_c(handle, input, handle->rdm_matrix);

    ifx_Vector_C_t rdm_view;
    ifx_Vector_R_t output_vec;

    for (uint32_t i = 0; i < mRows(output); ++i)
    {
        ifx_mat_get_rowview_r(output, i, &output_vec);

        ifx_mat_get_rowview_c(handle->rdm_matrix, i, &rdm_view);

        ifx_vec_abs_c(&rdm_view, &output_vec);

        ifx_math_vec_clip_lt_threshold_r(&output_vec, handle->spect_threshold, CLIPPING_VALUE, &output_vec);

        if (scale != IFX_SCALE_TYPE_LINEAR)
        {
            ifx_vec_linear_to_dB(&output_vec, scale, &output_vec);
        }
    }
}

//-----------------------------------------------------------------------------

void ifx_rdm_set_threshold(ifx_RDM_t* handle,
                           ifx_Float_t threshold)
{
    IFX_ERR_BRK_NULL(handle)
    IFX_ERR_BRK_COND((threshold < 0), IFX_ERROR_ARGUMENT_OUT_OF_BOUNDS)

    handle->spect_threshold = threshold;
}

//-----------------------------------------------------------------------------

ifx_Float_t ifx_rdm_get_threshold(const ifx_RDM_t* handle)
{
    IFX_ERR_BRV_NULL(handle, 0)
    return handle->spect_threshold;
}

//-----------------------------------------------------------------------------

void ifx_rdm_set_output_scale_type(ifx_RDM_t* handle,
                                   ifx_Math_Scale_Type_t output_scale_type)
{
    IFX_ERR_BRK_NULL(handle)

    handle->output_scale_type = output_scale_type;
}

//-----------------------------------------------------------------------------

ifx_Math_Scale_Type_t ifx_rdm_get_output_scale_type(const ifx_RDM_t* handle)
{
    IFX_ERR_BRV_NULL(handle, 0)

    return handle->output_scale_type;
}

//-----------------------------------------------------------------------------

void ifx_rdm_set_range_window(const ifx_Window_Config_t* config,
                              ifx_RDM_t* handle)
{
    IFX_ERR_BRK_NULL(handle)
    ifx_ppfft_set_window(handle->range_ppfft_handle, config);
}

//-----------------------------------------------------------------------------

void ifx_rdm_set_doppler_window(const ifx_Window_Config_t* config,
                                ifx_RDM_t* handle)
{
    IFX_ERR_BRK_NULL(handle)
    ifx_ppfft_set_window(handle->doppler_ppfft_handle, config);
}
