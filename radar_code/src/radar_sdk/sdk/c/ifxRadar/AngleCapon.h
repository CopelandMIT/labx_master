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
 * @file AngleCapon.h
 *
 * \brief \copybrief gr_anglecapon
 *
 * For details refer to \ref gr_anglecapon
 */

#ifndef IFX_RADAR_ANGLE_CAPON_H
#define IFX_RADAR_ANGLE_CAPON_H

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include "ifxBase/Cube.h"
#include "ifxBase/Types.h"


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

/**
 * @brief A handle for an instance of AngleCapon module, see AngleCapon.h
 */
typedef struct ifx_AngleCapon_s ifx_AngleCapon_t;

/**
 * @brief Defines the structure for Angle Capon module related settings.
 */
typedef struct
{
    uint8_t range_win_size;           /**< Range window size. This defines range gate width. A typical value is `5`*/
    uint8_t selected_rx;              /**< Selected Rx Antenna for Doppler. Selects the best Rx channel (antenna)
                                           for finding and choosing proper Doppler index. This index is used to select*/
    uint16_t chirps_per_frame;        /**< Number of chirps per frame. This depends on the Radar configuration. Recommended value is `64`*/
    ifx_Float_t phase_offset_degrees; /**< Phase offset compensation. This is the phase compensation value between used Rx antennas in degrees. Typical value is `0.0`*/
    uint8_t num_virtual_antennas;     /**< Virtual number of antennas. For BGT60 devices, this corresponds to the number of activated
                                           antennas on a single axis making the typical value `2`*/
    uint8_t num_beams;                /**< Number of beams. This defines the search granularity of the target angle.
                                           The angle between the min_angle_degrees and max_angle_degrees is divided into
                                           num_beams sections*/
    ifx_Float_t min_angle_degrees;    /**< Minimum angle. The angle on left side of FoV in degrees.*/
    ifx_Float_t max_angle_degrees;    /**< Maximum angle. The angle on right side of FoV in degrees.*/
    ifx_Float_t d_by_lambda;          /**< Ratio between antenna spacing 'd' and wavelength of the Radar's operating
                                           frequency. For BGT60 Devices this is `0.5` and the algorithm is optimized for this value*/
} ifx_AngleCapon_Config_t;

/*
==============================================================================
   4. FUNCTION PROTOTYPES
==============================================================================
*/

/** @addtogroup gr_cat_Radar
 * @{
 */

/** @defgroup gr_anglecapon Angle Capon
 * @brief API for the 1-D Angle Capon algorithm
 *
 * This algorithm is used to generate the angle estimation of target in a single dimension.
 * The range FFT and the target range estimate information needs to be available beforehand.
 *
 * A brief explanation of the algorithm is available at the \ref ssct_radarsdk_algorithms_detect_capon SDK documentation.
 *
 * @{
 */

/**
 * @brief Creates a AngleCapon handle (object), based on the input parameters.
 *
 * The following figure shows the dependencies of the Capon object properties on the configuration parameters.
 * The yellow colored blocks depict scalars while the peach colored blocks are vectors and matrices for which memory
 * is allocated.
 *
 *  @image html img_capon_create.png "Capon create object" width=800px
 *
 * A depiction of the Capon algorithm's main configuration parameters is shown in the figure below. The number
 * of beams is `9` in this example. Minimum and maximum angles are measured from the axis perpendicular to the
 * axis of the antenna array elements.
 *
 *  @image html img_capon_figure.png "Capon algorithm configuration" width=800px
 *
 * @param [in]     config              AngleCapon configurations defined by \ref ifx_AngleCapon_Config_t.
 *
 * @return Handle to the newly created instance or NULL in case of failure.
 *
 */
IFX_DLL_PUBLIC
ifx_AngleCapon_t* ifx_anglecapon_create(const ifx_AngleCapon_Config_t* config);

/**
 * @brief Runs angle capon algorithm, based on the input parameters.
 *
 * The function expects the RX spectrum returned by \ref ifx_rai_get_rx_spectrum
 * as input and computes the angle of arrival of a target for a given distance.
 * The distance is given in terms of the range bin. The distance and the range bin are related by
 * \f[
 * \mathrm{distance} = \mathrm{range\_bin} \times \mathrm{distance\_per\_bin}
 * \f]
 * where \f$\mathrm{distance\_per\_bin}\f$ can be computed using \ref ifx_spectrum_axis_calc_dist_per_bin.
 *
 * The process flow is shown in the following figure:
 *
 * @image html img_capon_run.png "Capon processing flow" width=800px
 *
 * @param [in]     handle              A handle to the AngleCapon object
 * @param [in]     range_bin           Index derived from Range FFT where the target is estimated to be. This index
 *                                     should lie within the range spectrum limits.
 * @param [in]     rx_spectrum         Range spectrum returned by \ref ifx_rai_get_rx_spectrum
 *
 * @return Angle value in degrees.
 *
 */
IFX_DLL_PUBLIC
ifx_Float_t ifx_anglecapon_run(const ifx_AngleCapon_t* handle,
                               uint32_t range_bin,
                               const ifx_Cube_C_t* rx_spectrum);

/**
 * @brief Destroys AngleCapon handle (object) to clear internal states and memories.
 *
 * @param [in]     handle              A handle to the AngleCapon object
 *
 */
IFX_DLL_PUBLIC
void ifx_anglecapon_destroy(ifx_AngleCapon_t* handle);

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* IFX_RADAR_ANGLE_CAPON_H */
