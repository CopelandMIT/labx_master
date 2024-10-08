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

/**
 * @file    range_doppler_map.c
 *
 * @brief   Range-Doppler Map algorithm example.
 *
 * This example illustrates how to fetch and process time-domain data
 * from an Avian radar sensor using Range-Doppler Map (RDM) algorithm
 * and 2D MTI for filtering of the Radar SDK.
 *
 */


/*
==============================================================================
1. INCLUDE FILES
==============================================================================
*/

#include <string.h>

#include "common.h"
#include "ifxAlgo/Algo.h"
#include "ifxAvian/Avian.h"
#include "ifxRadar/Radar.h"
#include "range_doppler_map_defaults.h"

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

typedef struct
{
    ifx_Avian_Device_t* device_handle;
    ifx_Matrix_R_t* rdm;
    ifx_RDM_t* rdm_handle;
    ifx_2DMTI_R_t* mti_handle;
    ifx_Math_Axis_Spec_t range_spec;
    ifx_Math_Axis_Spec_t speed_spec;
} rdm_t;

/*
==============================================================================
4. DATA
==============================================================================
*/

static ifx_Avian_Metrics_t default_metrics =
    {
        .range_resolution_m = IFX_RANGE_RESOLUTION_M,
        .max_range_m = IFX_MAX_RANGE_M,
        .speed_resolution_m_s = IFX_SPEED_RESOLUTION_M_S,
        .max_speed_m_s = IFX_MAX_SPEED_M_S,
        .center_frequency_Hz = 60.5e9f,  // use default value for Avian
};

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
 * @brief Application specific initialization function
 *
 * Initializes the Range-Doppler Map (RDM) context
 *
 * @param rdm_context       context of the application
 */
ifx_Error_t rdm_init(rdm_t* rdm_context)
{
    memset(rdm_context, 0, sizeof(rdm_t));
    return ifx_error_get();
}

/**
 * @brief Application specific configuration function
 *
 * Configures the RDM and MTI (for filtering) with default configuration.
 * The parameters have been initialized with reasonable defaults for a Range Doppler example
 * of approximately 2 meters of range with a range resolution of around 3.5cm.
 *
 * @param rdm_context       context of the application
 * @param device            connected device handle
 * @param json              json handle with configuration (not used in this case)
 * @param dev_config        device configuration handle
 */
ifx_Error_t rdm_config(rdm_t* rdm_context, ifx_Avian_Device_t* device, ifx_json_t* json, ifx_Avian_Config_t* dev_config)
{
    ifx_Error_t ret = 0;

    const uint32_t range_fft_size = dev_config->num_samples_per_chirp * 4;   // Zero padding of 4 gives good range resolution in Range spectrum
    const uint32_t doppler_fft_size = dev_config->num_chirps_per_frame * 4;  // Zero padding of 4 gives good range resolution in Doppler spectrum

    ifx_PPFFT_Config_t range_fft_config = {
        .fft_type = IFX_FFT_TYPE_R2C,
        .fft_size = range_fft_size,
        .mean_removal_enabled = true,
        .window_config = {IFX_WINDOW_BLACKMANHARRIS, dev_config->num_samples_per_chirp, 0, 1},
        .is_normalized_window = 1};

    ifx_PPFFT_Config_t doppler_fft_config = {
        .fft_type = IFX_FFT_TYPE_C2C,
        .fft_size = doppler_fft_size,
        .mean_removal_enabled = true,
        .window_config = {IFX_WINDOW_CHEBYSHEV, dev_config->num_chirps_per_frame, 100, 1},
        .is_normalized_window = 1};

    ifx_RDM_Config_t rdm_config = (ifx_RDM_Config_t) {
        .spect_threshold = IFX_SPECT_THRESHOLD,
        .output_scale_type = IFX_SCALE_TYPE_LINEAR,
        .range_fft_config = range_fft_config,
        .doppler_fft_config = doppler_fft_config};

    rdm_context->rdm_handle = ifx_rdm_create(&rdm_config);
    if ((ret = ifx_error_get()))
    {
        return ret;
    }

    rdm_context->rdm = ifx_mat_create_r(range_fft_size / 2, doppler_fft_size);  // for real input, range dimension has info only in positive half
    if ((ret = ifx_error_get()))
    {
        return ret;
    }

    // MTI Filter for static target cancellation
    rdm_context->mti_handle = ifx_2dmti_create_r(IFX_ALPHA_MTI_FILTER,
                                                 IFX_MAT_ROWS(rdm_context->rdm),
                                                 IFX_MAT_COLS(rdm_context->rdm));
    if ((ret = ifx_error_get()))
    {
        return ret;
    }

    const float bandwidth_Hz = ifx_devconf_get_bandwidth(dev_config);
    rdm_context->range_spec = ifx_spectrum_axis_calc_range_axis(IFX_FFT_TYPE_R2C, range_fft_size, dev_config->num_samples_per_chirp, bandwidth_Hz);
    if ((ret = ifx_error_get()))
    {
        return ret;
    }

    const float center_freq_Hz = (float)ifx_avian_get_sampling_center_frequency(device, dev_config);
    const float chirptime_s = ifx_devconf_get_chirp_time(dev_config);
    rdm_context->speed_spec = ifx_spectrum_axis_calc_speed_axis(IFX_FFT_TYPE_C2C, doppler_fft_size, center_freq_Hz, chirptime_s);

    return (ifx_error_get());
}

/**
 * @brief Application specific cleanup function
 *
 * Destroys the RDM and 2D MTI instances and cleans up allocated memory.
 *
 * @param rdm_context       context of the application
 */
ifx_Error_t rdm_cleanup(rdm_t* rdm_context)
{
    ifx_rdm_destroy(rdm_context->rdm_handle);
    ifx_mat_destroy_r(rdm_context->rdm);
    ifx_2dmti_destroy_r(rdm_context->mti_handle);
    return ifx_error_get();
}

/**
 * @brief Helper function to search for a peak
 *
 * Traverses the map in search of a peak value.
 *
 * @param matrix        analyzed data (ifx_Matrix_R_t)
 * @param rmax          row index of a peak value
 * @param cmax          column index of a peak value
 */
void rdm_peak_search(const ifx_Matrix_R_t* matrix, uint32_t* rmax, uint32_t* cmax)
{
    *rmax = 0, *cmax = 0;
    ifx_Float_t max_value = IFX_MAT_AT(matrix, 0, 0);

    for (uint32_t r = 0; r < IFX_MAT_ROWS(matrix); r++)
    {
        for (uint32_t c = 0; c < IFX_MAT_COLS(matrix); c++)
        {
            ifx_Float_t value = IFX_MAT_AT(matrix, r, c);
            if (value > max_value)
            {
                *rmax = r;
                *cmax = c;
                max_value = value;
            }
        }
    }
}

/**
 * @brief Application specific processing function
 *
 * Runs a Range-Doppler Map algorithm on fetched data and informs about the results.
 * Processing runs also 2D MTI
 *
 * @param rdm_context       context of the application
 * @param frame             collected frame
 */
ifx_Error_t rdm_process(rdm_t* rdm_context, ifx_Cube_R_t* frame)
{
    ifx_Error_t ret = 0;
    ifx_Matrix_R_t antenna_data;
    ifx_cube_get_row_r(frame, 0, &antenna_data);
    ifx_rdm_run_r(rdm_context->rdm_handle, &antenna_data, rdm_context->rdm);
    if ((ret = ifx_error_get()))
    {
        return ret;
    }

    ifx_2dmti_run_r(rdm_context->mti_handle, rdm_context->rdm, rdm_context->rdm);
    if ((ret = ifx_error_get()))
    {
        return ret;
    }

    // do peak search
    uint32_t rmax;
    uint32_t cmax;
    rdm_peak_search(rdm_context->rdm, &rmax, &cmax);
    // range
    const ifx_Float_t range = rmax * rdm_context->range_spec.value_bin_per_step;
    // speed
    const ifx_Float_t speed = (((ifx_Float_t)IFX_MAT_COLS(rdm_context->rdm) / 2) - (ifx_Float_t)cmax) * rdm_context->speed_spec.value_bin_per_step;

    app_print(", range_m:%g, speed_m_s:%g", range, speed);

    return (ifx_error_get());
}

/*
==============================================================================
   7. MAIN METHOD
==============================================================================
 */

int main(int argc, char** argv)
{
    app_t s_rdm = {0};
    rdm_t rdm_context = {0};
    int exitcode = 0;

    static const char* app_description = "Range Doppler Map";
    static const char* app_epilog = NULL;

    s_rdm.app_description = app_description;
    s_rdm.app_epilog = app_epilog;

    s_rdm.app_init = (void*)&rdm_init;
    s_rdm.app_config = (void*)&rdm_config;
    s_rdm.app_process = (void*)&rdm_process;
    s_rdm.app_cleanup = (void*)&rdm_cleanup;

    s_rdm.default_metrics = &default_metrics;

    exitcode = app_start(argc, argv, &s_rdm, &rdm_context);
    return exitcode;
}
