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

#include "frame_configuration.h"
#include "ifxBase/Complex.h"
#include "ifxBase/Error.h"
#include "ifxMimose/DeviceMimose.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Do something with the frame: Just compute the absolute value of the sum of
// each row and print the result to stdout.
void process_frame(const ifx_Cube_C_t* cube, uint32_t frame_idx)
{
    ifx_Matrix_C_t matrix;
    ifx_cube_get_row_c(cube, 0, &matrix);

    for (uint32_t pulse_idx = 0; pulse_idx < IFX_MAT_ROWS(&matrix); ++pulse_idx)
    {
        printf("Frame[%d]: Sum of samples for pulse[%d] = ", frame_idx, pulse_idx);

        ifx_Vector_C_t view;
        ifx_mat_get_rowview_c(&matrix, pulse_idx, &view);

        printf("%g\n", ifx_complex_abs(ifx_vec_sum_c(&view)));
    }
    printf("\n");
}

void usage(const char* program)
{
    printf("Usage: %s frame_config_file (optional)\n", program);
}

void printConfiguration(const ifx_Mimose_Config_t* configuration)
{
    printf("Frame[0] configuration:\n");

    printf("num_samples: %" PRIu32 "\n", configuration->frame_config[0].num_samples);
    printf("pulse_repetition_time_s: %f\n", configuration->frame_config[0].pulse_repetition_time_s);
    printf("frame_repetition_time_s: %f\n", configuration->frame_config[0].frame_repetition_time_s);

    printf("\n\n");
    for (int i = 0; i < 4; i++)
    {
        printf("Pulse [%d] configuration:\n", i);
        printf("abb_gain_type: %" PRIu32 "\n", configuration->pulse_config[i].abb_gain_type);
        printf("aoc_mode: %" PRIu32 "\n", configuration->pulse_config[i].aoc_mode);
        printf("channel: %" PRIu32 "\n", configuration->pulse_config[i].channel);
        printf("tx_power_level: %" PRIu32 "\n", configuration->pulse_config[i].tx_power_level);
        printf("\n");
    }

    printf("\n\n");

    printf("AFC configuration:\n");
    printf("afc_period: %" PRIu32 "\n", configuration->afc_config.afc_period);
    printf("afc_duration_ct: %" PRIu32 "\n", configuration->afc_config.afc_duration_ct);
    printf("afc_repeat_count: %" PRIu32 "\n", configuration->afc_config.afc_repeat_count);
    printf("afc_threshold_course: %" PRIu32 "\n", configuration->afc_config.afc_threshold_course);
    printf("afc_threshold_fine: %" PRIu32 "\n", configuration->afc_config.afc_threshold_fine);
    printf("afc_period: %" PRIu64 "\n", configuration->afc_config.rf_center_frequency_Hz);

    printf("\n\n");
    printf("Clock configuration:\n");
    printf("reference_clock_Hz: %" PRIu32 "\n", configuration->clock_config.reference_clock_Hz);
    printf("system_clock_Hz: %" PRIu32 "\n", configuration->clock_config.system_clock_Hz);
    printf("rc_clock_enabled: %" PRIu16 "\n", (uint16_t)configuration->clock_config.rc_clock_enabled);
    printf("hf_on_time_usec: %" PRIu32 "\n", configuration->clock_config.hf_on_time_usec);
    printf("system_clock_divider: %" PRIu16 "\n", configuration->clock_config.system_clock_divider);
    printf("system_clock_div_flex: %" PRIu16 "\n", (uint16_t)configuration->clock_config.system_clock_div_flex);
    printf("sys_clk_to_i2c: %" PRIu16 "\n", (uint16_t)configuration->clock_config.sys_clk_to_i2c);

    printf("\n");
}


int main(int argc, char** argv)
{
    int ret = EXIT_FAILURE;

    ifx_Cube_C_t* frame = NULL;
    ifx_Mimose_Device_t* device_handle = NULL;

    device_handle = ifx_mimose_create();
    if (ifx_error_get() != IFX_OK)
    {
        fprintf(stderr, "Failed to open device: %s\n", ifx_error_to_string(ifx_error_get()));
        goto fail;
    }

    const ifx_Radar_Sensor_Info_t* device_info = ifx_mimose_get_sensor_information(device_handle);
    printf("Device: %s\n", device_info->description);

    ifx_Mimose_Config_t config;
    ifx_mimose_get_config_defaults(device_handle, &config);
    printConfiguration(&config);

    // change device configuration
    config.afc_config.rf_center_frequency_Hz = 24200000000;
    config.frame_config[0].pulse_repetition_time_s = 1000e-6f;
    config.frame_config[0].frame_repetition_time_s = 0.250f;
    config.pulse_config[0].tx_power_level = 63;
    config.pulse_config[1].tx_power_level = 63;
    config.pulse_config[2].tx_power_level = 50;
    config.pulse_config[3].tx_power_level = 50;

    ifx_Mimose_Config_Limits_t limits = {0};
    ifx_mimose_get_default_limits(device_handle, &limits);

    if (ifx_error_get() != IFX_OK)
    {
        fprintf(stderr, "Failed to get limits:  %s\n", ifx_error_to_string(ifx_error_get()));
        goto fail;
    }
    // ifx_mimose_update_rc_lut(device_handle);

    ifx_mimose_set_config(device_handle, &config);

    if (ifx_error_get() != IFX_OK)
    {
        fprintf(stderr, "Failed to set new device config:  %s\n", ifx_error_to_string(ifx_error_get()));
        goto fail;
    }

    ifx_mimose_start_acquisition(device_handle);

    if (ifx_error_get() != IFX_OK)
    {
        fprintf(stderr, "Failed to start acquisition:  %s\n", ifx_error_to_string(ifx_error_get()));
        goto fail;
    }

    // fetch data from device
    for (int i = 0; i < 20; i++)
    {
        frame = ifx_mimose_get_next_frame(device_handle, frame, NULL);
        if (ifx_error_get() != IFX_OK)
        {
            fprintf(stderr, "Failed to fetch frame: %s\n", ifx_error_to_string(ifx_error_get()));
            goto fail;
        }

        process_frame(frame, i);
    }
    ifx_mimose_stop_acquisition(device_handle);

    ret = EXIT_SUCCESS;

fail:
    ifx_cube_destroy_c(frame);
    ifx_mimose_destroy(device_handle);

    return ret;
}
