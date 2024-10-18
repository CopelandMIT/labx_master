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
 * @file DeviceLtr11Types.h
 *
 * @brief Header for DeviceLtr11Types
 */

#ifndef IFX_DEVICE_LTR11_TYPES_H
#define IFX_DEVICE_LTR11_TYPES_H

#include "ifxBase/Types.h"


/*
==============================================================================
   2. DEFINITIONS
==============================================================================
*/
#define IFX_LTR11_MAX_ALLOWED_NUM_SAMPLES 1024

/*
==============================================================================
   3. TYPES
==============================================================================
*/
typedef enum
{
    IFX_LTR11_SPI_PULSE_MODE = 0,     /**< SPI Pulse mode (Default mode)*/
    IFX_LTR11_SPI_CONTINUOUS_MODE = 1 /**< SPI Continuous wave mode */
} ifx_Ltr11_Mode_t;

typedef enum
{
    IFX_LTR11_PULSE_WIDTH_5us = 0,
    IFX_LTR11_PULSE_WIDTH_10us = 1,
    IFX_LTR11_PULSE_WIDTH_3us = 2,
    IFX_LTR11_PULSE_WIDTH_4us = 3
} ifx_Ltr11_PulseWidth_t;

typedef enum
{
    IFX_LTR11_PRT_250us = 0,
    IFX_LTR11_PRT_500us = 1,
    IFX_LTR11_PRT_1000us = 2,
    IFX_LTR11_PRT_2000us = 3
} ifx_Ltr11_PRT_t;

typedef enum
{
    IFX_LTR11_TX_POWER_LEVEL_MINUS_34dBm = 0,
    IFX_LTR11_TX_POWER_LEVEL_MINUS_31_5dBm = 1,
    IFX_LTR11_TX_POWER_LEVEL_MINUS_25dBm = 2,
    IFX_LTR11_TX_POWER_LEVEL_MINUS_18dBm = 3,
    IFX_LTR11_TX_POWER_LEVEL_MINUS_11dBm = 4,
    IFX_LTR11_TX_POWER_LEVEL_MINUS_5dBm = 5,
    IFX_LTR11_TX_POWER_LEVEL_0dBm = 6,
    IFX_LTR11_TX_POWER_LEVEL_4_5dBm = 7
} ifx_Ltr11_TxPowerLevel_t;

typedef enum
{
    IFX_LTR11_RX_IF_GAIN_10dB = 0,
    IFX_LTR11_RX_IF_GAIN_15dB = 1,
    IFX_LTR11_RX_IF_GAIN_20dB = 2,
    IFX_LTR11_RX_IF_GAIN_25dB = 3,
    IFX_LTR11_RX_IF_GAIN_30dB = 4,
    IFX_LTR11_RX_IF_GAIN_35dB = 5,
    IFX_LTR11_RX_IF_GAIN_40dB = 6,
    IFX_LTR11_RX_IF_GAIN_45dB = 7,
    IFX_LTR11_RX_IF_GAIN_50dB = 8
} ifx_Ltr11_RxIFGain_t;

typedef enum
{
    IFX_LTR11_HOLD_TIME_MIN = 0,
    IFX_LTR11_HOLD_TIME_512ms = 4,
    IFX_LTR11_HOLD_TIME_1s = 8,
    IFX_LTR11_HOLD_TIME_2s = 16,
    IFX_LTR11_HOLD_TIME_3s = 24,
    IFX_LTR11_HOLD_TIME_5s = 40,
    IFX_LTR11_HOLD_TIME_10s = 79,
    IFX_LTR11_HOLD_TIME_20s = 235,
    IFX_LTR11_HOLD_TIME_45s = 352,
    IFX_LTR11_HOLD_TIME_60s = 469,
    IFX_LTR11_HOLD_TIME_90s = 704,
    IFX_LTR11_HOLD_TIME_2min = 938,
    IFX_LTR11_HOLD_TIME_5min = 2345,
    IFX_LTR11_HOLD_TIME_10min = 4690,
    IFX_LTR11_HOLD_TIME_15min = 7035,
    IFX_LTR11_HOLD_TIME_30min = 14070
} ifx_Ltr11_Hold_Time_t;


typedef enum
{
    IFX_LTR11_APRT_FACTOR_4 = 0,
    IFX_LTR11_APRT_FACTOR_8 = 1,
    IFX_LTR11_APRT_FACTOR_16 = 2,
    IFX_LTR11_APRT_FACTOR_2 = 3,
    IFX_LTR11_APRT_FACTOR_1 = 4 /* aprt disabled */
} ifx_Ltr11_APRT_Factor_t;

typedef enum
{
    IFX_LTR11_INTERNAL_DETECTOR_THRESHOLD_66 = 66,
    IFX_LTR11_INTERNAL_DETECTOR_THRESHOLD_80 = 80,
    IFX_LTR11_INTERNAL_DETECTOR_THRESHOLD_90 = 90,
    IFX_LTR11_INTERNAL_DETECTOR_THRESHOLD_112 = 112,
    IFX_LTR11_INTERNAL_DETECTOR_THRESHOLD_136 = 136,
    IFX_LTR11_INTERNAL_DETECTOR_THRESHOLD_192 = 192,
    IFX_LTR11_INTERNAL_DETECTOR_THRESHOLD_248 = 248,
    IFX_LTR11_INTERNAL_DETECTOR_THRESHOLD_320 = 320,
    IFX_LTR11_INTERNAL_DETECTOR_THRESHOLD_384 = 384,
    IFX_LTR11_INTERNAL_DETECTOR_THRESHOLD_480 = 480,
    IFX_LTR11_INTERNAL_DETECTOR_THRESHOLD_640 = 640,
    IFX_LTR11_INTERNAL_DETECTOR_THRESHOLD_896 = 896,
    IFX_LTR11_INTERNAL_DETECTOR_THRESHOLD_1344 = 1344,
    IFX_LTR11_INTERNAL_DETECTOR_THRESHOLD_1920 = 1920,
    IFX_LTR11_INTERNAL_DETECTOR_THRESHOLD_2560 = 2560
} ifx_Ltr11_Internal_Detector_Threshold_t;

/**
 * @brief LTR11 Metadata that includes the digital detector output data, and information regarding the
 *        power operating mode of the chip (active mode or low power mode).
 */
typedef struct
{
    float avg_power; /**< Average power in W(Watts).*/
    bool active;     /**< Flag indicating the chip power mode: if true then active else low power mode*/
    bool motion;     /**< Motion given by the LTR11 digital detector. The target detection pin is active low.
                                 Hence, if motion = false, a target is detected else no target is detected.*/
    bool direction;  /**< Direction of motion given by the LTR11 digital detector. */
} ifx_Ltr11_Metadata_t;

/**
 * @brief LTR11 Configuration structure including the parameters exposed through the Fusion GUI.
 */
typedef struct
{
    ifx_Ltr11_Mode_t mode;                   /**< SPI Continuous wave mode or SPI Pulse mode expressed */
    uint64_t rf_frequency_Hz;                /**< Operational RF center frequency */
    uint16_t num_samples;                    /**< Number of samples */
    uint16_t internal_detector_threshold;    /**< Detection Threshold */
    ifx_Ltr11_PRT_t prt;                     /**< Pulse repetition time expressed as index / enum*/
    ifx_Ltr11_PulseWidth_t pulse_width;      /**< Pulse width expressed as index / enum  */
    ifx_Ltr11_TxPowerLevel_t tx_power_level; /**< Tx power level expressed as index / enum */
    ifx_Ltr11_RxIFGain_t rx_if_gain;         /**< Rx if gain expressed as index / enum */
    ifx_Ltr11_APRT_Factor_t aprt_factor;     /**< Adaptive prt factor expressed as index / enum */
    ifx_Ltr11_Hold_Time_t hold_time;         /**< Hold time expressed as index / enum */
    bool disable_internal_detector;          /**< Flag indicating when true that the internal detector needs to be disabled. */
} ifx_Ltr11_Config_t;


/**
 * @brief LTR11 Configuration Limits for the different parameters exposed within the Fusion GUI.
 */
typedef struct
{
    struct
    {
        uint64_t min;
        uint64_t max;
        uint64_t step;
    } rf_frequency_Hz;
    struct
    {
        uint16_t min;
        uint16_t max;
    } num_samples;
    struct
    {
        uint16_t min;
        uint16_t max;
    } internal_detector_threshold;
} ifx_Ltr11_Config_Limits_t;

#endif  // IFX_DEVICE_LTR11_TYPES_H
