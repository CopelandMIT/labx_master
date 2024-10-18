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

#include <string.h>

#include "internal/Util.h"

#include "Complex.h"
#include "Cube.h"
#include "Error.h"
#include "internal/Macros.h"
#include "Matrix.h"
#include "Mem.h"

/*
==============================================================================
   7. EXPORTED FUNCTIONS
==============================================================================
*/

ifx_Cube_R_t* ifx_cube_create_r(uint32_t rows, uint32_t columns, uint32_t slices)
{
    ifx_Cube_R_t* cube = IFX_MDA_CREATE_R(rows, columns, slices);
    if (cube)
        ifx_mda_clear_r(cube);
    return cube;
}

//----------------------------------------------------------------------------

ifx_Cube_C_t* ifx_cube_create_c(uint32_t rows, uint32_t columns, uint32_t slices)
{
    ifx_Cube_C_t* cube = IFX_MDA_CREATE_C(rows, columns, slices);
    if (cube)
        ifx_mda_clear_c(cube);
    return cube;
}

//----------------------------------------------------------------------------

void ifx_cube_get_slice_r(const ifx_Cube_R_t* cube, uint32_t depth_index, ifx_Matrix_R_t* slice)
{
    IFX_CUBE_BRK_VALID(cube);
    IFX_ERR_BRK_NULL(slice);

    IFX_MDA_VIEW_R(slice, cube, IFX_MDA_SLICE_FULL(), IFX_MDA_SLICE_FULL(), IFX_MDA_INDEX(depth_index));
}

//----------------------------------------------------------------------------

void ifx_cube_get_slice_c(const ifx_Cube_C_t* cube, uint32_t depth_index, ifx_Matrix_C_t* slice)
{
    IFX_CUBE_BRK_VALID(cube);
    IFX_ERR_BRK_NULL(slice);

    IFX_MDA_VIEW_C(slice, cube, IFX_MDA_SLICE_FULL(), IFX_MDA_SLICE_FULL(), IFX_MDA_INDEX(depth_index));
}

//----------------------------------------------------------------------------

void ifx_cube_get_row_r(const ifx_Cube_R_t* cube, uint32_t row_index, ifx_Matrix_R_t* row_matrix)
{
    IFX_CUBE_BRK_VALID(cube);
    IFX_ERR_BRK_NULL(row_matrix);

    IFX_MDA_VIEW_R(row_matrix, cube, IFX_MDA_INDEX(row_index), IFX_MDA_SLICE_FULL(), IFX_MDA_SLICE_FULL());
}

//----------------------------------------------------------------------------

void ifx_cube_get_row_c(const ifx_Cube_C_t* cube, uint32_t row_index, ifx_Matrix_C_t* row_matrix)
{
    IFX_CUBE_BRK_VALID(cube);
    IFX_ERR_BRK_NULL(row_matrix);

    IFX_MDA_VIEW_C(row_matrix, cube, IFX_MDA_INDEX(row_index), IFX_MDA_SLICE_FULL(), IFX_MDA_SLICE_FULL());
}

//----------------------------------------------------------------------------

void ifx_cube_get_col_r(const ifx_Cube_R_t* cube, uint32_t col_index, ifx_Matrix_R_t* col_matrix)
{
    IFX_CUBE_BRK_VALID(cube);
    IFX_ERR_BRK_NULL(col_matrix);

    IFX_MDA_VIEW_R(col_matrix, cube, IFX_MDA_SLICE_FULL(), IFX_MDA_INDEX(col_index), IFX_MDA_SLICE_FULL());
}

//----------------------------------------------------------------------------

void ifx_cube_get_col_c(const ifx_Cube_C_t* cube, uint32_t col_index, ifx_Matrix_C_t* col_matrix)
{
    IFX_CUBE_BRK_VALID(cube);
    IFX_ERR_BRK_NULL(col_matrix);

    IFX_MDA_VIEW_C(col_matrix, cube, IFX_MDA_SLICE_FULL(), IFX_MDA_INDEX(col_index), IFX_MDA_SLICE_FULL());
}

//----------------------------------------------------------------------------

void ifx_cube_destroy_r(ifx_Cube_R_t* cube)
{
    ifx_mda_destroy_r(cube);
}

//----------------------------------------------------------------------------

void ifx_cube_destroy_c(ifx_Cube_C_t* cube)
{
    ifx_mda_destroy_c(cube);
}

//----------------------------------------------------------------------------

void ifx_cube_copy_r(const ifx_Cube_R_t* cube, ifx_Cube_R_t* target)
{
    IFX_CUBE_BRK_VALID(cube);
    ifx_mda_copy_r(cube, target);
}

//----------------------------------------------------------------------------

void ifx_cube_copy_c(const ifx_Cube_C_t* cube, ifx_Cube_C_t* target)
{
    IFX_CUBE_BRK_VALID(cube);
    ifx_mda_copy_c(cube, target);
}

//----------------------------------------------------------------------------

ifx_Cube_R_t* ifx_cube_clone_r(const ifx_Cube_R_t* cube)
{
    return ifx_mda_clone_r(cube);
}

//----------------------------------------------------------------------------

ifx_Cube_C_t* ifx_cube_clone_c(const ifx_Cube_C_t* cube)
{
    return ifx_mda_clone_c(cube);
}

//----------------------------------------------------------------------------

void ifx_cube_col_abs_r(const ifx_Cube_C_t* cube,
                        uint32_t column_index,
                        ifx_Matrix_R_t* matrix)
{
    IFX_CUBE_BRK_VALID(cube);
    IFX_MAT_BRK_VALID(matrix);
    IFX_ERR_BRK_ARGUMENT(column_index >= cCols(cube));

    for (uint32_t r = 0; r < cRows(cube); r++)
    {
        for (uint32_t c = 0; c < cSlices(cube); c++)
        {
            mAt(matrix, r, c) = ifx_complex_abs(cAt(cube, r, column_index, c));
        }
    }
}

//----------------------------------------------------------------------------

void ifx_cube_clear_r(ifx_Cube_R_t* cube)
{
    IFX_CUBE_BRK_VALID(cube);
    ifx_mda_clear_r(cube);
}

//----------------------------------------------------------------------------

void ifx_cube_clear_c(ifx_Cube_C_t* cube)
{
    IFX_CUBE_BRK_VALID(cube);
    ifx_mda_clear_c(cube);
}
