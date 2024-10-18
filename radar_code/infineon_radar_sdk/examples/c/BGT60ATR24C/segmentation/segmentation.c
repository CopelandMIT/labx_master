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
 * @file   segmentation.c
 *
 * @brief  Segmentation algorithm example.
 *
 * This example illustrates how to fetch and process time-domain data
 * from an Avian radar sensor using Segmentation algorithm to detect
 * targets in segments in front of the sensor.
 *
 */

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include "ifxAvian/Avian.h"
#include "ifxRadarSegmentation/RadarSegmentation.h"

#include "common.h"

/*
==============================================================================
2. LOCAL DEFINITIONS
==============================================================================
*/

#define MAX_NUM_TRACKS        5  // currently max 5 tracks
#define NUM_SEGMENTS          6  // currently 6 segments
#define NUM_ENTRIES_PER_TRACK 4  // four entries per track: id, range, angle, speed
#define NUM_RX_ANTENNAS       2  // currently only works with two RX antennas activated

/*
==============================================================================
   3. LOCAL TYPES
==============================================================================
*/

typedef struct
{
    ifx_Segmentation_t* segmentation_handle;
    ifx_Segmentation_Config_t segmentation_config;
    ifx_Vector_R_t* segments;
    ifx_Matrix_R_t* tracks;
} segmentation_t;

/*
==============================================================================
   6. LOCAL FUNCTIONS
==============================================================================
*/

/**
 * @brief Helper function to create a view from the frame that contains the required RX antennas
 *
 * @param frame         processed frame
 * @param orientation   orientation information
 */
static ifx_Cube_R_t frame_view(const ifx_Cube_R_t* frame, ifx_Orientation_t orientation)
{
    ifx_Cube_R_t view = {0};

    if (IFX_CUBE_ROWS(frame) == 2)
    {
        // view is identical with frame
        // num_rx_antennas x num_chirps_per_frame x num_samples_per_chirp
        IFX_MDA_VIEW_R(&view, frame, IFX_MDA_SLICE_FULL(), IFX_MDA_SLICE_FULL(), IFX_MDA_SLICE_FULL());
        return view;
    }
    else if (orientation == IFX_ORIENTATION_PORTRAIT)
    {
        // RX2 and RX3, we skip row 0 (RX1)
        IFX_MDA_VIEW_R(&view, frame, IFX_MDA_SLICE(1, 3, 1), IFX_MDA_SLICE_FULL(), IFX_MDA_SLICE_FULL());
        return view;
    }
    else  // (orientation == IFX_ORIENTATION_LANDSCAPE)
    {
        // RX1 and RX3, we skip row 1 (RX2)
        IFX_MDA_VIEW_R(&view, frame, IFX_MDA_SLICE(0, 3, 2), IFX_MDA_SLICE_FULL(), IFX_MDA_SLICE_FULL());
        return view;
    }
}

/**
 * @brief Helper function to process segmentation results
 *
 * Prints the information about segmentation results.
 *
 * @param segments          information about segments in a vector
 * @param tracks            information about tracks in a matrix
 */
static void process_segmentation_result(ifx_Vector_R_t* segments, ifx_Matrix_R_t* tracks)
{
    IFX_ERR_BRK_NULL(segments);
    IFX_ERR_BRK_NULL(tracks);

    app_print(" ,\"segments\":[");
    for (uint32_t i = 0; i < IFX_VEC_LEN(segments); i++)
    {
        app_print("%d", (int)IFX_VEC_AT(segments, i));
        if (i < (IFX_VEC_LEN(segments) - 1))
            app_print(", ");
    }
    app_print("], \"tracks\":[");
    uint32_t valid_count = 0;
    for (uint32_t i = 0; i < IFX_MAT_ROWS(tracks); i++)
    {
        if (IFX_MAT_AT(tracks, i, 0) > 0)
        {
            if (valid_count)
                app_print(",");
            app_print("[%d,%5.2f,%5.2f,%5.2f]", (int)IFX_MAT_AT(tracks, i, 0),
                      IFX_MAT_AT(tracks, i, 1),
                      IFX_MAT_AT(tracks, i, 2),
                      IFX_MAT_AT(tracks, i, 3));
            valid_count++;
        }
    }
    app_print("]");
}

//----------------------------------------------------------------------------

/**
 * @brief Application specific initialization function
 *
 * @param ctx               context of the application
 */
ifx_Error_t segmentation_init(segmentation_t* ctx)
{
    return IFX_OK;
}

/**
 * @brief Application specific configuration function
 *
 * Creates and configures the segmentation algorithm instance
 * with specified in json or default configuration for a connected device
 *
 * @param ctx               context of the application
 * @param device            connected device handle
 * @param json              json handle with configuration
 * @param dev_config        device configuration handle
 */
ifx_Error_t segmentation_config(segmentation_t* ctx, ifx_Avian_Device_t* device, ifx_json_t* json, ifx_Avian_Config_t* dev_config)
{
    const ifx_Radar_Sensor_Info_t* sensor_info = ifx_avian_get_sensor_information(device);

    if (sensor_info->num_rx_antennas < 3)
    {
        fprintf(stderr, "Segmentation Seamless Tracking requires a radar sensor with at least three RX antennas\n");
        return IFX_ERROR_APP;
    }
    if (ifx_avian_get_sensor_type(device) != IFX_AVIAN_BGT60TR13C)
    {
        fprintf(stderr, "Warning: Segmentation Seamless Tracking was optimized for BGT60TR13C and will not perform well with connected radar sensor.\n");
    }

    // initialize segmentation configuration
    {
        const ifx_Segmentation_Config_t config = {
            .num_samples_per_chirp = dev_config->num_samples_per_chirp,
            .num_chirps_per_frame = dev_config->num_chirps_per_frame,
            .bandwidth_Hz = ifx_avian_get_sampling_bandwidth(device, dev_config),
            .center_frequency_Hz = ifx_avian_get_sampling_center_frequency(device, dev_config),
            .orientation = IFX_ORIENTATION_LANDSCAPE};
        ctx->segmentation_config = config;
    }

    if (ifx_json_has_segmentation(json))
    {
        // get configuration from json
        bool ret = ifx_json_get_segmentation(json, dev_config, &ctx->segmentation_config);
        if (!ret)
        {
            fprintf(stderr, "Invalid segmentation configuration: %s", ifx_json_get_error(json));
            return IFX_ERROR_APP;
        }
    }
    else
    {
        // add it to the json structure
        ifx_json_set_segmentation(json, &ctx->segmentation_config);
    }

    const uint32_t rx_mask = dev_config->rx_mask;
    if (ctx->segmentation_config.orientation == IFX_ORIENTATION_LANDSCAPE)
    {
        // requires RX1 and RX3 (1<<1 | 1<<2 = 5)
        if ((rx_mask & 5) != 5)
        {
            fprintf(stderr, "RX1 and RX3 must be activated for orientation landscape\n");
            return IFX_ERROR_APP;
        }
    }
    else  // IFX_ORIENTATION_PORTRAIT
    {
        // requires RX2 and RX3 (1<<1 | 1<<2 = 6)
        if ((rx_mask & 6) != 6)
        {
            fprintf(stderr, "RX2 and RX3 must be activated for orientation portrait\n");
            return IFX_ERROR_APP;
        }
    }

    ctx->segmentation_handle = ifx_segmentation_create(&ctx->segmentation_config);

    ctx->tracks = ifx_mat_create_r(MAX_NUM_TRACKS, NUM_ENTRIES_PER_TRACK);
    ctx->segments = ifx_vec_create_r(NUM_SEGMENTS);

    return ifx_error_get();
}

/**
 * @brief Application specific cleanup function
 *
 * Destroys the segmentation specific instances and cleans up allocated memory.
 *
 * @param ctx           context of the application
 */
ifx_Error_t segmentation_cleanup(segmentation_t* ctx)
{
    ifx_vec_destroy_r(ctx->segments);
    ifx_mat_destroy_r(ctx->tracks);
    ifx_segmentation_destroy(ctx->segmentation_handle);

    return ifx_error_get();
}

/**
 * @brief Application specific processing function
 *
 * Runs a segmentation algorithm on fetched data and informs about the results.
 *
 * @param ctx           context of the application
 * @param frame         collected frame
 */
ifx_Error_t segmentation_process(segmentation_t* ctx, ifx_Cube_R_t* frame)
{
    const ifx_Cube_R_t view = frame_view(frame, ctx->segmentation_config.orientation);

    ifx_segmentation_run(ctx->segmentation_handle, &view, ctx->segments, ctx->tracks);
    process_segmentation_result(ctx->segments, ctx->tracks);
    return ifx_error_get();
}

/*
==============================================================================
   7. MAIN METHOD
==============================================================================
 */

int main(int argc, char* argv[])
{
    app_t s_segmentation = {0};
    segmentation_t segmentation_context = {0};
    ifx_Avian_Config_t device_config = {0};

    // get default device configuration
    {
        ifx_Segmentation_t* seg = ifx_segmentation_create_from_mode(IFX_SEGMENTATION_500MHZ_LANDSCAPE, &device_config);
        ifx_segmentation_destroy(seg);
    }

    // function Description
    static const char* app_description = "Segmentation detection";
    static const char* app_epilog = "\n"
                                    "Output\n"
                                    "    The output format is given in JSON format. elapsed_time and frame_number\n"
                                    "    denote the time passed since starting the application and the current frame\n"
                                    "    number, respectively. The array segments list if an object was detected in\n"
                                    "    the respective segment (1) or not (0). The array tracks contains a list of\n"
                                    "    all current tracks. Within a track, the first number denotes the track number\n"
                                    "    (starting from 1). The following three numbers are the distance of the object\n"
                                    "    to the sensor in meters, the angle in degrees, and the speed in meters per\n"
                                    "    second.";

    s_segmentation.app_description = app_description;
    s_segmentation.app_epilog = app_epilog;

    s_segmentation.app_init = (void*)segmentation_init;
    s_segmentation.app_config = (void*)segmentation_config;
    s_segmentation.app_process = (void*)segmentation_process;
    s_segmentation.app_cleanup = (void*)segmentation_cleanup;

    s_segmentation.default_metrics = NULL;
    s_segmentation.default_config = &device_config;

    int exitcode = app_start(argc, argv, &s_segmentation, &segmentation_context);

    return exitcode;
}
