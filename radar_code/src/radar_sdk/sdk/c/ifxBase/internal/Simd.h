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

#ifndef IFX_SIMD_H
#define IFX_SIMD_H

// __SSE2__ is not defined by MSVC. Windows 8 and later requires SSE2. So, if
// _WIN64 is defined we can assume that SSE2 is also available.
#if defined(__SSE2__) || defined(_WIN64)
#include <xmmintrin.h>

#define IFX_SSE2

#define vf32x4                     __m128
#define vf32x4_set(e3, e2, e1, e0) _mm_set_ps((e3), (e2), (e1), (e0))
#define vf32x4_set1(e)             _mm_set_ps1(e)
#define vf32x4_setzero()           _mm_setzero_ps()
#define vf32x4_stor(addr, v)       _mm_store_ps((addr), (v))
#define vf32x4_load(addr)          _mm_load_ps((addr))
#define vf32x4_loadu(addr)         _mm_loadu_ps((addr))

#define vf32x4_load1(addr)    _mm_load_ps1((addr))
#define vf32x4_extract1(v, i) _mm_cvtss_f32(_mm_shuffle_ps((v), (v), (i)))
#define vf32x4_mul(v, u)      _mm_mul_ps(v, u)
#define vf32x4_add(v, u)      _mm_add_ps(v, u)
#define vf32x4_sub(v, u)      _mm_sub_ps(v, u)
#define vf32x4_mla(v, u, w)   vf32x4_add(v, vf32x4_mul(u, w))  // v + (u * w)
#define vf32x4_mls(v, u, w)   vf32x4_sub(v, vf32x4_mul(u, w))  // v - (u * w)
#define vf32x4_max(v, u)      _mm_max_ps(v, u)
#define vf32x4_rsqrt(v)       _mm_rsqrt_ps(v)

#endif

#endif  // IFX_SIMD_H
