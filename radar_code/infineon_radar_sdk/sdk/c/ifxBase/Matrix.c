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

#include "Matrix.h"
#include "Complex.h"
#include "Defines.h"
#include "Error.h"
#include "internal/Macros.h"
#include "internal/Util.h"
#include "Mda.h"
#include "Mem.h"
#include "Vector.h"
#include <stdlib.h>
#include <string.h>


/*
==============================================================================
   2. LOCAL DEFINITIONS
==============================================================================
*/

#define MAT_BLIT(from, to, from_row, num_rows, from_col, num_cols)        \
    for (uint32_t i = (from_row); i < ((from_row) + (num_rows)); i++)     \
    {                                                                     \
        for (uint32_t j = (from_col); j < ((from_col) + (num_cols)); j++) \
        {                                                                 \
            mAt(to, i, j) = mAt(from, i, j);                              \
        }                                                                 \
    }

#define MAT_CLONE(from, to) \
    MAT_COPY(from, to, 0, mRows(from), 0, mCols(from))

#define MAT_TRANSPOSE(m, t)                     \
    for (uint32_t i = 0; i < mRows(m); i++)     \
    {                                           \
        for (uint32_t j = 0; j < mCols(m); j++) \
        {                                       \
            mAt(t, j, i) = mAt(m, i, j);        \
        }                                       \
    }

/* apply unary operator to all elements from mat and store in result */
#define MAT_APPLY_UNOP(mat, op, result)                 \
    do                                                  \
    {                                                   \
        IFX_MAT_BRK_VALID(mat);                         \
        IFX_MAT_BRK_VALID(result);                      \
        IFX_MAT_BRK_DIM(mat, result);                   \
        for (uint32_t r = 0; r < mRows(mat); r++)       \
        {                                               \
            for (uint32_t c = 0; c < mCols(mat); c++)   \
            {                                           \
                mAt(result, r, c) = op(mAt(mat, r, c)); \
            }                                           \
        }                                               \
    } while (0)

/* apply binary operator to all pairs of elements from lhs and rhs and store in result */
#define MAT_APPLY_BINOP(lhs, op, rhs, result)                           \
    do                                                                  \
    {                                                                   \
        IFX_MAT_BRK_VALID(lhs);                                         \
        IFX_MAT_BRK_VALID(rhs);                                         \
        IFX_MAT_BRK_VALID(result);                                      \
        IFX_MAT_BRK_DIM(lhs, result);                                   \
        IFX_MAT_BRK_DIM(lhs, rhs);                                      \
        for (uint32_t r = 0; r < mRows(lhs); r++)                       \
        {                                                               \
            for (uint32_t c = 0; c < mCols(lhs); c++)                   \
            {                                                           \
                mAt(result, r, c) = op(mAt(lhs, r, c), mAt(rhs, r, c)); \
            }                                                           \
        }                                                               \
    } while (0)


/*
==============================================================================
   7. EXPORTED FUNCTIONS
==============================================================================
*/

void ifx_mat_rawview_r(ifx_Matrix_R_t* matrix,
                       ifx_Float_t* d,
                       uint32_t rows,
                       uint32_t columns,
                       uint32_t lda)
{
    IFX_ERR_BRK_NULL(matrix);
    IFX_ERR_BRK_NULL(d);

    const uint32_t shape[] = {rows, columns};
    const size_t stride[] = {lda, 1};
    const uint32_t flags = 0;
    ifx_mda_rawview_r(matrix, d, 2, shape, stride, flags);
}

//----------------------------------------------------------------------------

void ifx_mat_rawview_c(ifx_Matrix_C_t* matrix,
                       ifx_Complex_t* d,
                       uint32_t rows,
                       uint32_t columns,
                       uint32_t lda)
{
    IFX_ERR_BRK_NULL(matrix);
    IFX_ERR_BRK_NULL(d);

    const uint32_t shape[] = {rows, columns};
    const size_t stride[] = {lda, 1};
    const uint32_t flags = 0;
    ifx_mda_rawview_c(matrix, d, 2, shape, stride, flags);
}

//----------------------------------------------------------------------------

void ifx_mat_view_r(ifx_Matrix_R_t* matrix,
                    ifx_Matrix_R_t* source,
                    uint32_t row_offset,
                    uint32_t column_offset,
                    uint32_t rows,
                    uint32_t columns)
{
    IFX_ERR_BRK_NULL(matrix);
    IFX_MAT_BRK_VALID(source);

    IFX_MDA_VIEW_R(matrix, source, IFX_MDA_SLICE(row_offset, row_offset + rows, 1), IFX_MDA_SLICE(column_offset, column_offset + columns, 1));
}

//----------------------------------------------------------------------------

void ifx_mat_view_c(ifx_Matrix_C_t* matrix,
                    ifx_Matrix_C_t* source,
                    uint32_t row_offset,
                    uint32_t column_offset,
                    uint32_t rows,
                    uint32_t columns)
{
    IFX_ERR_BRK_NULL(matrix);
    IFX_MAT_BRK_VALID(source);

    IFX_MDA_VIEW_C(matrix, source, IFX_MDA_SLICE(row_offset, row_offset + rows, 1), IFX_MDA_SLICE(column_offset, column_offset + columns, 1));
}

//----------------------------------------------------------------------------

void ifx_mat_view_rows_r(ifx_Matrix_R_t* matrix,
                         ifx_Matrix_R_t* source,
                         uint32_t row_offset,
                         uint32_t rows)
{
    ifx_mat_view_r(matrix, source, row_offset, 0, rows, mCols(source));
}

//----------------------------------------------------------------------------

void ifx_mat_view_rows_c(ifx_Matrix_C_t* matrix,
                         ifx_Matrix_C_t* source,
                         uint32_t row_offset,
                         uint32_t rows)
{
    ifx_mat_view_c(matrix, source, row_offset, 0, rows, mCols(source));
}

//----------------------------------------------------------------------------

ifx_Matrix_R_t* ifx_mat_create_r(uint32_t rows,
                                 uint32_t columns)
{
    ifx_Matrix_R_t* mat = IFX_MDA_CREATE_R(rows, columns);
    if (mat)
        ifx_mda_clear_r(mat);
    return mat;
}

//----------------------------------------------------------------------------

ifx_Matrix_C_t* ifx_mat_create_c(uint32_t rows,
                                 uint32_t columns)
{
    ifx_Matrix_C_t* mat = IFX_MDA_CREATE_C(rows, columns);
    if (mat)
        ifx_mda_clear_c(mat);
    return mat;
}

//----------------------------------------------------------------------------

void ifx_mat_destroy_r(ifx_Matrix_R_t* matrix)
{
    ifx_mda_destroy_r(matrix);
}

//----------------------------------------------------------------------------

void ifx_mat_destroy_c(ifx_Matrix_C_t* matrix)
{
    ifx_mda_destroy_c(matrix);
}

//----------------------------------------------------------------------------

void ifx_mat_blit_r(const ifx_Matrix_R_t* from,
                    uint32_t from_row,
                    uint32_t num_rows,
                    uint32_t from_column,
                    uint32_t num_columns,
                    ifx_Matrix_R_t* to)
{
    IFX_MAT_BRK_VALID(from);
    IFX_MAT_BRK_VALID(to);
    IFX_MAT_BRK_ROWS(to, num_rows);
    IFX_MAT_BRK_COLS(to, num_columns);

    MAT_BLIT(from, to, from_row, num_rows, from_column, num_columns);
}

//----------------------------------------------------------------------------

void ifx_mat_blit_c(const ifx_Matrix_C_t* from,
                    uint32_t from_row,
                    uint32_t num_rows,
                    uint32_t from_column,
                    uint32_t num_columns,
                    ifx_Matrix_C_t* to)
{
    IFX_MAT_BRK_VALID(from);
    IFX_MAT_BRK_VALID(to);
    IFX_MAT_BRK_ROWS(to, num_rows);
    IFX_MAT_BRK_COLS(to, num_columns);

    MAT_BLIT(from, to, from_row, num_rows, from_column, num_columns);
}

//----------------------------------------------------------------------------

void ifx_mat_copy_r(const ifx_Matrix_R_t* from,
                    ifx_Matrix_R_t* to)
{
    IFX_MAT_BRK_VALID(from);
    IFX_MAT_BRK_VALID(to);

    ifx_mat_blit_r(from, 0, mRows(from), 0, mCols(from), to);
}

//----------------------------------------------------------------------------

void ifx_mat_copy_c(const ifx_Matrix_C_t* from,
                    ifx_Matrix_C_t* to)
{
    IFX_MAT_BRK_VALID(from);
    IFX_MAT_BRK_VALID(to);

    ifx_mat_blit_c(from, 0, mRows(from), 0, mCols(from), to);
}

//----------------------------------------------------------------------------

void ifx_mat_set_row_r(ifx_Matrix_R_t* matrix,
                       uint32_t row_index,
                       const ifx_Float_t* row_values,
                       uint32_t count)
{
    IFX_MAT_BRK_VALID(matrix);
    if (count == 0)
        return;

    IFX_MAT_BRK_IDX(matrix, row_index, count - 1);
    IFX_ERR_BRK_NULL(row_values);

    for (uint32_t i = 0; i < count; i++)
    {
        mAt(matrix, row_index, i) = row_values[i];
    }
}

//----------------------------------------------------------------------------

void ifx_mat_set_row_c(ifx_Matrix_C_t* matrix,
                       uint32_t row_index,
                       const ifx_Complex_t* row_values,
                       uint32_t count)
{
    IFX_MAT_BRK_VALID(matrix);
    if (count == 0)
        return;

    IFX_MAT_BRK_IDX(matrix, row_index, count - 1);
    IFX_ERR_BRK_NULL(row_values);

    for (uint32_t i = 0; i < count; i++)
    {
        mAt(matrix, row_index, i) = row_values[i];
    }
}

//----------------------------------------------------------------------------

void ifx_mat_set_row_vector_r(ifx_Matrix_R_t* matrix,
                              uint32_t row_index,
                              const ifx_Vector_R_t* row_values)
{
    IFX_MAT_BRK_VALID(matrix);
    IFX_VEC_BRK_VALID(row_values);
    IFX_MAT_BRK_IDX(matrix, row_index, 0);
    IFX_MAT_BRK_COLS(matrix, vLen(row_values));

    for (uint32_t i = 0; i < vLen(row_values); i++)
    {
        mAt(matrix, row_index, i) = vAt(row_values, i);
    }
}

//----------------------------------------------------------------------------

void ifx_mat_set_row_vector_c(ifx_Matrix_C_t* matrix,
                              uint32_t row_index,
                              const ifx_Vector_C_t* row_values)
{
    IFX_MAT_BRK_VALID(matrix);
    IFX_VEC_BRK_VALID(row_values);
    IFX_MAT_BRK_IDX(matrix, row_index, 0);
    IFX_MAT_BRK_COLS(matrix, vLen(row_values));

    for (uint32_t i = 0; i < vLen(row_values); i++)
    {
        mAt(matrix, row_index, i) = vAt(row_values, i);
    }
}

//----------------------------------------------------------------------------

void ifx_mat_get_rowview_r(const ifx_Matrix_R_t* matrix,
                           uint32_t row_index,
                           ifx_Vector_R_t* row_view)
{
    IFX_MAT_BRK_VALID(matrix);
    IFX_ERR_BRK_NULL(row_view);

    ifx_vec_rawview_r(row_view, &mAt(matrix, row_index, 0), mCols(matrix), (uint32_t)mStride(matrix, 1));
}

//----------------------------------------------------------------------------

void ifx_mat_get_rowview_c(const ifx_Matrix_C_t* matrix,
                           uint32_t row_index,
                           ifx_Vector_C_t* row_view)
{
    IFX_MAT_BRK_VALID(matrix);
    IFX_ERR_BRK_NULL(row_view);

    ifx_vec_rawview_c(row_view, &mAt(matrix, row_index, 0), mCols(matrix), (uint32_t)mStride(matrix, 1));
}

//----------------------------------------------------------------------------

void ifx_mat_get_colview_r(const ifx_Matrix_R_t* matrix,
                           uint32_t col_index,
                           ifx_Vector_R_t* col_view)
{
    IFX_MAT_BRK_VALID(matrix);
    IFX_ERR_BRK_NULL(col_view);

    ifx_vec_rawview_r(col_view, &mAt(matrix, 0, col_index), mRows(matrix), (uint32_t)mStride(matrix, 0));
}

//----------------------------------------------------------------------------

void ifx_mat_get_colview_c(const ifx_Matrix_C_t* matrix,
                           uint32_t col_index,
                           ifx_Vector_C_t* col_view)
{
    IFX_MAT_BRK_VALID(matrix);
    IFX_ERR_BRK_NULL(col_view);

    ifx_vec_rawview_c(col_view, &mAt(matrix, 0, col_index), mRows(matrix), (uint32_t)mStride(matrix, 0));
}

//----------------------------------------------------------------------------

void ifx_mat_transpose_r(const ifx_Matrix_R_t* matrix,
                         ifx_Matrix_R_t* transposed)
{
    IFX_MAT_BRK_VALID(matrix);
    IFX_MAT_BRK_VALID(transposed);
    IFX_ERR_BRK_COND(transposed == matrix, IFX_ERROR_IN_PLACE_CALCULATION_NOT_SUPPORTED);
    IFX_ERR_BRK_COND(mRows(matrix) != mCols(transposed), IFX_ERROR_DIMENSION_MISMATCH);
    IFX_ERR_BRK_COND(mCols(matrix) != mRows(transposed), IFX_ERROR_DIMENSION_MISMATCH);

    MAT_TRANSPOSE(matrix, transposed);
}

//----------------------------------------------------------------------------

void ifx_mat_transpose_c(const ifx_Matrix_C_t* matrix,
                         ifx_Matrix_C_t* transposed)
{
    IFX_MAT_BRK_VALID(matrix);
    IFX_MAT_BRK_VALID(transposed);
    IFX_ERR_BRK_COND(transposed == matrix, IFX_ERROR_IN_PLACE_CALCULATION_NOT_SUPPORTED);
    IFX_ERR_BRK_COND(mRows(matrix) != mCols(transposed), IFX_ERROR_DIMENSION_MISMATCH);
    IFX_ERR_BRK_COND(mCols(matrix) != mRows(transposed), IFX_ERROR_DIMENSION_MISMATCH);

    MAT_TRANSPOSE(matrix, transposed);
}

//----------------------------------------------------------------------------

void ifx_mat_add_r(const ifx_Matrix_R_t* matrix_l,
                   const ifx_Matrix_R_t* matrix_r,
                   ifx_Matrix_R_t* result)
{
#define OP(a, b) ((a) + (b))
    MAT_APPLY_BINOP(matrix_l, OP, matrix_r, result);
#undef OP
}

//----------------------------------------------------------------------------

void ifx_mat_add_rs(const ifx_Matrix_R_t* input,
                    ifx_Float_t scalar,
                    ifx_Matrix_R_t* output)
{
#define OP(elem) ((elem) + scalar)
    MAT_APPLY_UNOP(input, OP, output);
#undef OP
}

//----------------------------------------------------------------------------

void ifx_mat_add_c(const ifx_Matrix_C_t* matrix_l,
                   const ifx_Matrix_C_t* matrix_r,
                   ifx_Matrix_C_t* result)
{
#define OP(a, b) ifx_complex_add((a), (b))
    MAT_APPLY_BINOP(matrix_l, OP, matrix_r, result);
#undef OP
}

//----------------------------------------------------------------------------

void ifx_mat_add_cs(const ifx_Matrix_C_t* input,
                    ifx_Complex_t scalar,
                    ifx_Matrix_C_t* output)
{
#define OP(elem) ifx_complex_add(elem, scalar)
    MAT_APPLY_UNOP(input, OP, output);
#undef OP
}

//----------------------------------------------------------------------------

void ifx_mat_sub_r(const ifx_Matrix_R_t* matrix_l,
                   const ifx_Matrix_R_t* matrix_r,
                   ifx_Matrix_R_t* result)
{
#define OP(a, b) ((a) - (b))
    MAT_APPLY_BINOP(matrix_l, OP, matrix_r, result);
#undef OP
}

//----------------------------------------------------------------------------

void ifx_mat_sub_rs(const ifx_Matrix_R_t* input,
                    ifx_Float_t scalar,
                    ifx_Matrix_R_t* output)
{
#define OP(elem) ((elem)-scalar)
    MAT_APPLY_UNOP(input, OP, output);
#undef OP
}

//----------------------------------------------------------------------------

void ifx_mat_sub_c(const ifx_Matrix_C_t* matrix_l,
                   const ifx_Matrix_C_t* matrix_r,
                   ifx_Matrix_C_t* result)
{
#define OP(a, b) ifx_complex_sub((a), (b))
    MAT_APPLY_BINOP(matrix_l, OP, matrix_r, result);
#undef OP
}

//----------------------------------------------------------------------------

void ifx_mat_sub_cs(const ifx_Matrix_C_t* input,
                    ifx_Complex_t scalar,
                    ifx_Matrix_C_t* output)
{
#define OP(elem) ifx_complex_sub(elem, scalar)
    MAT_APPLY_UNOP(input, OP, output);
#undef OP
}

//----------------------------------------------------------------------------

void ifx_mat_scale_r(const ifx_Matrix_R_t* input,
                     ifx_Float_t scale,
                     ifx_Matrix_R_t* output)
{
#define OP(elem) elem* scale
    MAT_APPLY_UNOP(input, OP, output);
#undef OP
}

//----------------------------------------------------------------------------

void ifx_mat_scale_rc(const ifx_Matrix_R_t* input,
                      ifx_Complex_t scale,
                      ifx_Matrix_C_t* output)
{
    IFX_MAT_BRK_VALID(input);
    IFX_MAT_BRK_VALID(output);
    IFX_MAT_BRK_DIM(input, output);

    for (uint32_t r = 0; r < mRows(input); r++)
    {
        for (uint32_t c = 0; c < mCols(input); c++)
        {
            ifx_Complex_t tmp_r2c = IFX_COMPLEX_DEF(mAt(input, r, c), 0);
            mAt(output, r, c) = ifx_complex_mul(tmp_r2c, scale);
        }
    }
}

//----------------------------------------------------------------------------

void ifx_mat_scale_c(const ifx_Matrix_C_t* input,
                     ifx_Complex_t scale,
                     ifx_Matrix_C_t* output)
{
#define OP(elem) ifx_complex_mul(elem, scale)
    MAT_APPLY_UNOP(input, OP, output);
#undef OP
}

//----------------------------------------------------------------------------

void ifx_mat_scale_cr(const ifx_Matrix_C_t* input,
                      ifx_Float_t scale,
                      ifx_Matrix_C_t* output)
{
#define OP(elem) ifx_complex_mul_real(elem, scale)
    MAT_APPLY_UNOP(input, OP, output);
#undef OP
}

//----------------------------------------------------------------------------

void ifx_mat_mac_r(const ifx_Matrix_R_t* m1,
                   const ifx_Matrix_R_t* m2,
                   ifx_Float_t scale,
                   ifx_Matrix_R_t* result)
{
#define OP(m1, m2) ((m1) + (scale * (m2)))
    MAT_APPLY_BINOP(m1, OP, m2, result);
#undef OP
}

//----------------------------------------------------------------------------

void ifx_mat_mac_c(const ifx_Matrix_C_t* m1,
                   const ifx_Matrix_C_t* m2,
                   ifx_Complex_t scale,
                   ifx_Matrix_C_t* output)
{
#define OP(m1, m2) ifx_complex_add((m1), ifx_complex_mul((m2), scale))
    MAT_APPLY_BINOP(m1, OP, m2, output);
#undef OP
}

//----------------------------------------------------------------------------

void ifx_mat_abs_r(const ifx_Matrix_R_t* input,
                   ifx_Matrix_R_t* output)
{
#define OP(elem) FABS(elem)
    MAT_APPLY_UNOP(input, OP, output);
#undef OP
}

//----------------------------------------------------------------------------

void ifx_mat_abs_c(const ifx_Matrix_C_t* input,
                   ifx_Matrix_R_t* output)
{
#define OP(elem) ifx_complex_abs(elem);
    MAT_APPLY_UNOP(input, OP, output);
#undef OP
}

//----------------------------------------------------------------------------

ifx_Float_t ifx_mat_sum_r(const ifx_Matrix_R_t* matrix)
{
    IFX_MAT_BRV_VALID(matrix, 0);

    ifx_Float_t result = 0;

    for (uint32_t r = 0; r < mRows(matrix); r++)
    {
        for (uint32_t c = 0; c < mCols(matrix); c++)
        {
            result += mAt(matrix, r, c);
        }
    }

    return result;
}

//----------------------------------------------------------------------------

ifx_Complex_t ifx_mat_sum_c(const ifx_Matrix_C_t* matrix)
{
    const ifx_Complex_t zero = IFX_COMPLEX_DEF(0, 0);
    IFX_MAT_BRV_VALID(matrix, zero);

    ifx_Float_t acc_r = 0;
    ifx_Float_t acc_i = 0;
    ifx_Complex_t result;

    for (uint32_t r = 0; r < mRows(matrix); r++)
    {
        for (uint32_t c = 0; c < mCols(matrix); c++)
        {
            acc_r += IFX_COMPLEX_REAL(mAt(matrix, r, c));
            acc_i += IFX_COMPLEX_IMAG(mAt(matrix, r, c));
        }
    }

    IFX_COMPLEX_SET(result, acc_r, acc_i);

    return result;
}

//----------------------------------------------------------------------------

ifx_Float_t ifx_mat_sqsum_r(const ifx_Matrix_R_t* matrix)
{
    IFX_MAT_BRV_VALID(matrix, 0);

    ifx_Float_t result = 0;

    for (uint32_t r = 0; r < mRows(matrix); r++)
    {
        for (uint32_t c = 0; c < mCols(matrix); c++)
        {
            const ifx_Float_t val = mAt(matrix, r, c);
            result += val * val;
        }
    }

    return result;
}

//----------------------------------------------------------------------------

ifx_Float_t ifx_mat_sqsum_c(const ifx_Matrix_C_t* matrix)
{
    IFX_MAT_BRV_VALID(matrix, 0);

    ifx_Float_t result = 0;

    for (uint32_t r = 0; r < mRows(matrix); r++)
    {
        for (uint32_t c = 0; c < mCols(matrix); c++)
        {
            result += ifx_complex_sqnorm(mAt(matrix, r, c));
        }
    }

    return result;
}

//----------------------------------------------------------------------------

ifx_Float_t ifx_mat_maxabs_r(const ifx_Matrix_R_t* matrix)
{
    IFX_MAT_BRV_VALID(matrix, 0);

    ifx_Float_t result = FABS(mAt(matrix, 0, 0));

    for (uint32_t r = 0; r < mRows(matrix); r++)
    {
        for (uint32_t c = 0; c < mCols(matrix); c++)
        {
            const ifx_Float_t val = FABS(mAt(matrix, r, c));

            if (val > result)
            {
                result = val;
            }
        }
    }

    return result;
}

//----------------------------------------------------------------------------

ifx_Float_t ifx_mat_maxabs_c(const ifx_Matrix_C_t* matrix)
{
    IFX_MAT_BRV_VALID(matrix, 0);

    ifx_Float_t max = ifx_complex_abs(mAt(matrix, 0, 0));
    ifx_Float_t result = 0.f;

    for (uint32_t r = 0; r < mRows(matrix); r++)
    {
        for (uint32_t c = 0; c < mCols(matrix); c++)
        {
            const ifx_Float_t val = ifx_complex_sqnorm(mAt(matrix, r, c));

            if (val > max)
            {
                max = val;
            }
        }
    }
    result = SQRT(max);

    return result;
}

//----------------------------------------------------------------------------

ifx_Float_t ifx_mat_mean_r(const ifx_Matrix_R_t* matrix)
{
    IFX_MAT_BRV_VALID(matrix, 0);

    const uint32_t mat_dim = IFX_MAT_SIZE(matrix);
    ifx_Float_t result = ifx_mat_sum_r(matrix) / mat_dim;

    return result;
}

//----------------------------------------------------------------------------

ifx_Complex_t ifx_mat_mean_c(const ifx_Matrix_C_t* matrix)
{
    const ifx_Complex_t zero = IFX_COMPLEX_DEF(0, 0);
    IFX_MAT_BRV_VALID(matrix, zero);

    const uint32_t mat_dim = IFX_MAT_SIZE(matrix);
    const ifx_Complex_t mat_sum = ifx_mat_sum_c(matrix);

    ifx_Complex_t result = ifx_complex_div_real(mat_sum, (ifx_Float_t)mat_dim);

    return result;
}

//----------------------------------------------------------------------------

ifx_Float_t ifx_mat_max_r(const ifx_Matrix_R_t* matrix)
{
    IFX_MAT_BRV_VALID(matrix, 0);

    ifx_Float_t result = mAt(matrix, 0, 0);

    for (uint32_t r = 0; r < mRows(matrix); r++)
    {
        for (uint32_t c = 0; c < mCols(matrix); c++)
        {
            const ifx_Float_t val = mAt(matrix, r, c);

            if (val > result)
            {
                result = val;
            }
        }
    }

    return result;
}

//----------------------------------------------------------------------------

ifx_Float_t ifx_mat_var_r(const ifx_Matrix_R_t* matrix)
{
    IFX_MAT_BRV_VALID(matrix, 0);

    const uint32_t mat_dim = IFX_MAT_SIZE(matrix);
    const ifx_Float_t mean = ifx_mat_mean_r(matrix);
    ifx_Float_t acc = 0;

    for (uint32_t r = 0; r < mRows(matrix); r++)
    {
        for (uint32_t c = 0; c < mCols(matrix); c++)
        {
            acc += (mAt(matrix, r, c) - mean) * (mAt(matrix, r, c) - mean);
        }
    }
    ifx_Float_t result = acc / mat_dim;

    return result;
}

//----------------------------------------------------------------------------

void ifx_mat_abt_r(const ifx_Matrix_R_t* inputA,
                   const ifx_Matrix_R_t* inputB,
                   ifx_Matrix_R_t* output)
{
    IFX_MAT_BRK_VALID(inputA);
    IFX_MAT_BRK_VALID(inputB);
    IFX_MAT_BRK_VALID(output);
    // check dimension size
    IFX_ERR_BRK_COND((mRows(inputA) != mRows(output))
                         || (mRows(inputB) != mCols(output))
                         || (mCols(inputA) != mCols(inputB)),
                     IFX_ERROR_DIMENSION_MISMATCH)

    for (uint32_t i_row = 0; i_row < mRows(inputA); ++i_row)
    {
        for (uint32_t j_row = 0; j_row < mRows(inputB); ++j_row)
        {
            ifx_Float_t sum = 0;
            for (uint32_t col = 0; col < mCols(inputA); ++col)
                sum += mAt(inputA, i_row, col) * mAt(inputB, j_row, col);

            mAt(output, i_row, j_row) = sum;
        }
    }
}

//----------------------------------------------------------------------------

void ifx_mat_abct_c(const ifx_Matrix_C_t* inputA,
                    const ifx_Matrix_C_t* inputB,
                    ifx_Matrix_C_t* output)
{
    IFX_MAT_BRK_VALID(inputA);
    IFX_MAT_BRK_VALID(inputB);
    IFX_MAT_BRK_VALID(output);
    // check dimension size
    IFX_ERR_BRK_COND((mRows(inputA) != mRows(output))
                         || (mRows(inputB) != mCols(output))
                         || (mCols(inputA) != mCols(inputB)),
                     IFX_ERROR_DIMENSION_MISMATCH)

    for (uint32_t i_row = 0; i_row < mRows(inputA); ++i_row)
    {
        for (uint32_t j_row = 0; j_row < mRows(inputB); ++j_row)
        {
            ifx_Complex_t sum = IFX_COMPLEX_DEF(0, 0);
            for (uint32_t col = 0; col < mCols(inputA); ++col)
            {
                ifx_Float_t ra = IFX_COMPLEX_REAL(mAt(inputA, i_row, col));
                ifx_Float_t ia = IFX_COMPLEX_IMAG(mAt(inputA, i_row, col));
                ifx_Float_t rb = IFX_COMPLEX_REAL(mAt(inputB, j_row, col));
                ifx_Float_t ib = IFX_COMPLEX_IMAG(mAt(inputB, j_row, col));
                ifx_Complex_t tmp = IFX_COMPLEX_DEF(ra * rb + ia * ib, -ra * ib + rb * ia);
                sum = ifx_complex_add(sum, tmp);
            }
            mAt(output, i_row, j_row) = sum;
        }
    }
}

//----------------------------------------------------------------------------

void ifx_mat_abt_c(const ifx_Matrix_C_t* inputA,
                   const ifx_Matrix_C_t* inputB,
                   ifx_Matrix_C_t* output)
{
    IFX_MAT_BRK_VALID(inputA);
    IFX_MAT_BRK_VALID(inputB);
    IFX_MAT_BRK_VALID(output);
    // check dimension size
    IFX_ERR_BRK_COND((mRows(inputA) != mRows(output))
                         || (mRows(inputB) != mCols(output))
                         || (mCols(inputA) != mCols(inputB)),
                     IFX_ERROR_DIMENSION_MISMATCH)

    for (uint32_t i_row = 0; i_row < mRows(inputA); ++i_row)
    {
        for (uint32_t j_row = 0; j_row < mRows(inputB); ++j_row)
        {
            ifx_Complex_t sum = IFX_COMPLEX_DEF(0, 0);
            for (uint32_t col = 0; col < mCols(inputA); ++col)
            {
                ifx_Complex_t tmp = ifx_complex_mul(mAt(inputA, i_row, col), mAt(inputB, j_row, col));
                sum = ifx_complex_add(sum, tmp);
            }
            mAt(output, i_row, j_row) = sum;
        }
    }
}

//----------------------------------------------------------------------------

void ifx_mat_abt_rc(const ifx_Matrix_R_t* inputA,
                    const ifx_Matrix_C_t* inputB,
                    ifx_Matrix_C_t* output)
{
    IFX_MAT_BRK_VALID(inputA);
    IFX_MAT_BRK_VALID(inputB);
    IFX_MAT_BRK_VALID(output);
    // check dimension size
    IFX_ERR_BRK_COND((mRows(inputA) != mRows(output))
                         || (mRows(inputB) != mCols(output))
                         || (mCols(inputA) != mCols(inputB)),
                     IFX_ERROR_DIMENSION_MISMATCH)

    for (uint32_t i_row = 0; i_row < mRows(inputA); ++i_row)
    {
        for (uint32_t j_row = 0; j_row < mRows(inputB); ++j_row)
        {
            ifx_Complex_t sum = IFX_COMPLEX_DEF(0, 0);
            for (uint32_t col = 0; col < mCols(inputA); ++col)
            {
                ifx_Complex_t tmp = ifx_complex_mul_real(mAt(inputB, j_row, col), mAt(inputA, i_row, col));
                sum = ifx_complex_add(sum, tmp);
            }
            mAt(output, i_row, j_row) = sum;
        }
    }
}

//----------------------------------------------------------------------------

void ifx_mat_abt_cr(const ifx_Matrix_C_t* inputA,
                    const ifx_Matrix_R_t* inputB,
                    ifx_Matrix_C_t* output)
{
    IFX_MAT_BRK_VALID(inputA);
    IFX_MAT_BRK_VALID(inputB);
    IFX_MAT_BRK_VALID(output);
    // check dimension size
    IFX_ERR_BRK_COND((mRows(inputA) != mRows(output))
                         || (mRows(inputB) != mCols(output))
                         || (mCols(inputA) != mCols(inputB)),
                     IFX_ERROR_DIMENSION_MISMATCH)

    for (uint32_t i_row = 0; i_row < mRows(inputA); ++i_row)
    {
        for (uint32_t j_row = 0; j_row < mRows(inputB); ++j_row)
        {
            ifx_Complex_t sum = IFX_COMPLEX_DEF(0, 0);
            for (uint32_t col = 0; col < mCols(inputA); ++col)
            {
                ifx_Complex_t tmp = ifx_complex_mul_real(mAt(inputA, i_row, col), mAt(inputB, j_row, col));
                sum = ifx_complex_add(sum, tmp);
            }
            mAt(output, i_row, j_row) = sum;
        }
    }
}

//----------------------------------------------------------------------------

void ifx_mat_atb_r(const ifx_Matrix_R_t* inputA,
                   const ifx_Matrix_R_t* inputB,
                   ifx_Matrix_R_t* output)
{
    IFX_MAT_BRK_VALID(inputA);
    IFX_MAT_BRK_VALID(inputB);
    IFX_MAT_BRK_VALID(output);
    // check dimension size
    IFX_ERR_BRK_COND((mCols(inputA) != mRows(output))
                         || (mCols(inputB) != mCols(output))
                         || (mRows(inputA) != mRows(inputB)),
                     IFX_ERROR_DIMENSION_MISMATCH)

    for (uint32_t i_col = 0; i_col < mCols(inputA); ++i_col)
    {
        for (uint32_t j_col = 0; j_col < mCols(inputB); ++j_col)
        {
            ifx_Float_t sum = 0;
            for (uint32_t row = 0; row < mRows(inputA); ++row)
                sum += mAt(inputA, row, i_col) * mAt(inputB, row, j_col);

            mAt(output, i_col, j_col) = sum;
        }
    }
}

//----------------------------------------------------------------------------

void ifx_mat_atb_c(const ifx_Matrix_C_t* inputA,
                   const ifx_Matrix_C_t* inputB,
                   ifx_Matrix_C_t* output)
{
    IFX_MAT_BRK_VALID(inputA);
    IFX_MAT_BRK_VALID(inputB);
    IFX_MAT_BRK_VALID(output);
    // check dimension size
    IFX_ERR_BRK_COND((mCols(inputA) != mRows(output))
                         || (mCols(inputB) != mCols(output))
                         || (mRows(inputA) != mRows(inputB)),
                     IFX_ERROR_DIMENSION_MISMATCH)

    for (uint32_t i_col = 0; i_col < mCols(inputA); ++i_col)
    {
        for (uint32_t j_col = 0; j_col < mCols(inputB); ++j_col)
        {
            ifx_Complex_t sum = IFX_COMPLEX_DEF(0, 0);
            for (uint32_t row = 0; row < mRows(inputA); ++row)
            {
                ifx_Complex_t tmp = ifx_complex_mul(mAt(inputA, row, i_col), mAt(inputB, row, j_col));
                sum = ifx_complex_add(sum, tmp);
            }
            mAt(output, i_col, j_col) = sum;
        }
    }
}

//----------------------------------------------------------------------------

void ifx_mat_atb_rc(const ifx_Matrix_R_t* inputA,
                    const ifx_Matrix_C_t* inputB,
                    ifx_Matrix_C_t* output)
{
    IFX_MAT_BRK_VALID(inputA);
    IFX_MAT_BRK_VALID(inputB);
    IFX_MAT_BRK_VALID(output);
    // check dimension size
    IFX_ERR_BRK_COND((mCols(inputA) != mRows(output))
                         || (mCols(inputB) != mCols(output))
                         || (mRows(inputA) != mRows(inputB)),
                     IFX_ERROR_DIMENSION_MISMATCH)

    for (uint32_t i_col = 0; i_col < mCols(inputA); ++i_col)
    {
        for (uint32_t j_col = 0; j_col < mCols(inputB); ++j_col)
        {
            ifx_Complex_t sum = IFX_COMPLEX_DEF(0, 0);
            for (uint32_t row = 0; row < mRows(inputA); ++row)
            {
                ifx_Complex_t tmp = ifx_complex_mul_real(mAt(inputB, row, j_col), mAt(inputA, row, i_col));
                sum = ifx_complex_add(sum, tmp);
            }
            mAt(output, i_col, j_col) = sum;
        }
    }
}

//----------------------------------------------------------------------------

void ifx_mat_atb_cr(const ifx_Matrix_C_t* inputA,
                    const ifx_Matrix_R_t* inputB,
                    ifx_Matrix_C_t* output)
{
    IFX_MAT_BRK_VALID(inputA);
    IFX_MAT_BRK_VALID(inputB);
    IFX_MAT_BRK_VALID(output);
    // check dimension size
    IFX_ERR_BRK_COND((mCols(inputA) != mRows(output))
                         || (mCols(inputB) != mCols(output))
                         || (mRows(inputA) != mRows(inputB)),
                     IFX_ERROR_DIMENSION_MISMATCH)

    for (uint32_t i_col = 0; i_col < mCols(inputA); ++i_col)
    {
        for (uint32_t j_col = 0; j_col < mCols(inputB); ++j_col)
        {
            ifx_Complex_t sum = IFX_COMPLEX_DEF(0, 0);
            for (uint32_t row = 0; row < mRows(inputA); ++row)
            {
                ifx_Complex_t tmp = ifx_complex_mul_real(mAt(inputA, row, i_col), mAt(inputB, row, j_col));
                sum = ifx_complex_add(sum, tmp);
            }
            mAt(output, i_col, j_col) = sum;
        }
    }
}

//----------------------------------------------------------------------------

void ifx_mat_mul_rv(const ifx_Matrix_R_t* matrix,
                    const ifx_Vector_R_t* vector,
                    ifx_Vector_R_t* result)
{
    IFX_MAT_BRK_VALID(matrix);
    IFX_VEC_BRK_VALID(vector);
    IFX_VEC_BRK_VALID(result);

    // operation:  result = matrix * vector
    // dimensions:    M       MxN      N
    IFX_ERR_BRK_COND(mRows(matrix) != IFX_VEC_LEN(result), IFX_ERROR_DIMENSION_MISMATCH);
    IFX_ERR_BRK_COND(mCols(matrix) != IFX_VEC_LEN(vector), IFX_ERROR_DIMENSION_MISMATCH);

    // perform matrix vector computation: result = matrix*vector
    // using Einstein sum convention: result_j = matrix_{jk} vector_k
    for (uint32_t j = 0; j < mRows(matrix); j++)
    {
        ifx_Float_t s = 0;

        for (uint32_t k = 0; k < mCols(matrix); k++)
        {
            s += IFX_MAT_AT(matrix, j, k) * IFX_VEC_AT(vector, k);
        }

        IFX_VEC_AT(result, j) = s;
    }
}

//----------------------------------------------------------------------------

void ifx_mat_mul_trans_rv(const ifx_Matrix_R_t* matrix,
                          const ifx_Vector_R_t* vector,
                          ifx_Vector_R_t* result)
{
    IFX_MAT_BRK_VALID(matrix);
    IFX_VEC_BRK_VALID(vector);
    IFX_VEC_BRK_VALID(result);

    // operation:  result = matrix * vector
    // dimensions:    M       NxM      N
    IFX_ERR_BRK_COND(mCols(matrix) != IFX_VEC_LEN(result), IFX_ERROR_DIMENSION_MISMATCH);
    IFX_ERR_BRK_COND(mRows(matrix) != IFX_VEC_LEN(vector), IFX_ERROR_DIMENSION_MISMATCH);

    // perform matrix vector computation: result = matrix*vector
    // using Einstein sum convention: result_j = matrix_{kj} vector_k
    for (uint32_t j = 0; j < mRows(matrix); j++)
    {
        ifx_Float_t s = 0;

        for (uint32_t k = 0; k < mCols(matrix); k++)
        {
            s += IFX_MAT_AT(matrix, k, j) * IFX_VEC_AT(vector, k);
        }

        IFX_VEC_AT(result, j) = s;
    }
}

//----------------------------------------------------------------------------

void ifx_mat_mul_cv(const ifx_Matrix_C_t* matrix,
                    const ifx_Vector_C_t* vector,
                    ifx_Vector_C_t* result)
{
    IFX_MAT_BRK_VALID(matrix);
    IFX_VEC_BRK_VALID(vector);
    IFX_VEC_BRK_VALID(result);

    // operation:  result = matrix * vector
    // dimensions:    M       MxN      N
    IFX_ERR_BRK_COND(mRows(matrix) != IFX_VEC_LEN(result), IFX_ERROR_DIMENSION_MISMATCH);
    IFX_ERR_BRK_COND(mCols(matrix) != IFX_VEC_LEN(vector), IFX_ERROR_DIMENSION_MISMATCH);

    // perform matrix vector computation: result = matrix*vector
    // using Einstein sum convention: result_j = matrix_{jk} vector_k
    for (uint32_t j = 0; j < mRows(matrix); j++)
    {
        ifx_Complex_t s = IFX_COMPLEX_DEF(0, 0);

        for (uint32_t k = 0; k < mCols(matrix); k++)
        {
            s = ifx_complex_add(s, ifx_complex_mul(IFX_MAT_AT(matrix, j, k), IFX_VEC_AT(vector, k)));
        }

        IFX_VEC_AT(result, j) = s;
    }
}

//----------------------------------------------------------------------------

void ifx_mat_trans_mul_cv(const ifx_Matrix_C_t* matrix,
                          const ifx_Vector_C_t* vector,
                          ifx_Vector_C_t* result)
{
    IFX_MAT_BRK_VALID(matrix);
    IFX_VEC_BRK_VALID(vector);
    IFX_VEC_BRK_VALID(result);

    // operation:  result = matrix * vector
    // dimensions:    M       MxN      N
    IFX_ERR_BRK_COND(mCols(matrix) != IFX_VEC_LEN(result), IFX_ERROR_DIMENSION_MISMATCH);
    IFX_ERR_BRK_COND(mRows(matrix) != IFX_VEC_LEN(vector), IFX_ERROR_DIMENSION_MISMATCH);

    // perform matrix vector computation: result = matrix*vector
    // using Einstein sum convention: result_j = matrix_{kj} vector_k
    for (uint32_t j = 0; j < mRows(matrix); j++)
    {
        ifx_Complex_t s = IFX_COMPLEX_DEF(0, 0);

        for (uint32_t k = 0; k < mCols(matrix); k++)
        {
            s = ifx_complex_add(s, ifx_complex_mul(IFX_MAT_AT(matrix, k, j), IFX_VEC_AT(vector, k)));
        }

        IFX_VEC_AT(result, j) = s;
    }
}

//----------------------------------------------------------------------------

void ifx_mat_mul_r(const ifx_Matrix_R_t* matrix_l,
                   const ifx_Matrix_R_t* matrix_r,
                   ifx_Matrix_R_t* result)
{
    IFX_MAT_BRK_VALID(matrix_l);
    IFX_MAT_BRK_VALID(matrix_r);
    IFX_MAT_BRK_VALID(result);
    IFX_MAT_BRK_DIM_ROW(matrix_l, result);
    IFX_MAT_BRK_DIM_COL(matrix_r, result);
    IFX_MAT_BRK_DIM_COL_ROW(matrix_l, matrix_r);

    /* result_{jk} = (matrix_l)_{jl} * (matrix_r)_{lk} */

    for (uint32_t j = 0; j < mRows(matrix_l); j++)
    {
        for (uint32_t k = 0; k < mCols(matrix_r); k++)
        {
            ifx_Float_t sum = 0;

            for (uint32_t l = 0; l < mCols(matrix_l); l++)
            {
                sum += IFX_MAT_AT(matrix_l, j, l) * IFX_MAT_AT(matrix_r, l, k);
            }

            IFX_MAT_AT(result, j, k) = sum;
        }
    }
}

//----------------------------------------------------------------------------

void ifx_mat_mul_rc(const ifx_Matrix_R_t* matrix_l,
                    const ifx_Matrix_C_t* matrix_r,
                    ifx_Matrix_C_t* result)
{
    IFX_MAT_BRK_VALID(matrix_l);
    IFX_MAT_BRK_VALID(matrix_r);
    IFX_MAT_BRK_VALID(result);
    IFX_MAT_BRK_DIM_ROW(matrix_l, result);
    IFX_MAT_BRK_DIM_COL(matrix_r, result);
    IFX_MAT_BRK_DIM_COL_ROW(matrix_l, matrix_r);

    /* result_{jk} = (matrix_l)_{jl} * (matrix_r)_{lk} */

    for (uint32_t j = 0; j < mRows(matrix_l); j++)
    {
        for (uint32_t k = 0; k < mCols(matrix_r); k++)
        {
            ifx_Complex_t sum = IFX_COMPLEX_DEF(0, 0);

            for (uint32_t l = 0; l < mCols(matrix_l); l++)
            {
                ifx_Float_t a = IFX_MAT_AT(matrix_l, j, l);
                ifx_Complex_t b = IFX_MAT_AT(matrix_r, l, k);
                sum = ifx_complex_add(sum, ifx_complex_mul_real(b, a));
            }

            IFX_MAT_AT(result, j, k) = sum;
        }
    }
}

//----------------------------------------------------------------------------

void ifx_mat_mul_c(const ifx_Matrix_C_t* matrix_l,
                   const ifx_Matrix_C_t* matrix_r,
                   ifx_Matrix_C_t* result)
{
    IFX_MAT_BRK_VALID(matrix_l);
    IFX_MAT_BRK_VALID(matrix_r);
    IFX_MAT_BRK_VALID(result);
    IFX_MAT_BRK_DIM_ROW(matrix_l, result);
    IFX_MAT_BRK_DIM_COL(matrix_r, result);
    IFX_MAT_BRK_DIM_COL_ROW(matrix_l, matrix_r);

    /* result_{jk} = (matrix_l)_{jl} * (matrix_r)_{lk} */

    for (uint32_t j = 0; j < mRows(matrix_l); j++)
    {
        for (uint32_t k = 0; k < mCols(matrix_r); k++)
        {
            ifx_Complex_t sum = IFX_COMPLEX_DEF(0, 0);

            for (uint32_t l = 0; l < mCols(matrix_l); l++)
            {
                ifx_Complex_t a = IFX_MAT_AT(matrix_l, j, l);
                ifx_Complex_t b = IFX_MAT_AT(matrix_r, l, k);
                sum = ifx_complex_add(sum, ifx_complex_mul(a, b));
            }

            IFX_MAT_AT(result, j, k) = sum;
        }
    }
}

//----------------------------------------------------------------------------

void ifx_mat_mul_cr(const ifx_Matrix_C_t* matrix_l,
                    const ifx_Matrix_R_t* matrix_r,
                    ifx_Matrix_C_t* result)
{
    IFX_MAT_BRK_VALID(matrix_l);
    IFX_MAT_BRK_VALID(matrix_r);
    IFX_MAT_BRK_VALID(result);
    IFX_MAT_BRK_DIM_ROW(matrix_l, result);
    IFX_MAT_BRK_DIM_COL(matrix_r, result);
    IFX_MAT_BRK_DIM_COL_ROW(matrix_l, matrix_r);

    /* result_{jk} = (matrix_l)_{jl} * (matrix_r)_{lk} */

    for (uint32_t j = 0; j < mRows(matrix_l); j++)
    {
        for (uint32_t k = 0; k < mCols(matrix_r); k++)
        {
            ifx_Complex_t sum = IFX_COMPLEX_DEF(0, 0);

            for (uint32_t l = 0; l < mCols(matrix_l); l++)
            {
                ifx_Complex_t a = IFX_MAT_AT(matrix_l, j, l);
                ifx_Float_t b = IFX_MAT_AT(matrix_r, l, k);
                sum = ifx_complex_add(sum, ifx_complex_mul_real(a, b));
            }

            IFX_MAT_AT(result, j, k) = sum;
        }
    }
}

//----------------------------------------------------------------------------

void ifx_mat_clear_r(ifx_Matrix_R_t* matrix)
{
    IFX_MAT_BRK_VALID(matrix);

    ifx_mda_clear_r(matrix);
}

//----------------------------------------------------------------------------

void ifx_mat_clear_c(ifx_Matrix_C_t* matrix)
{
    IFX_MAT_BRK_VALID(matrix);

    ifx_mda_clear_c(matrix);
}

//----------------------------------------------------------------------------

ifx_Matrix_R_t* ifx_mat_clone_r(const ifx_Matrix_R_t* input)
{
    return ifx_mda_clone_r(input);
}

//----------------------------------------------------------------------------

ifx_Matrix_C_t* ifx_mat_clone_c(const ifx_Matrix_C_t* input)
{
    return ifx_mda_clone_c(input);
}
