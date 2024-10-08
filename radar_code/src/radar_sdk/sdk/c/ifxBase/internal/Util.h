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

#ifndef IFX_BASE_UTIL_INTERNAL_H
#define IFX_BASE_UTIL_INTERNAL_H

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include "../Types.h"


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

/*
==============================================================================
   4. FUNCTION PROTOTYPES
==============================================================================
*/

/*
 * @brief Create copy of string
 *
 * This function returns a pointer to a new string which is a duplicate of the string orig.
 * Memory for the new string is obtained with ifx_mem_alloc, and can be freed with ifx_mem_free.
 *
 * The caller is responsible to free the memory.
 *
 * @param [in]  orig    input string
 * @retval      copy    copy of string if successful
 * @retval      NULL    on errors
 */
IFX_DLL_PUBLIC
char* ifx_util_strdup(const char* orig);

/*
 * @brief Return number of bits set to 1 in variable
 *
 * Returns the number of bits that are set to 1 in the variable mask.
 *
 * @param [in]	mask	input
 * @retval	number of one bits in mask
 */
IFX_DLL_PUBLIC
uint32_t ifx_util_popcount(uint32_t mask);


#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* IFX_BASE_UTIL_INTERNAL_H */
