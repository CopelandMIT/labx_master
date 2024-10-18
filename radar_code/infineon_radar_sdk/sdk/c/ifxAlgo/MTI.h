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
 * @file MTI.h
 *
 * \brief \copybrief gr_mti
 *
 * For details refer to \ref gr_mti
 */

#ifndef IFX_ALGO_MTI_H
#define IFX_ALGO_MTI_H

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

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
 * @brief A handle for an instance of MTI module, see MTI.h.
 */
typedef struct ifx_MTI_s ifx_MTI_t;

/*
==============================================================================
   4. FUNCTION PROTOTYPES
==============================================================================
*/

/** @addtogroup gr_cat_Algorithms
 * @{
 */

/** @defgroup gr_mti Moving Target Indicator (MTI)
 * @brief API for Moving Target Indicator (MTI) filter.
 *
 * @if ssct_radarsdk_algorithms_mti
 * An algorithm explanation is also available at the \ref ssct_radarsdk_algorithms_mti SDK documentation.
 * @endif
 *
 * @{
 */

/**
 * @brief Creates a new MTI filter
 *
 * Instantiate an MTI filter \ref ifx_MTI_t which contains information
 * required to perform MTI (Moving Target Indication) filtering.
 *
 * @param [in]     alpha_mti_filter    Filter coefficient. Valid between 0.0 and 1.0
 * @param [in]     spectrum_length     Length of input / output vectors used within MTI filter
 *
 * @return Pointer to MTI structure of NULL in case of failure.
 *
 */
IFX_DLL_PUBLIC
ifx_MTI_t* ifx_mti_create(ifx_Float_t alpha_mti_filter,
                          uint32_t spectrum_length);

/**
 * @brief Destroys the MTI filter handle.
 *
 * @param [in,out] mti    Pointer to MTI structure.
 *
 */
IFX_DLL_PUBLIC
void ifx_mti_destroy(ifx_MTI_t* mti);

/**
 * @brief Uses a valid MTI filter handle to filter an input vector of real values.
 *
 * @param [in]     mti       Pointer to MTI structure.
 * @param [in]     input     Real value vector used as an input for MTI filter
 * @param [out]    output    Real value vector used as an output of MTI filter
 *
 */
IFX_DLL_PUBLIC
void ifx_mti_run(ifx_MTI_t* mti,
                 const ifx_Vector_R_t* input,
                 ifx_Vector_R_t* output);

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* IFX_ALGO_MTI_H */
