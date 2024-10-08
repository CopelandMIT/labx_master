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
 * @file Cube.h
 *
 * \brief \copybrief gr_cube
 *
 * For details refer to \ref gr_cube
 */

#ifndef IFX_BASE_CUBE_H
#define IFX_BASE_CUBE_H

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include "Defines.h"
#include "Matrix.h"
#include "Mda.h"


#ifdef __cplusplus
extern "C"
{
#endif


/*
==============================================================================
   2. DEFINITIONS
==============================================================================
*/

#define IFX_CUBE_ROWS(c)              (IFX_MDA_SHAPE(c)[0])
#define IFX_CUBE_COLS(c)              (IFX_MDA_SHAPE(c)[1])
#define IFX_CUBE_SLICES(c)            (IFX_MDA_SHAPE(c)[2])
#define IFX_CUBE_STRIDE(c, i)         (IFX_MDA_STRIDE(c)[i])
#define IFX_CUBE_DAT(c)               IFX_MDA_DATA(c)
#define IFX_CUBE_SLICE_SIZE(c)        ((size_t)IFX_CUBE_ROWS(c) * (size_t)IFX_CUBE_COLS(c))
#define IFX_CUBE_SIZE(c)              (IFX_CUBE_SLICE_SIZE(c) * (size_t)IFX_CUBE_SLICES(c))
#define IFX_CUBE_OFFSET(cub, r, c, s) IFX_MDA_OFFSET(cub, r, c, s)

#define IFX_CUBE_BRK_VALID(c)                                                       \
    do                                                                              \
    {                                                                               \
        IFX_ERR_BRK_NULL(c);                                                        \
        IFX_ERR_BRK_COND(IFX_MDA_DIMENSIONS(c) != 3, IFX_ERROR_DIMENSION_MISMATCH); \
        IFX_ERR_BRK_ARGUMENT(IFX_MDA_DATA(c) == NULL);                              \
    } while (0)

#define IFX_CUBE_BRV_VALID(c, r)                                                       \
    do                                                                                 \
    {                                                                                  \
        IFX_ERR_BRV_NULL(c, r);                                                        \
        IFX_ERR_BRV_COND(IFX_MDA_DIMENSIONS(c) != 3, IFX_ERROR_DIMENSION_MISMATCH, r); \
        IFX_ERR_BRV_ARGUMENT(IFX_MDA_DATA(c) == NULL, r);                              \
    } while (0)

/** @brief Access cube element
 *
 * The macro can be used to set and get elements of a cube, for example:
 * @code
 * foo = IFX_CUBE_AT(cube, row, col, slice);
 * IFX_CUBE_AT(cube, row, col, slice) = bar;
 * @endcode
 *
 * The macro works with both real (\ref ifx_Cube_R_t) and complex (\ref ifx_Cube_C_t) cubes.
 */
#define IFX_CUBE_AT(cub, r, c, s) IFX_MDA_AT(cub, r, c, s)

#define IFX_CUBE_BRK_DIM(c1, c2)    IFX_ERR_BRK_COND((IFX_CUBE_ROWS(c1) != IFX_CUBE_ROWS(c2)) || (IFX_CUBE_COLS(c1) != IFX_CUBE_COLS(c2)) || (IFX_CUBE_SLICES(c1) != IFX_CUBE_SLICES(c2)), IFX_ERROR_DIMENSION_MISMATCH)
#define IFX_CUBE_BRV_DIM(c1, c2, a) IFX_ERR_BRV_COND((IFX_CUBE_ROWS(c1) != IFX_CUBE_ROWS(c2)) || (IFX_CUBE_COLS(c1) != IFX_CUBE_COLS(c2)) || (IFX_CUBE_SLICES(c1) != IFX_CUBE_SLICES(c2)), IFX_ERROR_DIMENSION_MISMATCH, a)


/*
==============================================================================
   3. TYPES
==============================================================================
*/

/**
 * @brief Forward declaration structure to operate on real Cube.
 */
typedef ifx_Mda_R_t ifx_Cube_R_t;

/**
 * @brief Forward declaration structure to operate on complex Cube.
 */
typedef ifx_Mda_C_t ifx_Cube_C_t;

/*
==============================================================================
   4. FUNCTION PROTOTYPES
==============================================================================
*/

/** @addtogroup gr_cat_SDK_base
 * @{
 */

/** @defgroup gr_cube Cube
 * @brief API for operations on Cube data structures
 *
 * Supports operations on Cube (array of matrices) data structures.
 *
 * @{
 */

/**
 * @brief Allocates memory for a real cube with a specified number of
 *        rows and columns and slices and initializes it to zero.
 *        See \ref ifx_Cube_R_t for more details.
 *
 * @param [in]     rows      Number of rows in the cube.
 * @param [in]     columns   Number of columns in the cube.
 * @param [in]     slices    Number of slices in the cube.
 *
 * @return Pointer to allocated and initialized real cube structure or NULL if allocation failed.
 *
 */
IFX_DLL_PUBLIC
ifx_Cube_R_t* ifx_cube_create_r(uint32_t rows,
                                uint32_t columns,
                                uint32_t slices);

/**
 * @brief Allocates memory for a real cube with a specified number of
 *        rows and columns and slices and initializes it to zero.
 *        See \ref ifx_Cube_C_t for more details.
 *
 * @param [in]     rows      Number of rows in the cube.
 * @param [in]     columns   Number of columns in the cube.
 * @param [in]     slices    Number of slices in the cube.
 *
 * @return Pointer to allocated and initialized real cube structure or NULL if allocation failed.
 *
 */
IFX_DLL_PUBLIC
ifx_Cube_C_t* ifx_cube_create_c(uint32_t rows,
                                uint32_t columns,
                                uint32_t slices);

/**
 * @brief Frees memory for a real cube defined by \ref ifx_cube_create_r
 *        and sets the cube elements to zero.
 *
 * @param [in,out] cube      Pointer to an allocated cube instance defined
 *                           by \ref ifx_Cube_R_t.
 */
IFX_DLL_PUBLIC
void ifx_cube_destroy_r(ifx_Cube_R_t* cube);

/**
 * @brief Frees memory for a complex cube defined by \ref ifx_cube_create_r
 *        and sets the cube elements to zero.
 *
 * @param [in,out] cube      Pointer to an allocated cube instance defined
 *                           by \ref ifx_Cube_C_t.
 */
IFX_DLL_PUBLIC
void ifx_cube_destroy_c(ifx_Cube_C_t* cube);

/**
 * @brief Copy content of cube to target
 *
 * @param [in]  cube    Pointer to real cube
 * @param [in]  target  Pointer to real cube
 */
IFX_DLL_PUBLIC
void ifx_cube_copy_r(const ifx_Cube_R_t* cube, ifx_Cube_R_t* target);

/**
 * @brief Copy content of cube to target
 *
 * @param [in]  cube    Pointer to real cube
 * @param [in]  target  Pointer to real cube
 */
IFX_DLL_PUBLIC
void ifx_cube_copy_c(const ifx_Cube_C_t* cube, ifx_Cube_C_t* target);

/**
 * @brief Clones a real cube
 *
 * @param [in]  cube    Pointer to real cube
 *
 * @return Cloned cube
 */
IFX_DLL_PUBLIC
ifx_Cube_R_t* ifx_cube_clone_r(const ifx_Cube_R_t* cube);

/**
 * @brief Clones a complex cube
 *
 * @param [in]  cube    Pointer to complex cube
 *
 * @return Cloned cube
 */
IFX_DLL_PUBLIC
ifx_Cube_C_t* ifx_cube_clone_c(const ifx_Cube_C_t* cube);

/**
 * @brief Returns a slice of a real cube in the form
 *        of a matrix of type \ref ifx_Matrix_R_t
 * The rows and columns of the output matrix correspond to the
 * rows and columns of the cube respectively
 *
 * @param [in]     cube                Pointer to real cube from which slice shall be returned.
 * @param [in]     slice_index         index of slice to be returned.
 * @param [out]    slice               real matrix output representing the slice.
 *
 */
IFX_DLL_PUBLIC
void ifx_cube_get_slice_r(const ifx_Cube_R_t* cube,
                          uint32_t slice_index,
                          ifx_Matrix_R_t* slice);

/**
 * @brief Returns a slice of a complex cube in the form
 *        of a matrix of type \ref ifx_Matrix_C_t
 * The rows and columns of the output matrix correspond to the
 * rows and columns of the cube respectively
 *
 * @param [in]     cube                Pointer to complex cube from which slice shall be returned.
 * @param [in]     slice_index         Index of slice to be returned.
 * @param [out]    slice               Complex matrix output representing the slice.
 *
 */
IFX_DLL_PUBLIC
void ifx_cube_get_slice_c(const ifx_Cube_C_t* cube,
                          uint32_t slice_index,
                          ifx_Matrix_C_t* slice);

/**
 * @brief Returns a 2-d row of a real cube in the form
 *        of a matrix of type \ref ifx_Matrix_R_t
 * The rows and columns of the output matrix correspond to the
 * columns and slices of the cube respectively
 *
 * @param [in]     cube                Pointer to real cube from which a row matrix shall be returned.
 * @param [in]     row_index           index of row to be returned as a matrix.
 * @param [out]    row_matrix          real matrix output representing the 2-d row.
 *
 */
IFX_DLL_PUBLIC
void ifx_cube_get_row_r(const ifx_Cube_R_t* cube,
                        uint32_t row_index,
                        ifx_Matrix_R_t* row_matrix);

/**
 * @brief Returns a 2-d row of a complex cube in the form
 *        of a matrix of type \ref ifx_Matrix_C_t
 * The rows and columns of the output matrix correspond to the
 * columns and slices of the cube respectively
 *
 * @param [in]     cube                Pointer to complex cube from which a row matrix shall be returned.
 * @param [in]     row_index           Index of row to be returned as a matrix.
 * @param [out]    row_matrix          Complex matrix output representing the 2-d row.
 *
 */
IFX_DLL_PUBLIC
void ifx_cube_get_row_c(const ifx_Cube_C_t* cube,
                        uint32_t row_index,
                        ifx_Matrix_C_t* row_matrix);

/**
 * @brief Returns a 2-d column of a real cube in the form
 *        of a matrix of type \ref ifx_Matrix_R_t
 * The rows and columns of the output matrix correspond to the
 * rows and slices of the cube respectively
 *
 * @param [in]     cube                Pointer to real cube which a column matrix shall be returned.
 * @param [in]     col_index           index of column to be returned as a matrix.
 * @param [out]    col_matrix          real matrix output representing the 2-d column.
 *
 */
IFX_DLL_PUBLIC
void ifx_cube_get_col_r(const ifx_Cube_R_t* cube,
                        uint32_t col_index,
                        ifx_Matrix_R_t* col_matrix);

/**
 * @brief Returns a 2-d column of a complex cube in the form
 *        of a matrix of type \ref ifx_Matrix_C_t
 * The rows and columns of the output matrix correspond to the
 * rows and slices of the cube respectively
 *
 * @param [in]     cube                Pointer to complex cube from which a column matrix shall be returned.
 * @param [in]     col_index           Index of column to be returned as a matrix.
 * @param [out]    col_matrix          Complex matrix output representing the 2-d column.
 *
 */
IFX_DLL_PUBLIC
void ifx_cube_get_col_c(const ifx_Cube_C_t* cube,
                        uint32_t col_index,
                        ifx_Matrix_C_t* col_matrix);

/**
 * @brief Returns a real matrix extracted of a specified column, absolute values,
 *        of the a complex cube defined by \ref ifx_Matrix_R_t.
 *
 * @param [in]     cube                Pointer to real cube which slice shall be returned.
 * @param [in]     column_index        Index of slice to be returned.
 * @param [out]    matrix              Real matrix output containing absolute values of the
 *                                     selected complex matrix of the chosen column index.
 *
 */
IFX_DLL_PUBLIC
void ifx_cube_col_abs_r(const ifx_Cube_C_t* cube,
                        uint32_t column_index,
                        ifx_Matrix_R_t* matrix);

/**
 * @brief Clears all elements of real cube defined by \ref ifx_Cube_R_t.
 *
 * @param [in]     cube      Pointer to real cube to be cleared.
 *
 */
IFX_DLL_PUBLIC
void ifx_cube_clear_r(ifx_Cube_R_t* cube);

/**
 * @brief Clears all elements of complex cube defined by \ref ifx_Cube_C_t.
 *
 * @param [in]     cube      Pointer to complex cube to be cleared.
 *
 */
IFX_DLL_PUBLIC
void ifx_cube_clear_c(ifx_Cube_C_t* cube);

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* IFX_BASE_CUBE_H */
