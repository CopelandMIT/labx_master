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
 * @file AngleMonopulse.h
 *
 * \brief \copybrief gr_anglemonopulse
 *
 * For details refer to \ref gr_anglemonopulse
 */

#ifndef IFX_RADAR_ANGLE_MONOPULSE_H
#define IFX_RADAR_ANGLE_MONOPULSE_H

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include "ifxBase/Complex.h"
#include "ifxBase/Types.h"
#include "ifxBase/Vector.h"


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
 * @brief A handle for an instance of AngleMonopulse module, see AngleMonopulse.h
 */
typedef struct ifx_AngleMonopulse_s ifx_AngleMonopulse_t;

/*
==============================================================================
   4. FUNCTION PROTOTYPES
==============================================================================
*/

/** @addtogroup gr_cat_Radar
 * @{
 */

/** @defgroup gr_anglemonopulse Angle Monopulse
 * @brief API for Phase Monopulse algorithm
 *
 * Phase Monopulse algorithm is used for angle calculation.
 *
 * Interface provides as input:
 * - A vector of desired range/doppler FFT cells (complex number) for antenna x
 * - A vector of desired range/doppler FFT cells (complex number) for antenna y
 * - Each entry of the two vectors represents the same detected target we want to have the angle information for
 * Interface provides following configuration parameter:
 * - wavelength or center frequency
 * - distance of the two antennas
 * Interface provides as output:
 * - vector of angles in degrees for each row (target) the two input vectors
 *
 * @{
 */

/**
 * @brief Creates an angle monopulse handle, to contain two key parameters i.e. wavelength and antenna spacing.
 *
 * @param [in]     wavelength          Wavelength (in units of meters), computed from center frequency and speed of light
 * @param [in]     antenna_spacing     Physical spacing between antennas on BGT radar chip (in units of meters)
 *
 * @return Handle to the newly created instance or NULL in case of failure.
 *
 */
IFX_DLL_PUBLIC
ifx_AngleMonopulse_t* ifx_anglemonopulse_create(ifx_Float_t wavelength,
                                                ifx_Float_t antenna_spacing);

/**
 * @brief Destroys the angle monopulse handle.
 *
 * @param [in,out] handle    A handle to the angle monopulse object
 *
 */
IFX_DLL_PUBLIC
void ifx_anglemonopulse_destroy(ifx_AngleMonopulse_t* handle);

/**
 * @brief Based on complex input scalar from two receiver antennas, a corresponding
 *        angle scalar (in units of degrees) is calculated using phase monopulse algorithm.
 *
 * @param [in]     handle    A handle to the angle monopulse object
 * @param [in]     rx1       First Rx antenna used as numerator argument in algorithm
 * @param [in]     rx2       Second Rx antenna used as denominator argument in algorithm
 *
 * @return Real value corresponding to the angle in degrees for a given inputs Rx1 & Rx2.
 *
 */
IFX_DLL_PUBLIC
ifx_Float_t ifx_anglemonopulse_scalar_run(const ifx_AngleMonopulse_t* handle,
                                          ifx_Complex_t rx1,
                                          ifx_Complex_t rx2);

/**
 * @brief Based on complex input vector from two receiver antennas, a corresponding
 *        angle vector (in units of degrees) is calculated using phase monopulse algorithm.
 *
 * @param [in]     handle              A handle to the angle monopulse object
 * @param [in]     rx1                 First Rx antenna used as numerator argument in algorithm
 * @param [in]     rx2                 Second Rx antenna used as denominator argument in algorithm
 * @param [out]    target_angle_deg    Output vector of real values corresponding to the angle (in units of degrees)
 *                                     for a given input Rx1 & Rx2 vector
 *
 */
IFX_DLL_PUBLIC
void ifx_anglemonopulse_vector_run(const ifx_AngleMonopulse_t* handle,
                                   const ifx_Vector_C_t* rx1,
                                   const ifx_Vector_C_t* rx2,
                                   ifx_Vector_R_t* target_angle_deg);

/**
 * @brief Sets the new value of wavelength used in angle calculation
 *
 * @param [in]     handle              A handle to the angle monopulse object
 * @param [in]     wavelength          New value of wavelength (in units of meters) to be used by the monopulse algorithm
 *
 */
IFX_DLL_PUBLIC
void ifx_anglemonopulse_set_wavelength(ifx_AngleMonopulse_t* handle,
                                       ifx_Float_t wavelength);

/**
 * @brief Returns the current value of wavelength used in angle calculation
 *
 * @param [in]     handle              A handle to the angle monopulse object
 *
 * @return Value of wavelength (in units of meters) used by the algorithm.
 *
 */
IFX_DLL_PUBLIC
ifx_Float_t ifx_anglemonopulse_get_wavelength(const ifx_AngleMonopulse_t* handle);

/**
 * @brief Sets the new value of antenna spacing used in angle calculation
 *
 * @param [in]     handle              A handle to the angle monopulse object
 * @param [in]     antenna_spacing     New value of antenna spacing (in units of meters) to be used by the monopulse algorithm
 *
 */
IFX_DLL_PUBLIC
void ifx_anglemonopulse_set_antenna_spacing(ifx_AngleMonopulse_t* handle,
                                            ifx_Float_t antenna_spacing);

/**
 * @brief Returns the current value of antenna spacing used in angle calculation
 *
 * @param [in]     handle    A handle to the angle monopulse object
 *
 * @return Value of antenna spacing (in units of meters) used by the algorithm.
 *
 */
IFX_DLL_PUBLIC
ifx_Float_t ifx_anglemonopulse_get_antenna_spacing(const ifx_AngleMonopulse_t* handle);

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* IFX_RADAR_ANGLE_MONOPULSE_H */
