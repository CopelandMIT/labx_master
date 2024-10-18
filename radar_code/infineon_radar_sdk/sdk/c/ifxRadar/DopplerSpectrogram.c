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

#include "ifxBase/Complex.h"
#include "ifxBase/Defines.h"
#include "ifxBase/Error.h"
#include "ifxBase/internal/Macros.h"
#include "ifxBase/Matrix.h"
#include "ifxBase/Mem.h"
#include "ifxBase/Vector.h"

#include "DopplerSpectrogram.h"
#include "RangeDopplerMap.h"

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

/**
 * @brief Defines the structure for Range Doppler map module processing.
 *        Use type ifx_DopplerSpectrogram_t for this struct.
 */
struct ifx_DopplerSpectrogram_s
{
    ifx_Float_t spect_threshold;        /**< Threshold is in always linear scale, should be greater than 1-e6.
                                             Output spectrum values below this are set to 1-e6 (-120dB).*/
    ifx_RDM_t* rdm_handle;              /**< Range doppler map handle for FMCW devices */
    ifx_Matrix_R_t* rdm_output;         /**< Range doppler map output for internal usage */
    ifx_PPFFT_t* doppler_ppfft_handle;  /**< Preprocessed FFT settings for Doppler FFT defined by \ref ifx_PPFFT_t
                                             e.g. Mean removal, window settings, FFT settings.*/
    ifx_Vector_C_t* doppler_fft_result; /**< Container to store the result of Doppler FFT during range doppler spectrum
                                             calculation.*/
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

static void shift_buffer(ifx_Matrix_R_t* output, uint32_t num_rows)
{
    // shifts the matrix rows by 'num_rows'
    for (uint32_t curr_row = mRows(output) - 1; curr_row >= num_rows; --curr_row)
    {
        ifx_Vector_R_t out_row_1;
        ifx_Vector_R_t out_row_2;

        ifx_mat_get_rowview_r(output, curr_row - num_rows, &out_row_1);
        ifx_mat_get_rowview_r(output, curr_row, &out_row_2);

        ifx_vec_copy_r(&out_row_1, &out_row_2);
    }
}

/*
==============================================================================
   7. EXPORTED FUNCTIONS
==============================================================================
*/

ifx_DopplerSpectrogram_t* ifx_doppler_spectrogram_create(const ifx_DopplerSpectrogram_Config_t* config)
{
    IFX_ERR_BRN_NULL(config)

    ifx_DopplerSpectrogram_t* h = ifx_mem_calloc(1, sizeof(struct ifx_DopplerSpectrogram_s));
    IFX_ERR_BRN_MEMALLOC(h);

    // For more than one chirp, makes sense to compute range FFT else only Doppler FFT is enough
    if (config->chirps_per_frame > 1)
    {
        ifx_RDM_Config_t rdm_config = {
            config->spect_threshold,
            IFX_SCALE_TYPE_DECIBEL_20LOG,
            {/* .fft_type = */ IFX_FFT_TYPE_R2C,
             /* .fft_size = */ 4 * config->num_samples,
             /* .mean_removal_enabled = */ true,
             /* .window_config = */ {IFX_WINDOW_BLACKMANHARRIS, config->num_samples, 0, 1},
             /* .is_normalized_window = */ false},
            config->doppler_fft_config};

        IFX_ERR_HANDLE_N(h->rdm_handle = ifx_rdm_create(&rdm_config),
                         ifx_doppler_spectrogram_destroy(h));

        IFX_ERR_HANDLE_N(h->rdm_output = ifx_mat_create_r(rdm_config.range_fft_config.fft_size / 2,
                                                          rdm_config.doppler_fft_config.fft_size),
                         ifx_doppler_spectrogram_destroy(h));
    }
    else  // A pure doppler device is used for doppler spectrogram. Hence range doppler map is not required in this case
    {
        h->rdm_handle = NULL;
        h->rdm_output = NULL;
    }

    IFX_ERR_HANDLE_N(h->doppler_ppfft_handle = ifx_ppfft_create(&config->doppler_fft_config),
                     ifx_doppler_spectrogram_destroy(h));

    IFX_ERR_HANDLE_N(h->doppler_fft_result = ifx_vec_create_c(config->doppler_fft_config.fft_size),
                     ifx_doppler_spectrogram_destroy(h));

    IFX_ERR_HANDLE_N(ifx_doppler_spectrogram_set_threshold(h, config->spect_threshold),
                     ifx_doppler_spectrogram_destroy(h));

    return h;
}

//-----------------------------------------------------------------------------

void ifx_doppler_spectrogram_destroy(ifx_DopplerSpectrogram_t* handle)
{
    if (handle == NULL)
    {
        return;
    }

    if (handle->rdm_handle != NULL)
    {
        ifx_rdm_destroy(handle->rdm_handle);

        ifx_mat_destroy_r(handle->rdm_output);
    }

    ifx_vec_destroy_c(handle->doppler_fft_result);

    ifx_ppfft_destroy(handle->doppler_ppfft_handle);

    ifx_mem_free(handle);
}

//-----------------------------------------------------------------------------

void ifx_doppler_spectrogram_run_r(ifx_DopplerSpectrogram_t* handle, const ifx_Matrix_R_t* input,
                                   ifx_Matrix_R_t* output)
{
    IFX_ERR_BRK_NULL(handle);
    IFX_ERR_BRK_NULL(handle->rdm_handle);
    IFX_ERR_BRK_NULL(handle->rdm_output);
    IFX_MAT_BRK_VALID(input);
    IFX_MAT_BRK_VALID(output);

    // calculate range doppler map
    ifx_rdm_run_r(handle->rdm_handle, input, handle->rdm_output);

    ifx_Vector_R_t rdm_view;

    ifx_mat_get_rowview_r(handle->rdm_output, 0, &rdm_view);

    ifx_Float_t max_spect_power = ifx_vec_sum_r(&rdm_view);

    uint32_t max_bin_idx = 0;

    // Find doppler spectrum with maximum energy
    for (uint32_t i = 1; i < mRows(handle->rdm_output); ++i)
    {
        ifx_mat_get_rowview_r(handle->rdm_output, i, &rdm_view);

        ifx_Float_t spect_power = ifx_vec_sum_r(&rdm_view);

        if (spect_power > max_spect_power)
        {
            max_bin_idx = i;
            max_spect_power = spect_power;
        }
    }

    ifx_mat_get_rowview_r(handle->rdm_output, max_bin_idx, &rdm_view);

    ifx_Vector_R_t output_vec;

    // shift the history buffer (matrix) by one frame, so that new result can be placed at index = 0
    shift_buffer(output, 1);

    ifx_mat_get_rowview_r(output, 0, &output_vec);

    // copy result at the first row of output matrix
    ifx_vec_blit_r(&rdm_view, 0, vLen(&output_vec), 0, &output_vec);
}

//-----------------------------------------------------------------------------

void ifx_doppler_spectrogram_run_cr(ifx_DopplerSpectrogram_t* handle, const ifx_Vector_C_t* input, ifx_Matrix_R_t* output)
{
    IFX_ERR_BRK_NULL(handle);
    IFX_ERR_BRK_NULL(input);
    IFX_ERR_BRK_NULL(output);

    shift_buffer(output, 1);

    // compute new result and place at row index 0
    ifx_ppfft_run_c(handle->doppler_ppfft_handle, input, handle->doppler_fft_result);

    ifx_fft_shift_c(handle->doppler_fft_result, handle->doppler_fft_result);

    ifx_Vector_R_t output_vec;
    ifx_mat_get_rowview_r(output, 0, &output_vec);

    // compute squared norm of spectrum
    ifx_vec_squared_norm_c(handle->doppler_fft_result, &output_vec);

    // convert to dB
    ifx_vec_spectrum2_to_db(&output_vec, (ifx_Float_t)IFX_SCALE_TYPE_DECIBEL_20LOG, handle->spect_threshold);
}

//-----------------------------------------------------------------------------

void ifx_doppler_spectrogram_set_threshold(ifx_DopplerSpectrogram_t* handle, ifx_Float_t threshold)
{
    IFX_ERR_BRK_NULL(handle);
    IFX_ERR_BRK_COND((threshold < 0), IFX_ERROR_ARGUMENT_OUT_OF_BOUNDS);

    handle->spect_threshold = threshold;
}

//-----------------------------------------------------------------------------

ifx_Float_t ifx_doppler_spectrogram_get_threshold(const ifx_DopplerSpectrogram_t* handle)
{
    IFX_ERR_BRV_NULL(handle, 0);

    return handle->spect_threshold;
}
