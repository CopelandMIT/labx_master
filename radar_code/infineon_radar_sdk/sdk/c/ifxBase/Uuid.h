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

#ifndef IFX_BASE_UUID_H
#define IFX_BASE_UUID_H

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include "Types.h"


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

/**
 * @brief Convert string to uuid
 *
 * Convert string to a uint8_t uuid buffer.
 *
 * @param [in]  string  string representation of uuid
 * @param [out] uuid    array representation of uuid
 * @retval true         if conversion was successful
 * @retval false        if the string is not a valid uuid
 */
IFX_DLL_PUBLIC
bool ifx_uuid_from_string(const char* string, uint8_t uuid[16]);

/**
 * @brief Convert uuid to string
 *
 * Convert the uint8_t uuid buffer to the canonical string representation. The
 * uuid is written to string. The pointer string must have enough space for at
 * least 37 characters (36 characters of the uuid, and a terminating null
 * character).
 *
 * @param [in]  uuid    array representation of uuid
 * @param [out] string  string representation of uuid
 */
IFX_DLL_PUBLIC
void ifx_uuid_to_string(const uint8_t uuid[16], char* string);


#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* IFX_BASE_UUID_H */
