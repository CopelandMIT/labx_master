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
 * @file OSCFAR.h
 *
 * \brief \copybrief gr_oscfar
 *
 * For details refer to \ref gr_oscfar
 */

#ifndef IFX_ALGO_OSCFAR_H
#define IFX_ALGO_OSCFAR_H

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include "ifxBase/Matrix.h"
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
 * @brief A handle for an instance of OSCFAR module, see OSCFAR.h.
 */
typedef struct ifx_OSCFAR_s ifx_OSCFAR_t;

/**
 * @brief Defines the structure for OSCFAR module related settings.
 */
typedef struct
{
    uint8_t win_rank;          /**< Rank of CFAR reference window.*/
    uint8_t guard_band;        /**< Rank of CFAR guard band.*/
    ifx_Float_t sample;        /**< Constant used for setting CFAR threshold.*/
    ifx_Float_t pfa;           /**< Probability of false alarm.*/
    ifx_Float_t coarse_scalar; /**< Used for coarse thresholding 2D feature map.*/
} ifx_OSCFAR_Config_t;

/*
==============================================================================
   4. FUNCTION PROTOTYPES
==============================================================================
*/

/** @addtogroup gr_cat_Algorithms
 * @{
 */

/** @defgroup gr_oscfar OS-CFAR
 * @brief API for 2D ordered statistic constant false alarm rate (OS-CFAR) algorithm.
 *
 * Input of this module is a 2D matrix of real values, i.e. range angle map.
 * Output is the a 2D matrix of same size as Input with filtered contents.
 *
 * @if ssct_radarsdk_algorithms_detect_oscfar
 * An algorithm explanation is available at the \ref ssct_radarsdk_algorithms_detect_oscfar SDK documentation.
 * @endif
 *
 * @{
 */

/**
 * @brief Creates a OSCFAR handle (object), based on the input parameters.
 *
 * @param [in]     config    OSCFAR configurations defined by \ref ifx_OSCFAR_Config_t.
 *
 * @return Handle to the newly created instance or NULL in case of failure.
 *
 */
IFX_DLL_PUBLIC
ifx_OSCFAR_t* ifx_oscfar_create(const ifx_OSCFAR_Config_t* config);

/**
 * @brief Runs OS_CFAR algorithm, based on the input parameters.
 *
 * @param [in]     handle              A handle to the OSCFAR object
 * @param [in]     feature2D           rangeAngle/rangeDoppler 2D feature maps with dimensions i.e.
 *                                     rangeAngle: (numOfSamplesPerChirp/2,numOfBeams)
 *                                     rangeDoppler: (numOfSamplesPerChirp/2,numChirpsPerFrame)
 * @param [out] detector_output        Appropriate 2D feature map with updated target indices.
 *
 */
IFX_DLL_PUBLIC
void ifx_oscfar_run(const ifx_OSCFAR_t* handle,
                    ifx_Matrix_R_t* feature2D,
                    ifx_Matrix_R_t* detector_output);

/**
 * @brief Destroys OSCFAR handle (object) to clear internal states and memories.
 *
 * @param [in]     handle    A handle to the OSCFAR object
 *
 */
IFX_DLL_PUBLIC
void ifx_oscfar_destroy(ifx_OSCFAR_t* handle);

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* IFX_ALGO_OSCFAR_H */
