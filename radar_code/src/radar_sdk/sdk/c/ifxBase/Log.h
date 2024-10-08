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
 * @file Log.h
 *
 * \brief \copybrief gr_log
 *
 * For details refer to \ref gr_log
 */

#ifndef IFX_BASE_LOG_H
#define IFX_BASE_LOG_H

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#ifndef IFX_STDOUT
#include <stdio.h>  // NOLINT(modernize-deprecated-headers)
#define IFX_STDOUT stderr
#endif

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

#if defined(IFX_LOG_SEVERITY_DEBUG)
#define IFX_LOG_DEBUG(...) ifx_log(IFX_STDOUT, IFX_LOG_DEBUG, __VA_ARGS__)
#else
#define IFX_LOG_DEBUG(...)
#endif

#if defined(IFX_LOG_SEVERITY_INFO) \
    || defined(IFX_LOG_SEVERITY_DEBUG)
#define IFX_LOG_INFO(...) ifx_log(IFX_STDOUT, IFX_LOG_INFO, __VA_ARGS__)
#else
#define IFX_LOG_INFO(...)
#endif

#if defined(IFX_LOG_SEVERITY_INFO)       \
    || defined(IFX_LOG_SEVERITY_WARNING) \
    || defined(IFX_LOG_SEVERITY_DEBUG)
#define IFX_LOG_WARNING(...) ifx_log(IFX_STDOUT, IFX_LOG_WARNING, __VA_ARGS__)
#else
#define IFX_LOG_WARNING(...)
#endif

#if defined(IFX_LOG_SEVERITY_INFO)       \
    || defined(IFX_LOG_SEVERITY_WARNING) \
    || defined(IFX_LOG_SEVERITY_ERROR)   \
    || defined(IFX_LOG_SEVERITY_DEBUG)
#define IFX_LOG_ERROR(...) ifx_log(IFX_STDOUT, IFX_LOG_ERROR, __VA_ARGS__)
#else
#define IFX_LOG_ERROR(...)
#endif

/*
==============================================================================
   3. TYPES
==============================================================================
*/

/**
 * @brief Defines supported Log options.
 */
typedef enum
{
    IFX_LOG_INFO,
    IFX_LOG_WARNING,
    IFX_LOG_ERROR,
    IFX_LOG_DEBUG
} ifx_Log_Severity_t;

/*
==============================================================================
   4. FUNCTION PROTOTYPES
==============================================================================
*/

/** @addtogroup gr_cat_SDK_base
 * @{
 */

/** @defgroup gr_log Log
 * @brief API for logging
 * @{
 */

IFX_DLL_PUBLIC
void ifx_log(FILE* f, ifx_Log_Severity_t s, const char* msg, ...);

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* IFX_BASE_LOG_H */
