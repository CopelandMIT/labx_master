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
 * @file DeviceLtr11.h
 *
 * @brief Main header file for DeviceLtr11
 *
 * Includes all DeviceLtr11 headers.
 *
 * \defgroup gr_cat_device_ltr11               BGT60LTR11 Doppler Radar Sensors (ifxLtr11)
 */

#ifndef IFX_DEVICE_LTR11_H
#define IFX_DEVICE_LTR11_H

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include "DeviceLtr11Types.h"
#include "ifxBase/Error.h"
#include "ifxBase/List.h"
#include "ifxBase/Types.h"
#include "ifxBase/Vector.h"

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

/*
==============================================================================
   3. TYPES
==============================================================================
*/
typedef struct DeviceLtr11Base ifx_Ltr11_Device_t;  // forward declaration

/*
==============================================================================
   4. FUNCTION PROTOTYPES
==============================================================================
*/

/** @addtogroup gr_cat_device_ltr11
 * @{
 */

/** @defgroup gr_device_ltr11_control Device LTR11 (60GHz)
 * @brief API for 60GHz Radar device BGT60LTR11 to configure and get raw data
 * @{
 */

/**
 * @brief Get information about the firmware version.
 *
 * @param [in]  handle      Device handle for BGT60LTR11 device.
 * @return      pointer to \ref ifx_Firmware_Info_t structure
 */
IFX_DLL_PUBLIC
const ifx_Firmware_Info_t* ifx_ltr11_get_firmware_information(ifx_Ltr11_Device_t* handle);

/**
 * \brief Return list of connected BGT60LTR11 radar sensors.
 *
 * The function returns a list of available LTR11 radar devices. Each
 * list element is of type \ref ifx_Radar_Sensor_List_Entry_t.
 * The list must be freed after use using \ref ifx_list_destroy.
 *
 * Here is an example how to use the function:
 * @code
 *      ifx_List_t* device_list = ifx_ltr11_get_list();
 *      for (size_t i = 0; i < ifx_list_size(device_list); i++)
 *      {
 *          ifx_Radar_Sensor_List_Entry_t* entry = ifx_list_get(device_list, i);
 *          // ...
 *      }
 *      ifx_list_destroy(device_list);
 * @endcode
 * \return The function returns a list of all found devices.
 */
IFX_DLL_PUBLIC
ifx_List_t* ifx_ltr11_get_list(void);

/**
 * \brief Return sensor info.
 *
 * The ownership remains with the device. The returned pointer remains valid as
 * long as the device has not been destroyed.
 *
 * \param [in]   handle   Device handle for BGT60LTR11 device.
 * \return       info     Pointer to sensor info for LTR11 device structure.
 */
IFX_DLL_PUBLIC
const ifx_Radar_Sensor_Info_t* ifx_ltr11_get_sensor_information(ifx_Ltr11_Device_t* handle);

/**
 * \brief Open a BGT60LTR11 radar sensor.
 *
 * This function opens a BGT60LTR11 radar sensor and returns a device handle.
 * If no BGT60LTR11 sensor is connected or an error occurred NULL is returned
 * and the error code can be retrieved using \ref ifx_error_get.
 *
 * If several BGT60LTR11 radar sensors are connected the first device found
 * will be opened.
 *
 * \return device   Device handle for BGT60LTR11 device.
 * \return NULL     If no device is present or an error occurred.
 */
IFX_DLL_PUBLIC
ifx_Ltr11_Device_t* ifx_ltr11_create(void);

/**
 * \brief Open BGT60LTR11 radar sensor with given unique id.
 *
 * This function opens the BGT60LTR11 radar sensor with the unique id given by
 * uuid and returns a device handle. If no BGT60LTR11 sensor is connected with
 * that unique id or an error occurred NULL is returned and the error code can
 * be retrieved using \ref ifx_error_get.
 *
 * \param  uuid     Unique id as string.
 * \return device   Device handle for BGT60LTR11 device.
 * \return NULL     If no sensor with that unique id is present or an error occurred.
 */
IFX_DLL_PUBLIC
ifx_Ltr11_Device_t* ifx_ltr11_create_by_uuid(const char* uuid);

/**
 * \brief Open mocking device.
 *
 * Opens a mocking device.
 *
 * \return device   Device handle for BGT60LTR11 device.
 * \return NULL     If no sensor with that unique id is present or an error occurred.
 */
IFX_DLL_PUBLIC
ifx_Ltr11_Device_t* ifx_ltr11_create_dummy(void);

/**
 * \brief Destroy device handle.
 *
 * Disconnects from the radar sensor and destroys the device handle.
 *
 * The device handle must not be used after calling this function.
 *
 * \param [in]   handle   Device handle for BGT60LTR11 device.
 */
IFX_DLL_PUBLIC
void ifx_ltr11_destroy(ifx_Ltr11_Device_t* handle);

/**
 * \brief Set device configuration of a LTR11 device
 *
 * \param [in]   handle   Device handle for BGT60LTR11 device.
 * \param [in]   config   Device configuration.
 */
IFX_DLL_PUBLIC
void ifx_ltr11_set_config(ifx_Ltr11_Device_t* handle, const ifx_Ltr11_Config_t* config);

/**
 * \brief Get currently set device configuration.
 *
 * Write the currently set device configuration to config.
 *
 * \param [in]   handle   Device handle for BGT60LTR11 device.
 * \param [out]  config   Device configuration.
 */
IFX_DLL_PUBLIC
void ifx_ltr11_get_config(ifx_Ltr11_Device_t* handle, ifx_Ltr11_Config_t* config);

/**
 * \brief Get default device configuration.
 *
 * Write the default device configuration to config.
 *
 * \param [in]   handle   Device handle for BGT60LTR11 device.
 * \param [out]  config   Device configuration.
 */
IFX_DLL_PUBLIC
void ifx_ltr11_get_config_defaults(ifx_Ltr11_Device_t* handle, ifx_Ltr11_Config_t* config);

/**
 * \brief Start data acquisition.
 *
 * \param [in]   handle   Device handle for BGT60LTR11 device.
 */
IFX_DLL_PUBLIC
void ifx_ltr11_start_acquisition(ifx_Ltr11_Device_t* handle);

/**
 * \brief Stop data acquisition.
 *
 * \param [in]   handle   Device handle for BGT60LTR11 device.
 */
IFX_DLL_PUBLIC
void ifx_ltr11_stop_acquisition(ifx_Ltr11_Device_t* handle);

/**
 * \brief Retrieves the next frame of time domain data and the next detector data from the LTR11 device.
 *
 * This function retrieves the next frame of time domain data from the connected BGT60LTR11 device, and stores it
 * in the *frame_data* structure of type \ref ifx_Vector_C_t (where len = num_samples which is a member of
 * the configuration structure \ref ifx_Ltr11_Config_t, either set by the caller or the default config is used).
 * If the provided *frame_data* structure is NULL, then the function allocates the memory for the returned structure,
 * and the caller is responsible to free it.
 * This function also retrieves the metadata including the next detector output values and the chip power mode
 * to the meta_data structure of type \ref ifx_Ltr11_Metadata_t.
 * This function blocks for 1000ms until a full frame is available, otherwise the function sets the error
 * \ref IFX_ERROR_TIMEOUT.
 * This function should be used only when the APRT feature is disabled. Indeed, the default 1000 ms timeout to fetch one frame
 * could be too short when the APRT is enabled and no target is detected. Hence, \ref ifx_ltr11_get_next_frame_timeout should be used
 * when APRT is enabled.
 *
 * The possible error codes are as following:
 * \ref IFX_OK                           if the function succeeded.
 * \ref IFX_ERROR_COMMUNICATION_ERROR    if a communication error occurred.
 * \ref IFX_ERROR_TIMEOUT                if a timeout occurred.
 * \ref IFX_ERROR_FIFO_OVERFLOW          if a FIFO overflow occurred.
 * \ref IFX_ERROR                        if an unknown error occurred.
 *
 * For a description of the *frame_data* structure see \ref ifx_Vector_C_t, and
 * for a description of the *metadata* structure see \ref ifx_Ltr11_Metadata_t.
 *
 * Here is a typical usage of this function:
 * @code
 *      const uint16_t timeout_1000ms = 1000;
 *      // In this example, the frame_data vector is NULL
 *      ifx_Vector_C_t* frame_data = NULL;
 *
 *      // Otherwise, the caller needs to allocate memory for the vector and free it.
 *      // Given the current config of type \ref ifx_Ltr11_Config_t:
 *      // num_samples = config.num_samples;
 *      // frame_data = ifx_vec_create_c(num_samples);
 *
 *      ifx_Ltr11_Metadata_t metadata;
 *      while(1)
 *      {
 *          frame_data = ifx_ltr11_get_next_frame(handle, frame_data, metadata);
 *          ifx_Error_t ret = ifx_error_get_and_clear();
 *          if(ret == IFX_ERROR_TIMEOUT)
 *              continue; // no data available, do something else
 *          else if(ret != IFX_OK)
 *              // error handling
 *              break;
 *
 *          // process data
 *          // ...
 *      }
 *      ifx_vec_destroy_c(frame_data);
 * @endcode
 *
 * See also \ref ifx_ltr11_get_next_frame_timeout for a non-blocking version of this function.
 *
 * \param [in]   handle         Device handle for BGT60LTR11 device.
 * \param [out]  frame_data     Frame data structure.
 * \param [out]  metadata       Metadata structure, where the detector
 *                              output processing data, and information regarding the chip
 *                              power mode is copied to.
 *
 * @return \ref ifx_Vector_C_t  Pointer to structure where the time domain data is populated.
 *                              This is pointing to the same memory location as the *frame_data*
 *                              pointer given as an argument if the latter is not NULL.
 *                              If *frame_data* is NULL, the returned pointer points to a newly allocated memory
 *                              inside this function containing a frame_data structure (\ref ifx_Vector_C_t), which
 *                              the caller is responsible to free.
 */
IFX_DLL_PUBLIC
ifx_Vector_C_t* ifx_ltr11_get_next_frame(ifx_Ltr11_Device_t* handle, ifx_Vector_C_t* frame_data, ifx_Ltr11_Metadata_t* metadata);

/**
 * \brief Retrieves the next frame of time domain data and the next detector data from the LTR11 device (non-blocking).
 *
 * This function retrieves the next frame of time domain data from the connected BGT60LTR11 device, and stores it
 * in the *frame_data* structure of type \ref ifx_Vector_C_t (where len = num_samples which is a member of
 * the configuration structure \ref ifx_Ltr11_Config_t, either set by the caller or the default config is used).
 * If the provided *frame_data* structure is NULL, then the function allocates the memory for the returned structure,
 * and the caller is responsible to free it.
 * This function also retrieves the metadata including the next detector output values and the chip power mode
 * to the meta_data structure of type \ref ifx_Ltr11_Metadata_t.
 *
 * When the APRT feature of the chip is enabled, the caller should make sure to give a sufficient timeout, at least equal to the
 * product of the prt multiplied by the number of samples in the frame to fetch and the aprt_factor:
 * (timeout_ms = prt * num_samples * aprt_factor).
 *
 * If no full frame is available within timeout_ms (milliseconds), the function sets the error \ref IFX_ERROR_TIMEOUT.
 * The possible error codes are as following:
 * \ref IFX_OK                           if the function succeeded.
 * \ref IFX_ERROR_COMMUNICATION_ERROR    if a communication error occurred.
 * \ref IFX_ERROR_TIMEOUT                if a timeout occurred.
 * \ref IFX_ERROR_FIFO_OVERFLOW          if a FIFO overflow occurred.
 * \ref IFX_ERROR                        if an unknown error occurred.
 *
 * For a description of the *frame_data* structure see \ref ifx_Vector_C_t, and
 * for a description of the *metadata* structure see \ref ifx_Ltr11_Metadata_t.
 *
 * Here is a typical usage of this function:
 * @code
 *      const uint16_t timeout_1000ms = 1000;
 *      // In this example, the frame_data vector is NULL
 *      ifx_Vector_C_t* frame_data = NULL;
 *
 *      // Otherwise, the caller needs to allocate memory for the vector and free it.
 *      // Given the current config of type \ref ifx_Ltr11_Config_t:
 *      // num_samples = config.num_samples;
 *      // frame_data = ifx_vec_create_c(num_samples);
 *
 *      ifx_Ltr11_Metadata_t metadata;
 *      while(1)
 *      {
 *          frame_data = ifx_ltr11_get_next_frame_timeout(handle, frame_data, metadata, timeout_1000ms);
 *          ifx_Error_t ret = ifx_error_get_and_clear();
 *          if(ret == IFX_ERROR_TIMEOUT)
 *              continue; // no data available, do something else
 *          else if(ret != IFX_OK)
 *              // error handling
 *              break;
 *
 *          // process data
 *          // ...
 *      }
 *      ifx_vec_destroy_c(frame_data);
 * @endcode
 *
 * See also \ref ifx_ltr11_get_next_frame for a blocking version of this function.
 *
 *
 * \param [in]   handle         Device handle for BGT60LTR11 device.
 * \param [out]  frame_data     Frame data structure.
 * \param [out]  metadata       Metadata structure, where the detector
 *                              output processing data, and information regarding the chip
 *                              power mode is copied to.
 * \param [in]   timeout_ms     Timeout in milliseconds. The caller should make sure the timeout is sufficient
 *                              when the APRT feature of the chip is enabled.
 *
 * @return \ref ifx_Vector_C_t  Pointer to structure where the time domain data is populated.
 *                              This is pointing to the same memory location as the *frame_data*
 *                              pointer given as an argument if the latter is not NULL.
 *                              If *frame_data* is NULL, the returned pointer points to a newly allocated memory
 *                              inside this function containing a frame_data structure (\ref ifx_Vector_C_t), which
 *                              the caller is responsible to free.
 */
IFX_DLL_PUBLIC
ifx_Vector_C_t* ifx_ltr11_get_next_frame_timeout(ifx_Ltr11_Device_t* handle, ifx_Vector_C_t* frame_data, ifx_Ltr11_Metadata_t* metadata, uint16_t timeout_ms);

/**
 * \brief Return the limiting values for the LTR11 configuration.
 *
 * This function writes out the limits that restrain the LTR11 configuration (\ref ifx_Ltr11_Config_t).
 *
 * \param [in]   handle      Device handle for BGT60LTR11 device.
 * \param [out]  limits      The limits instance to be written.
 */
IFX_DLL_PUBLIC
void ifx_ltr11_get_limits(ifx_Ltr11_Device_t* handle, ifx_Ltr11_Config_Limits_t* limits);

/**
 * \brief Dump registers to a file specified in argument
 *
 * To get the returned error, please use \ref ifx_error_get
 *
 * \param [in]   handle     Device handle for BGT60LTR11 device.
 * \param [in]   filename   The file name where the registers will be dumped to.
 */
IFX_DLL_PUBLIC
void ifx_ltr11_register_dump_to_file(ifx_Ltr11_Device_t* handle, const char* filename);

/**
 * \brief Returns the power in Watts in active mode for the given config.
 *
 * To get the returned error, please use \ref ifx_error_get
 *
 * \param [in]   handle     Device handle for BGT60LTR11 device.
 * \param [in]   config     The config instance for which the power is calculated.
 * @return A value of type float holding the power in active mode for the given config in W.
 */
IFX_DLL_PUBLIC
float ifx_ltr11_get_active_mode_power(ifx_Ltr11_Device_t* handle, const ifx_Ltr11_Config_t* config);

/**
 * \brief Returns the sampling frequency in Hz for the given prt_index.
 *
 *  Note: In case the APRT feature of the chip is enabled and no target is detected,
 *  the pulse repetition time is multiplied by the aprt_factor. Hence, the sampling
 *  frequency is divided by this factor.
 *  This function does not include this specific case, and only returns the sampling frequency
 *  value computed from the prt when the chip is in active mode.
 *
 * To get the returned error, please use \ref ifx_error_get
 *
 * \param [in]   handle     Device handle for BGT60LTR11 device.
 * \param [in]   prt_index  The pulse repetition time index for which the corresponds a prt, inverse value of the sampling frequency.
 * @return A value of type uint32_t holding the sampling frequency in Hz for the given prt_index.
 */
IFX_DLL_PUBLIC
uint32_t ifx_ltr11_get_sampling_frequency(ifx_Ltr11_Device_t* handle, ifx_Ltr11_PRT_t prt_index);

/**
 * \brief Checks if the given config is valid.
 *
 * To get the returned error, please use \ref ifx_error_get
 *
 * \param[in]    handle      Device handle for BGT60LTR11 device.
 * \param[in]    config      Pointer to the config structure to check that it is valid.
 * @return true if the given config is valid, false otherwise.
 */
IFX_DLL_PUBLIC
bool ifx_ltr11_check_config(ifx_Ltr11_Device_t* handle, const ifx_Ltr11_Config_t* config);


/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* IFX_DEVICE_LTR11_H */
