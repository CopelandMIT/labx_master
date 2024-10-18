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
 * @file Defines.h
 *
 * @brief Internal definitions used by standard math.h functions as Macros,
 *        to use float or double precision.
 */

#ifndef IFX_BASE_DEFINES_H
#define IFX_BASE_DEFINES_H

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/
#include <math.h>  // NOLINT(modernize-deprecated-headers)


#ifdef __cplusplus
extern "C"
{
#endif


/*
==============================================================================
   2. DEFINITIONS
==============================================================================
*/

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

// Ignore variable if unused (prevents compiler warnings about unused variable)
#define IFX_IGNORE(x) (void)(x)

/* M_PI is not defined by the C standard. E.g., with gcc and -std=gnu11 it is
 * defined, with -std=c99 it isn't. Therefore, don't rely on M_PI.
 */
#define IFX_PI ((ifx_Float_t)(3.14159265358979323846))

/**
 * NaN on types on infineon
 */
#define IFX_NAN NAN

/**
 * Infinity on types on infineon
 */
#define IFX_INF_POS INFINITY
#define IFX_INF     IFX_INF_POS

/**
 * -Infinity on types on infineon
 */
#define IFX_INF_NEG (-INFINITY)


// macro replacement for sincos and sincosf
#define SINCOS(x, s, c)      \
    do                       \
    {                        \
        ifx_Float_t arg = x; \
        *(s) = SIN(arg);     \
        *(c) = COS(arg);     \
    } while (0)

#define CABS      cabsf
#define FABS      fabsf
#define CREAL     crealf
#define CIMAG     cimagf
#define LOG10     log10f
#define LOGN      logf
#define LOG1P     log1pf
#define SIN       sinf
#define ASIN      asinf
#define SINH      sinhf
#define ASINH     asinhf
#define COS       cosf
#define ACOS      acosf
#define ATAN      atanf
#define ATAN2     atan2f
#define COSH      coshf
#define ACOSH     acoshf
#define FLOOR     floorf
#define CEIL      ceilf
#define ROUND     roundf
#define HYPOT     hypotf
#define POW       powf
#define SQRT      sqrtf
#define SIND(x)   (sinf(fmodf((x), 360) * IFX_PI / 180))
#define TGAMMA(x) tgammaf(x)
#define EXP(x)    expf(x)
#define COPYSIGN  copysignf
#define TAN       tanf

#ifdef DISABLE_THREAD_LOCAL
// IFX_THREAD_LOCAL is a noop
#define IFX_THREAD_LOCAL
#else
#if defined(__GNUC__)
#define IFX_THREAD_LOCAL __thread
#elif defined(_MSC_VER)
#define IFX_THREAD_LOCAL __declspec(thread)
#endif
#endif

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

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* IFX_BASE_DEFINES_H */
