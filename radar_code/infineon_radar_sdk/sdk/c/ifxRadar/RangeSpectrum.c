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

#include "ifxAlgo/PreprocessedFFT.h"

#include "ifxBase/Complex.h"
#include "ifxBase/Defines.h"
#include "ifxBase/Error.h"
#include "ifxBase/internal/Macros.h"
#include "ifxBase/Math.h"
#include "ifxBase/Matrix.h"
#include "ifxBase/Mem.h"
#include "ifxBase/Vector.h"

#include "ifxAlgo/MTI.h"

#include "ifxRadar/RangeSpectrum.h"

/*
==============================================================================
   2. LOCAL DEFINITIONS
==============================================================================
*/

#define CLIPPING_VALUE (1e-6f)  // Corresponds to -120dB
#define MAX_RX         (4U)     // Maximum Rx antenna are 4 for BGTATR24C

/*
==============================================================================
   3. LOCAL TYPES
==============================================================================
*/

/**
 * @brief Defines the structure for Range Spectrum processing module.
 *        Use type ifx_RS_t for this struct for Range Spectrum processing to obtain range resolution.
 *
 * Please check \ref sct_radarsdk_introduction_rangeres chapter to see how range
 * resolution is achieved.
 *
 */
struct ifx_RS_s
{
    ifx_Vector_C_t* fft_mean_result;         /**< Container to store the result of mean removal stage
                                                  during range spectrum calculation.*/
    ifx_Matrix_C_t* fft_spectrum_matrix;     /**< Container to store the range spectrum matrix.*/
    ifx_RS_Mode_t mode;                      /**< Specifies the range spectrum mode defined in \ref ifx_RS_Mode_t.*/
    uint32_t num_of_chirps;                  /**< Specifies the number of chirps present in input data matrix.*/
    uint32_t single_chirp_mode_index;        /**< Specifies the index of the chirp to be considered if mode configured
                                                  in \ref IFX_RS_MODE_SINGLE_CHIRP. else ignored.*/
    ifx_Float_t spect_threshold;             /**< Threshold is always in linear scale, should be greater than 1-e6.
                                                  Range spectrum output values below this are set to 1-e6 (-120dB).*/
    ifx_Math_Scale_Type_t output_scale_type; /**< Linear or dB scale for the output of range spectrum module.*/
    ifx_PPFFT_t* ppfft_handle;               /**< Handle to an ifx_PPFFT_t object.*/
    ifx_MTI_t* mti_handle[MAX_RX];           /**< Only used in range spectrogram function to remove static targets*/
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

static uint32_t get_index_of_highest_energy_r(const ifx_Matrix_R_t* input);

static uint32_t get_index_of_highest_energy_c(const ifx_Matrix_C_t* input);

static void coh_integ_run_rc(ifx_RS_t* handle,
                             const ifx_Matrix_R_t* input,
                             ifx_Vector_C_t* output);

static void coh_integ_run_c(ifx_RS_t* handle,
                            const ifx_Matrix_C_t* input,
                            ifx_Vector_C_t* output);

/*
==============================================================================
   6. LOCAL FUNCTIONS
==============================================================================
*/

static uint32_t get_index_of_highest_energy_r(const ifx_Matrix_R_t* input)
{
    ifx_Float_t max = 0;
    uint32_t max_index = 0;

    for (uint32_t i = 0; i < mRows(input); i++)
    {
        ifx_Vector_R_t view;
        ifx_mat_get_rowview_r(input, i, &view);

        ifx_Float_t row_sum = ifx_vec_sqsum_r(&view);

        if (row_sum > max)
        {
            max = row_sum;
            max_index = i;
        }
    }

    return max_index;
}

//----------------------------------------------------------------------------

static uint32_t get_index_of_highest_energy_c(const ifx_Matrix_C_t* input)
{
    ifx_Float_t max = 0;
    uint32_t max_index = 0;

    for (uint32_t i = 0; i < mRows(input); i++)
    {
        ifx_Vector_C_t view;
        ifx_mat_get_rowview_c(input, i, &view);

        ifx_Float_t row_sum = ifx_vec_sqsum_c(&view);

        if (row_sum > max)
        {
            max = row_sum;
            max_index = i;
        }
    }

    return max_index;
}

//----------------------------------------------------------------------------

static void coh_integ_run_rc(ifx_RS_t* handle,
                             const ifx_Matrix_R_t* input,
                             ifx_Vector_C_t* output)
{
    IFX_ERR_BRK_COND(mRows(input) > mRows(handle->fft_spectrum_matrix), IFX_ERROR_DIMENSION_MISMATCH);

    ifx_Vector_C_t fft_result;
    ifx_Vector_R_t input_view;

    if (handle->mode == IFX_RS_MODE_COHERENT_INTEGRATION)
    {
        ifx_Complex_t zero;

        IFX_COMPLEX_SET(zero, 0, 0);

        ifx_vec_setall_c(output, zero);
    }

    for (uint32_t i = 0; i < mRows(input); i++)
    {
        ifx_mat_get_rowview_r(input, i, &input_view);

        ifx_mat_get_rowview_c(handle->fft_spectrum_matrix, i, &fft_result);

        ifx_ppfft_run_rc(handle->ppfft_handle, &input_view, &fft_result);

        if (handle->mode == IFX_RS_MODE_COHERENT_INTEGRATION)
        {
            ifx_vec_add_c(&fft_result, output, output);
        }
    }

    if (handle->mode == IFX_RS_MODE_MAX_BIN)
    {
        for (uint32_t c = 0; c < mCols(handle->fft_spectrum_matrix); c++)
        {
            ifx_Vector_C_t view;

            ifx_mat_get_colview_c(handle->fft_spectrum_matrix, c, &view);

            uint32_t idx = ifx_vec_max_idx_c(&view);

            vAt(output, c) = vAt(&view, idx);
        }
    }
    else  // IFX_RS_MODE_COHERENT_INTEGRATION
    {
        ifx_Float_t avg_scale = 1.0f / (ifx_Float_t)(mRows(input));

        ifx_vec_scale_cr(output, avg_scale, output);
    }
}

//----------------------------------------------------------------------------

static void coh_integ_run_c(ifx_RS_t* handle,
                            const ifx_Matrix_C_t* input,
                            ifx_Vector_C_t* output)
{
    IFX_ERR_BRK_COND(mRows(input) > mRows(handle->fft_spectrum_matrix), IFX_ERROR_DIMENSION_MISMATCH);

    ifx_Vector_C_t fft_result;
    ifx_Vector_C_t input_view;

    if (handle->mode == IFX_RS_MODE_COHERENT_INTEGRATION)
    {
        ifx_Complex_t zero;

        IFX_COMPLEX_SET(zero, 0, 0);

        ifx_vec_setall_c(output, zero);
    }

    for (uint32_t i = 0; i < mRows(input); i++)
    {
        ifx_mat_get_rowview_c(input, i, &input_view);

        ifx_mat_get_rowview_c(handle->fft_spectrum_matrix, i, &fft_result);

        ifx_ppfft_run_c(handle->ppfft_handle, &input_view, &fft_result);

        if (handle->mode == IFX_RS_MODE_COHERENT_INTEGRATION)
        {
            ifx_vec_add_c(&fft_result, output, output);
        }
    }

    if (handle->mode == IFX_RS_MODE_MAX_BIN)
    {
        for (uint32_t c = 0; c < mCols(handle->fft_spectrum_matrix); c++)
        {
            ifx_Vector_C_t view;

            ifx_mat_get_colview_c(handle->fft_spectrum_matrix, c, &view);

            uint32_t idx = ifx_vec_max_idx_c(&view);

            vAt(output, c) = vAt(&view, idx);
        }
    }
    else  // IFX_RS_MODE_COHERENT_INTEGRATION
    {
        ifx_Float_t avg_scale = 1.0f / (ifx_Float_t)(mRows(input));

        ifx_vec_scale_cr(output, avg_scale, output);
    }
}

/*
==============================================================================
   7. EXPORTED FUNCTIONS
==============================================================================
*/

ifx_RS_t* ifx_rs_create(const ifx_RS_Config_t* config)
{
    IFX_ERR_BRN_NULL(config);
    IFX_ERR_BRN_ARGUMENT(config->num_of_chirps_per_frame == 0);

    ifx_RS_t* h = ifx_mem_calloc(1, sizeof(struct ifx_RS_s));
    IFX_ERR_BRN_MEMALLOC(h);

    uint32_t fft_out_size;

    if (config->fft_config.fft_type == IFX_FFT_TYPE_R2C)
    {
        fft_out_size = config->fft_config.fft_size / 2;  // half output for real input data
    }
    else                                                 /* IFX_FFT_TYPE_C2C */
    {
        fft_out_size = config->fft_config.fft_size;      // full output for complex input data
    }

    h->fft_mean_result = ifx_vec_create_c(fft_out_size);

    IFX_ERR_HANDLE_N(h->fft_spectrum_matrix = ifx_mat_create_c(config->num_of_chirps_per_frame, fft_out_size),
                     ifx_rs_destroy(h));

    IFX_ERR_HANDLE_N(h->ppfft_handle = ifx_ppfft_create(&config->fft_config),
                     ifx_rs_destroy(h));

    for (uint32_t i = 0; i < MAX_RX; ++i)
    {
        IFX_ERR_HANDLE_N(h->mti_handle[i] = ifx_mti_create(0.5, fft_out_size),
                         ifx_rs_destroy(h));
    }

    if (config->num_of_chirps_per_frame == 1)
        h->mode = IFX_RS_MODE_SINGLE_CHIRP;  // for doppler devices, number of chirps is one, thus Single chirp mode is set as default
    else
        h->mode = IFX_RS_MODE_COHERENT_INTEGRATION;

    h->single_chirp_mode_index = 0;
    h->num_of_chirps = config->num_of_chirps_per_frame;

    h->output_scale_type = config->output_scale_type;
    h->spect_threshold = (config->spect_threshold <= CLIPPING_VALUE) ? CLIPPING_VALUE : config->spect_threshold;

    return h;
}

//----------------------------------------------------------------------------

void ifx_rs_destroy(ifx_RS_t* handle)
{
    if (handle == NULL)
    {
        return;
    }

    ifx_ppfft_destroy(handle->ppfft_handle);
    ifx_mat_destroy_c(handle->fft_spectrum_matrix);
    ifx_vec_destroy_c(handle->fft_mean_result);

    for (uint32_t i = 0; i < MAX_RX; ++i)
    {
        ifx_mti_destroy(handle->mti_handle[i]);
    }


    ifx_mem_free(handle);
}

//----------------------------------------------------------------------------

void ifx_rs_run_r(ifx_RS_t* handle,
                  const ifx_Matrix_R_t* input,
                  ifx_Vector_R_t* output)
{
    IFX_MAT_BRK_VALID(input);
    IFX_VEC_BRK_VALID(output);
    IFX_VEC_BRK_DIM(output, handle->fft_mean_result);

    ifx_rs_run_rc(handle, input, handle->fft_mean_result);

    ifx_vec_abs_c(handle->fft_mean_result, output);

    ifx_math_vec_clip_lt_threshold_r(output, handle->spect_threshold, CLIPPING_VALUE, output);

    if (handle->output_scale_type != IFX_SCALE_TYPE_LINEAR)
    {
        ifx_vec_linear_to_dB(output, handle->output_scale_type, output);
    }
}

//----------------------------------------------------------------------------

void ifx_rs_run_rc(ifx_RS_t* handle,
                   const ifx_Matrix_R_t* input,
                   ifx_Vector_C_t* output)
{
    IFX_ERR_BRK_NULL(handle);
    IFX_MAT_BRK_VALID(input);
    IFX_VEC_BRK_VALID(output);

    ifx_Vector_R_t view_in;

    if (handle->mode == IFX_RS_MODE_MAX_ENERGY)
    {
        const uint32_t i = get_index_of_highest_energy_r(input);

        ifx_mat_get_rowview_r((ifx_Matrix_R_t*)input, i, &view_in);

        ifx_ppfft_run_rc(handle->ppfft_handle, &view_in, output);
    }
    else if (handle->mode == IFX_RS_MODE_SINGLE_CHIRP)
    {
        IFX_ERR_BRK_COND(mRows(input) < handle->single_chirp_mode_index, IFX_ERROR_DIMENSION_MISMATCH);
        ifx_mat_get_rowview_r((ifx_Matrix_R_t*)input, handle->single_chirp_mode_index, &view_in);

        ifx_ppfft_run_rc(handle->ppfft_handle, &view_in, output);
    }
    else
    {
        coh_integ_run_rc(handle, input, output);
    }
}

//----------------------------------------------------------------------------

void ifx_rs_run_c(ifx_RS_t* handle,
                  const ifx_Matrix_C_t* input,
                  ifx_Vector_C_t* output)
{
    IFX_ERR_BRK_NULL(handle);
    IFX_MAT_BRK_VALID(input);
    IFX_VEC_BRK_VALID(output);

    ifx_Vector_C_t view_in;

    if (handle->mode == IFX_RS_MODE_MAX_ENERGY)
    {
        const uint32_t i = get_index_of_highest_energy_c(input);

        ifx_mat_get_rowview_c((ifx_Matrix_C_t*)input, i, &view_in);

        ifx_ppfft_run_c(handle->ppfft_handle, &view_in, output);
    }
    else if (handle->mode == IFX_RS_MODE_SINGLE_CHIRP)
    {
        IFX_ERR_BRK_COND(mRows(input) < handle->single_chirp_mode_index, IFX_ERROR_DIMENSION_MISMATCH);
        ifx_mat_get_rowview_c((ifx_Matrix_C_t*)input, handle->single_chirp_mode_index, &view_in);

        ifx_ppfft_run_c(handle->ppfft_handle, &view_in, output);
    }
    else
    {
        coh_integ_run_c(handle, input, output);
    }
}

//----------------------------------------------------------------------------

void ifx_rs_run_cr(ifx_RS_t* handle,
                   const ifx_Matrix_C_t* input,
                   ifx_Vector_R_t* output)
{
    IFX_MAT_BRK_VALID(input);
    IFX_VEC_BRK_VALID(output);
    IFX_VEC_BRK_DIM(output, handle->fft_mean_result);

    ifx_rs_run_c(handle, input, handle->fft_mean_result);

    ifx_vec_abs_c(handle->fft_mean_result, output);

    ifx_math_vec_clip_lt_threshold_r(output, handle->spect_threshold, CLIPPING_VALUE, output);

    if (handle->output_scale_type != IFX_SCALE_TYPE_LINEAR)
    {
        ifx_vec_linear_to_dB(output, handle->output_scale_type, output);
    }
}

//----------------------------------------------------------------------------

void ifx_rs_set_mode(ifx_RS_t* handle,
                     const ifx_RS_Mode_t mode)
{
    IFX_ERR_BRK_NULL(handle);
    IFX_ERR_BRK_ARGUMENT(mode > 3);

    handle->mode = mode;
}

//----------------------------------------------------------------------------

ifx_RS_Mode_t ifx_rs_get_mode(const ifx_RS_t* handle)
{
    IFX_ERR_BRV_NULL(handle, IFX_RS_MODE_SINGLE_CHIRP);
    return handle->mode;
}

//----------------------------------------------------------------------------

void ifx_rs_set_single_chirp_mode_index(ifx_RS_t* handle,
                                        uint32_t index)
{
    IFX_ERR_BRK_NULL(handle);
    IFX_ERR_BRK_COND(index > handle->num_of_chirps, IFX_ERROR_ARGUMENT_OUT_OF_BOUNDS);

    handle->single_chirp_mode_index = index;
}

//----------------------------------------------------------------------------

uint32_t ifx_rs_get_single_chirp_mode_index(const ifx_RS_t* handle)
{
    IFX_ERR_BRV_NULL(handle, 0);
    return handle->single_chirp_mode_index;
}

//----------------------------------------------------------------------------

void ifx_rs_set_output_scale_type(ifx_RS_t* handle,
                                  const ifx_Math_Scale_Type_t output_scale_type)
{
    IFX_ERR_BRK_NULL(handle);
    handle->output_scale_type = output_scale_type;
}

//----------------------------------------------------------------------------

ifx_Math_Scale_Type_t ifx_rs_get_output_scale_type(const ifx_RS_t* handle)
{
    IFX_ERR_BRV_NULL(handle, IFX_SCALE_TYPE_LINEAR);
    return handle->output_scale_type;
}

//----------------------------------------------------------------------------

void ifx_rs_set_window(ifx_RS_t* handle,
                       const ifx_Window_Config_t* config)
{
    ifx_ppfft_set_window(handle->ppfft_handle, config);
}

//----------------------------------------------------------------------------

void ifx_rs_copy_fft_matrix(const ifx_RS_t* handle,
                            ifx_Matrix_C_t* output)
{
    ifx_mat_blit_c(handle->fft_spectrum_matrix, 0, mRows(output), 0, mCols(output), output);
}

//----------------------------------------------------------------------------

void ifx_rs_set_threshold(ifx_RS_t* handle, ifx_Float_t threshold)
{
    IFX_ERR_BRK_NULL(handle);
    IFX_ERR_BRK_COND((threshold < 0), IFX_ERROR_ARGUMENT_OUT_OF_BOUNDS);

    handle->spect_threshold = threshold;
}

//----------------------------------------------------------------------------

ifx_Float_t ifx_rs_get_threshold(const ifx_RS_t* handle)
{
    IFX_ERR_BRV_NULL(handle, CLIPPING_VALUE);

    return handle->spect_threshold;
}

//----------------------------------------------------------------------------

void ifx_rs_spectrogram_r(ifx_RS_t* handle, uint32_t rx_idx, bool static_target_removal, const ifx_Vector_R_t* input, ifx_Matrix_R_t* output)
{
    IFX_ERR_BRK_NULL(handle);
    IFX_ERR_BRK_NULL(input);
    IFX_ERR_BRK_NULL(output);
    IFX_ERR_BRK_COND((rx_idx >= MAX_RX), IFX_ERROR_ARGUMENT_OUT_OF_BOUNDS);

    ifx_Vector_R_t out_row_view;
    ifx_mat_get_rowview_r(output, 0, &out_row_view);
    IFX_VEC_BRK_DIM(input, &out_row_view);

    // shifts the N-1 rows down by 1, so that new output row is placed at index 0
    for (uint32_t i = mRows(output) - 1; i >= 1; --i)
    {
        ifx_Vector_R_t out_row_1;
        ifx_Vector_R_t out_row_2;
        ifx_mat_get_rowview_r(output, i - 1, &out_row_1);
        ifx_mat_get_rowview_r(output, i, &out_row_2);

        ifx_vec_copy_r(&out_row_1, &out_row_2);
    }

    if (static_target_removal)
    {
        ifx_mti_run(handle->mti_handle[rx_idx], input, &out_row_view);
    }
    else
    {
        // Copy new output row at index 0
        ifx_vec_copy_r(input, &out_row_view);
    }
}
