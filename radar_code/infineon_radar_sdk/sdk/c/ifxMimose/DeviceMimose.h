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
 * @file DeviceMimose.h
 *
 * @brief Main header file for ifxMimose
 *
 * Includes all ifxMimose headers.
 *
 * \defgroup gr_cat_device_mimose               Device Mimose (ifxMimose)
 */

#ifndef IFX_DEVICE_MIMOSE_H
#define IFX_DEVICE_MIMOSE_H

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include "DeviceMimoseTypes.h"

#include "ifxBase/Cube.h"
#include "ifxBase/List.h"
#include "ifxRadarDeviceCommon/RadarDeviceCommon.h"


#ifdef __cplusplus
extern "C"
{
#endif

/*
==============================================================================
2. DEFINITIONS
==============================================================================
*/
#define MIMOSE_API_VERSION 230516u
/*
==============================================================================
   3. TYPES
==============================================================================
*/

/*
==============================================================================
   4. FUNCTION PROTOTYPES
==============================================================================
*/


/** @addtogroup gr_cat_device_mimose
 * @{
 */

/** @defgroup gr_device_mimose_control Device Mimose Control (24GHz)
 * @brief API for 24GHz Radar Device Mimose to configure and get raw data
 * @{
 */

/**
 * \brief This function returns current API version as integer.
 *
 * \return A number which corresponds to the current API version.
 *
 */
IFX_DLL_PUBLIC
int ifx_mimose_api_version(void);

/**
 * \brief Returns a list of available MIMOSE devices.
 *
 * This function returns a list of available MIMOSE radar devices.
 * Each list element is of type \ref ifx_Radar_Sensor_List_Entry_t.
 *
 * The list must be freed after use using \ref ifx_list_destroy.
 *
 * Here is an example how to use the function:
 * @code
 *      ifx_List_t* device_list = ifx_mimose_get_list();
 *      for (size_t i = 0; i < ifx_list_size(device_list); i++)
 *      {
 *          ifx_Radar_Sensor_List_Entry_t* entry = ifx_list_get(device_list, i);
 *          // ...
 *      }
 *      ifx_list_destroy(device_list);
 * @endcode
 *
 * \return The function returns a list of all found devices.
 */
IFX_DLL_PUBLIC
ifx_List_t* ifx_mimose_get_list(void);

/**
 * \brief Returns the sensor information defined by \ref ifx_Radar_Sensor_Info_t.
 *
 * The returned pointer is valid until the device handle is destroyed.
 *
 * \param [in]  handle      A handle to the MIMOSE device object.
 * \return      pointer to \ref ifx_Radar_Sensor_Info_t structure.
 *
 */
IFX_DLL_PUBLIC
const ifx_Radar_Sensor_Info_t* ifx_mimose_get_sensor_information(ifx_Mimose_Device_t* handle);

/**
 * \brief Creates an un-initialized handle for MIMOSE device.
 *
 * After creating this empty handle, application needs to call the \ref ifx_mimose_get_config_defaults()
 * to get the default configuration and the relative limits. Then application can initialize the device handle
 * with these default values using \ref ifx_mimose_set_config().
 *
 * \return Handle to the newly created MIMOSE instance or NULL in case of failure.
 *
 */
IFX_DLL_PUBLIC
ifx_Mimose_Device_t* ifx_mimose_create(void);

/**
 * @brief Creates a device handle.
 *
 * This function searches for a Mimose radar sensor connected to the host machine and
 * connects to the sensor device with the matching UUID.
 *
 * For converting the UUID from byte array to/from string see
 * \ref ifx_uuid_from_string and \ref ifx_uuid_to_string.
 *
 * @param [in]     uuid       uuid as string in the format xxxxxxxx-xxxx-Mxxx-Nxxx-xxxxxxxxxxxx
 *
 * @return Handle to the newly created instance or NULL in case of failure.
 *
 */
IFX_DLL_PUBLIC
ifx_Mimose_Device_t* ifx_mimose_create_by_uuid(const char* uuid);

/**
 * \brief Creates a mocking interface of a MIMOSE device.
 *
 * \return Handle to the newly created dummy MIMOSE instance or NULL in case of failure.
 *
 */
IFX_DLL_PUBLIC
ifx_Mimose_Device_t* ifx_mimose_create_dummy(void);

/**
 * \brief Closes the connection to the MIMOSE sensor device.
 * \param [in]  handle      A handle to the MIMOSE device object.
 */
IFX_DLL_PUBLIC
void ifx_mimose_destroy(ifx_Mimose_Device_t* handle);

/**
 * \brief Sets the configurations of a MIMOSE device.
 * \param [in]  handle      A handle to the MIMOSE device object.
 * \param [in]  config      The configuration to be set.
 */
IFX_DLL_PUBLIC
void ifx_mimose_set_config(ifx_Mimose_Device_t* handle, const ifx_Mimose_Config_t* config);

/**
 * \brief Gets the current configurations of a MIMOSE device.
 * \param [in]  handle      A handle to the MIMOSE device object.
 * \param [out] config      The configuration to be retrieved.
 */
IFX_DLL_PUBLIC
void ifx_mimose_get_config(const ifx_Mimose_Device_t* handle, ifx_Mimose_Config_t* config);

/**
 * \brief Gets the default configurations of a MIMOSE device.
 * \param [in]  handle      A handle to the MIMOSE device object.
 * \param [out] config      The default configuration to be retrieved.
 */
IFX_DLL_PUBLIC
void ifx_mimose_get_config_defaults(const ifx_Mimose_Device_t* handle, ifx_Mimose_Config_t* config);

/**
 * \brief Switches the active frame configuration to be used during the acquisition procedure.
 * \param [in]  handle      A handle to the MIMOSE device object.
 * \param [in]  active_frame_config_index   The index of the frame configuration to be used.
 */
IFX_DLL_PUBLIC
void ifx_mimose_switch_frame_configuration(ifx_Mimose_Device_t* handle, uint16_t active_frame_config_index);

/**
 * \brief Starts the data acquisition in relation to the active frame configuration index.
 * \param [in]  handle      A handle to the MIMOSE device object.
 */
IFX_DLL_PUBLIC
void ifx_mimose_start_acquisition(ifx_Mimose_Device_t* handle);

/**
 * \brief Stops the data acquisition.
 * \param [in]  handle       A handle to the MIMOSE device object.
 */
IFX_DLL_PUBLIC
void ifx_mimose_stop_acquisition(ifx_Mimose_Device_t* handle);

/**
 * \brief Fetches one complete frame arranged as a cube having dimensions of 1 x num-pulses x num-samples and metadata.
 *
 * If there is no data available then the function returns immediately by giving back the IFX_ERROR_TIMEOUT error code;
 * otherwise it returns a preallocated and sample filled \ref ifx_Cube_C_t frame if no other error has occurred.
 * If the metadata output parameter is given (is not null) then the metadata will be filled up.
 * Note: the output frame cube can also be allocated by the user, but it is recommended that the function does the allocation
 * on first call. This can be triggered by using a null as the output frame parameter. The pointer to the output frame cube is
 * output from the function as a return value. The cube needs to be destroyed by the caller using \ref ifx_cube_destroy_c.
 *
 * \param [in]  handle     A handle to the MIMOSE device object.
 * \param [out] frame      The cube primitive instance for frame storage to be filled out.
 * \param [out] metadata   The frame metadata to be filled out.
 *
 * \return The cube primitive instance for frame storage filled with frame data.
 */
IFX_DLL_PUBLIC
ifx_Cube_C_t* ifx_mimose_get_next_frame(ifx_Mimose_Device_t* handle, ifx_Cube_C_t* frame, ifx_Mimose_Metadata_t* metadata);

/**
 * \brief Fetches one complete frame arranged as a cube having dimensions of 1 x num-pulses x num-samples and metadata with timeout constraint.
 *
 * If there is no data available within the timeout supplied then the function returns immediately by giving back the IFX_ERROR_TIMEOUT error code;
 * otherwise it returns a preallocated and sample filled \ref ifx_Cube_C_t frame if no other error has occurred.
 * If the metadata output parameter is given (is not null) then the metadata will be filled up.
 * Note: the output frame cube can also be allocated by the user, but it is recommended that the function does the allocation
 * on first call. This can be triggered by using a null as the output frame parameter. The pointer to the output frame cube is
 * output from the function as a return value. The cube needs to be destroyed by the caller using \ref ifx_cube_destroy_c.
 *
 * \param [in]  handle     A handle to the MIMOSE device object.
 * \param [out] frame      The cube primitive instance for frame storage to be filled out.
 * \param [out] metadata   The frame metadata to be filled out.
 * \param [in]  timeout_ms The timeout expressed in milliseconds.
 *
 * \return The cube primitive instance for frame storage filled with frame data.
 */
IFX_DLL_PUBLIC
ifx_Cube_C_t* ifx_mimose_get_next_frame_timeout(ifx_Mimose_Device_t* handle, ifx_Cube_C_t* frame, ifx_Mimose_Metadata_t* metadata, uint16_t timeout_ms);

/**
 * \brief Dumps registers to a file specified in argument.
 * \param [in]  handle     A handle to the MIMOSE device object.
 * \param [in]  filename   The file name where the register will be dumped to.
 */
IFX_DLL_PUBLIC
void ifx_mimose_register_dump_to_file(const ifx_Mimose_Device_t* handle, const char* filename);

/**
 * \brief Gets the register count from the device definitions.
 * \param [in]  handle      A handle to the MIMOSE device object.
 * \return The total number of registers in device.
 */
IFX_DLL_PUBLIC
size_t ifx_mimose_get_register_count(ifx_Mimose_Device_t* handle);

/**
 * \brief Returns the complete list of register values from the MIMOSE Device object in the SDK.
 * The returned values are from the SDK object and may not contain the actual register status or readout.
 * for reading an updated register value directly from the device hardware, use \ref ifx_mimose_get_register_value.
 * \param [in]  handle      A handle to the MIMOSE device object.
 * \param [in]  registers   pointer to register list to be populated.
 *                          register address and values paired in 32 bit integers.
 *                          0xAAAAVVVV
 *                          The caller needs to ensure enough memory (uint32_t * total registers) is allocated.
 *                          The total number of registers needed for allocating memory can be retrieved
 *                          by the API \ref ifx_mimose_get_register_count.
 */
IFX_DLL_PUBLIC
void ifx_mimose_get_registers(ifx_Mimose_Device_t* handle, uint32_t* registers);

/**
 * \brief Writes a set of registers to the MIMOSE Device.
 * \param [in]  handle      A handle to the MIMOSE device object.
 * \param [in]  registers   pointer to register list to be written.
 *                          register address and values paired in 32 bit integers.
 *                          0xAAAAVVVV
 * \param [in]  count       The number of registers to be written
 */
IFX_DLL_PUBLIC
void ifx_mimose_set_registers(ifx_Mimose_Device_t* handle, uint32_t* registers, size_t count);

/**
 * \brief Reads the value of a single register from the MIMOSE device and returns the value.
 * \param [in]  handle              A handle to the MIMOSE device object.
 * \param [in]  register_address    Address of the register to be read. The address validity is checked inside the function.
 */
IFX_DLL_PUBLIC
uint16_t ifx_mimose_get_register_value(ifx_Mimose_Device_t* handle, uint16_t register_address);

/**
 * \brief updates RC look up table through device tuning.
 * If the returned system clock in RC mode differs more than a certain extent compared to the desired clock,
 * this function can be called to retune the RC Look up table (LUT) which can have device
 * and environment specific variations.
 * \param [in]  handle         A handle to the MIMOSE device object.
 */
IFX_DLL_PUBLIC
void ifx_mimose_update_rc_lut(ifx_Mimose_Device_t* handle);

/**
 * \brief Reads sensor values at a synchronous period (temperature and center frequency).
 * \param [in]  handle         A handle to the MIMOSE device object.
 * \param [out] sensor_values  The sensor values to be written.
 */
IFX_DLL_PUBLIC
void ifx_mimose_get_sensor_values(ifx_Mimose_Device_t* handle, ifx_Mimose_Sensor_t* sensor_values);

/**
 * \brief Returns the firmware information
 *
 * The function returns the firmware information.
 *
 * The returned pointer is only valid until the device handle is destroyed.
 *
 * \param [in]  handle         A handle to the MIMOSE device object.
 * \return      info           Firmware information.
 */
IFX_DLL_PUBLIC
const ifx_Firmware_Info_t* ifx_mimose_get_firmware_information(ifx_Mimose_Device_t* handle);

/**
 * \brief Checks if the given configuration is valid.
 *
 * It is possible that only one frame configuration is used (no switching), allowing the second frame
 * configuration to be potentially un-intitalized. The function checks for one of the two frame configurations
 * controlled by the frame_configuratrion_index parameter. If both configurations are intended to be used (frame-switching
 * intended) then it is recommended to check both configuration indices 0, and 1.
 * To get the returned error, please use \ref ifx_error_get
 *
 * \param[in]    handle                      A handle to the MIMOSE device object.
 * \param[in]    config                      Pointer to the config structure to check that it is valid.
 * \param [in]   frame_configuration_index   Configuration index (two are possible)
 * @return true if the given config is valid, false otherwise.
 */
IFX_DLL_PUBLIC
bool ifx_mimose_check_config(ifx_Mimose_Device_t* handle, const ifx_Mimose_Config_t* config, uint16_t frame_configuration_index);

/**
 * \brief Returns the number of samples limits.
 *
 * Returns the number of samples limits (min and max), given the selected pulse configurations.
 *
 * \param [in]  handle                  A handle to the MIMOSE device object.
 * \param [in]	selected_pulse_configs	Selected pulse configurations.
 * \param [out] min_number_of_samples   Minimum number of samples.
 * \param [out] max_number_of_samples   Maximum number of samples.
 *
 */
IFX_DLL_PUBLIC
void ifx_mimose_get_number_of_samples_limits(ifx_Mimose_Device_t* handle, const bool selected_pulse_configs[4], uint16_t* min_number_of_samples, uint16_t* max_number_of_samples);

/**
 * \brief Returns the pulse repetition time limits in seconds.
 *
 * Returns the pulse repetition time limits (min and max), given the selected pulse configurations.
 *
 * \param [in]  handle                  A handle to the MIMOSE device object.
 * \param [in]  selected_pulse_configs	Selected pulse configurations.
 * \param [out] min_prt                 Minimum prt (depending on selected pulse configs).
 * \param [out] max_prt                 Maximum prt (Fixed not depending on selected pulse configs).
 *
 */
IFX_DLL_PUBLIC
void ifx_mimose_get_pulse_repetition_time_limits_s(ifx_Mimose_Device_t* handle, const bool selected_pulse_configs[4], float* min_prt, float* max_prt);


/**
 * \brief Returns the frame repetition time limits in seconds.
 *
 * Returns the frame repetition time limits (min and max) in seconds, given the arguments.
 *
 * \param [in]  handle					A handle to the MIMOSE device object.
 * \param [in]  selected_pulse_configs	Selected pulse configurations.
 * \param [in]  number_of_samples		Number of samples.
 * \param [in]  afc_duration_ct			AFC duration
 * \param [in]  prt						Pulse Repetition Time.
 * \param [out] min_frt					Minimum frame repetition time in seconds.
 * \param [out] max_frt					Maximum frame repetition time in seconds (Fixed not depending on the parameters).
 */
IFX_DLL_PUBLIC
void ifx_mimose_get_frame_repetition_time_limits_s(ifx_Mimose_Device_t* handle, const bool selected_pulse_configs[4], uint16_t number_of_samples,
                                                   uint16_t afc_duration_ct, float prt, float* min_frt, float* max_frt);

/**
 * \brief Returns the RF Center Frequency limits in Hz.
 *
 * Returns the RF center Frequency limits (min and max), given the RF band.
 *
 * \param [in]  handle                  A handle to the MIMOSE device object.
 * \param [in]  band	                RF band \ref ifx_Mimose_RF_Band_t.
 * \param [out] min_rf_center_frequency Minimum RF center Frequency.
 * \param [out] max_rf_center_frequency Maximum RF center Frequency.
 *
 */
IFX_DLL_PUBLIC
void ifx_mimose_get_rf_center_frequency_limits_Hz(ifx_Mimose_Device_t* handle, ifx_Mimose_RF_Band_t band,
                                                  uint64_t* min_rf_center_frequency, uint64_t* max_rf_center_frequency);

/**
 * \brief Returns the default limiting values for a configuration.
 *
 * This function writes out the default limits for MIMOSE device configuration \ref ifx_Mimose_Config_t.
 *
 * In any case the limits output parameter must be not null.
 *
 * \param [in]  handle  A handle to the MIMOSE device object.
 * \param [out] limits  The limits instance to be written.
 */
IFX_DLL_PUBLIC
void ifx_mimose_get_default_limits(const ifx_Mimose_Device_t* handle, ifx_Mimose_Config_Limits_t* limits);

/**
 * \brief Applies the given AOC mode array elements consecutively to each of the active pulses.
 *
 * The function applies the given AOC mode array elements consecutively to each of the active pulses.
 *
 * \param [in]  handle      A handle to the MIMOSE device object.
 * \param [in]  aoc_mode    Automatic Offset Compensation mode array.
 */
void ifx_mimose_set_automatic_offset_compensation_mode(ifx_Mimose_Device_t* handle, const ifx_Mimose_AOC_Mode_t aoc_mode[4]);

/**
 * @}
 */


/**
 * @}
 */

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* IFX_DEVICE_MIMOSE_H */
