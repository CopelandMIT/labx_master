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
 * @file DBF.h
 *
 * \brief \copybrief gr_dbf
 *
 * For details refer to \ref gr_dbf
 */

#ifndef IFX_RADAR_DBF_H
#define IFX_RADAR_DBF_H

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
 * @brief A handle for an instance of DigitalBeamForming module, see DBF.h
 */
typedef struct ifx_DBF_s ifx_DBF_t;

/**
 * @brief Defines the structure for DBF module related settings.
 */
typedef struct
{
    uint8_t num_beams;       /**< Number of beams.*/
    uint8_t num_antennas;    /**< Number of antennas.*/
    ifx_Float_t min_angle;   /**< Minimum angle on left side of FoV.*/
    ifx_Float_t max_angle;   /**< Maximum angle on right side of FoV.*/
    ifx_Float_t d_by_lambda; /**< Ratio between antenna spacing 'd' and wavelength.*/
} ifx_DBF_Config_t;

/*
==============================================================================
   4. FUNCTION PROTOTYPES
==============================================================================
*/

/** @addtogroup gr_cat_Radar
 * @{
 */

/** @defgroup gr_dbf DBF
 * @brief API for Digital Beamforming (DBF) algorithm
 *
 * Digital Beamforming (DBF) is a method to focus a radar transmitter or receiver
 * in a certain direction in 3D space. In this context, the left to right direction
 * is commonly referred to as azimuth and the high to low direction as elevation.
 * The antennas of DBF systems are set up as a Uniform Linear Array (ULA) while
 * each direction needs a ULA of at least two antennas positioned in its plane to
 * perform beamforming. Thus, the minimum requirement for detecting azimuth and
 * elevation is a ULA with three antennas in an L shape.
 * In DBF all ULA's antenna signals are sampled and therefore present for processing
 * at the same time. Thus DBF allows for a precise and simultaneous processing of
 * different angles and frequencies, only limited by the computational effort, but
 * theoretically with arbitrary precision. The theory and implementation of DBF
 * algorithms as described here can not only be used to analyze and weight signals
 * in 3D space but also for FFT processed spectral representations or derived
 * properties like range doppler maps.
 *
 * Input to this module is a cube i.e. an array of range doppler spectrums
 * (complex values) over all rx antennas and ouput is also a complex cube
 * i.e. stack of matrices (of dimension equal to the dimension of range doppler
 * spectrum) and number of slices of cube are equal to the number of beams.
 *
 * An algorithm explanation is also available at the \ref ssct_radarsdk_algorithms_dbf SDK documentation.
 *
 * @{
 */

/**
 * @brief Creates a DBF handle (object), based on the input parameters
 *
 * @param [in]     config    DBF configurations defined by \ref ifx_DBF_Config_t.
 *
 * @return Handle to the newly created instance or NULL in case of failure.
 *
 */
IFX_DLL_PUBLIC
ifx_DBF_t* ifx_dbf_create(const ifx_DBF_Config_t* config);

/**
 * @brief Computes beams for a given range Doppler spectrum overs across Rx antennas.
 *
 * @param [in]     handle              A handle to the DBF object
 * @param [in]     rng_dopp_spectrum   A complex Cube (3D) of range Doppler spectrum for all Rx channels i.e.
 *                                     (Nsamples x NumChirps x Number of Antennas)
 * @param [out]    rng_dopp_image_beam A complex Cube (3D) containing range Doppler image beams i.e.
 *                                     (Nsamples x NumChirps x NumberofBeams)
 *
 */
IFX_DLL_PUBLIC
void ifx_dbf_run_c(ifx_DBF_t* handle,
                   const ifx_Cube_C_t* rng_dopp_spectrum,
                   ifx_Cube_C_t* rng_dopp_image_beam);

/**
 * @brief Performs destruction of DBF handle (object) to clear internal states and memories.
 *
 * @param [in]     handle    A handle to the DBF object
 *
 */
IFX_DLL_PUBLIC
void ifx_dbf_destroy(ifx_DBF_t* handle);

/**
 * @brief Returns number of beams configured in the DBF handle
 *
 * @param [in]     handle    A handle to the DBF object
 *
 * @return  Currently configured number of beams
 *
 */
IFX_DLL_PUBLIC
uint32_t ifx_dbf_get_beam_count(ifx_DBF_t* handle);

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* IFX_RADAR_DBF_H */
