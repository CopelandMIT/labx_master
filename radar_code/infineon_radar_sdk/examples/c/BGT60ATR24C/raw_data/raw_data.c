/* ===========================================================================
** Copyright (C) 2021-2022 Infineon Technologies AG
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
 * @file    raw_data.c
 *
 * @brief   Raw data example.
 *
 * This example illustrates how to fetch time-domain data from an FMCW
 * radar sensor like BGT60TR13, BGT60UTR11AIP or BGT60ATR24 using the Radar SDK.
 */

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "ifxAvian/DeviceControl.h"
#include "ifxBase/Base.h"

/*
==============================================================================
   2. LOCAL DEFINITIONS
==============================================================================
*/

#define NUM_FETCHED_FRAMES 10 /**< Number of frames to fetch */

/*
==============================================================================
   6. LOCAL FUNCTIONS
==============================================================================
*/

/**
 * @brief Helper function to process data
 *
 * This function is an example showing a possible way
 * of processing antenna signal. The goal in this example is to
 * sum up all chirps into one vector.
 *
 * @param antenna_data data from one antenna containing multiple chirps
 */
static void process_antenna_data(const ifx_Matrix_R_t* antenna_data)
{
    ifx_Vector_R_t chirp = {0};
    // Create the sum vector
    ifx_Vector_R_t* sum = ifx_vec_create_r(IFX_MAT_COLS(antenna_data));

    // Iterate through all chirps
    for (uint32_t i = 0; i < IFX_MAT_ROWS(antenna_data); i++)
    {
        // Fetch a chirp from the antenna data matrix
        ifx_mat_get_rowview_r(antenna_data, i, &chirp);
        // add it to the sum vector
        ifx_vec_add_r(&chirp, sum, sum);
    }

    // Divide the sum vector element wise by number of chirps in the antenna data
    ifx_vec_scale_r(sum, 1.0f / IFX_MAT_ROWS(antenna_data), sum);

    for (uint32_t i = 0; i < IFX_VEC_LEN(sum); i++)
    {
        printf("%.4f ", IFX_VEC_AT(sum, i));
    }
    printf("\n\n");
    ifx_vec_destroy_r(sum);
}

//----------------------------------------------------------------------------

/**
 * @brief Helper function to separate signals
 *
 * Separates different antenna signals
 * and pass them for further processing.
 *
 * @param frame The frame may contain multiple antenna signals,
 *              depending on the device configuration.
 *              Each antenna signal can contain multiple chirps.
 */
static void process_frame(ifx_Cube_R_t* frame)
{
    ifx_Matrix_R_t antenna_data;

    for (uint32_t i = 0; i < IFX_CUBE_ROWS(frame); i++)
    {
        ifx_cube_get_row_r(frame, i, &antenna_data);
        process_antenna_data(&antenna_data);
    }
}

/**
 * @brief Helper function to print device configuration
 *
 * Print the device configuration to stdout.
 *
 * @param config        device configuration
 */
static void print_device_config(const ifx_Avian_Config_t* config)
{
    printf("Device configuration:\n");
    printf("sample_rate_Hz:          %" PRIu32 "\n", config->sample_rate_Hz);
    printf("rx_mask:                 %" PRIu32 "\n", config->rx_mask);
    printf("tx_mask:                 %" PRIu32 "\n", config->tx_mask);
    printf("tx_power_level:          %" PRIu32 "\n", config->tx_power_level);
    printf("if_gain_dB:              %" PRIu32 "\n", config->if_gain_dB);
    printf("start_frequency_Hz:      %" PRIu64 "\n", config->start_frequency_Hz);
    printf("end_frequency_Hz:        %" PRIu64 "\n", config->end_frequency_Hz);
    printf("num_samples_per_chirp:   %" PRIu32 "\n", config->num_samples_per_chirp);
    printf("num_chirps_per_frame:    %" PRIu32 "\n", config->num_chirps_per_frame);
    printf("chirp_repetition_time_s: %g\n", config->chirp_repetition_time_s);
    printf("frame_repetition_time_s: %g\n", config->frame_repetition_time_s);
    printf("hp_cutoff_Hz:            %" PRIu32 "\n", config->hp_cutoff_Hz);
    printf("aaf_cutoff_Hz:           %" PRIu32 "\n", config->aaf_cutoff_Hz);
    printf("mimo_mode:               %s\n", config->mimo_mode == IFX_MIMO_TDM ? "time-domain multiplexed" : "off");
    printf("\n");
}

/*
==============================================================================
   7. MAIN METHOD
==============================================================================
 */

int main(int argc, char** argv)
{
    ifx_Error_t error = IFX_OK;
    ifx_Avian_Config_t device_config = {0};
    ifx_Avian_Device_t* device_handle = NULL;
    ifx_Cube_R_t* frame = NULL;

    printf("Radar SDK Version: %s\n", ifx_sdk_get_version_string_full());

    /* Open the device: Connect to the first radar sensor found. */
    device_handle = ifx_avian_create();
    if ((error = ifx_error_get()) != IFX_OK)
    {
        fprintf(stderr, "Failed to open device: %s\n", ifx_error_to_string(error));
        goto out;
    }

    const char* uuid = ifx_avian_get_board_uuid(device_handle);
    printf("UUID of board: %s\n", uuid);

    /* Get default device configuration for connected radar sensor.
     * Here, we use the default configuration that depends on the connected
     * radar sensor.
     * Typically, the radar sensor is either configured by setting all members
     * of the device configuration (ifx_Avian_Config_t) or by setting the
     * members of a structure of type ifx_Avian_Metrics_t and converting the
     * structure to a device configuration using the function
     * ifx_avian_metrics_to_config. The latter approach using the
     * ifx_Avian_Metrics_t structure provides an easy way to configure the
     * radar sensor in terms of high-level parameters, while the device
     * configuration structure ifx_Avian_Config_t allows to configure the
     * sensor in a more detailed way.
     */
    ifx_avian_get_config_defaults(device_handle, &device_config);
    if ((error = ifx_error_get()) != IFX_OK)
    {
        fprintf(stderr, "Failed to get default device config:  %s\n", ifx_error_to_string(error));
        goto out;
    }

    /* Apply the device settings based on the device configuration structure. */
    ifx_avian_set_config(device_handle, &device_config);
    if ((error = ifx_error_get()) != IFX_OK)
    {
        fprintf(stderr, "Failed to set device config:  %s\n", ifx_error_to_string(error));
        goto out;
    }

    /* Print the current device configuration to stdout:
     * Read back the current device configuration. Due to discrete register
     * values in the radar sensors, the device configuration passed to
     * ifx_avian_set_config and the actual device configuration set might
     * differ slightly.
     * We then print the read-back device configuration to stdout.
     */
    ifx_avian_get_config(device_handle, &device_config);
    print_device_config(&device_config);

    /* Fetch NUM_FETCHED_FRAMES number of frames. */
    for (int frame_number = 0; frame_number < NUM_FETCHED_FRAMES; frame_number++)
    {
        /* Get the time-domain data for the next frame. The function will block
         * until the full frame is available and copy the data into the frame
         * handle.
         * This function also creates a frame structure for time domain data
         * acquisition, if not created already. It is the responsibility of
         * the caller to free the returned frame in this scope.
         */
        frame = ifx_avian_get_next_frame(device_handle, frame);
        if ((error = ifx_error_get()) != IFX_OK)
        {
            fprintf(stderr, "Failed to get next frame: %s\n", ifx_error_to_string(error));
            goto out;
        }

        /* Process the frame. */
        process_frame(frame);
    }

out:
    /* Close the device after processing all frames. It is valid to pass NULL
     * to destroy functions.
     */
    ifx_avian_destroy(device_handle);
    ifx_cube_destroy_r(frame);

    return error == IFX_OK ? EXIT_SUCCESS : EXIT_FAILURE;
}
