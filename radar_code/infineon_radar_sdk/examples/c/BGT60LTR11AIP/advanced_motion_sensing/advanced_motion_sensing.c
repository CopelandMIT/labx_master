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
 * @file    advanced_motion_sensing.c
 *
 * @brief   Advanced motion sensing example.
 *
 * This example illustrates how to fetch data from an LTR11 Doppler radar sensor
 * and process it utilizing an advanced motion sensing algorithm of the Radar SDK.
 *
 */

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/
#include "ifxAdvancedMotionSensing/AdvancedMotionSensing.h"
#include "ifxBase/Error.h"
#include "ifxLtr11/DeviceLtr11.h"
#include <argparse.h>

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
    ifx_Advanced_Motion_Sensing_t* advanced_motion_sensing_instance;
    ifx_Advanced_Motion_Sensing_Config_t config;
    ifx_Advanced_Motion_Sensing_Output_t output;
    ifx_Ltr11_Device_t* device;
    ifx_Ltr11_Config_t device_config;
    ifx_Vector_C_t* frame;
    ifx_Ltr11_Metadata_t metadata;
} advanced_motion_sensing_context_t;

/*
==============================================================================
   6. LOCAL FUNCTIONS
==============================================================================
*/

/**
 * @brief Helper function to process the data from motion sensing algorithm
 *
 * Prints out information about the results.
 *
 * @param algo_output       output of the motion sensing algorithm
 */
static void process_advanced_motion_sensing_result(ifx_Advanced_Motion_Sensing_Output_t* algo_output)
{
    IFX_ERR_BRK_NULL(algo_output);

    printf("Output of the interference mitigation algorithm: \n");
    printf("Computed Amplitude: %d", algo_output->peak_to_peak_amplitude);
    printf(" Target detection output: ");
    if (algo_output->target_detection == TARGET_MOTION_DETECTED)
    {
        printf("Target detected \n");
    }
    else if (algo_output->target_detection == POTENTIAL_TARGET_DETECTED)
    {
        printf("Potential target detected\n");
    }
    else
    {
        printf("No target detected\n");
    }
}

/**
 * @brief Application specific initialization function
 *
 * Creates a motion sensing instance
 *
 * @param ctx           context of the application
 */
ifx_Error_t advanced_motion_sensing_init(advanced_motion_sensing_context_t* ctx)
{
    ctx->advanced_motion_sensing_instance = ifx_advanced_motion_sensing_create(&ctx->config);
    if (ifx_error_get())
    {
        return ifx_error_get();
    }
    ctx->frame = NULL;

    return IFX_OK;
}

/**
 * @brief Application specific configuration function
 *
 * Connects to the LTR11 device, creates a device handle and sets the configuration.
 *
 * @param ctx           context of the application
 */
ifx_Error_t configure_device(advanced_motion_sensing_context_t* ctx)
{
    ctx->device = ifx_ltr11_create();
    if (ifx_error_get())
    {
        return ifx_error_get();
    }

    ifx_ltr11_get_config_defaults(ctx->device, &ctx->device_config);
    if (ifx_error_get())
    {
        return ifx_error_get();
    }

    /* Apply the device settings based on the device configuration structure. */
    ifx_ltr11_set_config(ctx->device, &ctx->device_config);

    if (ifx_error_get())
    {
        return ifx_error_get();
    }

    return IFX_OK;
}

/**
 * @brief Application specific cleanup function
 *
 * Destroys instances and cleans up allocated memory.
 *
 * @param ctx           context of the application
 */
ifx_Error_t advanced_motion_sensing_cleanup(advanced_motion_sensing_context_t* ctx)
{
    ifx_vec_destroy_c(ctx->frame);
    ifx_advanced_motion_sensing_destroy(ctx->advanced_motion_sensing_instance);

    return ifx_error_get();
}

/**
 * @brief Application specific processing function
 *
 * Runs a motion sensing algorithm on a fetched data and processes the results.
 *
 * @param ctx           context of the application
 */
ifx_Error_t advanced_motion_sensing_process(advanced_motion_sensing_context_t* ctx)
{
    ifx_advanced_motion_sensing_run(ctx->advanced_motion_sensing_instance, ctx->frame,
                                    &ctx->output);
    if (ifx_error_get())
    {
        return ifx_error_get();
    }

    process_advanced_motion_sensing_result(&ctx->output);

    return IFX_OK;
}

/*
==============================================================================
   7. MAIN METHOD
==============================================================================
 */

int main(int argc, char* argv[])
{
    ifx_Error_t error_code;
    int num_of_frames_to_fetch = 220;
    advanced_motion_sensing_context_t advanced_motion_sensing_context = {0};
    advanced_motion_sensing_context.config.advanced_motion_sensing_threshold = 40;
    advanced_motion_sensing_context.config.enable_interference_mitigation = true;

    const char* const usage_str[] = {
        "app_advanced_motion_sensing [options] ",
        NULL,
    };

    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_INTEGER('f', "frames", &num_of_frames_to_fetch, "number of frames (<=0 is infinity)", NULL, 0, 0),
        OPT_INTEGER('t', "advanced_motion_sensing_threshold", &advanced_motion_sensing_context.config.advanced_motion_sensing_threshold, "Interference mitigation algorithm threshold", NULL, 0, 0),
        OPT_INTEGER('e', "enable_interference_mitigation", &advanced_motion_sensing_context.config.enable_interference_mitigation, "Enable interference mitigation flag", NULL, 0, 0),
        OPT_END(),
    };

    struct argparse argparse;
    argparse_init(&argparse, options, usage_str, 0);
    argparse_describe(&argparse, "Interference Mitigation Algorithm Application Example", NULL);
    argparse_parse(&argparse, argc, argv);

    /* Create and configure device */
    error_code = configure_device(&advanced_motion_sensing_context);
    if (error_code != IFX_OK)
    {
        fprintf(stderr, "Failed to configure device: %s\n", ifx_error_to_string(error_code));
        goto fail;
    }

    ifx_ltr11_start_acquisition(advanced_motion_sensing_context.device);
    if (ifx_error_get() != IFX_OK)
    {
        fprintf(stderr, "Data Aquisition starting failed");
        goto fail;
    }

    /* Initialize the number of samples in the algo with the num_samples in the device config */
    advanced_motion_sensing_context.config.num_samples = advanced_motion_sensing_context.device_config.num_samples;

    error_code = advanced_motion_sensing_init(&advanced_motion_sensing_context);
    if (error_code != IFX_OK)
    {
        fprintf(stderr, "Failed to initialize the advanced motion sensing context: %s\n", ifx_error_to_string(error_code));
        goto fail;
    }

    for (int i = 0; i < num_of_frames_to_fetch; i++)
    {
        advanced_motion_sensing_context.frame = ifx_ltr11_get_next_frame(advanced_motion_sensing_context.device, advanced_motion_sensing_context.frame, &advanced_motion_sensing_context.metadata);
        error_code = ifx_error_get();
        if (error_code != IFX_OK)
        {
            fprintf(stderr, "Failed to fetch frame: %s\n", ifx_error_to_string(error_code));
            goto fail;
        }
        error_code = advanced_motion_sensing_process(&advanced_motion_sensing_context);
        if (error_code)
        {
            fprintf(stderr, "Failed to run the advanced motion sensing algorithm: %s\n", ifx_error_to_string(error_code));
            goto fail;
        }
    }

fail:
    /* Stop data acquisition */
    if (advanced_motion_sensing_context.device)
    {
        ifx_ltr11_stop_acquisition(advanced_motion_sensing_context.device);
        ifx_ltr11_destroy(advanced_motion_sensing_context.device);
    }

    if (advanced_motion_sensing_context.advanced_motion_sensing_instance)
    {
        error_code = advanced_motion_sensing_cleanup(&advanced_motion_sensing_context);
        if (error_code)
        {
            fprintf(stderr, "Failed to cleanup the advanced motion sensing algorithm instance: %s\n", ifx_error_to_string(error_code));
        }
    }
}
