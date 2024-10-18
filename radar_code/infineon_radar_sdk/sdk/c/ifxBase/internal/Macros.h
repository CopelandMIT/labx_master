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
 * @internal
 * @file Macros.h
 *
 * @brief Internal Macro definitions
 */

#ifndef IFX_BASE_INTERNAL_MACROS_H
#define IFX_BASE_INTERNAL_MACROS_H

#ifdef __cplusplus
extern "C"
{
#endif

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

/*
==============================================================================
   2. DEFINITIONS
==============================================================================
*/

#define vStride(v)      IFX_VEC_STRIDE(v)
#define vOffset(v, idx) IFX_VEC_OFFSET(v, idx)
#define vLen(v)         IFX_VEC_LEN(v)
#define vDat(v)         IFX_VEC_DAT(v)
#define vAt(v, idx)     IFX_VEC_AT(v, idx)

#define mDat(m)          IFX_MAT_DAT(m)
#define mRows(m)         IFX_MAT_ROWS(m)
#define mCols(m)         IFX_MAT_COLS(m)
#define mStride(m, i)    IFX_MAT_STRIDE(m, i)
#define mSize(m)         IFX_MAT_SIZE(m)
#define mOffset(m, r, c) IFX_MAT_OFFSET(m, r, c)
#define mAt(m, r, c)     IFX_MAT_AT(m, r, c)

#define cRows(c)      IFX_CUBE_ROWS(c)
#define cCols(c)      IFX_CUBE_COLS(c)
#define cSlices(c)    IFX_CUBE_SLICES(c)
#define cDat(c)       IFX_CUBE_DAT(c)
#define cSliceSize(c) IFX_CUBE_SLICE_SIZE(c)
#define cSize(c)      IFX_CUBE_SIZE(c)
#define cSliceOffset(c, s) \
    IFX_CUBE_SLICE_OFFSET(C, s)
#define cOffset(cub, r, c, s) \
    IFX_CUBE_OFFSET(cub, r, c, s)
#define cAt(cub, r, c, s) IFX_CUBE_AT(cub, r, c, s)

#define cStride(cub, i) IFX_CUBE_STRIDE(cub, i)

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

#endif /* #define IFX_BASE_INTERNAL_MACROS_H */
