/* ===========================================================================
** Copyright (C) 2020-2023 Infineon Technologies AG
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
 * @file PresenceSensing.h
 *
 * \defgroup gr_cat_RadarPresence       Radar Presence (ifxRadarPresenceSensing)
 *
 * \brief  API to use Xensiv presence sensing detection algorithm
 *
 *
 */

#ifndef IFX_RADAR_PRESENCE_SENSING_H
#define IFX_RADAR_PRESENCE_SENSING_H

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include "ifxBase/Matrix.h"
#include "ifxBase/Types.h"

#include "ifxAvian/DeviceConfig.h"
#include "ifxAvian/DeviceControl.h"


#ifdef __cplusplus
extern "C"
{
#endif

/*
==============================================================================
   3. TYPES
==============================================================================
*/

/** @addtogroup gr_cat_RadarPresence
 * @{
 */

/**
 * @brief Presence sensing algorithm configurations
 *
 * You can use \ref ifx_presence_sensing_get_config_defaults to get a default
 * configuration that can serve as a starting point.
 */
typedef struct
{
    ifx_Float_t min_detection_range_m; /**< Target below this range are ignored. */
    ifx_Float_t max_detection_range_m; /**< Target above this range are ignored. */
    ifx_Float_t sensitivity_threshold; /**< Threshold for macro presence detection. Valid range is [0,1].
                                             - Low: [0, 0.1]
                                             - Medium: (0.1, 0.5]
                                             - High: (0.5, 0.99]
                                             - Max: 1.0 */
} ifx_Presence_Sensing_Config_t;

/**
 * @brief Result structure for Presence Sensing algorithm
 *
 * This structure holds two key outputs i.e. Presence state and in case of presence detected,
 * then the distance of strongest target from sensor in meters.
 *
 */
typedef struct
{
    bool target_state;             /**< Current presence state.*/
    ifx_Float_t target_distance_m; /**< Distance of strongest target from sensor in meters. Only valid in presence state. */
} ifx_Presence_Sensing_Result_t;

/**
 * @brief Handle for presence sensing algorithm
 */
typedef struct ifx_Presence_Sensing_s ifx_Presence_Sensing_t;

/*
==============================================================================
   4. FUNCTION PROTOTYPES
==============================================================================
*/

/**
 * @brief Obtain default configurations for presence sensing
 *
 * This function returns the presence sensing configuration and
 * sensor configuration for presence sensing with default values
 * for a given sensor type.
 *
 * @param [in]     sensor_type     Radar sensor type defined by \ref ifx_Radar_Sensor_t.
 * @param [out]    sensor_config   Radar sensor configurations used to fetch the raw frame of data for processing.
 * @param [out]    presence_config Presence algorithm configurations defined by \ref ifx_Presence_Sensing_Config_t.
 */
IFX_DLL_PUBLIC
void ifx_presence_sensing_get_config_defaults(ifx_Radar_Sensor_t sensor_type,
                                              ifx_Avian_Config_t* sensor_config,
                                              ifx_Presence_Sensing_Config_t* presence_config);

/**
 * @brief Creates a Presence Sensing handle (object) and initializes internal state of presence algorithm.
 *
 * It creates a presence sensing processing handle for a provided sensor configurations. The caller is responsible for
 * setting the sensor configuration.
 *
 * @param [in]     sensor_config    Sensor configurations used to fetch the raw frame of data for processing
 * @param [in]     presence_config  Presence algorithm configurations defined by \ref ifx_Presence_Sensing_Config_t.
 *
 * @return Handle to the newly created instance or NULL in case of failure.
 *
 */
IFX_DLL_PUBLIC
ifx_Presence_Sensing_t* ifx_presence_sensing_create(const ifx_Avian_Config_t* sensor_config,
                                                    const ifx_Presence_Sensing_Config_t* presence_config);

/**
 * @brief Destroys Presence Sensing handle (object) to clear internal states and memories.
 *
 * @param [in]     handle    Pointer to a handle of a presence sensing structure
 */
IFX_DLL_PUBLIC
void ifx_presence_sensing_destroy(ifx_Presence_Sensing_t* handle);

/**
 * @brief Performs the presence sensing algorithm on given raw data.
 *
 * Perform the presence sensing algorithm on the given matrix consisting of
 * raw frame data coming from the configured radar transceiver.
 *
 * The matrix frame_Data consists of frame data from the radar transceiver.
 * The number of rows of the input matrix should match the number of
 * chirps per frame and the number of columns should match the number
 * of samples per chirp expected by the \ref ifx_Presence_Sensing_t.
 *
 * @param [in]     handle      Handle to the presence sensing object.
 * @param [in]     frame_data  Cube consisting of raw frame data from the radar transceiver.
 * @param [out]    result      Structure containing the result of the presence algorithm.
 */
IFX_DLL_PUBLIC
void ifx_presence_sensing_run(ifx_Presence_Sensing_t* handle, const ifx_Cube_R_t* frame_data,
                              ifx_Presence_Sensing_Result_t* result);

/**
 * @}
 */

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* IFX_RADAR_PRESENCE_SENSING_H */
