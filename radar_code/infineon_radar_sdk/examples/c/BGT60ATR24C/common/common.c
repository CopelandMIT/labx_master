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
 * @file common.c
 *
 * @brief This file contains common functionality used by rdk examples.
 *
 */

// disable warnings about unsafe functions with MSVC
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include <inttypes.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>

#include "common.h"
#include "argparse.h"
#include "ifxBase/Version.h"
#include "time_formatter.h"
#include "util.h"

/*
==============================================================================
   2. LOCAL DEFINITIONS
==============================================================================
*/
//  SUPPORTED RECORDING FORMATS
//---------------------------------
//  RECORD_FORMAT_DEFAULT [one sample per line, one empty line after each chirp]
//==============================
//  Ant0_Chirp_0_samples
//
//  Ant0_Chirp_1_samples
//
//  ....
//  Ant0_Chirp_last_samples
//
//  Ant1_Chirp_0_samples
//
//  Ant1_Chirp_1_samples
//  .... till last antenna Data for each frame

//  RECORD_FORMAT_ANTENNA_TABLE [sample index and corresponding sample for each antennae in every line]
//==============================
//  0, Ant_0_Chirp_0_sample_0,Ant_1_Chirp_0_sample_0, ... ,Ant_last_Chirp_0_sample_0,
//  1, Ant_0_Chirp_0_sample_1,Ant_1_Chirp_0_sample_1, ... ,Ant_last_Chirp_0_sample_1,
//  ....
//  last,Ant_0_Chirp_0_sample_last,Ant_1_Chirp_0_sample_last, ... ,Ant_last_Chirp_0_sample_last,
//  0,Ant_0_Chirp_1_sample_0, Ant_1_Chirp_1_sample_0, ... ,Ant_last_Chirp_1_sample_0,
//  1,Ant_0_Chirp_1_sample_1, Ant_1_Chirp_1_sample_1, ... ,Ant_last_Chirp_1_sample_1,
//  ....
//  last,Ant_0_Chirp_1_sample_last, Ant_1_Chirp_1_sample_last, ... ,Ant_last_Chirp_1_sample_last,
//  ...
//  ...
//  0,Ant_0_Chirp_last_sample_0,Ant_1_Chirp_last_sample_0, ... ,Ant_last_Chirp_last_sample_0,
//  1,Ant_0_Chirp_last_sample_1,Ant_1_Chirp_last_sample_1, ... ,Ant_last_Chirp_last_sample_1,
//  ....
//  last,Ant_0_Chirp_last_sample_last,Ant_1_Chirp_last_sample_last, ... ,Ant_last_Chirp_last_sample_last,

#define RECORD_FORMAT_DEFAULT       0
#define RECORD_FORMAT_ANTENNA_TABLE 1

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
static struct
{
    bool verbose;  // detailed information printout
    ifx_Time_Handle_t time_handle;
    FILE* file_results;
    volatile bool is_running;
} app_common;


static ifx_Avian_Metrics_t default_metrics = {0};

/*
==============================================================================
   5. LOCAL FUNCTION PROTOTYPES
==============================================================================
*/

#if defined __WIN32__ || defined _WIN32 || defined _Windows
#if !defined S_ISDIR
#define S_ISDIR(m) (((m)&_S_IFDIR) == _S_IFDIR)
#endif
#endif

static bool is_directory(const char* path)
{
    struct stat s = {0};
    stat(path, &s);
    return S_ISDIR(s.st_mode);
}

/**
 * @brief Return new string which is s1 + s2
 *
 * The caller is responsible for freeing the returned memory. On error the
 * function returns NULL. If s2 is NULL a copy of s1 is returned.
 */
char* str_append(const char* s1, const char* s2)
{
    const size_t len_s1 = strlen(s1);
    const size_t len_s2 = s2 ? strlen(s2) : 0;

    char* p = calloc(len_s1 + len_s2 + 1, 1);
    if (!p)
        return NULL;

    strcat(p, s1);
    if (s2)
        strcat(p, s2);

    return p;
}


/*
==============================================================================
   6. LOCAL FUNCTIONS
==============================================================================
*/
#ifdef _WIN32
#include <conio.h>

int app_kbhit(void)
{
    return _kbhit();
}

#else
#include <fcntl.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

int app_kbhit(void)
{
    struct termios oldt;
    struct termios newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF)
    {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}
#endif

void app_verbose(const char* message, ...)
{
    va_list args;

    va_start(args, message);
    if (app_common.verbose)
    {
        vfprintf(app_common.file_results, message, args);
    }
    va_end(args);
}

void app_print(const char* fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vfprintf(app_common.file_results, fmt, args);
    va_end(args);
}

void app_printtime(void)
{
    if (app_common.time_handle)
    {
        fprintf(app_common.file_results, "\"%s\"", ifx_time_get_cstr(app_common.time_handle));
    }
}

void signal_handler(int sig)
{
    if (sig == SIGINT)
        app_common.is_running = false;
}

static void printf_frame_to_file_r(FILE* f,
                                   ifx_Cube_R_t* frame)
{
    if (!frame)
        return;

    for (uint32_t chirp = 0; chirp < IFX_CUBE_ROWS(frame); chirp++)
    {
        for (uint32_t sample = 0; sample < IFX_CUBE_COLS(frame); sample++)
        {
            fprintf(f, "%4d,", sample);
            for (uint32_t virtual_antenna = 0; virtual_antenna < IFX_CUBE_SLICES(frame); virtual_antenna++)
            {
                ifx_Float_t value = IFX_CUBE_AT(frame, chirp, sample, virtual_antenna);
                fprintf(f, "%.6f,", value);
            }
            fprintf(f, "\n");
        }
    }
}

void error_callback(const char* filename, const char* functionname, int line, ifx_Error_t error)
{
    // Ignore end of file errors
    if (error == IFX_ERROR_END_OF_FILE)
        return;

    fprintf(stderr, "File:     | %s\n", filename);
    fprintf(stderr, "Function: | %s\n", functionname);
    fprintf(stderr, "Line:     | %d\n", line);
    fprintf(stderr, "Reason:   | %s\n", ifx_error_to_string(error));
    fprintf(stderr, "Errorcode:| 0x%x\n", error);
}

int app_start(int argc, char** argv, app_t* application, void* app_context)
{
    char* record_file_path = NULL;
    char* data_file_path = NULL;
    char* config_file_path = NULL;
    char* result_file_path = NULL;
    char* device_port_name = NULL;
    char* device_uuid = NULL;
    char* app_name = NULL;
    char* epilog = NULL;

    bool buffer = false;
    struct argparse argparse;

    uint32_t time_limit = 0;
    uint32_t frame_limit = 0;
    int record_format = RECORD_FORMAT_DEFAULT;
    bool is_daq_recording = false;
    uint32_t frame_count;

    // initialize
    FILE* file_record = NULL;
    FILE* file_data = NULL;
    ifx_json_t* json = NULL;

    ifx_Avian_Device_t* device_handle = NULL;
    ifx_Cube_R_t* frame = NULL;
    char* app_usage = NULL;
    char* self = argv[0];

    // this is the value we return to main if something goes wrong
    int exitcode = EXIT_FAILURE;

    //---------------- Usage Description String derivation -----------------------
    app_name = extract_filename_from_path(self);
    if (app_name == NULL)
    {
        fprintf(stderr, "Could not extract app name for description string \n");
        goto cleanup;
    }

    app_usage = calloc((strlen(app_name) + 12), sizeof(char));
    if (app_usage == NULL)
    {
        fprintf(stderr, "Could not allocate memory for app description string \n");
        goto cleanup;
    }

    sprintf(app_usage, "%s [OPTIONS]", app_name);
    const char* const usage_str[] = {app_usage, NULL};  // to be used in argparse


    // -------------------------------------------------------------------------------
    // -------------------------  Initialization  ------------------------------------
    // -------------------------------------------------------------------------------
    app_common.file_results = stdout;  // by default print outputs to console

    app_common.time_handle = NULL;
    app_common.is_running = true;

    // parse commandline
    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_GROUP("Options"),
        OPT_STRING('d', "data", &data_file_path, "data filename: recorded data (either SDK txt file format or daqlib file format)", NULL, 0, 0),
        OPT_STRING('c', "config", &config_file_path, "configuration filename: radar configuration to be used", NULL, 0, 0),
        OPT_STRING('r', "record", &record_file_path, "recording filename: records data to this file", NULL, 0, 0),
        OPT_INTEGER('R', "format", &record_format, "recording format default:0, antenna_table:1", NULL, 0, 0),
        OPT_STRING('o', "output", &result_file_path, "results filename: switches results display from stdout to file", NULL, 0, 0),
        OPT_STRING('p', "port", &device_port_name, "device port: attempt to connect to device on specified port", NULL, 0, 0),
        OPT_STRING('u', "uuid", &device_uuid, "device uuid: attempt to connect to device using specified uuid", NULL, 0, 0),
        OPT_BOOLEAN('b', "buffer", &buffer, "buffer output to stdout and stderr", NULL, 0, 0),
        OPT_BOOLEAN('v', "verbose", &app_common.verbose, "print detailed app output information", NULL, 0, 0),
        OPT_INTEGER('t', "time", &time_limit, "time in seconds to run", NULL, 0, 0),
        OPT_INTEGER('f', "frames", &frame_limit, "number of frames to run", NULL, 0, 0),
        OPT_END(),
    };

    const char* common_epilog = "\n"
                                "Recordings:\n"
                                "    If the argument of -d points to a file, the file is opened and the\n"
                                "    content of the file is assumed to be the txt based file format (the\n"
                                "    same file format that is written by this app when recording with -r).\n"
                                "    The txt based file format does not contain any information about\n"
                                "    the radar configuration. For a proper interpretation of the recording\n"
                                "    the matching device configuration must be passed using the option -c.\n"
                                "\n"
                                "    If the argument of -d points to a directory, it is assumed that the\n"
                                "    directory corresponds to a daqlib recording. daqlib recordings contain\n"
                                "    the matching device configuration. If a daqlib recording is opened and\n"
                                "    a configuration file is given as well using the parameter -c, the\n"
                                "    device configuration in the JSON configuration passed by -c is ignored,\n"
                                "    however, algorithm specific configurations are not ignored.\n"
                                "\n"
                                "    For more information on the current state of supported recordings\n"
                                "    please read the changelog of the Radar SDK documentation\n";

    epilog = str_append(common_epilog, application->app_epilog);
    if (!epilog)
    {
        fprintf(stderr, "Cannot allocate memory\n");
        goto cleanup;
    }

    argparse_init(&argparse, options, usage_str, 0);
    argparse_describe(&argparse, application->app_description, epilog);
    int not_arg = argparse_parse(&argparse, argc, argv);
    if (not_arg != 0)
    {
        fprintf(stderr, "Wrong arguments format\n");
        argparse_usage(&argparse);
        goto cleanup;
    }

    app_print(
        "Radar SDK Version: %s\n",
        ifx_sdk_get_version_string_full());

    // disable buffering unless --buffer was given
    if (!buffer)
    {
        disable_buffering(stdout);
        disable_buffering(stderr);
    }

    if (device_port_name && device_uuid)
    {
        fprintf(stderr, "uuid and portname are mutually exclusive!\n");
        goto cleanup;
    }

    ifx_error_set_callback(error_callback);

    if (ifx_time_create(&app_common.time_handle))
    {
        fprintf(stderr, "Failed creating time handle!\n");
        goto cleanup;
    }

    // --------------------------------------------------------------------------
    // -------------------------  app specific init -----------------------------
    // --------------------------------------------------------------------------
    if (application->app_init(app_context) != IFX_OK)
        goto cleanup;

    //------------------------ Check File options ------------------------------
    if (record_file_path)
    {
        file_record = fopen(record_file_path, "w");
        if (file_record == NULL)
        {
            fprintf(stderr, "Could not open file %s for writing", record_file_path);
            goto cleanup;
        }
    }

    if (result_file_path)
    {
        app_common.file_results = fopen(result_file_path, "w");
        if (app_common.file_results == NULL)
        {
            fprintf(stderr, "Could not open file %s for writing", result_file_path);
            goto cleanup;
        }
    }


    // --------------------------------------------------------------------------
    // ---------------------  Initialize Device ---------------------------------
    // --------------------------------------------------------------------------

    if (data_file_path == NULL)
    {
        if (device_uuid)
            device_handle = ifx_avian_create_by_uuid(device_uuid);
        else
            device_handle = ifx_avian_create_by_port(device_port_name);
        is_daq_recording = false;
    }
    else if (is_directory(data_file_path))
    {
        fprintf(stderr, "Opening recordings is not supported in legacy API\n");
        goto cleanup;
        //        device_handle = ifx_avian_playback_create(data_file_path, false);
        //        is_daq_recording = true;
        //        if (!device_handle)
        //        {
        //            fprintf(stderr, "Could not open %s\n", data_file_path);
        //            goto cleanup;
        //        }
    }
    else
    {
        file_data = fopen(data_file_path, "r");
        if (file_data == NULL)
        {
            fprintf(stderr, "Could not open file %s for reading", data_file_path);
            goto cleanup;
        }

        device_handle = ifx_avian_create_dummy(IFX_AVIAN_BGT60TR13C);
        is_daq_recording = false;
    }

    if (ifx_error_get() != IFX_OK)
    {
        fprintf(stderr, "Failed to open Device. (%x)\n", ifx_error_get());
        goto cleanup;
    }

    // --------------------------------------------------------------------------
    // ---------------------  Initialize JSON -----------------------------------
    // --------------------------------------------------------------------------

    json = ifx_json_create();
    if (!json)
    {
        fprintf(stderr, "Cannot create JSON structure");
        goto cleanup;
    }

    ifx_Avian_Config_t device_config = {0};
    if (is_daq_recording)
    {
        ifx_avian_get_config(device_handle, &device_config);
    }

    if (config_file_path)
    {
        /* read configuration from json file */
        bool ret = ifx_json_load_from_file(json, config_file_path);
        if (!ret)
        {
            fprintf(stderr, "Error parsing configuration file %s: %s", config_file_path, ifx_json_get_error(json));
            goto cleanup;
        }

        // If we have opened a daqlib recording the configuration was already correctly set. The loaded json configuration
        // file is then only required for algorithm configurations.
        if (!is_daq_recording)
        {
            if (ifx_json_has_config_single_shape(json))
            {
                ret = ifx_json_get_device_config_single_shape(json, &device_config);
                if (!ret)
                {
                    fprintf(stderr, "Error parsing fmcw_single_shape configuration: %s", ifx_json_get_error(json));
                    goto cleanup;
                }
            }
            else if (ifx_json_has_config_scene(json))
            {
                ifx_Avian_Metrics_t scene_config;
                ret = ifx_json_get_device_config_scene(json, &scene_config);
                if (!ret)
                {
                    fprintf(stderr, "Error parsing fmcw_scene configuration: %s", ifx_json_get_error(json));
                    goto cleanup;
                }

                // round number of samples and chirps to next power of 2 ("true")
                // this is required for backwards-compatibility with json files
                ifx_avian_metrics_to_config(device_handle, &scene_config, true, &device_config);
                if (ifx_error_get() != IFX_OK)
                {
                    fprintf(stderr, "Error converting scene to device configuration");
                    goto cleanup;
                }
            }
        }
    }
    else if (!is_daq_recording)
    {
        /* if it is a daqkit recording we already have read the device configuration */

        if (application->default_config)
        {
            device_config = *application->default_config;

            /* and save it in the json structure*/
            ifx_json_set_device_config_single_shape(json, &device_config);
        }
        else
        {
            /* use default configuration
             * round number of samples and chirps to next power of 2 ("true") to ensure
             * the same behavior as in previous SDK versions.
             */
            const ifx_Radar_Sensor_t sensor_type = ifx_avian_get_sensor_type(device_handle);
            ifx_Avian_Device_t* dummy = ifx_avian_create_dummy(sensor_type);
            ifx_avian_get_config(dummy, &device_config);

            if (application->default_metrics != NULL)
            {
                ifx_avian_metrics_to_config(device_handle, application->default_metrics, true, &device_config);
            }
            else
            {
                ifx_avian_metrics_from_config(dummy, &device_config, &default_metrics);
                application->default_metrics = &default_metrics;
            }

            /* and save it in the json structure*/
            ifx_json_set_device_config_single_shape(json, &device_config);
        }
    }

    // --------------------------------------------------------------------------
    // ---------------------  app specific json config --------------------------
    // --------------------------------------------------------------------------

    {
        ifx_Error_t ret = application->app_config(app_context, device_handle, json, &device_config);
        if (ret != IFX_OK)
        {
            fprintf(stderr, "Not able to config given app: %s\n", ifx_error_to_string(ret));
            goto cleanup;
        }
    }

    // --------------------------------------------------------------------------
    // ---------------------  Write final json config  --------------------------
    // --------------------------------------------------------------------------

    // Write final configuration to file
    if (file_record)
    {
        // Prepare Config file write if record enabled
        const char* extension = "_config.json";
        char* record_config_file_path = calloc((strlen(record_file_path) + strlen(extension) + 1), sizeof(char));
        if (record_config_file_path == NULL)
        {
            fprintf(stderr, "Could not allocate memory for config filename\n");
            goto cleanup;
        }
        strcat(record_config_file_path, record_file_path);
        strtok(record_config_file_path, ".");
        strcat(record_config_file_path, extension);

        ifx_json_save_to_file(json, record_config_file_path);

        free(record_config_file_path);
    }

    // --------------------------------------------------------------------------
    // --------------------- Create device --------------------------------------
    // --------------------------------------------------------------------------

    if (data_file_path == NULL)
    {
        // we opened a real device, so print firmware info and set configuration
        const ifx_Firmware_Info_t* fw_info = ifx_avian_get_firmware_information(device_handle);

        app_verbose("Firmware Version: %d.%d.%d %s | %s\n",
                    fw_info->version_major,
                    fw_info->version_minor,
                    fw_info->version_build,
                    fw_info->description,
                    fw_info->extendedVersion);

        // set configuration
        ifx_avian_set_config(device_handle, &device_config);

        if (ifx_error_get() != IFX_OK)
        {
            fprintf(stderr, "Failed to initialize Device. (%x)\n", ifx_error_get());
            goto cleanup;
        }
    }


    // --------------------------------------------------------------------------
    // ---------------------  Frames init ---------------------------------------
    // --------------------------------------------------------------------------
    // get antenna count from configuration
    uint8_t rx_antenna_count = ifx_devconf_count_rx_antennas(&device_config);
    if (file_data)
    {
        frame = ifx_cube_create_r(rx_antenna_count,
                                  device_config.num_chirps_per_frame,
                                  device_config.num_samples_per_chirp);
    }

    // install signal handler
    signal(SIGINT, signal_handler);

    frame_count = 0;
    uint32_t num_virtual_antennas = (uint32_t)rx_antenna_count;

    while (app_common.is_running)
    {
        ifx_Error_t ret;

        if (file_data)
        {
            bool is_eof = false;
            for (uint32_t i = 0; i < num_virtual_antennas; i++)
            {
                ifx_Matrix_R_t antenna_data;
                ifx_cube_get_row_r(frame, i, &antenna_data);
                if ((is_eof = !get_matrix_from_file_r(file_data, &antenna_data)))
                    break;
            }
            if (is_eof)
                break;

            // increase frame count
            frame_count++;
        }
        else
        {
            frame = ifx_avian_get_next_frame(device_handle, frame);
            ret = ifx_error_get_and_clear();
            /* in case of a timeout we read to fast, so we should just try again */
            if (ret == IFX_ERROR_TIMEOUT)
                continue;

            /* FIFO overflow occurred */
            if (ret == IFX_ERROR_FIFO_OVERFLOW)
            {
                fprintf(stderr, "FIFO overflow\n");
                // In case of overflow this frame can be ignored (continue)
                // but when recording is enabled it is necessary not to lose frames
                if (file_record)
                {
                    fprintf(stderr, "Recording not valid. Abort!\n");
                    fprintf(file_record, "\nFIFO Overflow. Abort!\n");
                    goto cleanup;
                }
                continue;
            }
            else if (ret == IFX_ERROR_END_OF_FILE)
                break;
            else if (ret != IFX_OK)
            {
                fprintf(stderr, "Error getting next frame: %s (%d)\n", ifx_error_to_string(ret), ret);
                goto cleanup;
            }

            // increase frame count
            frame_count++;
        }
        if (file_record)
        {
            if (record_format == RECORD_FORMAT_ANTENNA_TABLE)
            {
                printf_frame_to_file_r(file_record, frame);
            }
            else
            {  // RECORD FORMAT_DEFAULT
                for (uint32_t i = 0; i < num_virtual_antennas; i++)
                {
                    ifx_Matrix_R_t antenna_data;
                    ifx_cube_get_row_r(frame, i, &antenna_data);
                    print_matrix_to_file_r(file_record, &antenna_data);
                }
            }
        }
        if (ifx_error_get() != IFX_OK)
            goto cleanup;
        // --------------------------------------------------------------------------
        // ---------------------  app specific functionality ------------------------
        // --------------------------------------------------------------------------
        app_print("{ \"elapsed_time\":\"%s\", \"frame_number\":%d",
                  ifx_time_get_cstr(app_common.time_handle), frame_count);
        if (application->app_process(app_context, frame) != IFX_OK)
        {
            app_print(" }\n");
            goto cleanup;
        }
        app_print(" }\n");

        // ---------------------  Exit conditions ----------------------------------
        if (frame_limit > 0 && frame_count == frame_limit)
        {
            app_common.is_running = false;
            printf("frame limit reached.\n");
        }

        if (time_limit > 0)
        {
            if (device_config.frame_repetition_time_s * frame_count >= time_limit)
            {
                app_common.is_running = false;
                printf("time limit reached.\n");
            }
        }
    }

    // everything successful
    exitcode = EXIT_SUCCESS;
cleanup:
    // --------------------------------------------------------------------------
    // --------------------------------  common ---------------------------------
    // --------------------------------------------------------------------------
    if (app_common.time_handle)
    {
        ifx_time_destroy(app_common.time_handle);
        app_common.time_handle = NULL;
    }

    ifx_cube_destroy_r(frame);

    if (device_handle)
    {
        if (data_file_path == NULL)
        {
            // Print this only if we opened a real device
            fprintf(stderr, "Closing Device\n");
        }
        ifx_avian_destroy(device_handle);
        device_handle = NULL;
    }
    fflush(stdout);

    ifx_json_destroy(json);
    free(app_usage);

    if (file_data)
        fclose(file_data);
    if (file_record)
        fclose(file_record);
    if (app_common.file_results)
        fclose(app_common.file_results);

    // --------------------------------------------------------------------------
    // --------------------------------  app specific ---------------------------
    // --------------------------------------------------------------------------
    application->app_cleanup(app_context);


    return exitcode;
}


//----------------------------------------------------------------------------


/*
==============================================================================
   7. EXPORTED FUNCTIONS
==============================================================================
*/


//----------------------------------------------------------------------------
