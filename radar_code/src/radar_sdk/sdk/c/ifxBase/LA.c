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

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/


#include "LA.h"
#include "Complex.h"
#include "Defines.h"
#include "Error.h"
#include "internal/Macros.h"
#include "Math.h"
#include "Matrix.h"
#include "Mem.h"

/*
==============================================================================
   2. LOCAL DEFINITIONS
==============================================================================
*/

/*
==============================================================================
   3. LOCAL TYPES
==============================================================================
*/

/*
==============================================================================
   4. LOCAL DATA
==============================================================================
*/

static const ifx_Complex_t complex_zero = IFX_COMPLEX_DEF(0, 0);
static const ifx_Complex_t complex_one = IFX_COMPLEX_DEF(1, 0);

/*
==============================================================================
   5. LOCAL FUNCTION PROTOTYPES
==============================================================================
*/

static void lu_r_inplace(ifx_Matrix_R_t* A,
                         uint32_t* P,
                         uint32_t* S);

static void lu_c_inplace(ifx_Matrix_C_t* A,
                         uint32_t* P,
                         uint32_t* S);

static void lu_r(const ifx_Matrix_R_t* A,
                 ifx_Matrix_R_t* LU,
                 uint32_t* P,
                 uint32_t* S);

static void lu_c(const ifx_Matrix_C_t* A,
                 ifx_Matrix_C_t* LU,
                 uint32_t* P,
                 uint32_t* S);

static void lu_invert_r(const ifx_Matrix_R_t* LU,
                        const uint32_t* P,
                        ifx_Matrix_R_t* inverse);

static void lu_invert_c(const ifx_Matrix_C_t* LU,
                        const uint32_t* P,
                        ifx_Matrix_C_t* inverse);

static void cholesky_r_inplace(ifx_Matrix_R_t* A);

static void cholesky_c_inplace(ifx_Matrix_C_t* A);

static void determinant_r_inplace(ifx_Matrix_R_t* A,
                                  ifx_Float_t* determinant);

static void determinant_c_inplace(ifx_Matrix_C_t* A,
                                  ifx_Complex_t* determinant);

/*
==============================================================================
   6. LOCAL FUNCTIONS
==============================================================================
*/

/**
 * @brief Performs an inplace LU decomposition of a real matrix
 *
 * This functions performs an LU decomposition of the NxN matrix A with partial
 * pivoting (row permutations). The matrix is decomposed as
 * \f[
 *      P A = L U
 * \f]
 * where A is the original matrix, P is a permutation matrix, L is a lower
 * triangular matrix, and U is an upper triangular matrix.
 *
 * A permutation matrix has in each row exactly one element that is 1 and all
 * other elements are 0. For this reason the permutation matrix P is saved as
 * an array. The array P must have enough space for N elements. The array
 * stores the non-zero matrix elements. The array index corresponds to the row,
 * the value at that index to the column of the non-zero matrix element. For
 * example, P=[1,0,2] corresponds to the matrix
 * \f[
 * P = \begin{pmatrix} 0 & 1 & 0 \\ 1 & 0 & 0 \\ 0 & 0 & 1 \end{pmatrix} \,.
 * \f]
 *
 * The matrix A is overwritten by this function. After exit, A implicitly
 * contains the matrices L and U. The diagonal and the upper triangular part of
 * U corresponds to the diagonal and the upper triangular part of P*A, the
 * lower triangular part of U is zero. The matrix L has 1 on its diagonal, the
 * upper triangular matrix elements are zero, and the lower triangular matrix
 * elements correspond to the lower triangular matrix elements of P*A.
 *
 * The number of permutations performed is saved to S if S is not NULL.
 *
 * Here is an example:
 *
 * Input:
 * \f[
 * A^\mathrm{(in)} = \begin{pmatrix} 0 & 1 & 2 \\ 3 & 2 & 1 \\ 1 & 1 & 0 \end{pmatrix}
 * \f]
 * Output:
 * \f[
 * A^\mathrm{(out)} = \begin{pmatrix} 0 & 1 & 2 \\ 3 & 2 & 1 \\ 1/3 & 1/3 & -1 \end{pmatrix}, \quad P = (1,0,2)
 * \f]
 * This corresponds to
 * \f[
 * P = \begin{pmatrix} 0 & 1 & 0 \\ 1 & 0 & 0 \\ 0 & 0 & 1 \end{pmatrix}, \quad
 * P A^\mathrm{(out)} = \begin{pmatrix} 3 & 2 & 1 \\ 0 & 1 & 2 \\ 1/3 & 1/3 & -1 \end{pmatrix}, \quad
 * L = \begin{pmatrix} 1 & 0 & 0 \\ 0 & 1 & 0 \\ 1/3 & 1/3 & 1 \end{pmatrix}, \quad
 * U = \begin{pmatrix} 3 & 2 & 1 \\ 0 & 1 & 2 \\ 0 & 0 & -1 \end{pmatrix}
 * \f]
 * such that \f$P A^\mathrm{(in)} = L U\f$.
 *
 * If A is numerically singular, the error IFX_ERROR_MATRIX_SINGULAR is set.
 * In this the decomposition might be inaccurate or wrong.
 *
 * See also:
 * https://en.wikipedia.org/w/index.php?title=LU_decomposition&oldid=923660898#C_code_examples
 *
 * @param [in]     A         input matrix of dimension NxN
 * @param [out]    P         permutation vector of length N
 * @param [out]    S         number of permutations (might be NULL)
 *
 */
static void lu_r_inplace(ifx_Matrix_R_t* A,
                         uint32_t* P,
                         uint32_t* S)
{
    // Check that P and A are not null, and A is a square matrix
    IFX_ERR_BRK_NULL(P);
    IFX_MAT_BRK_VALID(A);
    IFX_MAT_BRK_SQUARE(A);

    // Dimension
    const uint32_t N = mCols(A);

    // Number of performed permutations (useful for computing the determinant)
    uint32_t permutations = 0;

    // initialize P as unit permutation
    for (uint32_t i = 0; i < N; i++)
    {
        P[i] = i;
    }

    for (uint32_t i = 0; i < N; i++)
    {
        ifx_Float_t maxA = 0.0;
        uint32_t imax = i;

        // Find the maximum element in the current row
        for (uint32_t k = i; k < N; k++)
        {
            const ifx_Float_t absA = FABS(IFX_MAT_AT(A, P[k], i));

            if (absA > maxA)
            {
                maxA = absA;
                imax = k;
            }
        }

        // if maxA < tol the matrix is (numerically) singular
        if (maxA == 0)
        {
            ifx_error_set(IFX_ERROR_MATRIX_SINGULAR);
            return;
        }

        if (imax != i)
        {
            // Pivot the rows of A; i.e., interchange row i and imax
            uint32_t temp = P[i];
            P[i] = P[imax];
            P[imax] = temp;

            // Count number of permutations
            permutations++;
        }

        for (uint32_t j = i + 1; j < N; j++)
        {
            IFX_MAT_AT(A, P[j], i) = IFX_MAT_AT(A, P[j], i) / IFX_MAT_AT(A, P[i], i);

            for (uint32_t k = i + 1; k < N; k++)
            {
                IFX_MAT_AT(A, P[j], k) -= IFX_MAT_AT(A, P[j], i) * IFX_MAT_AT(A, P[i], k);
            }
        }
    }

    if (S)
    {
        *S = permutations;
    }
}

//----------------------------------------------------------------------------

/**
 * @brief Performs an inplace LU decomposition of a complex matrix
 *
 * This functions performs an LU decomposition of the NxN matrix A with partial
 * pivoting (row permutations). The matrix is decomposed as
 * \f[
 *      P A = L U
 * \f]
 * where A is the original matrix, P is a permutation matrix, L is a lower
 * triangular matrix, and U is an upper triangular matrix.
 *
 * See lu_r_inplace for more information.
 *
 * @param [in]     A         input matrix of dimension NxN
 * @param [out]    P         permutation vector of length N
 * @param [out]    S         number of permutations (might be NULL)
 *
 */
static void lu_c_inplace(ifx_Matrix_C_t* A,
                         uint32_t* P,
                         uint32_t* S)
{
    // Check that P and A are not null, and A is a square matrix
    IFX_ERR_BRK_NULL(P);
    IFX_MAT_BRK_VALID(A);
    IFX_MAT_BRK_SQUARE(A);

    // Dimension of A
    const uint32_t N = mCols(A);

    // Number of performed permutations (useful for computing the determinant)
    uint32_t permutations = 0;

    // Initialize P as unit permutation
    for (uint32_t i = 0; i < N; i++)
    {
        P[i] = i;
    }

    for (uint32_t i = 0; i < N; i++)
    {
        ifx_Float_t maxA = 0.0;
        uint32_t imax = i;

        // Find the maximum element in the current row
        for (uint32_t k = i; k < N; k++)
        {
            const ifx_Float_t absA = ifx_complex_abs(IFX_MAT_AT(A, P[k], i));

            if (absA > maxA)
            {
                maxA = absA;
                imax = k;
            }
        }

        // if maxA < tol the matrix is (numerically) singular
        if (maxA == 0)
        {
            ifx_error_set(IFX_ERROR_MATRIX_SINGULAR);
            return;
        }

        if (imax != i)
        {
            // Pivot the rows of A; i.e., interchange row i and imax
            uint32_t temp = P[i];
            P[i] = P[imax];
            P[imax] = temp;

            // Count number of permutations
            permutations++;
        }

        for (uint32_t j = i + 1; j < N; j++)
        {
            IFX_MAT_AT(A, P[j], i) = ifx_complex_div(IFX_MAT_AT(A, P[j], i), IFX_MAT_AT(A, P[i], i));

            for (uint32_t k = i + 1; k < N; k++)
            {
                // IFX_MAT_AT(A, P[j], k) -= IFX_MAT_AT(A, P[j], i) * IFX_MAT_AT(A, P[i], k);
                IFX_MAT_AT(A, P[j], k) = ifx_complex_sub(IFX_MAT_AT(A, P[j], k), ifx_complex_mul(IFX_MAT_AT(A, P[j], i), IFX_MAT_AT(A, P[i], k)));
            }
        }
    }

    if (S)
    {
        *S = permutations;
    }
}

//----------------------------------------------------------------------------

/**
 * @brief Performs an LU decomposition of a real matrix
 *
 * The functions performs an LU decomposition of an NxN matrix and saves the
 * result in the matrix LU. For more information, see lu_r_inplace.
 *
 * @param [in]     A         input matrix of dimension NxN
 * @param [out]    LU        matrix saving the matrices L and U
 * @param [out]    P         permutation vector of length N
 * @param [out]    S         number of permutations (might be NULL)
 *
 */
static void lu_r(const ifx_Matrix_R_t* A,
                 ifx_Matrix_R_t* LU,
                 uint32_t* P,
                 uint32_t* S)
{
    IFX_ERR_BRK_NULL(P);
    IFX_MAT_BRK_VALID(A);
    IFX_MAT_BRK_VALID(LU);
    IFX_MAT_BRK_SQUARE(A);
    IFX_MAT_BRK_SQUARE(LU);
    IFX_MAT_BRK_DIM_COL(A, LU);

    // Copy matrix to LU
    ifx_mat_copy_r(A, LU);

    // Perform LU decomposition inplace
    lu_r_inplace(LU, P, S);
}

//----------------------------------------------------------------------------

/**
 * @brief Performs an LU decomposition of a complex matrix
 *
 * The functions performs an LU decomposition of an NxN matrix and saves the
 * result in the matrix LU. For more information, see lu_r_inplace.
 *
 * @param [in]     A         input matrix of dimension NxN
 * @param [out]    LU        matrix saving the matrices L and U
 * @param [out]    P         permutation vector of length N
 * @param [out]    S         number of permutations (might be NULL)
 *
 */
static void lu_c(const ifx_Matrix_C_t* A,
                 ifx_Matrix_C_t* LU,
                 uint32_t* P,
                 uint32_t* S)
{
    IFX_ERR_BRK_NULL(P);
    IFX_MAT_BRK_VALID(A);
    IFX_MAT_BRK_VALID(LU);
    IFX_MAT_BRK_SQUARE(A);
    IFX_MAT_BRK_SQUARE(LU);
    IFX_MAT_BRK_DIM_COL(A, LU);

    // Copy A to LU
    ifx_mat_copy_c(A, LU);

    // Perform LU decomposition inplace
    lu_c_inplace(LU, P, S);
}

//----------------------------------------------------------------------------

/**
 * @brief Computes inverse matrix given the matrix's LU decomposition
 *
 * Given an LU decomposition of a matrix A computed by either lu_r
 * or lu_r_inplace, this function computes the inverse matrix
 * \f$A^{-1}\f$.
 *
 * @param [in]     LU        LU decomposition of the matrix A
 * @param [in]     P         permutation vector of LU decomposition
 * @param [out]    inverse   inverse matrix of A
 *
 */
static void lu_invert_r(const ifx_Matrix_R_t* LU,
                        const uint32_t* P,
                        ifx_Matrix_R_t* inverse)
{
    IFX_ERR_BRK_NULL(P);
    IFX_MAT_BRK_VALID(LU);
    IFX_MAT_BRK_VALID(inverse);
    IFX_MAT_BRK_SQUARE(LU);
    IFX_MAT_BRK_SQUARE(inverse);
    IFX_MAT_BRK_DIM_COL(LU, inverse);

    // Dimension of LU and inverse
    const uint32_t N = mCols(LU);

    for (uint32_t j = 0; j < N; j++)
    {
        for (uint32_t i = 0; i < N; i++)
        {
            // this corresponds to the Kronecker delta
            IFX_MAT_AT(inverse, i, j) = (P[i] == j) ? (ifx_Float_t)1 : (ifx_Float_t)0;

            for (uint32_t k = 0; k < i; k++)
            {
                IFX_MAT_AT(inverse, i, j) -= IFX_MAT_AT(LU, P[i], k) * IFX_MAT_AT(inverse, k, j);
            }
        }

        for (uint32_t n = N; n > 0; n--)
        {
            uint32_t i = n - 1;

            for (uint32_t k = i + 1; k < N; k++)
            {
                IFX_MAT_AT(inverse, i, j) -= IFX_MAT_AT(LU, P[i], k) * IFX_MAT_AT(inverse, k, j);
            }

            IFX_MAT_AT(inverse, i, j) = IFX_MAT_AT(inverse, i, j) / IFX_MAT_AT(LU, P[i], i);
        }
    }
}

//----------------------------------------------------------------------------

/**
 * @brief Compute inverse matrix given the matrix's LU decomposition
 *
 * Given an LU decomposition of a matrix A computed by either lu_c
 * or lu_c_inplace, this function computes the inverse matrix
 * \f$A^{-1}\f$.
 *
 * @param [in]     LU        LU decomposition of the matrix A
 * @param [in]     P         permutation vector of LU decomposition
 * @param [out]    inverse   inverse matrix of A
 *
 */
static void lu_invert_c(const ifx_Matrix_C_t* LU,
                        const uint32_t* P,
                        ifx_Matrix_C_t* inverse)
{
    IFX_ERR_BRK_NULL(P);
    IFX_MAT_BRK_VALID(LU);
    IFX_MAT_BRK_VALID(inverse);
    IFX_MAT_BRK_SQUARE(LU);
    IFX_MAT_BRK_SQUARE(inverse);
    IFX_MAT_BRK_DIM_COL(LU, inverse);

    // Dimension of LU and inverse
    const uint32_t N = mCols(LU);

    for (uint32_t j = 0; j < N; j++)
    {
        for (uint32_t i = 0; i < N; i++)
        {
            // This corresponds to the Kronecker delta
            IFX_MAT_AT(inverse, i, j) = (P[i] == j) ? complex_one : complex_zero;

            for (uint32_t k = 0; k < i; k++)
            {
                // IFX_MAT_AT(inverse, i, j) -= IFX_MAT_AT(LU, P[i], k) * IFX_MAT_AT(inverse, k, j);
                IFX_MAT_AT(inverse, i, j) = ifx_complex_sub(IFX_MAT_AT(inverse, i, j), ifx_complex_mul(IFX_MAT_AT(LU, P[i], k), IFX_MAT_AT(inverse, k, j)));
            }
        }

        for (uint32_t n = N; n > 0; n--)
        {
            uint32_t i = n - 1;

            for (uint32_t k = i + 1; k < N; k++)
            {
                // IFX_MAT_AT(inverse, i, j) -= IFX_MAT_AT(LU, P[i], k) * IFX_MAT_AT(inverse, k, j);
                IFX_MAT_AT(inverse, i, j) = ifx_complex_sub(IFX_MAT_AT(inverse, i, j), ifx_complex_mul(IFX_MAT_AT(LU, P[i], k), IFX_MAT_AT(inverse, k, j)));
            }

            // IFX_MAT_AT(inverse, i, j) = IFX_MAT_AT(inverse, i, j) / IFX_MAT_AT(LU, P[i], i);
            IFX_MAT_AT(inverse, i, j) = ifx_complex_div(IFX_MAT_AT(inverse, i, j), IFX_MAT_AT(LU, P[i], i));
        }
    }
}

//----------------------------------------------------------------------------

/**
 * @brief Computes inplace Cholesky decomposition
 *
 * Compute the Cholesky decomposition of the real and symmetric positive definite
 * definite matrix A
 * \f[
 *      A = L L^\mathrm{T},
 * \f]
 * where L is a lower triangular matrix with positive diagonal elements. Only
 * the lower triangular part of A (diagonal and matrix elements below diagonal)
 * are read. After exit the lower triangular part of A corresponds to L.
 *
 * If A is not positive definite, IFX_ERROR_MATRIX_NOT_POSITIVE_DEFINITE is
 * set.
 *
 * @param [in,out] A         matrix
 *
 */
static void cholesky_r_inplace(ifx_Matrix_R_t* A)
{
    IFX_MAT_BRK_VALID(A);
    IFX_MAT_BRK_SQUARE(A);

    // Dimension of A
    uint32_t N = mRows(A);

    for (uint32_t i = 0; i < N; i++)
    {
        for (uint32_t j = i; j < N; j++)
        {
            ifx_Float_t sum = IFX_MAT_AT(A, j, i);

            for (uint32_t k = 0; k < i; k++)
            {
                sum -= IFX_MAT_AT(A, i, k) * IFX_MAT_AT(A, j, k);
            }

            if (i == j)
            {
                if (sum < 0)
                {
                    ifx_error_set(IFX_ERROR_MATRIX_NOT_POSITIVE_DEFINITE);
                    return;
                }

                IFX_MAT_AT(A, i, i) = SQRT(sum);
            }
            else
            {
                IFX_MAT_AT(A, j, i) = sum / IFX_MAT_AT(A, i, i);
            }
        }
    }
}

//----------------------------------------------------------------------------

/**
 * @brief Computes inplace Cholesky decomposition
 *
 * Compute the Cholesky decomposition of the complex and hermitian positive
 * definite definite matrix A
 * \f[
 *      A = L L^\dagger
 * \f]
 * where L is a lower triangular matrix with real and positive diagonal
 * elements. Only the lower triangular part of A (diagonal and matrix elements
 * below diagonal) are read. After exit the lower triangular part of A
 * corresponds to L.
 *
 * If A is not positive definite, IFX_ERROR_MATRIX_NOT_POSITIVE_DEFINITE is
 * set.
 *
 * @param [in,out] A         matrix
 *
 */
static void cholesky_c_inplace(ifx_Matrix_C_t* A)
{
    IFX_MAT_BRK_VALID(A);
    IFX_MAT_BRK_SQUARE(A);

    // Dimension of A
    uint32_t N = mRows(A);

    for (uint32_t i = 0; i < N; i++)
    {
        for (uint32_t j = i; j < N; j++)
        {
            ifx_Complex_t sum_c = IFX_MAT_AT(A, j, i);

            for (uint32_t k = 0; k < i; k++)
            {
                // sum -= IFX_MAT_AT(A, i, k) * IFX_MAT_AT(A, j, k);
                const ifx_Complex_t Aik = IFX_MAT_AT(A, i, k);
                const ifx_Complex_t Ajk = IFX_MAT_AT(A, j, k);
                sum_c = ifx_complex_sub(sum_c, ifx_complex_mul(ifx_complex_conj(Aik), Ajk));
            }

            if (i == j)
            {
                // sum_c is real if i=j
                ifx_Float_t sum = IFX_COMPLEX_REAL(sum_c);

                if (sum < 0)
                {
                    ifx_error_set(IFX_ERROR_MATRIX_NOT_POSITIVE_DEFINITE);
                    return;
                }

                IFX_COMPLEX_SET_REAL(IFX_MAT_AT(A, i, i), SQRT(sum));
                IFX_COMPLEX_SET_IMAG(IFX_MAT_AT(A, i, i), 0);
            }
            else
            {
                const ifx_Complex_t Aii = IFX_MAT_AT(A, i, i);
                IFX_MAT_AT(A, j, i) = ifx_complex_div(sum_c, Aii);
            }
        }
    }
}

//----------------------------------------------------------------------------

/**
 * @brief Computes determinant of real matrix A inplace
 *
 * Compute the LU decomposition PA=LU inplace. The determinant of A is then
 * given as a product of the diagonal elements of U
 * \f[
 * \mathrm{det}(A) = (-1)^S Prod_j U_{ii} \,.
 * \f]
 *
 * @param [in,out] A                   matrix A
 * @param [out]    determinant         determinant of A
 *
 */
static void determinant_r_inplace(ifx_Matrix_R_t* A,
                                  ifx_Float_t* determinant)
{
    IFX_MAT_BRK_VALID(A);
    IFX_MAT_BRK_SQUARE(A);
    IFX_ERR_BRK_NULL(determinant);

    uint32_t S = 0;
    uint32_t* P = NULL;

    // Dimension
    uint32_t N = mCols(A);

    // Allocate memory for permutation vector
    P = ifx_mem_alloc(sizeof(uint32_t) * N);
    IFX_ERR_BRF_MEMALLOC(P);

    // Save old error
    ifx_Error_t error_old = ifx_error_get_and_clear();

    // Perform LU decomposition
    lu_r_inplace(A, P, &S);

    // New error
    ifx_Error_t error_new = ifx_error_get_and_clear();

    if (error_new == IFX_ERROR_MATRIX_SINGULAR)
    {
        // Restore old error
        ifx_error_set_no_callback(error_old);
        *determinant = 0;
        goto fail;
    }
    else if (error_new == IFX_OK)
    {
        // Restore old error
        ifx_error_set_no_callback(error_old);
    }

    *determinant = 1;

    for (uint32_t j = 0; j < N; j++)
    {
        *determinant *= IFX_MAT_AT(A, P[j], j);
    }

    if (S % 2)
    {
        *determinant = -*determinant;
    }

fail:

    if (P)
    {
        ifx_mem_free(P);
    }
}

//----------------------------------------------------------------------------

/**
 * @brief Computes determinant of complex matrix A inplace
 *
 * Compute the LU decomposition PA=LU inplace. The determinant of A is then
 * given as a product of the diagonal elements of U
 * \f[
 * \mathrm{det}(A) = (-1)^S Prod_j U_{ii} \,.
 * \f]
 *
 * @param [in,out] A                   matrix A
 * @param [out]    determinant         determinant of A
 *
 */
static void determinant_c_inplace(ifx_Matrix_C_t* A,
                                  ifx_Complex_t* determinant)
{
    IFX_MAT_BRK_VALID(A);
    IFX_MAT_BRK_SQUARE(A);
    IFX_ERR_BRK_NULL(determinant);

    uint32_t S = 0;
    uint32_t* P = NULL;

    // Dimension
    uint32_t N = mCols(A);

    // Allocate memory for permutation vector
    P = ifx_mem_alloc(sizeof(uint32_t) * N);
    IFX_ERR_BRF_MEMALLOC(P);

    // Save old error
    ifx_Error_t error_old = ifx_error_get_and_clear();

    // Perform LU decomposition
    lu_c_inplace(A, P, &S);

    // New error
    ifx_Error_t error_new = ifx_error_get_and_clear();

    if (error_new == IFX_ERROR_MATRIX_SINGULAR)
    {
        // Restore old error
        ifx_error_set_no_callback(error_old);
        *determinant = complex_zero;
        goto fail;
    }
    else if (error_new == IFX_OK)
    {
        // Restore old error
        ifx_error_set_no_callback(error_old);
    }

    *determinant = complex_one;

    for (uint32_t j = 0; j < N; j++)
    {
        *determinant = ifx_complex_mul(*determinant, IFX_MAT_AT(A, P[j], j));
    }

    if (S % 2)
    {
        *determinant = ifx_complex_mul_real(*determinant, -1);
    }

fail:

    if (P)
    {
        ifx_mem_free(P);
    }
}

/*
==============================================================================
   7. EXPORTED FUNCTIONS
==============================================================================
*/

void ifx_la_invert_r(const ifx_Matrix_R_t* A,
                     ifx_Matrix_R_t* Ainv)
{
    IFX_MAT_BRK_VALID(A);
    IFX_MAT_BRK_VALID(Ainv);
    IFX_MAT_BRK_SQUARE(A);
    IFX_MAT_BRK_SQUARE(Ainv);
    IFX_MAT_BRK_DIM_COL(A, Ainv);

    // Dimension
    uint32_t N = mCols(A);

    uint32_t* P = NULL;
    ifx_Matrix_R_t* LU = NULL;

    // Allocate memory for A
    LU = ifx_mat_create_r(N, N);
    IFX_ERR_BRF_MEMALLOC(LU);

    // Allocate memory for permutation vector
    P = ifx_mem_alloc(sizeof(uint32_t) * N);
    IFX_ERR_BRF_MEMALLOC(P);

    // Perform LU decomposition
    lu_r(A, LU, P, NULL);

    // Invert A
    lu_invert_r(LU, P, Ainv);

fail:

    // Free memory if necessary
    if (LU)
    {
        ifx_mat_destroy_r(LU);
    }

    if (P)
    {
        ifx_mem_free(P);
    }
}

void ifx_la_invert_c(const ifx_Matrix_C_t* A,
                     ifx_Matrix_C_t* Ainv)
{
    IFX_MAT_BRK_VALID(A);
    IFX_MAT_BRK_VALID(Ainv);
    IFX_MAT_BRK_SQUARE(A);
    IFX_MAT_BRK_SQUARE(Ainv);
    IFX_MAT_BRK_DIM_COL(A, Ainv);

    // Dimension
    uint32_t N = mCols(A);

    uint32_t* P = NULL;
    ifx_Matrix_C_t* LU = NULL;

    // Allocate memory for A
    LU = ifx_mat_create_c(N, N);
    IFX_ERR_BRF_MEMALLOC(LU);

    // Allocate memory for permutation vector
    P = ifx_mem_alloc(sizeof(uint32_t) * N);
    IFX_ERR_BRF_MEMALLOC(P);

    // Perform LU decomposition
    lu_c(A, LU, P, NULL);

    // Invert A
    lu_invert_c(LU, P, Ainv);

fail:

    // Free memory if necessary
    if (LU)
    {
        ifx_mat_destroy_c(LU);
    }

    if (P)
    {
        ifx_mem_free(P);
    }
}

void ifx_la_cholesky_r(const ifx_Matrix_R_t* A,
                       ifx_Matrix_R_t* L)
{
    IFX_MAT_BRK_VALID(A);
    IFX_MAT_BRK_SQUARE(A);
    IFX_MAT_BRK_VALID(L);
    IFX_MAT_BRK_SQUARE(L);
    IFX_MAT_BRK_DIM_COL(A, L);

    // Dimension
    const uint32_t N = mRows(A);

    // Copy lower triangular elements from A to L, set elements of L above diagonal to 0
    for (uint32_t j = 0; j < N; j++)
    {
        // Copy diagonal and elements below diagonal from A
        for (uint32_t k = 0; k <= j; k++)
        {
            IFX_MAT_AT(L, j, k) = IFX_MAT_AT(A, j, k);
        }

        // Matrix elements above diagonal are zero
        for (uint32_t k = j + 1; k < N; k++)
        {
            IFX_MAT_AT(L, j, k) = 0;
        }
    }

    cholesky_r_inplace(L);
}

void ifx_la_cholesky_c(const ifx_Matrix_C_t* A, ifx_Matrix_C_t* L)
{
    IFX_MAT_BRK_VALID(A);
    IFX_MAT_BRK_SQUARE(A);
    IFX_MAT_BRK_VALID(L);
    IFX_MAT_BRK_SQUARE(L);
    IFX_MAT_BRK_DIM_COL(A, L);

    // Dimension
    const uint32_t N = mRows(A);

    // Copy lower triangular elements from A to L, set elements of L above diagonal to 0
    for (uint32_t j = 0; j < N; j++)
    {
        // Copy diagonal and elements below diagonal from A
        for (uint32_t k = 0; k <= j; k++)
        {
            // IFX_MAT_AT(L, j, k) = IFX_MAT_AT(A, j, k);
            const ifx_Complex_t Ajk = IFX_MAT_AT(A, j, k);
            IFX_COMPLEX_SET(IFX_MAT_AT(L, j, k), IFX_COMPLEX_REAL(Ajk), IFX_COMPLEX_IMAG(Ajk));
        }

        // Matrix elements above diagonal are zero
        for (uint32_t k = j + 1; k < N; k++)
        {
            IFX_MAT_AT(L, j, k) = complex_zero;
        }
    }

    cholesky_c_inplace(L);
}

void ifx_la_determinant_r(const ifx_Matrix_R_t* A,
                          ifx_Float_t* determinant)
{
    IFX_MAT_BRK_VALID(A);
    IFX_MAT_BRK_SQUARE(A);
    IFX_ERR_BRK_NULL(determinant);

    // Dimension
    uint32_t N = mCols(A);

    ifx_Matrix_R_t* B = ifx_mat_create_r(N, N);

    if (!B)
    {
        return;
    }

    ifx_mat_copy_r(A, B);
    determinant_r_inplace(B, determinant);
    ifx_mat_destroy_r(B);
}

void ifx_la_determinant_c(const ifx_Matrix_C_t* A,
                          ifx_Complex_t* determinant)
{
    IFX_MAT_BRK_VALID(A);
    IFX_MAT_BRK_SQUARE(A);
    IFX_ERR_BRK_NULL(determinant);

    // Dimension
    uint32_t N = mCols(A);

    ifx_Matrix_C_t* B = ifx_mat_create_c(N, N);

    if (!B)
    {
        return;
    }

    ifx_mat_copy_c(A, B);
    determinant_c_inplace(B, determinant);
    ifx_mat_destroy_c(B);
}
