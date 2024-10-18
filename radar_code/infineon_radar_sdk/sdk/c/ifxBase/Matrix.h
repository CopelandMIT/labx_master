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
 * @file Matrix.h
 *
 * \brief \copybrief gr_matrix
 *
 * For details refer to \ref gr_matrix
 */

#ifndef IFX_BASE_MATRIX_H
#define IFX_BASE_MATRIX_H

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include "Mda.h"
#include "Types.h"
#include "Vector.h"


#ifdef __cplusplus
extern "C"
{
#endif


/*
==============================================================================
   2. DEFINITIONS
==============================================================================
*/

// Access macros -------------------------------------------------------------
#define IFX_MAT_DAT(m)          IFX_MDA_DATA(m)
#define IFX_MAT_ROWS(m)         (IFX_MDA_SHAPE(m)[0])
#define IFX_MAT_COLS(m)         (IFX_MDA_SHAPE(m)[1])
#define IFX_MAT_STRIDE(m, i)    (IFX_MDA_STRIDE(m)[i])
#define IFX_MAT_SIZE(m)         ((size_t)IFX_MAT_COLS(m) * (size_t)IFX_MAT_ROWS(m))
#define IFX_MAT_OFFSET(m, r, c) IFX_MDA_OFFSET(m, r, c)

/** @brief Access matrix element
 *
 * The macro can be used to set and get elements of a matrix, for example:
 * @code
 * foo = IFX_MAT_AT(matrix, row, col);
 * IFX_MAT_AT(matrix, row, col) = bar;
 * @endcode
 *
 * The macro works with both real (\ref ifx_Matrix_R_t) and complex (\ref ifx_Matrix_C_t) matrices.
 */
#define IFX_MAT_AT(m, r, c) IFX_MDA_AT(m, r, c)

// Condition check macro adaptations for Matrix module -----------------------
#define IFX_MAT_BRK_DIM(m1, m2)    IFX_ERR_BRK_COND((mCols(m1) != mCols(m2)) || (mRows(m1) != mRows(m2)), IFX_ERROR_DIMENSION_MISMATCH)
#define IFX_MAT_BRV_DIM(m1, m2, v) IFX_ERR_BRV_COND((mCols(m1) != mCols(m2)) || (mRows(m1) != mRows(m2)), IFX_ERROR_DIMENSION_MISMATCH, v)

#define IFX_MAT_BRK_SIZE(m1, m2)    IFX_ERR_BRK_COND(mSize(m1) != mSize(m2), IFX_ERROR_DIMENSION_MISMATCH)
#define IFX_MAT_BRV_SIZE(m1, m2, v) IFX_ERR_BRV_COND(mSize(m1) != mSize(m2), IFX_ERROR_DIMENSION_MISMATCH, v)

#define IFX_MAT_BRK_SQUARE(m)    IFX_ERR_BRK_COND(mRows(m) != mCols(m), IFX_ERROR_DIMENSION_MISMATCH)
#define IFX_MAT_BRV_SQUARE(m, v) IFX_ERR_BRV_COND(mRows(m) != mCols(m), IFX_ERROR_DIMENSION_MISMATCH, v)

#define IFX_MAT_BRK_DIM_COL_ROW(m1, m2)    IFX_ERR_BRK_COND(mCols(m1) != mRows(m2), IFX_ERROR_DIMENSION_MISMATCH)
#define IFX_MAT_BRV_DIM_COL_ROW(m1, m2, v) IFX_ERR_BRV_COND(mCols(m1) != mRows(m2), IFX_ERROR_DIMENSION_MISMATCH, v)

#define IFX_MAT_BRK_DIM_COL(m1, m2)    IFX_ERR_BRK_COND(mCols(m1) != mCols(m2), IFX_ERROR_DIMENSION_MISMATCH)
#define IFX_MAT_BRV_DIM_COL(m1, m2, v) IFX_ERR_BRV_COND(mCols(m1) != mCols(m2), IFX_ERROR_DIMENSION_MISMATCH, v)

#define IFX_MAT_BRK_DIM_ROW(m1, m2)    IFX_ERR_BRK_COND(mRows(m1) != mRows(m2), IFX_ERROR_DIMENSION_MISMATCH)
#define IFX_MAT_BRV_DIM_ROW(m1, m2, v) IFX_ERR_BRV_COND(mRows(m1) != mRows(m2), IFX_ERROR_DIMENSION_MISMATCH, v)

#define IFX_MAT_BRK_IDX(m, r, c)    IFX_ERR_BRK_COND(((r) >= mRows(m)) || ((c) >= mCols(m)), IFX_ERROR_INDEX_OUT_OF_BOUNDS)
#define IFX_MAT_BRV_IDX(m, r, c, v) IFX_ERR_BRV_COND(((r) >= mRows(m)) || ((c) >= mCols(m)), IFX_ERROR_INDEX_OUT_OF_BOUNDS, v)

#define IFX_MAT_BRK_ROWS(m, r)    IFX_ERR_BRK_COND((r) > mRows(m), IFX_ERROR_INDEX_OUT_OF_BOUNDS)
#define IFX_MAT_BRV_ROWS(m, r, v) IFX_ERR_BRK_COND((r) > mRows(m), IFX_ERROR_INDEX_OUT_OF_BOUNDS, v)

#define IFX_MAT_BRK_COLS(m, c)    IFX_ERR_BRK_COND((c) > mCols(m), IFX_ERROR_INDEX_OUT_OF_BOUNDS)
#define IFX_MAT_BRV_COLS(m, c, v) IFX_ERR_BRK_COND((c) > mCols(m), IFX_ERROR_INDEX_OUT_OF_BOUNDS, v)

#define IFX_MAT_BRK_VALID(m)                                                        \
    do                                                                              \
    {                                                                               \
        IFX_ERR_BRK_NULL(m);                                                        \
        IFX_ERR_BRK_COND(IFX_MDA_DIMENSIONS(m) != 2, IFX_ERROR_DIMENSION_MISMATCH); \
        IFX_ERR_BRK_ARGUMENT(IFX_MDA_DATA(m) == NULL)                               \
    } while (0)
#define IFX_MAT_BRV_VALID(m, r)                                                        \
    do                                                                                 \
    {                                                                                  \
        IFX_ERR_BRV_NULL(m, r);                                                        \
        IFX_ERR_BRV_COND(IFX_MDA_DIMENSIONS(m) != 2, IFX_ERROR_DIMENSION_MISMATCH, r); \
        IFX_ERR_BRV_ARGUMENT(IFX_MDA_DATA(m) == NULL, r)                               \
    } while (0)

/*
==============================================================================
   3. TYPES
==============================================================================
*/

/**
 * @brief Forward declaration structure for real Matrix.
 */
typedef ifx_Mda_R_t ifx_Matrix_R_t;

/**
 * @brief Forward declaration structure for complex Matrix.
 */
typedef ifx_Mda_C_t ifx_Matrix_C_t;


/*
==============================================================================
   4. FUNCTION PROTOTYPES
==============================================================================
*/

/** @addtogroup gr_cat_SDK_base
 * @{
 */

/** @defgroup gr_matrix Matrix
 * @brief API for operations on Matrix data structures
 *
 * Supports matrix operations such as creation, destruction
 *        and mathematical manipulations.
 *
 * @{
 */

/**
 * @brief Assigns real raw data to the \ref ifx_Matrix_R_t structure.
 * Example usage:
 *
 *   Given \ref ifx_Matrix_R_t out_matrix, dimensions of the matrix num_rows and num_cols
 *   and real raw data in array arr_data of size num_rows * num_cols, one can assign:
 * @code
 *     ifx_mat_rawview_r(&out_matrix, arr_data, num_rows, num_cols, num_cols);
 * @endcode
 *   e.g. for 2x2 identity matrix one can assign:
 * @code
 *     ifx_Float_t arr_data[2*2] = {1, 0, 0, 1};
 *     ifx_mat_rawview_r(&out_matrix, arr_data, 2, 2, 2);
 * @endcode
 *
 * @param [in,out] matrix    Pointer to data memory defined by \ref ifx_Matrix_R_t
 * @param [in]     d         Data pointer to assign the matrix
 * @param [in]     rows      Number of row
 * @param [in]     columns   Number of columns
 * @param [in]     lda       Number of sequential memory locations to jump for the next row
 *
 */
IFX_DLL_PUBLIC
void ifx_mat_rawview_r(ifx_Matrix_R_t* matrix,
                       ifx_Float_t* d,
                       uint32_t rows,
                       uint32_t columns,
                       uint32_t lda);

/**
 * @brief Assigns complex raw data to the \ref ifx_Matrix_C_t structure.
 * Example usage:
 *
 *   Given \ref ifx_Matrix_C_t out_matrix, dimensions of the matrix num_rows and num_cols
 *   and complex data in array arr_data of size num_rows * num_cols, one can assign:
 * @code
 *     ifx_mat_rawview_c(&out_matrix, arr_data, num_rows, num_cols, num_cols);
 * @endcode
 *   e.g. for 2x2 matrix one can assign:
 * @code
 *     ifx_Complex_t arr_data[2*2] = {{1,1}, {0,0}, {0,0}, {1,1}};
 *     ifx_mat_rawview_c(&out_matrix, arr_data, 2, 2, 2);
 * @endcode
 *
 * @param [in,out] matrix    Pointer to data memory defined by \ref ifx_Matrix_C_t
 * @param [in]     d         Data pointer to assign the matrix
 * @param [in]     rows      Number of rows
 * @param [in]     columns   Number of columns
 * @param [in]     lda       Number of sequential memory locations to jump for the next row
 *
 */
IFX_DLL_PUBLIC
void ifx_mat_rawview_c(ifx_Matrix_C_t* matrix,
                       ifx_Complex_t* d,
                       uint32_t rows,
                       uint32_t columns,
                       uint32_t lda);

/**
 * @brief Assigns real data from source matrix to the destination matrix.
 * Example usage:
 *
 *   Given \ref ifx_Matrix_R_t dest_matrix and \ref ifx_Matrix_R_t source_matrix,
 *   dimensions of the dest_matrix num_rows and num_cols and
 *   offset for rows and columns of source_matrix one can assign:
 * @code
 *     ifx_mat_view_r(&dest_matrix, &source_matrix, row_offset, col_offset, num_rows, num_cols);
 * @endcode
 *   e.g. for taking a view of 2x2 matrix from 3x3 matrix:
 * @code
 *     ifx_Float_t arr_data[3*3] = {1, 2, 3,
 *                                  4, 5, 6,
 *                                  7, 8, 9};
 *     ifx_mat_rawview_r(&source_matrix, arr_data, 3, 3, 3);
 *
 *     ifx_mat_view_r(&dest_matrix, &source_matrix, 1, 1, 2, 2);
 * @endcode
 *     will give a dest_matrix with values:
 * @code
 *     [5, 6,
 *      8, 9]
 * @endcode
 *
 * @param [in,out] matrix              Pointer to data memory defined by \ref ifx_Matrix_R_t
 * @param [in]     source              Pointer to data memory defined by \ref ifx_Matrix_R_t
 * @param [in]     row_offset          Row offset
 * @param [in]     column_offset       Column offset
 * @param [in]     rows                Number of rows to view
 * @param [in]     columns             Number of columns to view
 *
 */
IFX_DLL_PUBLIC
void ifx_mat_view_r(ifx_Matrix_R_t* matrix,
                    ifx_Matrix_R_t* source,
                    uint32_t row_offset,
                    uint32_t column_offset,
                    uint32_t rows,
                    uint32_t columns);

/**
 * @brief Assigns complex data from source matrix to the destination matrix.
 * Example usage:
 *
 *   Given \ref ifx_Matrix_C_t dest_matrix and \ref ifx_Matrix_C_t source_matrix,
 *   dimensions of the dest_matrix num_rows and num_cols and
 *   offset for rows and columns of source_matrix one can assign:
 * @code
 *     ifx_mat_view_c(&dest_matrix, &source_matrix, row_offset, col_offset, num_rows, num_cols);
 * @endcode
 *   e.g. for taking a view of 1x2 matrix from 2x2 matrix (second row):
 * @code
 *     ifx_Complex_t arr_data[2*2] = {{1, 1}, {2, 2}
 *                                    {3, 3}, {4, 4}};
 *     ifx_mat_rawview_c(&source_matrix, arr_data, 2, 2, 2);
 *
 *     ifx_mat_view_c(&dest_matrix, &source_matrix, 1, 0, 1, 2);
 * @endcode
 *     will give a dest_matrix with values:
 * @code
 *     [{3, 3}, {4, 4}]
 * @endcode
 *
 * @param [in,out] matrix              Pointer to data memory defined by \ref ifx_Matrix_C_t
 * @param [in]     source              Pointer to data memory defined by \ref ifx_Matrix_C_t
 * @param [in]     row_offset          Row offset
 * @param [in]     column_offset       Column offset
 * @param [in]     rows                Number of rows to view
 * @param [in]     columns             Number of columns to view
 *
 */
IFX_DLL_PUBLIC
void ifx_mat_view_c(ifx_Matrix_C_t* matrix,
                    ifx_Matrix_C_t* source,
                    uint32_t row_offset,
                    uint32_t column_offset,
                    uint32_t rows,
                    uint32_t columns);

/**
 * @brief Assigns real data from source matrix given rows to the destination matrix.
 * Example usage:
 *
 *   Given \ref ifx_Matrix_R_t dest_matrix and \ref ifx_Matrix_R_t source_matrix,
 *   demanded number of rows num_rows and
 *   offset for rows of source_matrix one can assign:
 * @code
 *     ifx_mat_view_rows_r(&dest_matrix, &source_matrix, row_offset, num_rows);
 * @endcode
 *   e.g. for taking a view of two last rows from 3x3 matrix:
 * @code
 *     ifx_Float_t arr_data[3*3] = {1, 2, 3,
 *                                  4, 5, 6,
 *                                  7, 8, 9};
 *     ifx_mat_rawview_r(&source_matrix, arr_data, 3, 3, 3);
 *
 *     ifx_mat_view_rows_r(&dest_matrix, &source_matrix, 1, 2);
 * @endcode
 *     will give a dest_matrix with values:
 * @code
 *     [4, 5, 6,
 *      7, 8, 9]
 * @endcode
 *
 * @param [in,out] matrix              Pointer to data memory defined by \ref ifx_Matrix_R_t
 * @param [in]     source              Pointer to data memory defined by \ref ifx_Matrix_R_t
 * @param [in]     row_offset          Row offset
 * @param [in]     rows                Number of rows to view
 *
 */
IFX_DLL_PUBLIC
void ifx_mat_view_rows_r(ifx_Matrix_R_t* matrix,
                         ifx_Matrix_R_t* source,
                         uint32_t row_offset,
                         uint32_t rows);

/**
 * @brief Assigns complex data from source matrix given rows to the destination matrix.
 * Example usage:
 *
 *   Given \ref ifx_Matrix_C_t dest_matrix and \ref ifx_Matrix_C_t source_matrix,
 *   demanded number of rows num_rows and
 *   offset for rows of source_matrix one can assign:
 * @code
 *     ifx_mat_view_rows_c(&dest_matrix, &source_matrix, row_offset, num_rows);
 * @endcode
 *   e.g. for taking a view of a last row from 2x2 matrix:
 * @code
 *     ifx_Complex_t arr_data[2*2] = {{1, 1}, {2, 2}
 *                                    {3, 3}, {4, 4}};
 *     ifx_mat_rawview_c(&source_matrix, arr_data, 2, 2, 2);
 *
 *     ifx_mat_view_rows_c(&dest_matrix, &source_matrix, 1, 1);
 * @endcode
 *     will give a matrix with values:
 * @code
 *     [{3, 3}, {4, 4}]
 * @endcode
 *
 * @param [in,out] matrix              Pointer to data memory defined by \ref ifx_Matrix_C_t
 * @param [in]     source              Pointer to data memory defined by \ref ifx_Matrix_C_t
 * @param [in]     row_offset          Row offset
 * @param [in]     rows                Number of rows to view
 */
IFX_DLL_PUBLIC
void ifx_mat_view_rows_c(ifx_Matrix_C_t* matrix,
                         ifx_Matrix_C_t* source,
                         uint32_t row_offset,
                         uint32_t rows);

/**
 * @brief Allocates memory for a real matrix with a specified number of
 *        rows and columns and initializes it to zero.
 *        The data is arranged sequentially row-wise, i.e., all elements of a given row are
 *        placed in successive memory locations, as depicted in the illustration.
 *
 * @image html img_matrix_memory_map.png "Illustration showing memory arrangement of matrix data in ifx_Matrix_R_t" width=600px
 *
 * see \ref ifx_Matrix_R_t for more details.
 *
 * @param [in]     rows      Number of rows
 * @param [in]     columns   Number of columns
 *
 * @return Pointer to allocated and initialized real matrix structure or NULL if allocation failed.
 *
 */
IFX_DLL_PUBLIC
ifx_Matrix_R_t* ifx_mat_create_r(uint32_t rows,
                                 uint32_t columns);

/**
 * @brief Allocates memory for a complex matrix with a specified number of
 *        rows and columns and initializes it to zero.
 *        The data is arranged sequentially row-wise, i.e., all elements of a given row are
 *        placed in successive memory locations, as depicted in the illustration.
 *
 * @image html img_matrix_memory_map.png "Illustration showing memory arrangement of matrix data in ifx_Matrix_C_t" width=600px
 *
 * see \ref ifx_Matrix_C_t for more details.
 *
 * @param [in]     rows      Number of rows
 * @param [in]     columns   Number of columns
 *
 * @return Pointer to allocated and initialized complex matrix structure or NULL if allocation failed.
 *
 */
IFX_DLL_PUBLIC
ifx_Matrix_C_t* ifx_mat_create_c(uint32_t rows,
                                 uint32_t columns);

/**
 * @brief Frees memory for a real matrix defined by \ref ifx_mat_create_r
 *        and sets the length of the matrix to zero.
 *
 * @param [in,out] matrix    Pointer to an allocated matrix instance defined
 *                           by \ref ifx_Matrix_R_t
 */
IFX_DLL_PUBLIC
void ifx_mat_destroy_r(ifx_Matrix_R_t* matrix);

/**
 * @brief Frees memory for a complex matrix defined by \ref ifx_mat_create_c
 *        and sets the length of the matrix to zero.
 *
 * @param [in,out] matrix    Pointer to an allocated matrix instance defined
 *                           by \ref ifx_Matrix_C_t
 */
IFX_DLL_PUBLIC
void ifx_mat_destroy_c(ifx_Matrix_C_t* matrix);

/**
 * @brief Blits elements of a given real matrix, to a new created real matrix
 *        with user defined dimensions.
 *
 * @param [in]     from                Pointer to data memory defined by \ref ifx_Matrix_R_t
 * @param [in]     from_row            Row location from where to blit
 * @param [in]     num_rows            Number of rows to blit
 * @param [in]     from_column         Column location from where to blit
 * @param [in]     num_columns         Number of columns to blit
 * @param [out]    to                  Pointer to data memory defined by \ref ifx_Matrix_R_t
 *
 */
IFX_DLL_PUBLIC
void ifx_mat_blit_r(const ifx_Matrix_R_t* from,
                    uint32_t from_row,
                    uint32_t num_rows,
                    uint32_t from_column,
                    uint32_t num_columns,
                    ifx_Matrix_R_t* to);

/**
 * @brief Blits elements of a given complex matrix, to a new created complex matrix
 *        with user defined dimensions.
 *
 * @param [in]     from                Pointer to data memory defined by \ref ifx_Matrix_C_t
 * @param [in]     from_row            Row location from where to blit
 * @param [in]     num_rows            Number of rows to blit
 * @param [in]     from_column         Column location from where to blit
 * @param [in]     num_columns         Number of columns to blit
 * @param [out]    to                  Pointer to data memory defined by \ref ifx_Matrix_C_t
 *
 */
IFX_DLL_PUBLIC
void ifx_mat_blit_c(const ifx_Matrix_C_t* from,
                    uint32_t from_row,
                    uint32_t num_rows,
                    uint32_t from_column,
                    uint32_t num_columns,
                    ifx_Matrix_C_t* to);
/**
 * @brief Copies the elements from a real source matrix to a real destination matrix.
 *        The condition is that both source and destination matrix instance should be allocated
 *        and populated (pointers inside the matrix instance should not be null) using the
 *        \ref ifx_mat_create_r. No memory allocation is done in this function.
 *
 * @param [in]     from      Pointer to an allocated and populated source
 *                           matrix instance defined by \ref ifx_Matrix_R_t
 * @param [in,out] to        Pointer to an allocated and populated, yet empty
 *                           (data values of the matrix are still zero) destination
 *                           matrix instance defined by \ref ifx_Matrix_R_t
 *
 */
IFX_DLL_PUBLIC
void ifx_mat_copy_r(const ifx_Matrix_R_t* from,
                    ifx_Matrix_R_t* to);

/**
 * @brief Copies the elements from a complex source matrix to a complex destination matrix.
 *        The condition is that both source and destination matrix instance should be allocated
 *        and populated (pointers inside the matrix instance should not be null) using the
 *        \ref ifx_mat_create_c. No memory allocation is done in this function.
 *
 * @param [in]     from      Pointer to an allocated and populated source
 *                           matrix instance defined by \ref ifx_Matrix_C_t
 * @param [in,out] to        Pointer to an allocated and populated, yet empty
 *                           (data values of the matrix are still zero) destination
 *                           matrix instance defined by \ref ifx_Matrix_C_t
 *
 */
IFX_DLL_PUBLIC
void ifx_mat_copy_c(const ifx_Matrix_C_t* from,
                    ifx_Matrix_C_t* to);

/**
 * @brief Copies a user defined sequence of real values to a user defined row index in a real matrix.
 *        The count of the input real values should not be greater than the number of columns in the matrix.
 *
 * @param [in,out] matrix              Pointer to an allocated and populated real-valued
 *                                     matrix instance defined by \ref ifx_Matrix_R_t
 * @param [in]     row_index           Row number that is to be filled by the user defined vector
 * @param [in]     row_values          Pointer to user defined vector defined by \see ifx_Float_t
 * @param [in]     count               Number of elements in the user defined vector
 *
 */
IFX_DLL_PUBLIC
void ifx_mat_set_row_r(ifx_Matrix_R_t* matrix,
                       uint32_t row_index,
                       const ifx_Float_t* row_values,
                       uint32_t count);

/**
 * @brief Copies a user defined sequence of complex numbers values to a user defined row index in a complex matrix.
 *        The count of the input complex numbers should not be greater than the number of columns in the matrix.
 *
 * @param [in,out] matrix              Pointer to an allocated and populated complex-valued
 *                                     matrix instance defined by \ref ifx_Matrix_C_t
 * @param [in]     row_index           Row number that is to be filled by the user defined vector
 * @param [in]     row_values          Pointer to User defined vector defined by \ref ifx_Complex_t
 * @param [in]     count               Number of elements in the user defined vector
 *
 */
IFX_DLL_PUBLIC
void ifx_mat_set_row_c(ifx_Matrix_C_t* matrix,
                       uint32_t row_index,
                       const ifx_Complex_t* row_values,
                       uint32_t count);

/**
 * @brief Copies a user defined real-valued vector \ref ifx_Vector_R_t to a user defined row index in a real matrix.
 *        The length of the input real-valued matrix should not be greater than the number of columns in the matrix.
 *
 * @param [in,out] matrix              Pointer to an allocated and populated real-valued
 *                                     matrix instance defined by \ref ifx_Matrix_R_t
 * @param [in]     row_index           Row number that is to be filled by the user defined vector
 * @param [in]     row_values          Pointer to vector from which the data is to be copied to the specified row
 *
 */
IFX_DLL_PUBLIC
void ifx_mat_set_row_vector_r(ifx_Matrix_R_t* matrix,
                              uint32_t row_index,
                              const ifx_Vector_R_t* row_values);

/**
 * @brief Copies a user defined complex-valued matrix \ref ifx_Vector_C_t to a user defined row index in a complex matrix.
 *        The length of the input complex-valued matrix should not be greater than the number of columns in the matrix.
 *
 * @param [in,out] matrix              Pointer to an allocated and populated complex-valued
 *                                     matrix instance defined by \ref ifx_Matrix_C_t
 * @param [in]     row_index           Row number that is to be filled by the user defined vector
 * @param [in]     row_values          Pointer to vector from which the data is to be copied to the specified row
 *
 */
IFX_DLL_PUBLIC
void ifx_mat_set_row_vector_c(ifx_Matrix_C_t* matrix,
                              uint32_t row_index,
                              const ifx_Vector_C_t* row_values);

/**
 * @brief Returns a complex-valued vector pointing to defined row of the given complex matrix.
 *
 * @param [in]     matrix              The complex matrix, from which one row would be pointed to by the output vector
 * @param [in]     row_index           The row of the matrix the vector will point to.
 * @param [in]     row_view            Pointer to vector view to be initialized
 *
 */
IFX_DLL_PUBLIC
void ifx_mat_get_rowview_c(const ifx_Matrix_C_t* matrix,
                           uint32_t row_index,
                           ifx_Vector_C_t* row_view);

/**
 * @brief Returns a real-valued vector pointing to defined row of the given real matrix.
 *
 * @param [in]     matrix              The real matrix, from which one row would be pointed to by the output vector
 * @param [in]     row_index           The row of the matrix the vector will point to.
 * @param [in]     row_view            Pointer to vector view to be initialized
 *
 */
IFX_DLL_PUBLIC
void ifx_mat_get_rowview_r(const ifx_Matrix_R_t* matrix,
                           uint32_t row_index,
                           ifx_Vector_R_t* row_view);

/**
 * @brief Returns a real-valued vector pointing to defined column of the given real matrix.
 *
 * @param [in]     matrix              The real matrix, from which one row would be pointed to by the output vector.
 * @param [in]     col_index           The row of the matrix the vector will point to.
 * @param [in]     col_view            Pointer to vector view to be initialized
 *
 */
IFX_DLL_PUBLIC
void ifx_mat_get_colview_r(const ifx_Matrix_R_t* matrix,
                           uint32_t col_index,
                           ifx_Vector_R_t* col_view);

/**
 * @brief Returns a complex-valued vector pointing to defined column of the given complex matrix.
 *
 * @param [in]     matrix              The complex matrix, from which one row would be pointed to by the output vector
 * @param [in]     col_index           The row of the matrix the vector will point to.
 * @param [in]     col_view            Pointer to vector view to be initialized
 *
 */
IFX_DLL_PUBLIC
void ifx_mat_get_colview_c(const ifx_Matrix_C_t* matrix,
                           uint32_t col_index,
                           ifx_Vector_C_t* col_view);

/**
 * @brief Transposes a given real matrix and saves it in the given output matrix. This function doesn't support in-place
 *        transposing therefore it is not allowed input and output matrix pointing to the same matrix.
 *        This will result in IFX_ERROR_IN_PLACE_CALCULATION_NOT_SUPPORTED.
 *
 * @param [in]     matrix              Real matrix to transpose.
 * @param [out]    transposed          Real matrix the transposed matrix will be stored in must be allocated
 *                                     and of the same size as the given input matrix.
 *
 */
IFX_DLL_PUBLIC
void ifx_mat_transpose_r(const ifx_Matrix_R_t* matrix,
                         ifx_Matrix_R_t* transposed);

/**
 * @brief Transposes a given complex matrix and saves it in the given output matrix. This function doesn't support
 *        in-place transposing therefore it is not allowed input and output matrix pointing to the same matrix.
 *        This will result in IFX_ERROR_IN_PLACE_CALCULATION_NOT_SUPPORTED.

 * @param [in]     matrix              Complex matrix to transpose.
 * @param [out]    transposed          Complex matrix the transposed matrix will be stored in must be allocated
 *                                     and of the same size as the given input matrix.
 *
 */
IFX_DLL_PUBLIC
void ifx_mat_transpose_c(const ifx_Matrix_C_t* matrix,
                         ifx_Matrix_C_t* transposed);

/**
 * @brief Adds two real matrices and saves the result of addition into given output matrix.
 *
 * Math operation implemented by this method: Z = X + Y
 *
 * @param [in]     matrix_l  Real matrix as left argument (X in above equation).
 * @param [in]     matrix_r  Real matrix as right argument (Y in above equation).
 * @param [out]    result    Real matrix where the result of addition will be stored (Z in above equation).
 *                           It must be allocated and of the same dimensions as the given input matrix. Can be in-place.
 *
 */
IFX_DLL_PUBLIC
void ifx_mat_add_r(const ifx_Matrix_R_t* matrix_l,
                   const ifx_Matrix_R_t* matrix_r,
                   ifx_Matrix_R_t* result);

IFX_DLL_PUBLIC
void ifx_mat_add_rs(const ifx_Matrix_R_t* input,
                    ifx_Float_t scalar,
                    ifx_Matrix_R_t* output);

/**
 * @brief Adds two complex matrices and saves the result of addition into given output matrix.
 *
 * Math operation implemented by this method: Z = X + Y
 *
 * @param [in]     matrix_l  Complex matrix as left argument (X in above equation).
 * @param [in]     matrix_r  Complex matrix as right argument (Y in above equation).
 * @param [out]    result    Complex matrix where the result of addition will be stored (Z in above equation).
 *                           It must be allocated and of the same dimensions as the given input matrix. Can be in-place.
 *
 */
IFX_DLL_PUBLIC
void ifx_mat_add_c(const ifx_Matrix_C_t* matrix_l,
                   const ifx_Matrix_C_t* matrix_r,
                   ifx_Matrix_C_t* result);


IFX_DLL_PUBLIC
void ifx_mat_add_cs(const ifx_Matrix_C_t* input,
                    ifx_Complex_t scalar,
                    ifx_Matrix_C_t* output);

/**
 * @brief Subtracts two real matrices and saves the result of subtraction into given output matrix.
 *
 * Math operation implemented by this method: Z = X - Y
 *
 * @param [in]     matrix_l  Real matrix as left argument (X in above equation).
 * @param [in]     matrix_r  Real matrix as right argument (Y in above equation).
 * @param [out]    result    Real matrix where the result of subtraction will be stored (Z in above equation).
 *                           It must be allocated and of the same dimensions as the given input matrix. Can be in-place.
 *
 */
IFX_DLL_PUBLIC
void ifx_mat_sub_r(const ifx_Matrix_R_t* matrix_l,
                   const ifx_Matrix_R_t* matrix_r,
                   ifx_Matrix_R_t* result);

IFX_DLL_PUBLIC
void ifx_mat_sub_rs(const ifx_Matrix_R_t* input,
                    ifx_Float_t scalar,
                    ifx_Matrix_R_t* output);

/**
 * @brief Subtracts two complex matrices and saves the result of subtraction into given output matrix.
 *
 * Math operation implemented by this method: Z = X - Y
 *
 * @param [in]     matrix_l  Complex matrix as left argument (X in above equation).
 * @param [in]     matrix_r  Complex matrix as right argument (Y in above equation).
 * @param [out]    result    Complex matrix where the result of subtraction will be stored (Z in above equation).
 *                           It must be allocated and of the same dimensions as the given input matrix. Can be in-place.
 *
 */
IFX_DLL_PUBLIC
void ifx_mat_sub_c(const ifx_Matrix_C_t* matrix_l,
                   const ifx_Matrix_C_t* matrix_r,
                   ifx_Matrix_C_t* result);

IFX_DLL_PUBLIC
void ifx_mat_sub_cs(const ifx_Matrix_C_t* input,
                    ifx_Complex_t scalar,
                    ifx_Matrix_C_t* output);

/**
 * @brief Applies linear real value scaling to a real matrix.
 *
 * Math operation implemented by this method: Z = a * X
 *
 * @param [in]     input     Real matrix on which scaling is to be applied (X in above equation).
 * @param [in]     scale     Real floating point values of scalar (a in above equation).
 * @param [out]    output    Real matrix where the result of scaling will be stored (Z in above equation).
 *                           It must be allocated and of the same dimensions as the given input matrix. Can be in-place.
 *
 */
IFX_DLL_PUBLIC
void ifx_mat_scale_r(const ifx_Matrix_R_t* input,
                     ifx_Float_t scale,
                     ifx_Matrix_R_t* output);

/**
 * @brief Applies linear complex value scaling to a real matrix.
 *
 * Math operation implemented by this method: Z = a * X
 *
 * @param [in]     input     Real matrix on which scaling is to be applied (X in above equation).
 * @param [in]     scale     Complex floating point values of scalar (a in above equation).
 * @param [out]    output    Complex matrix where the result of scaling will be stored (Z in above equation).
 *                           It must be allocated and of the same dimensions as the given input matrix. Can be in-place.
 *
 */
IFX_DLL_PUBLIC
void ifx_mat_scale_rc(const ifx_Matrix_R_t* input,
                      ifx_Complex_t scale,
                      ifx_Matrix_C_t* output);

/**
 * @brief Applies linear complex value scaling to a complex matrix.
 *
 * Math operation implemented by this method: Z = a * X
 *
 * @param [in]     input     Complex matrix on which scaling is to be applied (X in above equation).
 * @param [in]     scale     Complex floating point values of scalar (a in above equation).
 * @param [out]    output    Complex matrix where the result of scaling will be stored (Z in above equation).
 *                           It must be allocated and of the same dimensions as the given input matrix. Can be in-place.
 *
 */
IFX_DLL_PUBLIC
void ifx_mat_scale_c(const ifx_Matrix_C_t* input,
                     ifx_Complex_t scale,
                     ifx_Matrix_C_t* output);

/**
 * @brief Applies linear real value scaling to a complex matrix.
 *
 * Math operation implemented by this method: Z = a * X
 *
 * @param [in]     input     Complex matrix on which scaling is to be applied (X in above equation).
 * @param [in]     scale     Real floating point values of scalar (a in above equation).
 * @param [out]    output    Complex matrix where the result of scaling will be stored (Z in above equation).
 *                           It must be allocated and of the same dimensions as the given input matrix. Can be in-place.
 *
 */
IFX_DLL_PUBLIC
void ifx_mat_scale_cr(const ifx_Matrix_C_t* input,
                      ifx_Float_t scale,
                      ifx_Matrix_C_t* output);

/**
 * @brief Applies multiply accumulate (MAC) operation on real matrices.
 *
 * Math operation implemented by this method: output = a + b * scale
 *
 * @param [in]     m1        Real matrix to be added i.e. "a" in above equation
 * @param [in]     m2        Real matrix to be scaled i.e. "b" in above equation
 * @param [in]     scale     Real floating point values of scalar
 * @param [out]    result    Real matrix where the result of scaling will be stored.
 *                           It must be allocated and of the same dimensions as the given input matrix. Can be in-place.
 *
 */
IFX_DLL_PUBLIC
void ifx_mat_mac_r(const ifx_Matrix_R_t* m1,
                   const ifx_Matrix_R_t* m2,
                   ifx_Float_t scale,
                   ifx_Matrix_R_t* result);

/**
 * @brief Applies multiply accumulate (MAC) operation on complex matrices.
 *
 * Math operation implemented by this method: output = a + b * scale
 *
 * @param [in]     m1        Complex matrix to be added i.e. "a" in above equation
 * @param [in]     m2        Complex matrix to be scaled i.e. "b" in above equation
 * @param [in]     scale     Complex floating point values of scalar
 *
 * @param [out]    result    Complex matrix where the result of scaling will be stored.
 *                           It must be allocated and of the same dimensions as the given input matrix. Can be in-place.
 *
 */
IFX_DLL_PUBLIC
void ifx_mat_mac_c(const ifx_Matrix_C_t* m1,
                   const ifx_Matrix_C_t* m2,
                   ifx_Complex_t scale,
                   ifx_Matrix_C_t* result);

/**
 * @brief Computes absolute matrix (modulus) from real input matrix. Absolute = |data|
 *
 * @param [in]     input     Pointer to data memory defined by \ref ifx_Matrix_R_t,
 *                           on which Absolute operation is to be performed.
 * @param [out]    output    Pointer to data memory defined by \ref ifx_Matrix_R_t,
 *                           containing the absolute values.
 */
IFX_DLL_PUBLIC
void ifx_mat_abs_r(const ifx_Matrix_R_t* input,
                   ifx_Matrix_R_t* output);

/**
 * @brief Computes absolute matrix (modulus) from complex input matrix. Absolute = |data|
 *
 * @param [in]     input     Pointer to data memory defined by \ref ifx_Matrix_C_t,
 *                           on which Absolute operation is to be performed.
 * @param [out]    output    Pointer to data memory defined by \ref ifx_Matrix_R_t,
 *                           containing the absolute values.
 */
IFX_DLL_PUBLIC
void ifx_mat_abs_c(const ifx_Matrix_C_t* input,
                   ifx_Matrix_R_t* output);

/**
 * @brief Computes the arithmetic sum of a real-valued matrix.
 *
 * @param [in]     matrix    Pointer to a data memory defined by \ref ifx_Matrix_R_t
 *                           to calculate the sum of all its elements.
 *
 * @return Sum of all elements' values of the passed input matrix.
 *
 */
IFX_DLL_PUBLIC
ifx_Float_t ifx_mat_sum_r(const ifx_Matrix_R_t* matrix);

/**
 * @brief Computes the arithmetic sum from a complex-valued matrix.
 *
 * @param [in]     matrix    Pointer to a data memory defined by \ref ifx_Matrix_C_t
 *                           to calculate the sum of all its elements.
 *
 * @return Sum of all elements' values of the passed input matrix.
 *
 */
IFX_DLL_PUBLIC
ifx_Complex_t ifx_mat_sum_c(const ifx_Matrix_C_t* matrix);

/**
 * @brief Computes the sum of squared values of a given real-valued matrix.
 *
 * @param [in]     matrix    Pointer to the memory containing array defined by \ref ifx_Matrix_R_t
 *
 * @return Sum of squared values of the passed input matrix.
 *
 */
IFX_DLL_PUBLIC
ifx_Float_t ifx_mat_sqsum_r(const ifx_Matrix_R_t* matrix);

/**
 * @brief Computes the sum of squared values of a given complex-valued matrix.
 *
 * @param [in]     matrix    Pointer to the memory containing array defined by \ref ifx_Matrix_C_t
 *
 * @return Sum of squared values of the passed input matrix.
 *
 */
IFX_DLL_PUBLIC
ifx_Float_t ifx_mat_sqsum_c(const ifx_Matrix_C_t* matrix);

/**
 * @brief Returns the biggest absolute value of a given real-valued matrix.
 *
 * @param [in]     matrix    Pointer to the memory containing array defined by \ref ifx_Matrix_R_t.
 *
 * @return Maximum absolute value of the passed input matrix.
 *
 */
IFX_DLL_PUBLIC
ifx_Float_t ifx_mat_maxabs_r(const ifx_Matrix_R_t* matrix);

/**
 * @brief Returns the maximum absolute value of a given complex-valued matrix.
 *
 * @param [in]     matrix    Pointer to the memory containing array defined by \ref ifx_Matrix_C_t.
 *
 * @return Maximum absolute value of the passed input matrix.
 *
 */
IFX_DLL_PUBLIC
ifx_Float_t ifx_mat_maxabs_c(const ifx_Matrix_C_t* matrix);

/**
 * @brief Computes the arithmetic mean from a real-valued matrix.
 *
 * @param [in]     matrix    Pointer to a data memory defined by \ref ifx_Matrix_R_t
 *                           from which mean is calculated.
 *
 * @return Mean real value of the passed input matrix.
 *
 */
IFX_DLL_PUBLIC
ifx_Float_t ifx_mat_mean_r(const ifx_Matrix_R_t* matrix);

/**
 * @brief Computes the arithmetic mean from a complex-valued matrix.
 *
 * @param [in]     matrix    Pointer to a data memory defined by \ref ifx_Matrix_C_t
 *                           from which mean is calculated.
 *
 * @return Mean complex value of the passed input matrix.
 *
 */
IFX_DLL_PUBLIC
ifx_Complex_t ifx_mat_mean_c(const ifx_Matrix_C_t* matrix);

/**
 * @brief Returns the maximum value of a real-valued matrix.
 *
 * @param [in]     matrix    Pointer to a data memory defined by \ref ifx_Matrix_R_t
 *                           from which max value is extracted.
 *
 * @return Maximum value of the passed input matrix.
 *
 */
IFX_DLL_PUBLIC
ifx_Float_t ifx_mat_max_r(const ifx_Matrix_R_t* matrix);

/**
 * @brief Computes the variance of a real-valued matrix.
 *
 * @param [in]     matrix    Pointer to a data memory defined by \ref ifx_Matrix_R_t
 *                           from which variance is calculated.
 *
 * @return Variance value of the passed input matrix.
 *
 */
IFX_DLL_PUBLIC
ifx_Float_t ifx_mat_var_r(const ifx_Matrix_R_t* matrix);

/**
 * @brief Computes matrix multiplication for:
 *          output = inputA * inputB-Transpose
 *
 * @param [in]     inputA    ...
 * @param [in]     inputB    ...
 * @param [out]    output    ...
 */
IFX_DLL_PUBLIC
void ifx_mat_abt_r(const ifx_Matrix_R_t* inputA,
                   const ifx_Matrix_R_t* inputB,
                   ifx_Matrix_R_t* output);

/**
 * @brief Computes matrix multiplication for:
 *          output = inputA * inputB-conjugate-Transpose
 *
 * @param [in]     inputA    ...
 * @param [in]     inputB    ...
 * @param [out]    output    ...
 */
IFX_DLL_PUBLIC
void ifx_mat_abct_c(const ifx_Matrix_C_t* inputA,
                    const ifx_Matrix_C_t* inputB,
                    ifx_Matrix_C_t* output);

/**
 * @brief Computes matrix multiplication for:
 *          output = inputA * inputB-Transpose
 *
 * @param [in]     inputA    ...
 * @param [in]     inputB    ...
 * @param [out]    output    ...
 */
IFX_DLL_PUBLIC
void ifx_mat_abt_c(const ifx_Matrix_C_t* inputA,
                   const ifx_Matrix_C_t* inputB,
                   ifx_Matrix_C_t* output);

/**
 * @brief Computes matrix multiplication for:
 *          output = inputA * inputB-Transpose
 *
 * @param [in]     inputA    ...
 * @param [in]     inputB    ...
 * @param [out]    output    ...
 */
IFX_DLL_PUBLIC
void ifx_mat_abt_rc(const ifx_Matrix_R_t* inputA,
                    const ifx_Matrix_C_t* inputB,
                    ifx_Matrix_C_t* output);

/**
 * @brief Computes matrix multiplication for:
 *          output = inputA * inputB-Transpose
 *
 * @param [in]     inputA    ...
 * @param [in]     inputB    ...
 * @param [out]    output    ...
 */
IFX_DLL_PUBLIC
void ifx_mat_abt_cr(const ifx_Matrix_C_t* inputA,
                    const ifx_Matrix_R_t* inputB,
                    ifx_Matrix_C_t* output);

/**
 * @brief Computes matrix multiplication for:
 *          output = inputA-Transpose * inputB
 *
 * @param [in]     inputA    ...
 * @param [in]     inputB    ...
 * @param [out]    output    ...
 */
IFX_DLL_PUBLIC
void ifx_mat_atb_r(const ifx_Matrix_R_t* inputA,
                   const ifx_Matrix_R_t* inputB,
                   ifx_Matrix_R_t* output);

/**
 * @brief Computes matrix multiplication for:
 *          output = inputA-Transpose * inputB
 *
 * @param [in]     inputA    ...
 * @param [in]     inputB    ...
 * @param [out]    output    ...
 */
IFX_DLL_PUBLIC
void ifx_mat_atb_c(const ifx_Matrix_C_t* inputA,
                   const ifx_Matrix_C_t* inputB,
                   ifx_Matrix_C_t* output);

/**
 * @brief Computes matrix multiplication for:
 *          output = inputA-Transpose * inputB
 *
 * @param [in]     inputA    ...
 * @param [in]     inputB    ...
 * @param [out]    output    ...
 */
IFX_DLL_PUBLIC
void ifx_mat_atb_rc(const ifx_Matrix_R_t* inputA,
                    const ifx_Matrix_C_t* inputB,
                    ifx_Matrix_C_t* output);

/**
 * @brief Computes matrix multiplication for:
 *          output = inputA-Transpose * inputB
 *
 * @param [in]     inputA    ...
 * @param [in]     inputB    ...
 * @param [out]    output    ...
 */
IFX_DLL_PUBLIC
void ifx_mat_atb_cr(const ifx_Matrix_C_t* inputA,
                    const ifx_Matrix_R_t* inputB,
                    ifx_Matrix_C_t* output);
/**
 * @brief Compute the matrix-vector product for real-valued matrix and vector
 * @brief Computes the matrix-vector product for real-valued matrix and vector.
 *
 * Compute the matrix-vector product: result=matrix*vector
 *
 * For matrix having dimension MxN, vector must be of dimension N and result
 * must be of dimension M.
 *
 * @param [in]     matrix    matrix
 * @param [in]     vector    vector
 * @param [out]    result    resulting vector of matrix-vector multiplication
 */
IFX_DLL_PUBLIC
void ifx_mat_mul_rv(const ifx_Matrix_R_t* matrix,
                    const ifx_Vector_R_t* vector,
                    ifx_Vector_R_t* result);

/**
 * @brief Computes the matrix-vector product for real-valued and transposed matrix and vector.
 *
 * Compute the matrix-vector product: result=(matrix)^T*vector
 *
 * For matrix having dimension MxN, vector must be of dimension M and result
 * must be of dimension N.
 *
 * Note that in contrast to \ref ifx_mat_mul_rv the matrix in the
 * multiplication is transposed.
 *
 * @param [in]     matrix    matrix
 * @param [in]     vector    vector
 * @param [out]    result    resulting vector of matrix-vector multiplication
 */
IFX_DLL_PUBLIC
void ifx_mat_mul_trans_rv(const ifx_Matrix_R_t* matrix,
                          const ifx_Vector_R_t* vector,
                          ifx_Vector_R_t* result);

/**
 * @brief Computes the matrix-vector product for complex-valued matrix and vector.
 *
 * Compute the matrix-vector product: result=matrix*vector
 *
 * For matrix having dimension MxN, vector must be of dimension N and result
 * must be of dimension M.
 *
 * @param [in]     matrix    matrix
 * @param [in]     vector    vector
 * @param [out]    result    resulting vector of matrix-vector multiplication
 */
IFX_DLL_PUBLIC
void ifx_mat_mul_cv(const ifx_Matrix_C_t* matrix,
                    const ifx_Vector_C_t* vector,
                    ifx_Vector_C_t* result);

/**
 * @brief Computes the matrix-vector product for complex-valued and transposed matrix and vector.
 *
 * Compute the matrix-vector product: result=(matrix)^T*vector
 *
 * For matrix having dimension MxN, vector must be of dimension M and result
 * must be of dimension N.
 *
 * Note that in contrast to \ref ifx_mat_mul_cv the matrix in the
 * multiplication is transposed.
 *
 * @param [in]     matrix    matrix
 * @param [in]     vector    vector
 * @param [out]    result    resulting vector of matrix-vector multiplication
 */
IFX_DLL_PUBLIC
void ifx_mat_trans_mul_cv(const ifx_Matrix_C_t* matrix,
                          const ifx_Vector_C_t* vector,
                          ifx_Vector_C_t* result);

/**
 * @brief Computes matrix product of two real matrices.
 *
 * Compute the matrix product matrix_l*matrix_r where matrix_l and matrix_r are
 * real matrices. The result is saved in result.
 *
 * The number of columns of matrix_l must match the number of rows of matrix_r.
 *
 * @param [in]     matrix_l  left matrix
 * @param [in]     matrix_r  right matrix
 * @param [in]     result    result, i.e., result=matrix_l*matrix_r
 *
 */
IFX_DLL_PUBLIC
void ifx_mat_mul_r(const ifx_Matrix_R_t* matrix_l,
                   const ifx_Matrix_R_t* matrix_r,
                   ifx_Matrix_R_t* result);

/**
 * @brief Computes matrix product of a real and a complex matrix.
 *
 * Compute the matrix product matrix_l*matrix_r where matrix_l is a real and
 * matrix_r is a complex matrix. The result is saved in result.
 *
 * The number of columns of matrix_l must match the number of rows of matrix_r.
 *
 * @param [in]     matrix_l  left matrix
 * @param [in]     matrix_r  right matrix
 * @param [in]     result    result, i.e., result=matrix_l*matrix_r
 *
 */
IFX_DLL_PUBLIC
void ifx_mat_mul_rc(const ifx_Matrix_R_t* matrix_l,
                    const ifx_Matrix_C_t* matrix_r,
                    ifx_Matrix_C_t* result);

/**
 * @brief Computes matrix product of two complex matrices.
 *
 * Compute the matrix product matrix_l*matrix_r where matrix_l and matrix_r are
 * complex matrices. The result is saved in result.
 *
 * The number of columns of matrix_l must match the number of rows of matrix_r.
 *
 * @param [in]     matrix_l  left matrix
 * @param [in]     matrix_r  right matrix
 * @param [in]     result    result, i.e., result=matrix_l*matrix_r
 *
 */
IFX_DLL_PUBLIC
void ifx_mat_mul_c(const ifx_Matrix_C_t* matrix_l,
                   const ifx_Matrix_C_t* matrix_r,
                   ifx_Matrix_C_t* result);

/**
 * @brief Computes matrix product of a complex and a real matrix.
 *
 * Compute the matrix product matrix_l*matrix_r where matrix_l is a complex and
 * matrix_r is a real matrix. The result is saved in result.
 *
 * The number of columns of matrix_l must match the number of rows of matrix_r.
 *
 * @param [in]     matrix_l  left matrix
 * @param [in]     matrix_r  right matrix
 * @param [in]     result    result, i.e., result=matrix_l*matrix_r
 *
 */
IFX_DLL_PUBLIC
void ifx_mat_mul_cr(const ifx_Matrix_C_t* matrix_l,
                    const ifx_Matrix_R_t* matrix_r,
                    ifx_Matrix_C_t* result);

/**
 * @brief Clears all elements of real matrix defined by \ref ifx_Matrix_R_t.
 *
 * Set all elements of the matrix to 0.
 *
 * @param [in]     matrix    Pointer to real matrix to be cleared.
 *
 */
IFX_DLL_PUBLIC
void ifx_mat_clear_r(ifx_Matrix_R_t* matrix);

/**
 * @brief Clears all elements of complex matrix defined by \ref ifx_Matrix_C_t.
 *
 * Set all elements of the matrix to 0.
 *
 * @param [in]     matrix    Pointer to complex matrix to be cleared.
 *
 */
IFX_DLL_PUBLIC
void ifx_mat_clear_c(ifx_Matrix_C_t* matrix);

/**
 * @brief Create a copy of real matrix.
 *
 * @param [in]  input  Input matrix to be cloned.
 * @retval  copy of matrix
 */
IFX_DLL_PUBLIC
ifx_Matrix_R_t* ifx_mat_clone_r(const ifx_Matrix_R_t* input);

/**
 * @brief Create a copy of complex matrix.
 *
 * @param [in]  input  Input matrix to be cloned.
 * @retval  copy of matrix
 */
IFX_DLL_PUBLIC
ifx_Matrix_C_t* ifx_mat_clone_c(const ifx_Matrix_C_t* input);

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* IFX_BASE_MATRIX_H */
