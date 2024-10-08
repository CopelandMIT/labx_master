/* ===========================================================================
** Copyright (C) 2022 Infineon Technologies AG
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
 * This example illustrates how to fetch time-domain data from a BGT60LTR11
 * sensor using the Radar SDK.
 */

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/
#include <stdio.h>
#include <stdlib.h>

#include "ifxBase/Base.h"
#include "ifxLtr11/DeviceLtr11.h"

/*
==============================================================================
   2. LOCAL DEFINITIONS
==============================================================================
*/

#define NUM_FETCHED_FRAMES 10 /**< Number of frames to fetch */

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

/**
 * @brief Helper function to process fetched frame
 *
 * This function is an example showing a possible way
 * of processing the ifi and ifq signals by making a sum of the samples.
 * The goal in this example is to print the sum of samples given one frame.
 *
 * @param frame    frame data containing multiple samples (vector)
 */
void process_frame(const ifx_Vector_C_t* frame)
{
    ifx_Float_t sum = ifx_complex_abs(ifx_vec_sum_c(frame));
    printf("frame data sum: %g\n", sum);
}

/*
==============================================================================
   7. MAIN METHOD
==============================================================================
*/

int main(int argc, char** argv)
{
    ifx_Error_t error = IFX_OK;
    ifx_Ltr11_Device_t* device = NULL;
    ifx_Vector_C_t* vector = NULL;
    ifx_Ltr11_Config_t config = {0};

    /* Open the device: Connect to the first radar sensor found */
    device = ifx_ltr11_create();
    if ((error = ifx_error_get()) != IFX_OK)
    {
        fprintf(stderr, "Failed to open device: %s\n", ifx_error_to_string(error));
        goto out;
    }

    /* Get default device configuration for connected BGT60LTR11 sensor.
     * Here, we use the default configuration that depends on the device.
     */
    ifx_ltr11_get_config_defaults(device, &config);
    if ((error = ifx_error_get()) != IFX_OK)
    {
        fprintf(stderr, "Failed to get default device config: %s\n", ifx_error_to_string(error));
        goto out;
    }

    /* Apply the device settings based on the device configuration structure */
    ifx_ltr11_set_config(device, &config);
    if ((error = ifx_error_get()) != IFX_OK)
    {
        fprintf(stderr, "Failed to set device config: %s\n", ifx_error_to_string(error));
        goto out;
    }

    /* Start data acquisition. */
    ifx_ltr11_start_acquisition(device);
    if ((error = ifx_error_get()) != IFX_OK)
    {
        fprintf(stderr, "Failed to start acquisition: %s\n", ifx_error_to_string(error));
        goto out;
    }

    /* Fetch NUM_FETCHED_FRAMES number of frames */
    for (int frame_number = 0; frame_number < NUM_FETCHED_FRAMES; frame_number++)
    {
        printf("Reading vector data (%d)\n", frame_number);

        /* Get the time-domain data for the next frame. The function blocks
         * until the full frame is available and copy the data into the vector.
         * This function also creates a vector for time domain data
         * acquisition, if not created already. It is the  responsibility of
         * the caller to free the returned frame in this scope.
         * This function also fills the values of the metadata structure
         * declared by the caller.
         */
        ifx_Ltr11_Metadata_t metadata = {0};
        vector = ifx_ltr11_get_next_frame(device, vector, &metadata);
        if ((error = ifx_error_get()) != IFX_OK)
        {
            fprintf(stderr, "Failed to fetch data: %s\n", ifx_error_to_string(error));
            goto out;
        }

        /* Process the frame. */
        process_frame(vector);
    }

    /* Stop data acquisition */
    ifx_ltr11_stop_acquisition(device);
    if ((error = ifx_error_get()) != IFX_OK)
    {
        fprintf(stderr, "Failed to stop data acquisition: %s\n", ifx_error_to_string(error));
        goto out;
    }

out:
    /* Free memory and close device */
    ifx_vec_destroy_c(vector);
    ifx_ltr11_destroy(device);

    return error == IFX_OK ? EXIT_SUCCESS : EXIT_FAILURE;
}
