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
 * @file MotionAngle.h
 *
 * \brief \copybrief gr_cat_MotionAngle
 *
 * For details refer to \ref gr_cat_MotionAngle
 *
 * \defgroup gr_cat_MotionAngle              Motion-Angle algorithm (ifxMotionAngle)
 */

#ifndef IFX_MOTIONANGLE_H
#define IFX_MOTIONANGLE_H

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include "ifxBase/Cube.h"
#include "ifxBase/Types.h"

#include "ifxAvian/DeviceConfig.h"


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

/** @addtogroup gr_cat_MotionAngle
 * @{
 */

/** @defgroup gr_motionangle Motion Angle
  * @brief API for MotionAngle Sensing algorithm
  *
  * Efficient calculation of motion (distance, speed) and angle (azimuth and elevation) of the closest moving
  * object with enough RCS (radar cross section) in the field of view.
  * Objects with high-enough RCS will be referred to persons in this module.
  *
  * The function \ref ifx_motionangle_set_maxrange allows configuring the maximum range a person should be detected.
  * The required RCS and movement can be configured with \ref ifx_motionangle_set_sensitivity.
  *
  * This algorithm always reports the closest person in the field of view, without a dedicated track per person:
  * When multiple person are within the configured range, only the closest will be reported.
  * If one person gets closer while the previous closest person get further away, the distance, speed
  * and angle will be interpolated between both persons, no absence state is in between.
  *
  * There are two main differences to the Infineon segmentation algorithm:
  * First this algorithm is not report the angles as fixed segments.
  * The angle is interpolated directly from the phase differences between receiving antennas (monopulse method) as opposed to a
  * beamforming method applied in the segmentation algorithm.
  * Secondly this algorithm does not maintain multiple tracks. Only the closest signal is tracked, it is not tried to
  * distinguish multiple persons based on this method currently. This limits the memory and compute requirements for this
  * algorithm (on minimal configuration it allows to run on a M0 class CPU with <8kb of RAM).
  *
  * The limit of the maximum range depends on the radar configuration. The algorithm provides a predefined setting \ref IFX_MOTIONANGLE_DEFAULT.
  *
  * The orientation of the sensor is itself not configurable here, as the sensor reports both azimuth and elevation.
  * The term elevation assumes a portrait mode installation.
  * If the sensor is mounted in landscape, it is up to the application to interpret azimuth as elevation and vice-versa.
  *
  * To use the library, you first create a handle \ref ifx_MotionAngle_t of the motion angle for the used type of sensor \ref ifx_Radar_Sensor_t
  * using your own preferred radar configuration \ref ifx_Avian_Config_t using \ref ifx_motionangle_create or
  * from an already tested mode \ref ifx_motionangle_create_from_mode, that returns a desired \ref ifx_Avian_Config_t.
  *
  * Here is a code snipped to create an handle from a predefined mode on the BGT60TR13C:
  *
  * \code{.c}
  ifx_MotionAngle_t *handle;
  ifx_Avian_Config_t avian_config;
  handle = ifx_motionangle_create_from_mode(IFX_AVIAN_BGT60TR13C, IFX_MOTIONANGLE_DEFAULT, &avian_config);
  \endcode
  *
  * Then you can set the desired maximum range and/or sensitivity.
  *
  * \code{.c}
  ifx_motionangle_set_sensitivity(handle, 0.6f); // slightly higher sensitivity (0.5 is default)
  ifx_motionangle_set_maxrange(handle, 3.0f); // 3 meters
  \endcode
  *
  * Now, after configuring the data with the returned avian_config, you feed each frame to the algorithm and interpret the result.
  *
  * \code{.c}
  // in loop
  ifx_MotionAngle_Result_t result;
  ifx_motionangle_run(handle, frame, &result);
  if (result.distance > 0) {
     // interpret result distance, speed, azimuth and elevation
  } else {
     // handle absence (no person in field of view) case
  }
  \endcode
  *
  * Finally, when you want to stop the processing, you can free up allocated resources from this handle.
  *
  * \code{.c}
  ifx_motionangle_destroy(handle);
  \endcode
  *
  * @{
  */

/**
 * @brief Handle of MotionAngle module
 */
typedef struct ifx_MotionAngle_s ifx_MotionAngle_t;

/**
 * @brief MotionAngle modes
 */
typedef enum
{
    IFX_MOTIONANGLE_DEFAULT = 4u,                  /**< default is IFX_MOTIONANGLE_430MHZ_128SPC_50FPS_8CPF */
    IFX_MOTIONANGLE_430MHZ_32SPC_50FPS_8CPF = 0u,  /**< bandwidth of 430MHz, 32 samples per chirp, 50fps,  8cpf */
    IFX_MOTIONANGLE_430MHZ_32SPC_50FPS_16CPF = 1u, /**< bandwidth of 430MHz, 32 samples per chirp, 50fps, 16cpf */
    IFX_MOTIONANGLE_430MHZ_32SPC_50FPS_32CPF = 2u, /**< bandwidth of 430MHz, 32 samples per chirp, 50fps, 32cpf */
    IFX_MOTIONANGLE_430MHZ_32SPC_50FPS_64CPF = 3u, /**< bandwidth of 430MHz, 32 samples per chirp, 50fps, 64cpf */
    IFX_MOTIONANGLE_430MHZ_128SPC_50FPS_8CPF = 4u, /**< bandwidth of 430MHz, 128 samples per chirp, 50fps, 8cpf */
} ifx_MotionAngle_Mode_t;

/**
 * @brief Motion-Angle result data
 *
 * The current motion angle information is returned from \ref ifx_motionangle_run.
 *
 * The distance of the closest person in meters if the sensor location.
 * If no object is detected in the configured range (see \ref ifx_motionangle_set_maxrange)
 * then distance is set to -1.
 *
 * The angles azimuth and elevation are returned independently in polar coordinates, with zero angles
 * indication the direction in front of the sensor.
 * On the \ref IFX_AVIAN_BGT60TR13C the
 * azimuth is the polar coordinate (degrees) on the two dimensional plane spanned by sensor front face direction and antenna 1 and 3 and
 * elevation is the polar coordinate (degrees) on the two dimensional plane spanned by sensor front face direction and antenna 2 and 3.
 *
 * The speed is returned as m/s.
 */
typedef struct
{
    ifx_Float_t distance;        /*!< Distance of object in [m], -1 if undefined/absence */
    ifx_Float_t dx;              /*!< X position in meters, 0 if undefined/absence */
    ifx_Float_t dy;              /*!< Y position in meters, 0 if undefined/absence */
    ifx_Float_t dz;              /*!< X position in meters, 0 if undefined/absence */
    ifx_Float_t angle_azimuth;   /*!< Azimuth angle of the closest object in degrees, 0 if undefined/absence */
    ifx_Float_t angle_elevation; /*!< Elevation angle of the closest object in degrees, 0 if undefined/absence */
    ifx_Float_t speed;           /*!< Speed of object in [m/s], 0 if undefined/absence */
} ifx_MotionAngle_Result_t;

/*
==============================================================================
   4. FUNCTION PROTOTYPES
==============================================================================
*/

/**
 * @brief Creates a MotionAngle handle from a specific device configuration.
 *
 * The function creates a motionangle handle for a specific device configuration.
 * The caller is responsible for setting the device configuration.
 * In case the algorithm can not work with the device config NULL is returned
 *
 * @param [in]     sensor_type      Radar sensor type \ref ifx_Radar_Sensor_t.
 * @param [in]     device_config    Desired device configuration
 * @return Handle to the newly created instance or NULL in case of failure or
 * unsupported device configuration.
 */
IFX_DLL_PUBLIC
ifx_MotionAngle_t* ifx_motionangle_create(ifx_Radar_Sensor_t sensor_type, const ifx_Avian_Config_t* device_config);

/**
 * @brief Creates a MotionAngle handle from a specific/tested configuration mode.
 *
 * The function creates a motionangle handle for a specific scenario described
 * by mode. The associated device configuration is written to device_config.
 * The caller is responsible for setting the device configuration.
 *
 * @param [in]     sensor_type      Radar sensor type \ref ifx_Radar_Sensor_t.
 * @param [in]     mode             MotionAngle configurations defined by \ref ifx_MotionAngle_Mode_t.
 * @param [out]    device_config    Device configuration
 * @return Handle to the newly created instance or NULL in case of failure.
 */
IFX_DLL_PUBLIC
ifx_MotionAngle_t* ifx_motionangle_create_from_mode(ifx_Radar_Sensor_t sensor_type, ifx_MotionAngle_Mode_t mode, ifx_Avian_Config_t* device_config);

/**
 * @brief Set sensitivity
 *
 * @param [in]     handle              A handle to the MotionAngle object.
 * @param [in]     sensitivity         Sensitivity [0-1], min to max.
 */
IFX_DLL_PUBLIC
void ifx_motionangle_set_sensitivity(ifx_MotionAngle_t* handle, ifx_Float_t sensitivity);

/**
 * @brief Get sensitivity, default is 0.5
 *
 * @param [in]     handle              A handle to the MotionAngle object.
 * @return sensitivity.
 */
IFX_DLL_PUBLIC
ifx_Float_t ifx_motionangle_get_sensitivity(ifx_MotionAngle_t* handle);

/**
 * @brief Set handle
 *
 * @param [in]     handle              A handle to the MotionAngle object.
 * @param [in]     maxrange            Max range in [m]
 */
IFX_DLL_PUBLIC
void ifx_motionangle_set_maxrange(ifx_MotionAngle_t* handle, ifx_Float_t maxrange);

/**
 * @brief Get maxrange
 *
 * @param [in]     handle              A handle to the MotionAngle object.
 * @return maxrange.
 */
IFX_DLL_PUBLIC
ifx_Float_t ifx_motionangle_get_maxrange(ifx_MotionAngle_t* handle);

/**
 * @brief Performs the MotionAngle algorithm.
 *
 * The input is a cube with dimensions num_rx_antennas x num_chirps_per_frame x num_samples_per_chirp.
 *
 * The result is updated after this call.
 *
 * @param [in]     handle              A handle to the MotionAngle object.
 * @param [in]     frame               Input raw data, a cube (\ref ifx_Cube_R_t) of dimensions:
 *                                     num_rx_antennas (rows) x num_chirps_per_frame (cols) x num_samples_per_chirp (slices).
 * @param [out]    result              Result.
 */
IFX_DLL_PUBLIC
void ifx_motionangle_run(ifx_MotionAngle_t* handle, const ifx_Cube_R_t* frame, ifx_MotionAngle_Result_t* result);

/**
 * @brief Destroys MotionAngle instance.
 *
 * @param [in]  handle        Instance of Motion-Angle.
 */
IFX_DLL_PUBLIC
void ifx_motionangle_destroy(ifx_MotionAngle_t* handle);

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* IFX_MOTIONANGLE_H */
