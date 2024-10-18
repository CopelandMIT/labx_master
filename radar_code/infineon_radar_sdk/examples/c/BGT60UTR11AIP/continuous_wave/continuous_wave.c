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
 * @file    continous_wave.c
 *
 * @brief   Continuous wave signal capturing example.
 *
 * This example illustrates how to fetch data from a radar sensor
 * configured in Continuous Wave mode.
 *
 * This programm accepts three paramters:
 * 1. time of transmission in seconds (default = 0) means has no time limit
 * 2. the rf_frequency in Hz (default = 60000000000) 60 Ghz
 * 3. tx transmit power (deflaut = 0)
 *
 * If only tx transmit is to be set, the other parameters must also be set. The same applies to rf_frequency.
 * For the default values see above
 */

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "common.h"
#include "continuous_wave_defaults.h"
#include "ifxBase/Base.h"
#include "ifxCw/DeviceCw.h"
/*
==============================================================================
   2. LOCAL DEFINITIONS
==============================================================================
*/

#define NUM_FETCHED_FRAMES 5 /**< Number of frames to fetch */

/*
==============================================================================
   3. LOCAL TYPES
==============================================================================
*/

/*
==============================================================================
   4. DATA
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
 * @brief Processing function
 *
 * This function will separate different antenna signals
 * and prints all samples per RX antenna to console.
 *
 * @param frame             frame handle to fetch
 * @param frame_count       count of frames to fetch and process
 */
void process_frame(ifx_Matrix_R_t* frame, uint32_t frame_count)
{
    ifx_Vector_R_t samples = {0};

    printf("\n========== Frame: %d ===========\n", frame_count);

    for (uint32_t ant = 0; ant < IFX_MAT_ROWS(frame); ant++)
    {
        // Fetch samples for single antenna from the antenna matrix
        ifx_mat_get_rowview_r(frame, ant, &samples);

        printf("\n========== Rx Antenna: %d ===========\n", ant);
        for (uint32_t i = 0; i < IFX_VEC_LEN(&samples); i++)
        {
            printf("%.4f ", IFX_VEC_AT(&samples, i));
        }
        printf("\n");
    }
}

/*
==============================================================================
   7. MAIN METHOD
==============================================================================
 */

int main(int argc, char** argv)
{
    ifx_Cw_Baseband_Config_t baseband_config;
    ifx_Cw_Adc_Config_t adc_config;
    ifx_Cw_Test_Signal_Generator_Config_t test_signal_config;
    time_t duration = 0;

    printf("Radar SDK Version: %s\n", ifx_sdk_get_version_string_full());
    /* Open the device */
    ifx_Device_Cw_t* cw_device = ifx_cw_create();

    ifx_Error_t error;
    if ((error = ifx_error_get()) != IFX_OK)
    {
        fprintf(stderr, "Failed to open device: %s\n", ifx_error_to_string(error));
        return EXIT_FAILURE;
    }

    /* Baseband configurations */
    printf("\n************* Parameters passed  \n");
    baseband_config = *ifx_cw_get_baseband_config(cw_device);
    switch (argc)
    {
        case 1:
            printf("\nNo Extra Command Line Argument Passed Other Than Program Name");
            break;
        case 2:
            printf("\n Time for transmission in seconds:%s\n", argv[1]);
            duration = (time_t)atoll(argv[1]);
            break;
        case 3:
            printf("\n Time for transmission in seconds:%s\n", argv[1]);
            duration = (time_t)atoll(argv[1]);
            printf("\nFrequency in Hz: %s", argv[2]);  //! TODO check value limits
            baseband_config.start_frequency_Hz = atof(argv[2]);
            break;
        case 4:
            printf("\n Time for transmission in seconds:%s\n", argv[1]);
            duration = (time_t)atoll(argv[1]);
            printf("\n Frequency in Hz: %s", argv[2]);  //! TODO check value limits
            baseband_config.start_frequency_Hz = atof(argv[2]);
            printf("\n TX Powerlevel: %s\n", argv[3]);  //! TODO check value limits
            baseband_config.tx_power_level = atoi(argv[3]);
            break;
        default:
            break;
    }

    baseband_config.lp_cutoff_Hz = 1000000;  // 1MHz, only for D & E variants
    baseband_config.if_gain_dB = 23;
    baseband_config.hp_cutoff_Hz = 80000;    // 80kHz is for D and E variants
    baseband_config.sample_rate_Hz = IFX_CW_SAMPLING_FREQ_HZ;
    baseband_config.num_samples = IFX_CW_NUM_SAMPLES;
    baseband_config.rx_mask |= 1 << (IFX_CW_RX_ANTENNAS - 1);
    baseband_config.tx_mask |= 1 << (1 - 1);  // Enable Tx1 antenna

    ifx_cw_set_baseband_config(cw_device, &baseband_config);

    printf("\n************* Baseband configurations  \n");
    printf(" LP Cutoff (in Hz): %" PRIu32 "\n", baseband_config.lp_cutoff_Hz);
    printf(" HP Cutoff (in Hz): %" PRIu32 "\n", baseband_config.hp_cutoff_Hz);
    printf(" IF Gain Enum: %d\n", baseband_config.if_gain_dB);
    printf(" Sample Rate (Hz): %5.3e\n", baseband_config.sample_rate_Hz);
    printf(" Number of Samples : %" PRIu32 "\n", baseband_config.num_samples);
    printf(" Start Frequency (in Hz): %5.3e"
           "\n",
           baseband_config.start_frequency_Hz);
    printf(" TX Powerlevel: %d \n", baseband_config.tx_power_level);


    /* ADC configurations */
    adc_config.sample_and_hold_time_ns = 50;   // 50ns
    adc_config.oversampling_factor = 2;        // IFX_ADC_OVERSAMPLING_2x
    adc_config.additional_subconversions = 3;  // IFX_ADC_3_SUBCONVERSIONS
    ifx_cw_set_adc_config(cw_device, &adc_config);

    const ifx_Cw_Adc_Config_t* curr_adc_config = ifx_cw_get_adc_config(cw_device);
    const ifx_Radar_Sensor_Info_t* sensor_info = ifx_cw_get_sensor_information(cw_device);

    printf("\n************* ADC configurations \n");
    printf(" Lower Limit of Sample Rate (Hz): %5.3e\n", sensor_info->min_adc_sampling_rate);
    printf(" Upper Limit of Sample Rate (Hz): %5.3e\n", sensor_info->max_adc_sampling_rate);
    printf(" ADC resolution (bits) %d\n", sensor_info->adc_resolution_bits);
    printf(" Over sampling Enum: %d\n", curr_adc_config->oversampling_factor);
    printf(" Sample Time Enum: %d\n", curr_adc_config->sample_and_hold_time_ns);
    printf(" Tracking Enum: %d\n", curr_adc_config->additional_subconversions);

    /* Test signal configurations */
    test_signal_config = *ifx_cw_get_test_signal_generator_config(cw_device);
    test_signal_config.frequency_Hz = IFX_CW_TEST_SIGNAL_FREQ_HZ;
    ifx_cw_set_test_signal_generator_config(cw_device, &test_signal_config);

    printf("\n************* Test Signal configurations \n");
    printf(" Frequency (Hz): %10.1e\n", test_signal_config.frequency_Hz);
    printf(" Test Mode Enum value: %d\n", test_signal_config.mode);


    /* Overwrite CW defaults */
    ifx_cw_start_signal(cw_device);
    ifx_Matrix_R_t* frame = NULL;
    float temperature = 0;

    time_t start_time;
    time(&start_time);
    if (duration == 0)
    {
        duration = LONG_MAX;
    }

    while (((int)difftime(time(NULL), start_time)) < duration)
    {
        temperature = ifx_cw_measure_temperature(cw_device);
        ifx_Error_t ret = ifx_error_get_and_clear();
        if (ret != IFX_OK)
        {
            fprintf(stderr, "%s (%d)\n", ifx_error_to_string(ret), ret);
            break;
        }
        printf("    Temperature: %.4f\n", temperature);
        printf("    Press 'q' to exit programm\n");

        if (app_kbhit())
        {
            if ('q' == getchar())
            {
                ifx_cw_destroy(cw_device);  // CW controller will be destroyed within device handle
                ifx_mat_destroy_r(frame);   // explicitly destroy CW frame, otherwise a memory leak occurs
                cw_device = NULL;
                exit(0);
            }
        }
    }

    ifx_cw_destroy(cw_device);  // CW controller will be destroyed within device handle
    ifx_mat_destroy_r(frame);   // explicitly destroy CW frame, otherwise a memory leak occurs
    return EXIT_SUCCESS;
}
