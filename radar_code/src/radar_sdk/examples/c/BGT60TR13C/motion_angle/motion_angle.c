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
 * @file    motion_angle.c
 *
 * @brief   Motion Angle algorithm example.
 *
 * This example illustrates how to fetch and process time-domain data
 * from an Avian radar sensor using Motion Angle algorithm of the Radar SDK.
 *
 */

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include "ifxAvian/Avian.h"
#include "ifxMotionAngle/MotionAngle.h"

#include "common.h"
#include <string.h>

/*
==============================================================================
   3. LOCAL TYPES
==============================================================================
*/
typedef struct
{
    ifx_MotionAngle_t* motion_angle_handle;
    ifx_MotionAngle_Result_t result;
} motion_angle_t;

/*
==============================================================================
   6. LOCAL FUNCTIONS
==============================================================================
*/

/**
 * @brief Application specific initialization function
 *
 * Initializes the motion angle context
 *
 * @param ctx           context of the application
 */
ifx_Error_t motion_angle_init(motion_angle_t* ctx)
{
    memset(ctx, 0, sizeof(motion_angle_t));
    return IFX_OK;
}

/**
 * @brief Application specific configuration function
 *
 * Creates the motion angle algorithm instance for the connected device.
 * Configures the motion angle with default configuration.
 *
 * @param ctx           context of the application
 * @param device        connected device handle
 * @param json          json handle with configuration (not used in this case)
 * @param dev_config    device configuration handle
 */
ifx_Error_t motion_angle_config(motion_angle_t* ctx, ifx_Avian_Device_t* device, ifx_json_t* json, ifx_Avian_Config_t* dev_config)
{
    ctx->motion_angle_handle = ifx_motionangle_create(ifx_avian_get_sensor_type(device), dev_config);
    if (!ctx->motion_angle_handle)
        return ifx_error_get();
    ifx_motionangle_set_sensitivity(ctx->motion_angle_handle, 1.0f);
    ifx_motionangle_set_maxrange(ctx->motion_angle_handle, 5.0f);  // 5 meter max range

    return ifx_error_get();
}

/**
 * @brief Application specific cleanup function
 *
 * Destroys the motion angle instance and cleans up allocated memory.
 *
 * @param ctx           context of the application
 */
ifx_Error_t motion_angle_cleanup(motion_angle_t* ctx)
{
    ifx_motionangle_destroy(ctx->motion_angle_handle);

    return ifx_error_get();
}

/**
 * @brief Application specific processing function
 *
 * Runs a motion angle algorithm on fetched data and informs about the results.
 *
 * @param ctx           context of the application
 * @param frame         collected frame
 */
ifx_Error_t motion_angle_process(motion_angle_t* ctx, ifx_Cube_R_t* frame)
{
    ifx_motionangle_run(ctx->motion_angle_handle, frame, &ctx->result);
    if (ctx->result.distance <= 0.0)
    {
        app_print("\", state=\"absence\"");
    }
    else
    {
        app_print(", state=\"presence\", \"distance\":%.1f", ctx->result.distance);
        app_print(", \"speed\":%.1f", ctx->result.speed);
        app_print(", \"angle_azimuth\":%.1f", ctx->result.angle_azimuth);
        app_print(", \"angle_elevation\":%.1f", ctx->result.angle_elevation);
    }
    return ifx_error_get();
}

/*
==============================================================================
   7. MAIN METHOD
==============================================================================
 */

int main(int argc, char* argv[])
{
    app_t s_motion_angle = {0};
    motion_angle_t motion_angle_context = {0};
    ifx_Avian_Config_t device_config = {0};

    // get default device configuration
    {
        ifx_MotionAngle_t* handle = ifx_motionangle_create_from_mode(IFX_AVIAN_BGT60TR13C, IFX_MOTIONANGLE_DEFAULT, &device_config);
        ifx_motionangle_destroy(handle);
    }

    // function Description
    static const char* app_description = "MotionAngle detection";
    static const char* app_epilog = "\n"
                                    "Output\n"
                                    "    The output format is given in JSON format. elapsed_time and frame_number\n"
                                    "    denote the time passed since starting the application and the current frame\n"
                                    "    number, respectively. state indicates \"absence\" or \"presence\". In case a\n"
                                    "    target is detected (presence) then the distance of the closest object to the\n"
                                    "    sensor in meters, its speed in meters per second, and its angle_azimuth and\n"
                                    "    angle_elevation in degrees are reported.";

    s_motion_angle.app_description = app_description;
    s_motion_angle.app_epilog = app_epilog;

    s_motion_angle.app_init = (void*)motion_angle_init;
    s_motion_angle.app_config = (void*)motion_angle_config;
    s_motion_angle.app_process = (void*)motion_angle_process;
    s_motion_angle.app_cleanup = (void*)motion_angle_cleanup;

    s_motion_angle.default_metrics = NULL;
    s_motion_angle.default_config = &device_config;

    int exitcode = app_start(argc, argv, &s_motion_angle, &motion_angle_context);

    return exitcode;
}
