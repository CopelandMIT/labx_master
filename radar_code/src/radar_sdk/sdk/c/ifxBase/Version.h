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
 * @file Version.h
 *
 * @brief Version information of SDK
 *
 */

#ifndef IFX_BASE_SDK_VERSION_H
#define IFX_BASE_SDK_VERSION_H

#include "Types.h"


#ifdef __cplusplus
extern "C"
{
#endif


/*
==============================================================================
   4. FUNCTION PROTOTYPES
==============================================================================
*/

/** @addtogroup gr_cat_SDK_base
 * @{
 */

/** @defgroup gr_sdk_version Version
 * @brief SDK version query
 * @{
 */

/**
 * @brief Returns the radar sdk version as a string.
 *
 * The returned string has the format "vX.Y.Z" where X is the major number,
 * Y is the minor number, and Z is the patch level.
 *
 * @return Version string.
 */

IFX_DLL_PUBLIC
const char* ifx_sdk_get_version_string(void);

/**
 * @brief Returns the radar sdk full version as a string.
 *
 * The returned string has the format "vX.Y.Z-tag+N.hash" where
 * X is the major number, Y is the minor number, Z is the patch level,
 * tag is the name of the last release tag, N is a number how many commits
 * the current version is ahead of the git tag 'tag', and hash is the git hash
 * used to create this version of the SDK.
 *
 * @return Full version string.
 */
IFX_DLL_PUBLIC
const char* ifx_sdk_get_version_string_full(void);

/**
 * @brief Returns the SDK version hash as a string.
 *
 * Return the git version hash used to generate this version of the SDK.
 *
 * @return Version hash.
 */
IFX_DLL_PUBLIC
const char* ifx_sdk_get_version_hash(void);

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* IFX_BASE_SDK_VERSION_H */
