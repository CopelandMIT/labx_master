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
 * @file DeviceCw.h
 *
 * @brief Header for Continuous Wave Control. It provides APIs to use an device in continuous wave mode. All relevant
 * parameters for the CW mode are set/get through these APIs. It also provides an interface to acquire data received
 * through the RX antennas.
 */

#ifndef IFX_DEVICE_CW_H
#define IFX_DEVICE_CW_H

/*
==============================================================================
1. INCLUDE FILES
==============================================================================
*/

#include "DeviceCwTypes.h"

#include <ifxAvian/DeviceControl.h>

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

typedef struct DeviceCw ifx_Device_Cw_t;

/*
==============================================================================
4. FUNCTION PROTOTYPES
==============================================================================
*/

/** @addtogroup gr_cat_Cw
 * @{
 */

/**
 * @defgroup gr_devicecw CW Device Control
 *
 * @brief API for Radar continuous wave operations
 *
 *@{
 */

/**
 * @brief This function creates and initializes the continuous wave mode device instance.
 *
 * The provided port within the device handle is used to detect the sensor type of the connected device.
 * According to the detected sensor type the CW mode device is
 * initialized. If the detected device is not recognized, an error is set.
 *
 * After initialization the connection is still in deep sleep mode.
 *
 * @return A handle to the CW device
 */
IFX_DLL_PUBLIC
ifx_Device_Cw_t* ifx_cw_create();

/**
 * @brief This function returns a list of available devices.
 *
 * The function returns a list of available CW radar devices. Each list
 * element is of type @ref ifx_Radar_Sensor_List_Entry_t.
 *
 * The list must be freed after use using @ref ifx_list_destroy.
 *
 * Here is an example how to use the function:
 * @code
 *      ifx_List_t* device_list = ifx_fmcw_get_list();
 *      for (size_t i = 0; i < ifx_list_size(device_list); i++)
 *      {
 *          ifx_Radar_Sensor_List_Entry_t* entry = ifx_list_get(device_list, i);
 *          // ...
 *      }
 *      ifx_list_destroy(device_list);
 * @endcode
 *
 * @return The function returns a list of all found devices.
 */
IFX_DLL_PUBLIC
ifx_List_t* ifx_cw_get_list(void);

/**
 * @brief Creates a device handle.
 *
 * This function opens the CW radar sensor connected to the host machine with
 * the unique id given by UUID.
 *
 * For converting the UUID from byte array to/from string see
 * @see ifx_uuid_from_string and @see ifx_uuid_to_string.
 *
 * On failure one of the following error codes are set:
 * - @ref IFX_ERROR_ARGUMENT_INVALID : The format of the UUID is wrong.
 * - @ref IFX_ERROR_NO_DEVICE : Sensor with given UUID not found.
 * - @ref IFX_ERROR_MEMORY_ALLOCATION_FAILED : Memory allocation failed.
 * - @ref IFX_ERROR_NOT_SUPPORTED : Sensor is not an FMCW sensor.
 * - @ref IFX_ERROR_FIRMWARE_VERSION_NOT_SUPPORTED : Firmware version not
 *                                                  supported anymore.
 * - @ref IFX_ERROR_COMMUNICATION_ERROR : A communication error occurred.
 * - @ref IFX_ERROR : An unknown error occurred.
 *
 * @param[in] uuid  The unique ID (UUID) as a string in the format xxxxxxxx-xxxx-Mxxx-Nxxx-xxxxxxxxxxxx
 *
 * @return Handle to the newly created instance or NULL in case of failure.
 */
IFX_DLL_PUBLIC
ifx_Device_Cw_t* ifx_cw_create_by_uuid(const char* uuid);

/**
 * @brief Creates a Smartar device handle and configures it with the given reference frequency
 *        (usually 40 MHz or 80 MHz).
 *
 * This function opens the CW radar sensor connected to the host machine with
 * the unique id given by UUID.
 *
 * For converting the UUID from byte array to/from string see
 * @see ifx_uuid_from_string and @see ifx_uuid_to_string.
 *
 * On failure one of the following error codes are set:
 * - @ref IFX_ERROR_ARGUMENT_INVALID : The format of the UUID is wrong.
 * - @ref IFX_ERROR_NO_DEVICE : Sensor with given UUID not found.
 * - @ref IFX_ERROR_MEMORY_ALLOCATION_FAILED : Memory allocation failed.
 * - @ref IFX_ERROR_NOT_SUPPORTED : Sensor is not an FMCW sensor.
 * - @ref IFX_ERROR_FIRMWARE_VERSION_NOT_SUPPORTED : Firmware version not
 *                                                  supported anymore.
 * - @ref IFX_ERROR_COMMUNICATION_ERROR : A communication error occurred.
 * - @ref IFX_ERROR : An unknown error occurred.
 *
 * @param[in] uuid  The unique ID (UUID) as a string in the format xxxxxxxx-xxxx-Mxxx-Nxxx-xxxxxxxxxxxx
 * @param[in] reference_freq  The reference frequency for the device.
 *
 * @return Handle to the newly created instance or NULL in case of failure.
 */
IFX_DLL_PUBLIC
ifx_Device_Cw_t* ifx_cw_create_by_uuid_with_reference_freq(const char* uuid, double reference_freq);

/**
 * @brief Creates a dummy device handle.
 *
 * This function creates a dummy device of specified type.
 *
 * @param[in] sensor_type  The type of the dummy device to be created.
 *
 * @return Handle to the newly created dummy instance or NULL in case of
 *         failure.
 */
IFX_DLL_PUBLIC
ifx_Device_Cw_t* ifx_cw_create_dummy(ifx_Radar_Sensor_t sensor_type);

/**
 * @brief Creates a dummy device handle with the given external frequency.
 * Watch out: The firmware auto-detects the reference frequency on Avian devices and therefore "reference_freq" is ignored.
 *
 * This function creates a dummy device of specified type.
 *
 * @param[in] sensor_type  The type of the dummy device to be created.
 * @param[in] reference_freq The reference frequency for the device.
 *
 * @return Handle to the newly created dummy instance or NULL in case of
 *         failure.
 */
IFX_DLL_PUBLIC
ifx_Device_Cw_t* ifx_cw_create_dummy_with_reference_freq(ifx_Radar_Sensor_t sensor_type, double reference_freq);

/**
 * @brief creates a dummy device handle from device.
 *
 * This function creates a dummy device of specified type.
 *
 * @param[in] handle device handle.
 *
 * @return Handle to the newly created dummy instance or NULL in case of
 *         failure.
 */
IFX_DLL_PUBLIC
ifx_Device_Cw_t* ifx_cw_create_dummy_from_device(const ifx_Device_Cw_t* handle);

/**
 * @brief Delete instance of device
 *
 * If the signal is active, it will be stopped.<BR>
 *
 * @param [in] handle   A CW device handle to be destroyed
 */
IFX_DLL_PUBLIC
void ifx_cw_destroy(ifx_Device_Cw_t* handle);

/**
 * @brief This function gives the status of the continous wave signal.
 *
 * @param [in]     handle    A handle to the CW device
 *
 * @retval         Returns "true" if CW is enabled and transmitting signal, else returns "false"
 */
IFX_DLL_PUBLIC
bool ifx_cw_is_signal_active(ifx_Device_Cw_t* handle);

/**
 * @brief This function starts the continuous wave signal.
 *
 * The device is set to Active mode, and the continuous wave signal is started.
 *
 * In case the continuous wave can't be enabled due to hardware failure, an error is set.
 *
 * @param [in]     handle    A handle to the CW device
 *
 */
IFX_DLL_PUBLIC
void ifx_cw_start_signal(ifx_Device_Cw_t* handle);

/**
 * @brief This function stops the continuous wave signal.
 *
 * The continuous wave device is set to Deep Sleep mode, and the continuous wave signal is stopped.
 *
 * In case the continuous wave can't be enabled due to hardware failure, an error is set.
 *
 * @param [in]     handle    A handle to the CW device
 *
 */
IFX_DLL_PUBLIC
void ifx_cw_stop_signal(ifx_Device_Cw_t* handle);

/**
 * @brief This method configures the continuous wave device's baseband, defined by \ref ifx_Cw_Baseband_Config_t.
 *
 * It includes the configuration of gain and cutoff frequency of the high pass and anti aliasing filter and
 * VGA gain of baseband amplifier. It's recommended to read back the applied value using
 * \ref ifx_cw_get_baseband_config
 *
 * @param [in]     handle		A handle to the CW device
 * @param [in]	    config	  Baseband configurations defined by \ref ifx_Cw_Baseband_Config_t
 *
 */
IFX_DLL_PUBLIC
void ifx_cw_set_baseband_config(ifx_Device_Cw_t* handle, const ifx_Cw_Baseband_Config_t* config);

/**
 * @brief It provides the current baseband configuration of device.
 *
 * @param [in]     handle		A handle to the CW device
 * @return		      Current baseband configurations defined by \ref ifx_Cw_Baseband_Config_t
 *
 */
IFX_DLL_PUBLIC
const ifx_Cw_Baseband_Config_t* ifx_cw_get_baseband_config(ifx_Device_Cw_t* handle);

/**
 * @brief This method configures the device's ADC parameters, defined by \ref ifx_Cw_Adc_Config_t.
 *
 * It includes the configuration of Sample Time ifx_Avian_ADC_SampleTime, tracking or number of conversion
 *  ifx_Avian_ADC_Tracking, oversampling factor ifx_Avian_ADC_Oversampling and enable/disable of double the MSB time.
 *
 * It's recommended to read back the applied value using \ref ifx_cw_get_adc_config.
 *
 * @param [in]     handle		A handle to the CW device
 * @param [in]	    config	  ADC configurations defined by \ref ifx_Cw_Adc_Config_t
 *
 */
IFX_DLL_PUBLIC
void ifx_cw_set_adc_config(ifx_Device_Cw_t* handle, const ifx_Cw_Adc_Config_t* config);

/**
 * @brief It provides the current ADC configurations of continuous wave device.
 *
 * @param [in]     handle		A handle to the CW device
 *
 * @return	      Current ADC configurations defined by \ref ifx_Cw_Adc_Config_t
 *
 */
IFX_DLL_PUBLIC
const ifx_Cw_Adc_Config_t* ifx_cw_get_adc_config(ifx_Device_Cw_t* handle);

/**
 * @brief This method configures the device's test signal generator instead of actually TX transmitted signal
 *
 * It is used for testing the IF section with a known signal without involving TX antennas.
 *
 * @param [in]     handle		A handle to the CW device
 * @param [in]	    config	  Test signal configurations defined by \ref ifx_Cw_Test_Signal_Generator_Config_t
 *
 */
IFX_DLL_PUBLIC
void ifx_cw_set_test_signal_generator_config(ifx_Device_Cw_t* handle, const ifx_Cw_Test_Signal_Generator_Config_t* config);

/**
 * @brief It provides the current test signal configurations of continuous wave device.
 *
 * @param [in]     handle		A handle to the CW device
 * @return         Current test signal configurations defined by \ref ifx_Cw_Test_Signal_Generator_Config_t
 *
 */
IFX_DLL_PUBLIC
const ifx_Cw_Test_Signal_Generator_Config_t* ifx_cw_get_test_signal_generator_config(ifx_Device_Cw_t* handle);

/**
 * @brief This method measures the current temperature of the device.
 *
 * This method will set an error in case of a hardware failure.
 *
 * @param [in]     handle		A handle to the CW device
 *
 * @return The measured device temperature in Celsius degree is returned.
 */
IFX_DLL_PUBLIC
float ifx_cw_measure_temperature(ifx_Device_Cw_t* handle);

/**
 * @brief This method measures returns the power that is emitted from the specified TX antenna
 *
 * The power measurement is only performed if the continuous wave is enabled.
 *
 * @param [in]     handle		A handle to the CW device
 * @param [in]     antenna		The index of the antenna to be enabled or disabled
 *								The value is 0 based and must be less than the total
 *                               number of physical TX antennas within device. If the
 *								value is not in the allowed range, an error is set
 *
 * @return If the continuous wave is enabled the measured TX power in dBm is
 *         returned. Otherwise -infinity is returned.
 */
IFX_DLL_PUBLIC
float ifx_cw_measure_tx_power(ifx_Device_Cw_t* handle, uint32_t antenna);

/**
 * @brief This method captures one frame of raw data.
 *
 * This function captures for all activated antennas samples_per_antenna and
 * returns the samples as a matrix with dimensions
 * num_antennas (rows) x samples_per_antenna (columns).
 *
 * The samples of the raw data are normalized to values between -1...1. To
 * convert the samples to voltages, each sample must be multiplied by the
 * reference voltage 1.21V.
 *
 * If frame is NULL, memory for the matrix will be allocated and returned.
 * Otherwise the memory of frame will be used.
 *
 * @param [in]     handle		A handle to the CW device
 * @param [in]     frame			Pointer to the \ref ifx_Matrix_R_t *frame* where raw data is stored.
 *                               If this is NULL, then a new frame is created. the caller is responsible to
 *                               deallocate the memory.
 *
 * @return	pointer to the \ref ifx_Matrix_R_t *frame* containing the received samples.
 *
 */
IFX_DLL_PUBLIC
ifx_Matrix_R_t* ifx_cw_capture_frame(ifx_Device_Cw_t* handle, ifx_Matrix_R_t* frame);

/**
 * @brief Get information about the sensor on the connected device.
 *
 * The returned pointer is only valid until the device is destroyed.
 *
 * @param[in] handle  A handle to the radar device object.
 *
 * @return A structure containing sensor information.
 */
IFX_DLL_PUBLIC
const ifx_Radar_Sensor_Info_t* ifx_cw_get_sensor_information(const ifx_Device_Cw_t* handle);

/**
 * @brief Get information about the firmware version.
 *
 * The returned pointer is only valid until the device is destroyed.
 *
 * @param[in] handle  A handle to the radar device object.
 *
 * @return A pointer to an ifx_Firmware_Info_t structure
 */
IFX_DLL_PUBLIC
const ifx_Firmware_Info_t* ifx_cw_get_firmware_information(const ifx_Device_Cw_t* handle);

/**
 * @brief Get sensor type of connected device.
 *
 * @param[in] handle  A handle to the radar device object.
 *
 * @return The type of the connected sensor device.
 */
IFX_DLL_PUBLIC
ifx_Radar_Sensor_t ifx_cw_get_sensor_type(const ifx_Device_Cw_t* handle);

/**
 * @brief Save register list to a file
 *
 * This function writes a register list representing the current CW device
 * configuration to a file.
 *
 * @param[in] handle    A handle to the radar device object.
 * @param[in] filename  The path to the file the register list shall be written to.
 */
IFX_DLL_PUBLIC
void ifx_cw_save_register_file(ifx_Device_Cw_t* handle, const char* filename);

/**
 * @brief Load register list to a file
 *
 * This function loads a register list from a file and applies the CW device
 * configuration represented by that file.
 *
 * @warning There is no plausibility check on the loaded registers. A bad register
 * set list may not work at all or load to unexpected behavior. Especially
 * register lists made for a different device type usually don't work.
 *
 * @param[in] handle    A handle to the radar device object.
 * @param[in] filename  The path to the file the register list shall be loaded from.
 */
IFX_DLL_PUBLIC
void ifx_cw_load_register_file(ifx_Device_Cw_t* handle, const char* filename);

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* IFX_DEVICE_CW_H */
