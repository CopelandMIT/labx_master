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

#ifndef APP_COMMON_JSON_H
#define APP_COMMON_JSON_H

/*
==============================================================================
    1. INCLUDE FILES
==============================================================================
*/

#include "ifxAvian/DeviceControl.h"
#include "ifxRadarPresenceSensing/PresenceSensing.h"
#include "ifxRadarSegmentation/Segmentation.h"


#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

/*
==============================================================================
    2. DEFINITIONS
==============================================================================
*/


/*
==============================================================================
    3. TYPES
==============================================================================
*/

struct ifxJsonConfigurationC;
typedef struct ifxJsonConfigurationC ifx_json_t;

/*
==============================================================================
    4. FUNCTION PROTOTYPES
==============================================================================
*/

/**
 * @brief Create new json configuration object
 *
 * Create an empty json configuration object
 *
 * @retval json_object  if successful
 * @retval NULL         otherwise
 */
ifx_json_t* ifx_json_create(void);

/**
 * @brief Create json configuration object from file
 *
 * Read the configuration from the JSON configuration filename. This is a
 * convenience function for \ref ifx_json_create and
 * \ref ifx_json_load_from_file.
 *
 * @retval json_object  if successful
 * @retval NULL         otherwise
 */
ifx_json_t* ifx_json_create_from_file(const char* filename);

/**
 * @brief Destroy json object
 *
 * @param [in]  j   json object (might be NULL)
 */
void ifx_json_destroy(ifx_json_t* j);

/**
 * @brief Load configuration from file
 *
 * Load the configuration from the file given by filename.
 *
 * @param [in] json         json object
 * @param [in] filename     filename of configuration file
 * @retval  true    if successful
 * @retval  false   otherwise
 */
bool ifx_json_load_from_file(ifx_json_t* json, const char* filename);

/**
 * @brief Save configuration to file
 *
 * Save the configuration to the file given by filename.
 *
 * @param [in] json         json object
 * @param [in] filename     filename of configuration file
 * @retval  true    if successful
 * @retval  false   otherwise
 */
bool ifx_json_save_to_file(ifx_json_t* json, const char* filename);

/**
 * @brief Get error message
 *
 * Return a human-readable description of the last error.
 *
 * @param [in]  json    json object
 * @retval error message
 */
const char* ifx_json_get_error(const ifx_json_t* json);

/** @brief Return true if fmcw_single_shape configuration is present */
bool ifx_json_has_config_single_shape(const ifx_json_t* json);
void ifx_json_set_device_config_single_shape(ifx_json_t* json, const ifx_Avian_Config_t* config);
bool ifx_json_get_device_config_single_shape(ifx_json_t* json, ifx_Avian_Config_t* config);

/** @brief Return true if fmcw_scene configuration is present */
bool ifx_json_has_config_scene(const ifx_json_t* json);
void ifx_json_set_device_config_scene(ifx_json_t* json, const ifx_Avian_Metrics_t* metrics);
bool ifx_json_get_device_config_scene(ifx_json_t* json, ifx_Avian_Metrics_t* metrics);

/** @brief Return true if segmentation configuration is present */
bool ifx_json_has_segmentation(const ifx_json_t* json);
void ifx_json_set_segmentation(ifx_json_t* json, const ifx_Segmentation_Config_t* segmentation_config);
bool ifx_json_get_segmentation(ifx_json_t* json, const ifx_Avian_Config_t* device_config, ifx_Segmentation_Config_t* config_segmentation);

void ifx_json_set_config_presence_sensing(ifx_json_t* json, const ifx_Presence_Sensing_Config_t* presence_config);
bool ifx_json_has_config_presence_sensing(const ifx_json_t* json);
bool ifx_json_get_config_presence_sensing(ifx_json_t* json, ifx_Presence_Sensing_Config_t* presence_config);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // #ifndef APP_COMMON_JSON_H
