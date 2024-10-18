/* ===========================================================================
** Copyright (C) 2023 Infineon Technologies AG
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
 * @file DeviceMimoseTypes.h
 *
 * \brief \copybrief gr_device_mimose_types
 *
 * For details refer to \ref gr_device_mimose_types
 */

#ifndef IFX_DEVICE_MIMOSE_TYPES_H
#define IFX_DEVICE_MIMOSE_TYPES_H

#include "ifxBase/Types.h"


#ifdef __cplusplus
extern "C"
{
#endif


/** @addtogroup gr_cat_device_mimose
 * @{
 */

/** @defgroup gr_device_mimose_types Mimose Types
 * @brief Header file for Mimose types.
 *
 * This includes enumerations that can be used for the components of the Mimose device
 * configuration structure \ref ifx_Mimose_Config_t.
 * @{
 */

/**
 * @brief Defines the channel type by the TX and RX configuration (expressed as a pair / combination of the two TX and RX).
 *
 * This enum defines the channel type to be used. The ATR22 has 2 directional Antenna pairs and the channel
 * defines the TX and RX configuration expressed as a pair/combination.
 */
typedef enum
{
    /** Transmission from Antenna 1 received at Antenna 1 */
    IFX_MIMOSE_CHANNEL_TX1_RX1,
    /** Transmission from Antenna 2 received at Antenna 2 */
    IFX_MIMOSE_CHANNEL_TX2_RX2,
    /** Transmission from Antenna 1 received at Antenna 2 */
    IFX_MIMOSE_CHANNEL_TX1_RX2,
    /** Transmission from Antenna 2 received at Antenna 1 */
    IFX_MIMOSE_CHANNEL_TX2_RX1
} ifx_Mimose_Channel_t;

typedef enum
{
    /** This option is disabled */
    IFX_MIMOSE_ABB_GAIN_AUTOMATIC = 0xFF,
    IFX_MIMOSE_ABB_GAIN_3 = 0,
    IFX_MIMOSE_ABB_GAIN_6,
    IFX_MIMOSE_ABB_GAIN_12,
    IFX_MIMOSE_ABB_GAIN_24,
    IFX_MIMOSE_ABB_GAIN_48,
    IFX_MIMOSE_ABB_GAIN_96,
    IFX_MIMOSE_ABB_GAIN_192,
    IFX_MIMOSE_ABB_GAIN_384
} ifx_Mimose_ABB_type_t;

typedef enum
{
    IFX_MIMOSE_AOC_OFFSET_VALUE_IS_APPLIED = 0,
    IFX_MIMOSE_AOC_OFFSET_VALUE_IS_ONLY_TRACKED = 1,
    IFX_MIMOSE_AOC_OFFSET_VALUE_IS_FULLY_HANDLED_BY_THE_FSM_2 = 2,
    IFX_MIMOSE_AOC_OFFSET_VALUE_IS_FULLY_HANDLED_BY_THE_FSM_3 = 3
} ifx_Mimose_AOC_Mode_t;

typedef struct
{
    ifx_Mimose_Channel_t channel;
    uint8_t tx_power_level;               // a value 0 to 63, but take limits from limits structure!!!
    ifx_Mimose_ABB_type_t abb_gain_type;  // Analog BaseBand gain type
    ifx_Mimose_AOC_Mode_t aoc_mode;       // Automatic Offset Compensation mode
} ifx_Mimose_Pulse_Config_t;

typedef struct
{
    float frame_repetition_time_s;
    float pulse_repetition_time_s;  // PRT is cumulative for all the 4 pulse configs (independent of the specific pulse config)
    bool selected_pulse_configs[4];
    uint16_t num_samples;           // num_of_sample equivalent to num_samples, could be larger then 256! we just only offer 256 for now!
} ifx_Mimose_Frame_Config_t;

typedef struct
{
    uint32_t reference_clock_Hz;
    uint32_t system_clock_Hz;
    bool rc_clock_enabled;
    uint32_t hf_on_time_usec;
    uint16_t system_clock_divider;
    bool system_clock_div_flex;  // allow automatic change of SYS_CLK divider
    bool sys_clk_to_i2c;         // Set SYS_CLK divider to I2C_CLK divider
} ifx_Mimose_Clock_Config_t;

typedef enum
{
    IFX_MIMOSE_RF_BAND_ISM_SRR_24GHZ_100MHz,  // Short range radar, 24.150GHz ... 24.250GHz, BW 100MHz for STO
    IFX_MIMOSE_RF_BAND_ISM_SRR_24GHZ_200MHz   // 24.050GHZ ... 24.250GHz, BW 200MHz
} ifx_Mimose_RF_Band_t;

typedef enum
{
    IFX_MIMOSE_AFC_REPEAT_COUNT_1,
    IFX_MIMOSE_AFC_REPEAT_COUNT_2,
    IFX_MIMOSE_AFC_REPEAT_COUNT_4,
    IFX_MIMOSE_AFC_REPEAT_COUNT_8
} ifx_Mimose_AFC_Repeat_Count_t;

typedef struct
{
    ifx_Mimose_RF_Band_t band;
    uint64_t rf_center_frequency_Hz;
    uint16_t afc_duration_ct;                        // Duration is expressed in clock ticks
    uint16_t afc_threshold_course;                   // Threshold boundary for course steps
    uint16_t afc_threshold_fine;                     // Threshold boundary for fine steps
    uint8_t afc_period;                              // in units of frames past  (range from 0 to 255)
    ifx_Mimose_AFC_Repeat_Count_t afc_repeat_count;  // number of afc counts repeated
} ifx_Mimose_AFC_Control_t;

typedef struct
{
    ifx_Mimose_Pulse_Config_t pulse_config[4];
    ifx_Mimose_Frame_Config_t frame_config[2];
    ifx_Mimose_AFC_Control_t afc_config;
    ifx_Mimose_Clock_Config_t clock_config;
} ifx_Mimose_Config_t;

typedef struct
{
    float rf_frequency_Hz;
    float temperature;  // MMIC temperature in Celsius
} ifx_Mimose_Sensor_t;

/** Limits and defaults for params exposed within Fusion GUI */
typedef struct
{
    uint8_t min_tx_power_level;
    uint8_t max_tx_power_level;

    uint16_t min_num_samples;
    uint16_t max_num_samples;

    float min_pulse_repetition_time_s;
    float max_pulse_repetition_time_s;

    float min_frame_repetition_time_s;
    float max_frame_repetition_time_s;

    uint64_t min_rf_center_frequency_Hz;
    uint64_t max_rf_center_frequency_Hz;

    uint16_t min_afc_duration_ct;
    uint16_t max_afc_duration_ct;

    uint16_t min_afc_threshold_course;
    uint16_t max_afc_threshold_course;

    uint16_t min_afc_threshold_fine;
    uint16_t max_afc_threshold_fine;

} ifx_Mimose_Config_Limits_t;

/* @brief The Metadata structure for pulse specific gain and aoc offset data.
 *
 * Metadata consists of:
 * Abb gain data (pulse related, max 4 values)
 * Aoc offset data (pulse related, max 4 values arranged as IQ offset pairs - 0 index is I aoc offset, 1 index is Q aoc offset)
 */
typedef struct
{
    uint8_t abb_gains[4];      /**< Gain data, one gain per each pulse */
    int16_t aoc_offsets[4][2]; /**< Aoc offset data, one offset pair per each pulse */
} ifx_Mimose_Metadata_t;

/* forward declarations */

typedef struct DeviceMimoseBase ifx_Mimose_Device_t;

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* IFX_DEVICE_MIMOSE_TYPES_H */
