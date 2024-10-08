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
 * @file Types.h
 *
 * @brief Definitions of data types used within the SDK.
 */

#ifndef IFX_BASE_TYPES_H
#define IFX_BASE_TYPES_H


/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#ifdef __cplusplus
#include <cstddef>  // for size_t
#include <cstdint>

extern "C"
{
#else
#include <stdbool.h>
#include <stddef.h>  // for size_t
#include <stdint.h>
#endif

/*
==============================================================================
   2. DEFINITIONS
==============================================================================
*/

/* On Windows symbols (functions) in a DLL (shared library) are not exported by
 * default. You have to tell the linker to export the symbol when you build the
 * library and you have to tell the linker (?) to import the symbol when using
 * the library. This is done with __declspec:
 *     Compiling the library: __declspec(dllexport) void foo(void);
 *     Using the library:     __declspec(dllimport) void foo(void);
 * More information can be found here:
 * https://stackoverflow.com/questions/33062728/cmake-link-shared-library-on-windows/41618677
 *
 * In contrast, on Linux the default is to export all symbols. For gcc
 * (probably also clang), see https://gcc.gnu.org/wiki/Visibility.
 *
 * The build system has to set correct preprocessor defines (cmake works).
 * - If the library is build as a static library, no preprocessor name is
 *   needed.
 * - If the library is build as a dynamic library:
 *      - Windows: radar_sdk_EXPORTS must be set when compiling the library
 *      - Windows: radar_sdk_EXPORTS must not be set when linking
 *      - Linux: No preprocessor defines are needed
 */
#ifdef RADAR_SDK_BUILD_STATIC
// build as static library; no visibility
#define IFX_DLL_PUBLIC
#define IFX_DLL_HIDDEN
#elif defined(_MSC_VER) || defined(__MINGW64__) || defined(__WIN32__)
// default visibility is hidden, so IFX_DLL_HIDDEN is a noop
#define IFX_DLL_HIDDEN

#ifndef IFX_DLL_PUBLIC
#ifdef radar_sdk_EXPORTS
// We are building this library
#define IFX_DLL_PUBLIC __declspec(dllexport)
#else
// We are using this library
#define IFX_DLL_PUBLIC __declspec(dllimport)
#endif
#endif
#elif (__GNUC__ >= 4) || (__clang_major__ >= 5)
// see https://gcc.gnu.org/wiki/Visibility
#define IFX_DLL_PUBLIC __attribute__((visibility("default")))
#define IFX_DLL_HIDDEN __attribute__((visibility("hidden")))
#else
#define IFX_DLL_PUBLIC
#define IFX_DLL_HIDDEN
#endif

#define IFX_DLL_TEST IFX_DLL_PUBLIC

/** Speed of light in m/s */
#define IFX_LIGHT_SPEED_MPS ((ifx_Float_t)(299792458U))

// Macro to mark typedefs as deprecated.
// Example: IFX_TYPEDEF_DEPRECATED("my_int will be removed in RDK X.Y", int, my_int);
#ifdef _MSC_VER
#define IFX_TYPEDEF_DEPRECATED(msg, a, b) typedef __declspec(deprecated(msg)) a b
#elif defined(__GNUC__) || defined(__clang__)
#define IFX_TYPEDEF_DEPRECATED(msg, a, b) typedef a b __attribute__((deprecated))
#else
#define IFX_TYPEDEF_DEPRECATED(msg, a, b) typedef a b
#endif

// Macro to mark functions as deprecated.
// Example: IFX_FUNCTION_DEPRECATED("function will be removed in RDK X.Y", IFX_DLL_PUBLIC void my_outdated_function(int x, float y));
#if defined(__GNUC__) || defined(__clang__)
#define IFX_FUNCTION_DEPRECATED(msg, X) X __attribute__((deprecated))
#elif defined(_MSC_VER)
#define IFX_FUNCTION_DEPRECATED(msg, X) __declspec(deprecated(msg)) X
#else
#define IFX_FUNCTION_DEPRECATED(msg, X) X
#endif

/*
==============================================================================
   3. TYPES
==============================================================================
*/

typedef float ifx_Float_t;

/** @addtogroup gr_cat_SDK_base
 * @{
 */

/** @defgroup gr_types Types
 * @brief Definitions of data types used within the SDK.
 * @{
 */

/**
 * @brief Defines the structure for Complex data core parameters.
 *        Use type ifx_Complex_t for this struct.
 */
struct ifx_Complex_s
{
    ifx_Float_t data[2];
};

/**
 * @brief Defines the structure for Polar form.
 *        Use type ifx_Polar_t for this struct.
 */
struct ifx_Polar_s
{
    ifx_Float_t radius; /**< Radius.*/
    ifx_Float_t angle;  /**< Angle.*/
};

/**
 * @brief Polar type.
 */
typedef struct ifx_Polar_s ifx_Polar_t;

/**
 * @brief Complex number.
 */
typedef struct ifx_Complex_s ifx_Complex_t;

/*
==============================================================================
   4. FUNCTION PROTOTYPES
==============================================================================
*/

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* IFX_BASE_TYPES_H */
