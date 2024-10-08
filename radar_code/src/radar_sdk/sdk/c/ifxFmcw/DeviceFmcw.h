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
 * @file DeviceFmcw.h
 *
 * For details refer to @ref gr_devicefmcw
 */

#ifndef IFX_DEVICE_FMCW_H
#define IFX_DEVICE_FMCW_H

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include "ifxBase/Error.h"
#include "ifxBase/List.h"

#include "DeviceFmcwTypes.h"

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

typedef struct DeviceFmcw ifx_Device_Fmcw_t;


/*
==============================================================================
   4. FUNCTION PROTOTYPES
==============================================================================
*/

/**
 * @addtogroup gr_cat_Fmcw
 * @{
 */

/**
 * @defgroup gr_devicefmcw FMCW Device Control
 *
 * @brief API for FMCW Radar device control operations
 * @{
 */

/**
 * @brief This function returns a list of available devices.
 *
 * The function returns a list of available FMCW radar devices. Each list
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
ifx_List_t* ifx_fmcw_get_list(void);

/**
 * @brief This function returns a list of available specified devices.
 *
 * The function is similar to @ref ifx_fmcw_get_list. It returns a list of
 * available FMCW radar sensors of type @ref ifx_Radar_Sensor_t. Each list
 * element is of type @ref ifx_Radar_Sensor_List_Entry_t. If the specified
 * sensor is not found, an empty list is returned.
 *
 * The list must be freed after use using @ref ifx_list_destroy.
 *
 * Here is an example how to use the function:
 * @code
 *   ifx_List_t* device_list = ifx_fmcw_get_list_by_sensor_type(IFX_AVIAN_BGT60TR13C);
 *   for (size_t i = 0; i < ifx_list_size(device_list); i++)
 *   {
 *       ifx_Radar_Sensor_List_Entry_t* entry = ifx_list_get(device_list, i);
 *       // ...
 *   }
 *   ifx_list_destroy(device_list);
 * @endcode
 *
 * @param[in] sensor_type  The device type to search for.
 *
 * @return The function returns a list of all found devices.
 */
IFX_DLL_PUBLIC
ifx_List_t* ifx_fmcw_get_list_by_sensor_type(ifx_Radar_Sensor_t sensor_type);

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
ifx_Device_Fmcw_t* ifx_fmcw_create_dummy(ifx_Radar_Sensor_t sensor_type);

/**
 * @brief Creates a Smartar dummy device handle with the given reference frequency
 *        (usually 40 MHz or 80 MHz).
 *
 * This function creates a dummy device of specified type.
 *
 * @param[in] sensor_type  The type of the dummy device to be created.
 * @param[in] reference_freq  The reference frequency for the device.
 *
 * @return Handle to the newly created dummy instance or NULL in case of
 *         failure.
 */
IFX_DLL_PUBLIC
ifx_Device_Fmcw_t* ifx_fmcw_create_dummy_with_reference_freq(ifx_Radar_Sensor_t sensor_type, double reference_freq);


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
ifx_Device_Fmcw_t* ifx_fmcw_create_dummy_from_device(const ifx_Device_Fmcw_t* handle);

/**
 * @brief Creates a device handle.
 *
 * This function searches for an FMCW radar sensor connected to the host
 * machine and connects to the first sensor found.
 *
 * See also @ref ifx_fmcw_create_by_uuid for a list of potential errors.
 *
 * @return Handle to the newly created instance or NULL in case of failure.
 */
IFX_DLL_PUBLIC
ifx_Device_Fmcw_t* ifx_fmcw_create(void);

/**
 * @brief Opens an FMCW sensor.
 *
 * This function opens the FMCW sensor connected to the port given by port. If
 * given port is nullptr, @ref ifx_fmcw_create is called to open an FMCW
 * sensor. See also @ref ifx_fmcw_create_by_uuid for a list of potential
 * errors.
 *
 * @param[in] port  The name of port to open.
 *
 * @return Handle to the newly created instance or NULL in case of failure.
 */
IFX_DLL_PUBLIC
ifx_Device_Fmcw_t* ifx_fmcw_create_by_port(const char* port);

/**
 * @brief Creates a device handle.
 *
 * This function opens the FMCW radar sensor connected to the host machine with
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
ifx_Device_Fmcw_t* ifx_fmcw_create_by_uuid(const char* uuid);

/**
 * @brief Creates a Smartar device handle and configures it with the given reference frequency
 *        (usually 40 MHz or 80 MHz).
 *
 * This function opens the FMCW radar sensor connected to the host machine with
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
ifx_Device_Fmcw_t* ifx_fmcw_create_by_uuid_with_reference_freq(const char* uuid, double reference_freq);

/**
 * @brief Save register list to a file
 *
 * This function writes a register list representing the current acquisition
 * sequence to a file.
 *
 * @param[in] handle    A handle to the radar device object.
 * @param[in] filename  The path to the file the register list shall be written to.
 */
IFX_DLL_PUBLIC
void ifx_fmcw_save_register_file(ifx_Device_Fmcw_t* handle, const char* filename);

/**
 * @brief Load register list to a file
 *
 * This function loads a register list from a file and applies the acquisition
 * sequence represented by that file.
 *
 * @warning There is no plausibility check on the loaded registers. A bad register
 * set list may not work at all or load to unexpected behavior. Especially
 * register lists made for a different device type usually don't work.
 *
 * @param[in] handle    A handle to the radar device object.
 * @param[in] filename  The path to the file the register list shall be loaded from.
 */
IFX_DLL_PUBLIC
void ifx_fmcw_load_register_file(ifx_Device_Fmcw_t* handle, const char* filename);

/**
 * @brief Configures the radar device.
 *
 * This function tries to configure the radar device to generate the specified
 * acquisition sequence.
 *
 * If the specified frame sequence could not be applied, the current
 * configuration is not changed. In that case @ref ifx_error_get will return an
 * according error code and @ref ifx_error_to_string will return a human
 * readable string describing the reason, why the specified sequence could not
 * be applied.
 *
 * @param[in] handle    A handle to the radar device object
 * @param[in] sequence  A pointer to the acquisition sequence, which
 *                      is a linked list of \ref ifx_Fmcw_Sequence_Element_t
 *                      (Therefor it is a pointer to the first element)
 */
IFX_DLL_PUBLIC
void ifx_fmcw_set_acquisition_sequence(ifx_Device_Fmcw_t* handle,
                                       const ifx_Fmcw_Sequence_Element_t* sequence);

/**
 * @brief This function returns the current frame sequence.
 *
 * User takes ownership over the ifx_Fmcw_Sequence_Element_t.
 * The ifx_Fmcw_Sequence_Element_t must be freed after use using
 *
 * @param[in] handle  A handle to the radar device object.
 *
 * @return The first element of the current acquisition sequence.
 */
IFX_DLL_PUBLIC
ifx_Fmcw_Sequence_Element_t* ifx_fmcw_get_acquisition_sequence(ifx_Device_Fmcw_t* handle);


/**
 * @brief Get sensor type of connected device.
 *
 * @param[in] handle  A handle to the radar device object.
 *
 * @return The type of the connected sensor device.
 */
IFX_DLL_PUBLIC
ifx_Radar_Sensor_t ifx_fmcw_get_sensor_type(const ifx_Device_Fmcw_t* handle);

/**
 * @brief Reads the temperature of the radar sensor device.
 *
 * On success, the current temperature of the radar sensor is returned. The
 * temperature is in units of degrees Celsius.
 *
 * If an error occurs, an undefined value is returned and the error code can be
 * retrieved using the function @ref ifx_error_get
 *
 * It is currently not possible to read the temperature from a BGT60UTR11AIP.
 * For a BGT60UTR11AIP this function will return IFX_ERROR_NOT_SUPPORTED.
 *
 * @param[in] handle  A handle to the radar device object.
 *
 * @return The temperature of the radar device in Celsius degrees.
 */
IFX_DLL_PUBLIC
float ifx_fmcw_get_temperature(ifx_Device_Fmcw_t* handle);

/**
 * @brief Closes the connection to the radar sensor device.
 *
 * This function stops acquisition of time domain data, closes the connection
 * to the device and destroys the handle. The handle becomes invalid and must
 * not be used any more after this function was called.
 *
 * Please note that this function is not thread-safe and must not accessed
 * at the same time by multiple threads.
 *
 * @param[in] handle  A handle to the radar device object.
 */
IFX_DLL_PUBLIC
void ifx_fmcw_destroy(ifx_Device_Fmcw_t* handle);

/**
 * @brief Starts the acquisition of time domain data.
 *
 * This function starts the acquisition of time domain data. If data
 * acquisition is already running, the function has no effect.
 *
 * The time-domain signal can be retrieved by calling the function
 * @ref ifx_fmcw_get_next_frame or @ref ifx_fmcw_get_next_frame_timeout.
 *
 * This function is thread-safe.
 *
 * @param[in] handle  A handle to the radar device object.
 */
IFX_DLL_PUBLIC
void ifx_fmcw_start_acquisition(ifx_Device_Fmcw_t* handle);

/**
 * @brief Stops the acquisition of time domain data.
 *
 * This function stops the acquisition of time domain data. If data acquisition
 * in not running, the function has no effect.
 *
 * This function is thread-safe.
 *
 * @param[in] handle  A handle to the radar device object.
 */
IFX_DLL_PUBLIC
void ifx_fmcw_stop_acquisition(ifx_Device_Fmcw_t* handle);

/**
 * @brief Retrieves the next frame of time domain data from a radar device.
 * (non-blocking).
 *
 * This function retrieves the next complete frame of time domain data from the
 * connected device. The samples from all chirps and all enabled RX antennas
 * will be copied to the provided *frame* data structure. For details, how data
 * is stored an the frame structure, see @ref ifx_Fmcw_Frame_t.
 *
 * Here is a typical usage of this function:
 * @code
 *   ifx_Fmcw_Frame_t* frame = ifx_fmcw_allocate_frame(device_handle);
 *   while(1)
 *   {
 *       ifx_fmcw_get_next_frame(device_handle, frame);
 *       ifx_Error_t ret = ifx_error_get_and_clear();
 *       if(ret == IFX_ERROR_TIMEOUT)
 *           continue; // no data available, do something else
 *       else if(ret != IFX_OK)
 *           // error handling
 *           break;
 *
 *       // process data
 *       // ...
 *   }
 *   ifx_fmcw_destroy_frame(frame);
 * @endcode
 *
 * The function blocks until a full frame has been received or an error
 * occurred. Possible errors are:
 * - @ref IFX_ERROR_COMMUNICATION_ERROR : Communication error between board
 *                                       and host. This error typically occurs
 *                                       if the board was unplugged while
 *                                       fetching data.
 * - @ref IFX_ERROR_FIFO_OVERFLOW : The radar board could not fetch the data
 *                                 fast enough from the radar sensor. The FIFO
 *                                 of the radar sensor overflowed and the state
 *                                 machine of the radar sensor was stopped.
 *                                 This typically occurs with radar
 *                                 configurations that cause high data rates,
 *                                 e.g. high frame rate, many activated
 *                                 receiving antennas, many samples per frame,
 *                                 etc.
 * - @ref IFX_ERROR_TIMEOUT : No full frame was available within 10 seconds.
 * - @ref IFX_ERROR : An unknown error occurred.
 * The error status can be retrieved by calling @ref ifx_error_get after this
 * function call.
 *
 * This function is equivalent to calling @ref ifx_fmcw_get_next_frame_timeout
 * with a timeout of 10 seconds. For high values of the frame repetition time
 * (10 seconds or larger, corresponding to a frame rate of 0.1Hz or lower) the
 * function might return @ref IFX_ERROR_TIMEOUT. In that case, please use
 * @ref ifx_fmcw_get_next_frame_timeout instead.
 *
 * This function automatically starts data acquisition if it was not previously
 * started explicitly. It is not required to call
 * @ref ifx_fmcw_start_acquisition manually.
 *
 * The radar sensor sends the time-domain data in slices and not in full
 * frames. This means that after fetching a frame it does not necessarily take
 * frame_repetition_time_s seconds until this function returns the next frame.
 * For example, if both the frame and the frame repetition time are small (not
 * too many samples in a frame, high frame rate), one slice may contain several
 * frames. In this case two calls to @ref ifx_fmcw_get_next_frame will return
 * a frame immediately.
 *
 * See also @ref ifx_fmcw_start_acquisition and
 * @ref ifx_fmcw_get_next_frame_timeout.
 *
 * @param[in]  handle  A handle to the radar device object.
 * @param[out] frame   The frame structure where the time domain data shall be
 *                     copied to.
 */
IFX_DLL_PUBLIC
void ifx_fmcw_get_next_frame(ifx_Device_Fmcw_t* handle,
                             ifx_Fmcw_Frame_t* frame);

/**
 * @brief Retrieves the next frame of time domain data from a radar device
 * (non-blocking).
 *
 * This function retrieves the next complete frame of time domain data from the
 * connected device. The samples from all chirps and all enabled RX antennas
 * will be copied to the provided *frame* data structure. For details, how data
 * is stored an the frame structure, see @ref ifx_Fmcw_Raw_Frame_t.
 *
 * Here is a typical usage of this function:
 * @code
 *   ifx_Fmcw_Raw_Frame_t* frame = ifx_fmcw_allocate_raw_frame(device_handle);
 *   while(1)
 *   {
 *       ifx_fmcw_get_next_raw_frame(device_handle, frame);
 *       ifx_Error_t ret = ifx_error_get_and_clear();
 *       if(ret == IFX_ERROR_TIMEOUT)
 *           continue; // no data available, do something else
 *       else if(ret != IFX_OK)
 *           // error handling
 *           break;
 *
 *       // process data
 *       // ...
 *   }
 *   ifx_fmcw_destroy_raw_frame(frame);
 * @endcode
 *
 * The function blocks until a full frame has been received or an error
 * occurred. Possible errors are:
 * - @ref IFX_ERROR_COMMUNICATION_ERROR : Communication error between board
 *                                       and host. This error typically occurs
 *                                       if the board was unplugged while
 *                                       fetching data.
 * - @ref IFX_ERROR_FIFO_OVERFLOW : The radar board could not fetch the data
 *                                 fast enough from the radar sensor. The FIFO
 *                                 of the radar sensor overflowed and the state
 *                                 machine of the radar sensor was stopped.
 *                                 This typically occurs with radar
 *                                 configurations that cause high data rates,
 *                                 e.g. high frame rate, many activated
 *                                 receiving antennas, many samples per frame,
 *                                 etc.
 * - @ref IFX_ERROR_TIMEOUT : No full frame was available within 10 seconds.
 * - @ref IFX_ERROR : An unknown error occurred.
 * The error status can be retrieved by calling @ref ifx_error_get after this
 * function call.
 *
 * This function is equivalent to calling @ref ifx_fmcw_get_next_raw_frame_timeout
 * with a timeout of 10 seconds. For high values of the frame repetition time
 * (10 seconds or larger, corresponding to a frame rate of 0.1Hz or lower) the
 * function might return @ref IFX_ERROR_TIMEOUT. In that case, please use
 * @ref ifx_fmcw_get_next_frame_timeout instead.
 *
 * This function automatically starts data acquisition if it was not previously
 * started explicitly. It is not required to call
 * @ref ifx_fmcw_start_acquisition manually.
 *
 * The radar sensor sends the time-domain data in slices and not in full
 * frames. This means that after fetching a frame it does not necessarily take
 * frame_repetition_time_s seconds until this function returns the next frame.
 * For example, if both the frame and the frame repetition time are small (not
 * too many samples in a frame, high frame rate), one slice may contain several
 * frames. In this case two calls to @ref ifx_fmcw_get_next_frame will return
 * a frame immediately.
 *
 * See also @ref ifx_fmcw_start_acquisition and
 * @ref ifx_fmcw_get_next_raw_frame_timeout.
 *
 * @param[in]  handle  A handle to the radar device object.
 * @param[out] frame   The frame structure where the time domain data shall be
 *                     copied to.
 */
IFX_DLL_PUBLIC
void ifx_fmcw_get_next_raw_frame(ifx_Device_Fmcw_t* handle,
                                 ifx_Fmcw_Raw_Frame_t* frame);

/**
 * @brief Retrieves the next frame of time domain data from a radar device
 *        (blocking).
 *
 * This function retrieves the next complete frame of time domain data from the
 * connected device. The samples from all chirps and all enabled RX antennas
 * will be copied to the provided *frame* data structure. For details, how data
 * is stored an the frame structure, see @ref ifx_Fmcw_Frame_t.
 *
 * The function will either populate the current frame into the structure
 * as soon as a complete frame is available or return if an
 * @ref IFX_ERROR_TIMEOUT occurs if no frame was available within *timeout_ms*
 * of milliseconds. The possible errors are
 * @ref IFX_OK                           if the function succeeded.
 * @ref IFX_ERROR_COMMUNICATION_ERROR    if a communication error occurred.
 * @ref IFX_ERROR_TIMEOUT                if a timeout occurred.
 * @ref IFX_ERROR_FIFO_OVERFLOW          if a FIFO overflow occurred.
 * @ref IFX_ERROR                        if an unknown error occurred.
 * See @ref ifx_fmcw_get_next_frame for more information.
 *
 * Here is a typical usage of this function:
 * @code
 *   const uint16_t timeout_100ms = 100;
 *   ifx_Fmcw_Frame_t* frame = ifx_fmcw_allocate_frame(device_handle);
 *   while(1)
 *   {
 *       ifx_fmcw_get_next_frame_timeout(device_handle, frame, timeout_100ms);
 *       ifx_Error_t ret = ifx_error_get_and_clear();
 *       if(ret == IFX_ERROR_TIMEOUT)
 *           continue; // no data available, do something else
 *       else if(ret != IFX_OK)
 *           // error handling
 *           break;
 *
 *       // process data
 *       // ...
 *   }
 *   ifx_fmcw_destroy_frame(frame);
 * @endcode
 *
 * See also @ref ifx_fmcw_get_next_frame for a blocking version of this
 * function.
 *
 * @param[in]  handle      A handle to the radar device object.
 * @param[out] frame       The frame structure where the time domain data shall
 *                         be copied to.
 * @param[in]  timeout_ms  The maximum period of time in milliseconds to wait
 *                         for the next frame of radar data.
 */
IFX_DLL_PUBLIC
void ifx_fmcw_get_next_frame_timeout(ifx_Device_Fmcw_t* handle,
                                     ifx_Fmcw_Frame_t* frame,
                                     uint16_t timeout_ms);

/**
 * @brief Retrieves the next frame of time domain data from a radar device
 *        (blocking).
 *
 * This function retrieves the next complete frame of time domain data from the
 * connected device. The samples from all chirps and all enabled RX antennas
 * will be copied to the provided *frame* data structure. For details, how data
 * is stored an the frame structure, see @ref ifx_Fmcw_Raw_Frame_t.
 *
 * The function will either populate the current frame into the structure
 * as soon as a complete frame is available or return if an
 * @ref IFX_ERROR_TIMEOUT occurs if no frame was available within *timeout_ms*
 * of milliseconds. The possible errors are
 * @ref IFX_OK                           if the function succeeded.
 * @ref IFX_ERROR_COMMUNICATION_ERROR    if a communication error occurred.
 * @ref IFX_ERROR_TIMEOUT                if a timeout occurred.
 * @ref IFX_ERROR_FIFO_OVERFLOW          if a FIFO overflow occurred.
 * @ref IFX_ERROR                        if an unknown error occurred.
 * See @ref ifx_fmcw_get_next_frame for more information.
 *
 * Here is a typical usage of this function:
 * @code
 *   const uint16_t timeout_100ms = 100;
 *   ifx_Fmcw_Raw_Frame_t* frame = ifx_fmcw_allocate_raw_frame(device_handle);
 *   while(1)
 *   {
 *       ifx_fmcw_get_next_raw_frame_timeout(device_handle, frame, timeout_100ms);
 *       ifx_Error_t ret = ifx_error_get_and_clear();
 *       if(ret == IFX_ERROR_TIMEOUT)
 *           continue; // no data available, do something else
 *       else if(ret != IFX_OK)
 *           // error handling
 *           break;
 *
 *       // process data
 *       // ...
 *   }
 *   ifx_fmcw_destroy_raw_frame(frame);
 * @endcode
 *
 * See also @ref ifx_fmcw_get_next_raw_frame for a blocking version of this
 * function.
 *
 * @param[in]  handle      A handle to the radar device object.
 * @param[out] frame       The frame structure where the time domain data shall
 *                         be copied to.
 * @param[in]  timeout_ms  The maximum period of time in milliseconds to wait
 *                         for the next frame of radar data.
 */
IFX_DLL_PUBLIC
void ifx_fmcw_get_next_raw_frame_timeout(ifx_Device_Fmcw_t* handle,
                                         ifx_Fmcw_Raw_Frame_t* frame,
                                         uint16_t timeout_ms);

/**
 * @brief Allocates a frame structure.
 *
 * This function allocates a frame structure that holds a complete data frame
 * as acquired with the current configuration.
 *
 * @param[in] handle  A handle to the radar device object.
 *
 * @return A frame structure with the right dimensions to hold a complete frame
 *         for the current acquisition sequence.
 */
IFX_DLL_PUBLIC
ifx_Fmcw_Frame_t* ifx_fmcw_allocate_frame(ifx_Device_Fmcw_t* handle);

/**
 * @brief Allocates a raw frame structure.
 *
 * This function allocates a raw frame structure that holds a complete data frame
 * as acquired with the current configuration.
 *
 * @param[in] handle  A handle to the radar device object.
 *
 * @return A raw frame structure with the right dimensions to hold a complete frame
 *         for the current acquisition sequence.
 */
IFX_DLL_PUBLIC
ifx_Fmcw_Raw_Frame_t* ifx_fmcw_allocate_raw_frame(ifx_Device_Fmcw_t* handle);

/**
 * @brief Frees memory of all data cubes in a frame and the frame structure
 *        itself.
 *
 * @param[in] frame  The frame instance to be destroyed.
 */
IFX_DLL_PUBLIC
void ifx_fmcw_destroy_frame(ifx_Fmcw_Frame_t* frame);

/**
 * @brief Frees memory allocated for the frame structure
 *        including the frame data.
 *
 * @param[in] frame  The frame instance to be destroyed.
 */
IFX_DLL_PUBLIC
void ifx_fmcw_destroy_raw_frame(ifx_Fmcw_Raw_Frame_t* frame);

/**
 * @brief Retrieves the unique id (UUID) of the connected board.
 *
 * You can use @ref ifx_uuid_from_string to convert the string into an array
 * of 16 bytes.
 *
 * @param[in] handle  A handle to the radar device object.
 *
 * @return A string containing the unique ID (UUID).
 */
IFX_DLL_PUBLIC
const char* ifx_fmcw_get_board_uuid(const ifx_Device_Fmcw_t* handle);

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
const ifx_Radar_Sensor_Info_t* ifx_fmcw_get_sensor_information(const ifx_Device_Fmcw_t* handle);

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
const ifx_Firmware_Info_t* ifx_fmcw_get_firmware_information(const ifx_Device_Fmcw_t* handle);

/**
 * @brief Get duration of sequence element
 *
 * return the wait_time of a delay, the calculated duration of a single chirp,
 * or the result of loop duration.
 *
 * @param[in] handle  A handle to the radar device object.
 * @param[out] element  A pointer to an ifx_Fmcw_Sequence_Element_t
 */
IFX_DLL_PUBLIC
float ifx_fmcw_get_element_duration(const ifx_Device_Fmcw_t* handle, const ifx_Fmcw_Sequence_Element_t* element);

/**
 * @brief Get duration of sequence
 *
 * ifx_fmcw_get_sequence_duration calls \ref ifx_fmcw_get_element_duration on
 * the first element and all following elements until next_element is NULL.
 * The results of all these function calls be summed and returned.
 *
 * @param[in] handle    A handle to the radar device object.
 * @param[in] sequence  A pointer to the acquisition sequence, which
 *                      is a linked list of \ref ifx_Fmcw_Sequence_Element_t
 *                      (Therefor it is a pointer to the first element)
 *
 */
IFX_DLL_PUBLIC
float ifx_fmcw_get_sequence_duration(const ifx_Device_Fmcw_t* handle, const ifx_Fmcw_Sequence_Element_t* sequence);

/**
 * @brief Calculate minimum chirp repetition time.
 *
 * @param [in]  handle          A handle to the radar device object
 * @param [in]  num_samples     The number of samples in the chirp
 * @param [in]  sample_rate_Hz  The sampling rate of the chirp.
 * @return  The minimum possible chirp repetition time in seconds.
 */
IFX_DLL_PUBLIC
float ifx_fmcw_get_minimum_chirp_repetition_time(const ifx_Device_Fmcw_t* handle, uint32_t num_samples, float sample_rate_Hz);

/**
 * @brief Returns sampling bandwidth from the sensor device.
 *
 * @param [in]     handle       A handle to the radar device object.
 * @param [in]     chirp        A chirp element.
 * @return The sampling bandwidth
 */
IFX_DLL_PUBLIC
double ifx_fmcw_get_chirp_sampling_bandwidth(ifx_Device_Fmcw_t* handle, const ifx_Fmcw_Sequence_Chirp_t* chirp);

/**
 * @brief Returns the center frequency from the sensor device.
 *
 * @param [in]     handle       A handle to the radar device object.
 * @param [in]     chirp        A chirp element.
 * @return The sampling center frequency (i.e. the center of the sampling bandwidth)
 */
IFX_DLL_PUBLIC
double ifx_fmcw_get_chirp_sampling_center_frequency(ifx_Device_Fmcw_t* handle, const ifx_Fmcw_Sequence_Chirp_t* chirp);

/**
 * @brief Convert raw frame to an array of floats with values in the [-1,1] range.
 *
 * @param[in] handle A handle to the radar device object.
 * @param[in] num_samples The number of samples to convert.
 * @param[in] raw_data The time domain data.
 * @param[out] converted_frame The array where the converted data is copied to.
 */
IFX_DLL_PUBLIC
void ifx_fmcw_convert_raw_data_to_float_array(ifx_Device_Fmcw_t* handle, uint32_t num_samples, const uint16_t* raw_data, ifx_Float_t* converted_frame);

/**
 * @brief Returns a view of the deinterleaved frame given a pointer to the converted deinterleaved frame.
 *
 * @param[in] handle A handle to the radar device object.
 * @param[in] converted_frame A pointer to the converted frame array (\ref ifx_fmcw_convert_raw_data_to_float_array).
 * @param[out] deinterleaved_frame_view The frame view deinterleaved, and not owning the data, \ref ifx_Device_Fmcw_t.
 */
IFX_DLL_PUBLIC
void ifx_fmcw_view_deinterleaved_frame(ifx_Device_Fmcw_t* handle, ifx_Float_t* converted_frame, ifx_Fmcw_Frame_t* deinterleaved_frame_view);

/**
 * @brief Returns a view of a the raw deinterleaved frame.
 *
 * @param[in] handle A handle to the radar device object.
 * @param[in] raw_frame The raw frame \ref ifx_Fmcw_Raw_Frame_t.
 * @param[out] deinterleaved_frame The deinterleaved frame.
 */
IFX_DLL_PUBLIC
void ifx_fmcw_deinterleave_raw_frame(ifx_Device_Fmcw_t* handle, const ifx_Fmcw_Raw_Frame_t* raw_frame, ifx_Fmcw_Raw_Frame_t* deinterleaved_frame);

/**
 * @}
 */

/**
 * @}
 */


#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* IFX_DEVICE_FMCW_H */
