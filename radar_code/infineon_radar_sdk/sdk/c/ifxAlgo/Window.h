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
 * @file Window.h
 *
 * \brief \copybrief gr_window
 *
 * For details refer to \ref gr_window
 */

#ifndef IFX_ALGO_WINDOW_H
#define IFX_ALGO_WINDOW_H

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
 * @brief Defines supported Window options.
 */
typedef enum
{
    IFX_WINDOW_HAMM = 0U,           /**< Hamming window */
    IFX_WINDOW_HANN = 1U,           /**< Hanning window */
    IFX_WINDOW_BLACKMANHARRIS = 2U, /**< Blackmann Harris window */
    IFX_WINDOW_CHEBYSHEV = 3U,      /**< Chebyshev window */
    IFX_WINDOW_BLACKMAN = 4U,       /**< Blackman window */
} ifx_Window_Type_t;

/**
 * @brief Defines the structure for Window module related settings.
 */
typedef struct
{
    ifx_Window_Type_t type; /**< Type of window function defined by \ref ifx_Window_Type_t */
    uint32_t size;          /**< Number of elements in the window */
    ifx_Float_t at_dB;      /**< Attenuation parameter, in case of Chebyshev window.
                                 Defines the attenuation in dBs required to generate
                                 the pass band ripple for a Chebyshev window.
                                 This must be a positive number. */
    ifx_Float_t scale;      /**< Scale factor of all elements inside the window */
} ifx_Window_Config_t;

/*
==============================================================================
   4. FUNCTION PROTOTYPES
==============================================================================
*/

/** @addtogroup gr_cat_Algorithms
 * @{
 */

/** @defgroup gr_window Window
 * @brief API for Window function processing
 *
 * Supports various Window functions and their coefficients:
 * - Hamming
 * - Hanning
 * - Blackman
 * - Blackman-Harris
 * - Chebyshev
 *
 * Note: Dolph Chebyshev Window is implemented according to:
 * http://practicalcryptography.com/miscellaneous/machine-learning/implementing-dolph-chebyshev-window/
 *
 * @{
 */

/**
 * @brief Generates the coefficients of a user selected window for a given length.
 *
 * @param [in]     config    \ref ifx_Window_Config_t "Window configuration structure" defining the type of window,
 *                           the number of elements in the window and also
 *                           any additional parameter specific to certain window type.
 * @param [in,out] win       Pointer to an allocated and populated vector instance defined by \ref ifx_Vector_R_t
 *                           filled with Window coefficients defined in \ref ifx_Window_Config_t.
 */
IFX_DLL_PUBLIC
void ifx_window_init(const ifx_Window_Config_t* config,
                     ifx_Vector_R_t* win);

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* IFX_ALGO_WINDOW_H */
