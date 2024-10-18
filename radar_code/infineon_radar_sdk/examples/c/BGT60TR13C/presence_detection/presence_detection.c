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
 * @file    presence_detection.c
 *
 * @brief   Presence detection example.
 *
 * This example illustrates how to fetch time-domain data from an Avian radar sensor
 * and process it utilizing a presence sensing algorithm of the Radar SDK.
 *
 */


/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include <math.h>
#include <string.h>

#include "ifxAvian/Avian.h"
#include "ifxRadarPresenceSensing/PresenceSensing.h"

#include "common.h"
#include "json.h"

/*
==============================================================================
   2. LOCAL DEFINITIONS
==============================================================================
*/
#define RX_ANTENNA_ID 0  // This implementation uses only 1 Rx Antenna

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

static ifx_Avian_Config_t sensor_config_d;
static ifx_Presence_Sensing_Config_t presence_config_d;
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

typedef struct
{
    ifx_Presence_Sensing_t* handle;
    ifx_Presence_Sensing_Result_t result;
} presence_t;

/**
 * @brief Application specific initialization function
 *
 * Initializes the presence sensing context.
 *
 * @param ctx           context of the application
 */
ifx_Error_t presence_init(presence_t* ctx)
{
    return IFX_OK;
}

/**
 * @brief Application specific configuration function
 *
 * Configures the presence sensing with specified in json or default configuration.
 * Defines a state change callback and creates an instance of presence sensing algorithm.
 *
 * @param ctx           context of the application
 * @param device        connected device handle
 * @param json          json handle with configuration
 * @param dev_config    device configuration handle
 */
ifx_Error_t presence_config(presence_t* ctx, ifx_Avian_Device_t* device, ifx_json_t* json, ifx_Avian_Config_t* dev_config)
{
    (void)dev_config;  // ignore this input

    ifx_Avian_Config_t sensor_config;
    ifx_Presence_Sensing_Config_t presence_config;

    if (ifx_json_has_config_presence_sensing(json) && ifx_json_has_config_single_shape(json))
    {
        bool ret = ifx_json_get_config_presence_sensing(json, &presence_config);
        if (!ret)
        {
            fprintf(stderr, "Error parsing presence sensing configuration: %s\n", ifx_json_get_error(json));
            return IFX_ERROR_APP;
        }

        ret = ifx_json_get_device_config_single_shape(json, &sensor_config);

        if (!ret)
        {
            fprintf(stderr, "Error parsing single shape configuration: %s\n", ifx_json_get_error(json));
            return IFX_ERROR_APP;
        }
    }
    else
    {
        ifx_presence_sensing_get_config_defaults(IFX_AVIAN_BGT60TR13C, &sensor_config, &presence_config);
    }

    ctx->handle = ifx_presence_sensing_create(&sensor_config, &presence_config);

    *dev_config = sensor_config;

    return ifx_error_get();
}

/**
 * @brief Application specific cleanup function
 *
 * Destroys the presence sensing instances and cleans up allocated memory.
 *
 * @param ctx           context of the application
 */
ifx_Error_t presence_cleanup(presence_t* ctx)
{
    ifx_presence_sensing_destroy(ctx->handle);

    return ifx_error_get();
}

/**
 * @brief Application specific processing function
 *
 * Runs a presence sensing algorithm on fetched data and informs about the results.
 *
 * @param ctx           context of the application
 * @param frame         collected frame
 */
ifx_Error_t presence_process(presence_t* ctx, ifx_Cube_R_t* frame)
{
    ifx_Error_t err;
    ifx_presence_sensing_run(ctx->handle, frame, &ctx->result);

    err = ifx_error_get();

    if (err == IFX_OK)
    {
        if (ctx->result.target_state)
            app_print(", \"is_present\": true, \t\" target distance (m) \": %g", ctx->result.target_distance_m);
        else
            app_print(", \"is_present\": false");
    }

    return err;
}

/*
==============================================================================
   7. MAIN METHOD
==============================================================================
 */

int main(int argc, char* argv[])
{
    int exitcode;
    app_t s_presence = {0};
    presence_t presence_ctx;

    ifx_presence_sensing_get_config_defaults(IFX_AVIAN_BGT60TR13C, &sensor_config_d, &presence_config_d);

    static const char* app_description = "Presence Sensing";
    static const char* app_epilog = NULL;

    s_presence.app_description = app_description;
    s_presence.app_epilog = app_epilog;

    s_presence.app_init = (void*)&presence_init;
    s_presence.app_config = (void*)&presence_config;
    s_presence.app_process = (void*)&presence_process;
    s_presence.app_cleanup = (void*)&presence_cleanup;

    s_presence.default_metrics = NULL;
    s_presence.default_config = &sensor_config_d;

    exitcode = app_start(argc, argv, &s_presence, &presence_ctx);

    return exitcode;
}
