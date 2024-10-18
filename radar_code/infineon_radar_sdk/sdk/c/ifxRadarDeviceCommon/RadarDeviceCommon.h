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
 * @file RadarDeviceCommon.h
 *
 * @brief Header for ifxRadarDeviceCommon
 *
 * Includes all ifxRadarDeviceCommon headers.
 *
 * \defgroup gr_cat_Radar_Device_Common        Radar Device Common (ifxDeviceCommon)
 */

#ifndef IFX_RADAR_DEVICE_COMMON_H
#define IFX_RADAR_DEVICE_COMMON_H

#include <ifxBase/Types.h>

#define UUID_STRING_SIZE 37  // strlen("xxxxxxxx-xxxx-Mxxx-Nxxx-xxxxxxxxxxxx") + sizeof('\0')

#ifdef __cplusplus
extern "C"
{
#endif

/** @addtogroup gr_cat_Radar_Device_Common
 * @{
 */

/* When adding/changing values also make sure to update the function
 * board_type_to_string in rdk::RadarDeviceCommon.
 */
typedef enum
{
    IFX_BOARD_UNKNOWN = 0,         /**< Unknown board */
    IFX_BOARD_BASEBOARD_AURIX = 1, /**< Aurix board */
    IFX_BOARD_BASEBOARD_MCU7 = 2,  /**< Radar Baseboard MCU7 */
    IFX_BOARD_BASEBOARD_V9 = 3     /**< V9 board */
} ifx_Board_Type_t;

/* The values for the Avian devices must be identical to the values in
 * Infineon::Avian::Device_Type. As this is a C header and Infineon::Avian::Device_Type
 * is an enum class, we unfortunately cannot use the values of the original values
 * directly.
 *
 * When adding/changing values also make sure to update the functions
 * sensor_is_avian, sensor_is_mimose, sensor_to_string, and sensor_is_ltr11
 * in rdk::RadarDeviceCommon.
 */
/** Types of different radar sensors */
typedef enum
{
    IFX_AVIAN_BGT60TR13C = 0,          /**< BGT60TR13C */
    IFX_AVIAN_BGT60ATR24C = 1,         /**< BGT60ATR24C */
    IFX_AVIAN_BGT60UTR13D = 2,         /**< BGT60UTR13D */
    IFX_AVIAN_BGT60UTR11AIP = 4,       /**< BGT60UTR11 AIP (Antenna in Package) */
    IFX_AVIAN_UNKNOWN = 10,            /**< Unknown Avian sensor */


    IFX_BGT60LTR11AIP = 256,           /**< BGT60LTR11 AIP (Antenna in Package) */
    IFX_MIMOSE_BGT24ATR22 = 257,       /**< BGT24ATR22 */

    IFX_RADAR_SENSOR_UNKNOWN = 0xfff   /**< Unknown sensor */
} ifx_Radar_Sensor_t;

typedef struct
{
    const char* description;     /**< A pointer to a zero-terminated string
                                      containing a firmware description. */
    uint16_t version_major;      /**< The firmware version major number. */
    uint16_t version_minor;      /**< The firmware version minor number. */
    uint16_t version_build;      /**< The firmware version build number. */

    const char* extendedVersion; /**< Extended firmware version with additional version information */
} ifx_Firmware_Info_t;

typedef struct
{
    ifx_Radar_Sensor_t sensor_type; /**< Type of radar sensor. */
    ifx_Board_Type_t board_type;    /**< Type of board. */
    char uuid[UUID_STRING_SIZE];    /**< Unique id of board in the format xxxxxxxx-xxxx-Mxxx-Nxxx-xxxxxxxxxxxx */
} ifx_Radar_Sensor_List_Entry_t;

typedef struct
{
    const char* description;            /**< A pointer to a null terminated
                                             string holding a human
                                             readable description of the
                                             sensor. */
    double min_rf_frequency_Hz;         /**< The minimum RF frequency the
                                             the sensor can emit in Hz. */
    double max_rf_frequency_Hz;         /**< The maximum RF frequency the
                                             sensor can emit in Hz. */
    uint8_t num_tx_antennas;            /**< The number of RF antennas used
                                             for transmission. */
    uint8_t num_rx_antennas;            /**< The number of RF antennas used
                                             for reception. */
    uint8_t max_tx_power;               /**< The amount of RF transmission
                                             power can be controlled in the
                                             range of 0 ... max_tx_power.
                                             */
    uint64_t device_id;                 /**< Sensor specific device id.
                                             For Avian: If supported, a unique device id which is
                                             programmed during production. If the Avian sensor does
                                             not have a unique device id the value is 0.
                
                                             For radar sensors other than Avian the value is currently 0.
                                             */
    uint32_t max_num_samples_per_chirp; /**<The maixmum number of samples per chirp.
                                         */
    float min_adc_sampling_rate;        /**<The minmum adc sampling rate in Hz.
                                         */
    float max_adc_sampling_rate;        /**<The maximum adc sampling rate in Hz.
                                         */
    uint8_t adc_resolution_bits;        /**<The quantization level of the adc.
                                         */
    const int32_t* lp_cutoff_list;      /**< List of allowed LP cutoff values.
                                         */
    const int32_t* hp_cutoff_list;      /**< List of allowed HP cutoff values.
                                         */
    const int8_t* if_gain_list;         /**< List of allowed IF gain values.
                                         */
} ifx_Radar_Sensor_Info_t;

/**
 * @}
 */

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* IFX_RADAR_DEVICE_COMMON_H */
