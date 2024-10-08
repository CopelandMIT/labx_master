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
 * @file LA.h
 *
 * \brief \copybrief gr_la
 *
 * For details refer to \ref gr_la
 */

#ifndef IFX_BASE_LA_H
#define IFX_BASE_LA_H

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include "Matrix.h"
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

/** @addtogroup gr_cat_SDK_base
 * @{
 */

/** @defgroup gr_la Linear Algebra
 * @brief API for Linear Algebra (LA)
 *
 * Supports linear algebra operations such as LU,
 * Cholesky decomposition, or inverting matrices
 *
 * @{
 */

/**
 * @brief Computes inverse of a generic real matrix
 *
 * This function computes the inverse matrix of A (\f$A^{-1}\f$) and saves it in Ainv.
 *
 * This function works for generic real square matrices. If A is (numerically)
 * singular, IFX_ERROR_MATRIX_SINGULAR is set.
 *
 * @param [in]     A         input matrix A
 * @param [out]    Ainv      inverse of matrix A
 *
 */
IFX_DLL_PUBLIC
void ifx_la_invert_r(const ifx_Matrix_R_t* A,
                     ifx_Matrix_R_t* Ainv);

/**
 * @brief Computes inverse of a generic complex matrix
 *
 * This function computes the inverse matrix of A (\f$A^{-1}\f$) and saves it in Ainv.
 *
 * This function works for generic complex square matrices. If A is
 * (numerically) singular, IFX_ERROR_MATRIX_SINGULAR is set.
 *
 * @param [in]     A         input matrix A
 * @param [out]    Ainv      inverse of matrix A
 *
 */
IFX_DLL_PUBLIC
void ifx_la_invert_c(const ifx_Matrix_C_t* A,
                     ifx_Matrix_C_t* Ainv);

/**
 * @brief Performs Cholesky decomposition of A
 *
 * Compute the Cholesky decomposition of the real symmetric and positive
 * definite matrix A
 * \f[
 *      A = L L^\mathrm{T},
 * \f]
 * where L is a real lower triangular matrix with positive diagonal elements.
 * Only the lower triangular part of A (matrix elements on the diagonal and
 * below the diagonal) are read.
 *
 * If A is not positive definite, IFX_ERROR_MATRIX_NOT_POSITIVE_DEFINITE is
 * set.
 *
 * @param [in]     A         matrix A
 * @param [out]    L         lower triangular matrix
 *
 */
IFX_DLL_PUBLIC
void ifx_la_cholesky_r(const ifx_Matrix_R_t* A,
                       ifx_Matrix_R_t* L);

/**
 * @brief Performs Cholesky decomposition of A
 *
 * Compute the Cholesky decomposition of the hermitian and positive definite
 * matrix A
 * \f[
 *      A = L L^\dagger,
 * \f]
 * where L is a complex lower triangular matrix with real and positive diagonal
 * elements. Only the lower triangular part of A (matrix elements on the
 * diagonal and below the diagonal) are read.
 *
 * If A is not positive definite, IFX_ERROR_MATRIX_NOT_POSITIVE_DEFINITE is
 * set.
 *
 * @param [in]     A         matrix A
 * @param [out]    L         lower triangular matrix
 *
 */
IFX_DLL_PUBLIC
void ifx_la_cholesky_c(const ifx_Matrix_C_t* A,
                       ifx_Matrix_C_t* L);

/**
 * @brief Computes determinant of a generic real matrix
 *
 * This function computes the determinant of A and saves it in determinant.
 *
 * @param [in]     A                   matrix
 * @param [out]    determinant         determinant of A
 *
 */
IFX_DLL_PUBLIC
void ifx_la_determinant_r(const ifx_Matrix_R_t* A,
                          ifx_Float_t* determinant);

/**
 * @brief Computes determinant of a generic complex matrix
 *
 * This function computes the determinant of A and saves it in determinant.
 *
 * @param [in]     A                   matrix
 * @param [out]    determinant         determinant of A
 *
 */
IFX_DLL_PUBLIC
void ifx_la_determinant_c(const ifx_Matrix_C_t* A,
                          ifx_Complex_t* determinant);

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* IFX_BASE_LA_H */
