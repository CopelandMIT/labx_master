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

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "ifxAlgo/FFT.h"
#include "ifxAlgo/PreprocessedFFT.h"

#include "ifxBase/Error.h"
#include "ifxBase/internal/Macros.h"
#include "ifxBase/Mem.h"
#include "ifxBase/Vector.h"

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
 * @brief Defines the structure for pre-processed FFT module.
 *        Use type ifx_PPFFT_t for this struct.
 */
struct ifx_PPFFT_s
{
    bool mean_removal_enabled;         /**< If false, mean removal step is ignored during range spectrum calculation.*/
    ifx_Vector_R_t* fft_window;        /**< Vector specifying the window function to be used before FFT in range spectrum calculation.*/
    ifx_Window_Config_t window_config; /**< Window type, length and attenuation used for range FFT.*/
    ifx_FFT_t* fft_handle;             /**< Handle to an ifx_FFT_t object.*/
    ifx_Vector_R_t* pp_result_r;       /**< Container to store real pre-processing result in case fft_type is \ref IFX_FFT_TYPE_R2C. Otherwise ignored.*/
    ifx_Vector_C_t* pp_result_c;       /**< Container to store complex pre-processing result in case fft_type is \ref IFX_FFT_TYPE_C2C. Otherwise ignored.*/
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

/*
==============================================================================
   7. EXPORTED FUNCTIONS
==============================================================================
*/

ifx_PPFFT_t* ifx_ppfft_create(const ifx_PPFFT_Config_t* config)
{
    IFX_ERR_BRN_NULL(config);

    ifx_PPFFT_t* h = ifx_mem_calloc(1, sizeof(struct ifx_PPFFT_s));
    IFX_ERR_BRN_MEMALLOC(h);

    if (config->fft_type == IFX_FFT_TYPE_R2C)
    {
        IFX_ERR_HANDLE_N(h->pp_result_r = ifx_vec_create_r(config->window_config.size),
                         ifx_ppfft_destroy(h));
    }
    else /* IFX_FFT_TYPE_C2C */
    {
        IFX_ERR_HANDLE_N(h->pp_result_c = ifx_vec_create_c(config->window_config.size),
                         ifx_ppfft_destroy(h));
    }

    IFX_ERR_HANDLE_N(h->fft_handle = ifx_fft_create(config->fft_type, config->fft_size),
                     ifx_ppfft_destroy(h));

    IFX_ERR_HANDLE_N(h->fft_window = ifx_vec_create_r(config->window_config.size),
                     ifx_ppfft_destroy(h));

    ifx_window_init(&config->window_config, h->fft_window);

    h->window_config = config->window_config;
    h->mean_removal_enabled = config->mean_removal_enabled;

    if (config->is_normalized_window)
    {
        const ifx_Float_t sum = ifx_vec_sum_r(h->fft_window);

        ifx_vec_scale_r(h->fft_window, 1.0f / sum, h->fft_window);
    }

    // Moved out of window_init() as scaling got cancelled if window normalization is performed afterwards.

    if (config->window_config.scale != 0 && config->window_config.scale != 1)
    {
        ifx_vec_scale_r(h->fft_window, config->window_config.scale, h->fft_window);
    }

    return h;
}

//----------------------------------------------------------------------------

void ifx_ppfft_destroy(ifx_PPFFT_t* handle)
{
    if (handle == NULL)
    {
        return;
    }

    ifx_fft_destroy(handle->fft_handle);

    ifx_vec_destroy_r(handle->fft_window);
    ifx_vec_destroy_r(handle->pp_result_r);
    ifx_vec_destroy_c(handle->pp_result_c);

    ifx_mem_free(handle);
}

//----------------------------------------------------------------------------

void ifx_ppfft_run_rc(ifx_PPFFT_t* handle,
                      const ifx_Vector_R_t* input,
                      ifx_Vector_C_t* output)
{
    IFX_ERR_BRK_NULL(handle);
    IFX_VEC_BRK_VALID(input);
    IFX_VEC_BRK_VALID(output);

    ifx_Vector_R_t* fft_in = (ifx_Vector_R_t*)input;

    if (vLen(input) > vLen(handle->pp_result_r))  //  case: Input data is larger than FFT size
    {
        ifx_vec_blit_r(input, 0, vLen(handle->pp_result_r), 0, handle->pp_result_r);

        fft_in = handle->pp_result_r;
    }

    if (handle->mean_removal_enabled != 0)
    {
        ifx_Float_t mean = ifx_vec_mean_r(fft_in);

        ifx_vec_sub_rs(fft_in, mean, handle->pp_result_r);

        ifx_vec_mul_r(handle->pp_result_r, handle->fft_window, handle->pp_result_r);
    }
    else
    {
        ifx_vec_mul_r(fft_in, handle->fft_window, handle->pp_result_r);
    }

    ifx_fft_run_rc(handle->fft_handle, handle->pp_result_r, output);
}

//----------------------------------------------------------------------------

void ifx_ppfft_run_c(ifx_PPFFT_t* handle,
                     const ifx_Vector_C_t* input,
                     ifx_Vector_C_t* output)
{
    IFX_ERR_BRK_NULL(handle);
    IFX_VEC_BRK_VALID(input);
    IFX_VEC_BRK_VALID(output);

    ifx_Vector_C_t* fft_in = (ifx_Vector_C_t*)input;

    if (vLen(input) > vLen(handle->pp_result_c))  //  case: Input data is larger than FFT size
    {
        ifx_vec_blit_c(input, 0, vLen(handle->pp_result_c), 0, handle->pp_result_c);

        fft_in = handle->pp_result_c;
    }

    if (handle->mean_removal_enabled != 0)
    {
        ifx_Complex_t mean = ifx_vec_mean_c(fft_in);

        ifx_vec_sub_cs(fft_in, mean, handle->pp_result_c);

        ifx_vec_mul_cr(handle->pp_result_c, handle->fft_window, handle->pp_result_c);
    }
    else
    {
        ifx_vec_mul_cr(fft_in, handle->fft_window, handle->pp_result_c);
    }

    ifx_fft_run_c(handle->fft_handle, handle->pp_result_c, output);
}

//----------------------------------------------------------------------------

void ifx_ppfft_set_mean_removal_flag(ifx_PPFFT_t* handle, bool flag)
{
    IFX_ERR_BRK_NULL(handle);

    handle->mean_removal_enabled = flag;
}

//----------------------------------------------------------------------------

uint8_t ifx_ppfft_get_mean_removal_flag(const ifx_PPFFT_t* handle)
{
    IFX_ERR_BRV_NULL(handle, 0);

    return handle->mean_removal_enabled;
}

//----------------------------------------------------------------------------

uint32_t ifx_ppfft_get_fft_size(const ifx_PPFFT_t* handle)
{
    IFX_ERR_BRV_NULL(handle, 0);

    return ifx_fft_get_fft_size(handle->fft_handle);
}

//----------------------------------------------------------------------------

ifx_FFT_Type_t ifx_ppfft_get_fft_type(const ifx_PPFFT_t* handle)
{
    IFX_ERR_BRV_NULL(handle, IFX_FFT_TYPE_R2C);

    return ifx_fft_get_fft_type(handle->fft_handle);
}

//----------------------------------------------------------------------------

void ifx_ppfft_set_window(ifx_PPFFT_t* handle, const ifx_Window_Config_t* config)
{
    IFX_ERR_BRK_NULL(handle);
    IFX_ERR_BRK_NULL(config);

    if (config->size != handle->window_config.size)
    {
        ifx_vec_destroy_r(handle->fft_window);
        handle->fft_window = ifx_vec_create_r(config->size);
    }

    ifx_window_init(config, handle->fft_window);

    handle->window_config = *config;
}

//----------------------------------------------------------------------------

ifx_Vector_R_t* ifx_ppfft_get_window(ifx_PPFFT_t* handle)
{
    IFX_ERR_BRV_NULL(handle, NULL);

    return handle->fft_window;
}

//----------------------------------------------------------------------------

ifx_Window_Type_t ifx_ppfft_get_window_type(ifx_PPFFT_t* handle)
{
    IFX_ERR_BRV_NULL(handle, IFX_WINDOW_HAMM);

    return handle->window_config.type;
}

//----------------------------------------------------------------------------

uint32_t ifx_ppfft_get_window_size(ifx_PPFFT_t* handle)
{
    IFX_ERR_BRV_NULL(handle, 0);

    return handle->window_config.size;
}

//----------------------------------------------------------------------------

ifx_Float_t ifx_ppfft_get_window_attenuation(ifx_PPFFT_t* handle)
{
    IFX_ERR_BRV_NULL(handle, 0);

    return handle->window_config.at_dB;
}

//----------------------------------------------------------------------------

ifx_Window_Config_t* ifx_ppfft_get_window_config(ifx_PPFFT_t* handle)
{
    IFX_ERR_BRV_NULL(handle, NULL);

    return &handle->window_config;
}

//----------------------------------------------------------------------------

void ifx_ppfft_calc_freq_axis(const ifx_PPFFT_t* handle,
                              ifx_Float_t sampling_freq_Hz,
                              ifx_Math_Axis_Spec_t* fft_freq_axis_spec_Hz)
{
    IFX_ERR_BRK_NULL(handle);
    IFX_ERR_BRK_NULL(fft_freq_axis_spec_Hz);
    IFX_ERR_BRK_COND((sampling_freq_Hz <= 0), IFX_ERROR_ARGUMENT_OUT_OF_BOUNDS);

    uint32_t fft_size = ifx_fft_get_fft_size(handle->fft_handle);

    ifx_FFT_Type_t fft_type = ifx_fft_get_fft_type(handle->fft_handle);

    fft_freq_axis_spec_Hz->min_value = 0;

    fft_freq_axis_spec_Hz->value_bin_per_step = sampling_freq_Hz / fft_size;

    if (fft_type == IFX_FFT_TYPE_C2C)
    {
        fft_freq_axis_spec_Hz->max_value = sampling_freq_Hz;
    }
    else /* IFX_FFT_TYPE_R2C */
    {
        fft_freq_axis_spec_Hz->max_value = (sampling_freq_Hz / 2) - fft_freq_axis_spec_Hz->value_bin_per_step;
    }
}
