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

#include "ifxAlgo/2DMTI.h"

#include "ifxBase/Complex.h"
#include "ifxBase/Error.h"
#include "ifxBase/internal/Macros.h"
#include "ifxBase/Matrix.h"
#include "ifxBase/Mem.h"

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

/**
 * @brief Defines the structure for 2D MTI filter to operate on real matrix.
 *        Use type ifx_2DMTI_R_t for this struct.
 */
struct ifx_2DMTI_R_s
{
    ifx_Float_t alpha_MTI_filter;     /**< Decides the weight \f$ alpha \f$ of the 2D MTI filter.*/
    ifx_Matrix_R_t* filter_history_r; /**< A real matrix container that stores the historical
                                           data to be subtracted from the next incoming matrix data.*/
};

/**
 * @brief Defines the structure for 2D MTI filter to operate on complex matrix.
 *        Use type ifx_2DMTI_C_t for this struct.
 */
struct ifx_2DMTI_C_s
{
    ifx_Float_t alpha_MTI_filter;     /**< Decides the weight \f$ alpha \f$ of the 2D MTI filter.*/
    ifx_Matrix_C_t* filter_history_c; /**< A complex matrix container that stores the historical
                                           data to be subtracted from the next incoming matrix data.*/
};

/*
==============================================================================
   4. LOCAL DATA
==============================================================================
*/

/*
==============================================================================
   5. LOCAL FUNCTION PROTOTYPES
==============================================================================
*/

/*
==============================================================================
   6. LOCAL FUNCTIONS
==============================================================================
*/

/*
==============================================================================
   7. EXPORTED FUNCTIONS
==============================================================================
*/

ifx_2DMTI_R_t* ifx_2dmti_create_r(ifx_Float_t alpha_mti_filter,
                                  uint32_t rows,
                                  uint32_t columns)
{
    IFX_ERR_BRN_ARGUMENT(alpha_mti_filter < 0 || alpha_mti_filter > 1);
    IFX_ERR_BRN_ARGUMENT(rows == 0);
    IFX_ERR_BRN_ARGUMENT(columns == 0);

    ifx_2DMTI_R_t* h = ifx_mem_alloc(sizeof(struct ifx_2DMTI_R_s));
    IFX_ERR_BRN_MEMALLOC(h);

    IFX_ERR_HANDLE_N(h->filter_history_r = ifx_mat_create_r(rows, columns),
                     ifx_2dmti_destroy_r(h));

    h->alpha_MTI_filter = alpha_mti_filter;

    return h;
}

//----------------------------------------------------------------------------

ifx_2DMTI_C_t* ifx_2dmti_create_c(ifx_Float_t alpha_mti_filter,
                                  uint32_t rows,
                                  uint32_t columns)
{
    IFX_ERR_BRN_ARGUMENT(alpha_mti_filter < 0 || alpha_mti_filter > 1);
    IFX_ERR_BRN_ARGUMENT(rows == 0);
    IFX_ERR_BRN_ARGUMENT(columns == 0);

    ifx_2DMTI_C_t* h = ifx_mem_alloc(sizeof(struct ifx_2DMTI_C_s));
    IFX_ERR_BRN_MEMALLOC(h);

    IFX_ERR_HANDLE_N(h->filter_history_c = ifx_mat_create_c(rows, columns),
                     ifx_2dmti_destroy_c(h));

    h->alpha_MTI_filter = alpha_mti_filter;

    return h;
}

//----------------------------------------------------------------------------

void ifx_2dmti_destroy_r(ifx_2DMTI_R_t* handle)
{
    if (handle == NULL)
    {
        return;
    }

    ifx_mat_destroy_r(handle->filter_history_r);

    ifx_mem_free(handle);
}

//----------------------------------------------------------------------------

void ifx_2dmti_destroy_c(ifx_2DMTI_C_t* handle)
{
    if (handle == NULL)
    {
        return;
    }

    ifx_mat_destroy_c(handle->filter_history_c);

    ifx_mem_free(handle);
}

//----------------------------------------------------------------------------

void ifx_2dmti_run_r(ifx_2DMTI_R_t* handle,
                     const ifx_Matrix_R_t* input,
                     ifx_Matrix_R_t* output)
{
    IFX_ERR_BRK_NULL(handle);
    IFX_MAT_BRK_VALID(input);
    IFX_MAT_BRK_VALID(output);
    IFX_MAT_BRK_DIM(handle->filter_history_r, input);
    IFX_MAT_BRK_DIM(input, output);

    // for shorter names
    const uint32_t rows = mRows(input);
    const uint32_t cols = mCols(input);
    const ifx_Float_t alpha = handle->alpha_MTI_filter;
    ifx_Matrix_R_t* history = handle->filter_history_r;

    // output_n := input_n - history_n
    // history_n := alpha*input_n + (1-alpha)*history_{n-1}
    for (uint32_t r = 0; r < rows; r++)
    {
        for (uint32_t c = 0; c < cols; c++)
        {
            const ifx_Float_t input_rc = mAt(input, r, c);
            const ifx_Float_t history_rc = mAt(history, r, c);
            mAt(output, r, c) = input_rc - history_rc;
            mAt(history, r, c) = alpha * input_rc + (1 - alpha) * history_rc;
        }
    }
}

//----------------------------------------------------------------------------

void ifx_2dmti_run_c(ifx_2DMTI_C_t* handle,
                     const ifx_Matrix_C_t* input,
                     ifx_Matrix_C_t* output)
{
    IFX_ERR_BRK_NULL(handle);
    IFX_MAT_BRK_VALID(input);
    IFX_MAT_BRK_VALID(output);
    IFX_MAT_BRK_DIM(handle->filter_history_c, input);
    IFX_MAT_BRK_DIM(input, output);

    // for shorter names
    const uint32_t rows = mRows(input);
    const uint32_t cols = mCols(input);
    const ifx_Float_t alpha = handle->alpha_MTI_filter;
    ifx_Matrix_C_t* history = handle->filter_history_c;

    // output_n := input_n - history_n
    // history_n := alpha*input_n + (1-alpha)*history_{n-1}
    for (uint32_t r = 0; r < rows; r++)
    {
        for (uint32_t c = 0; c < cols; c++)
        {
            const ifx_Complex_t input_rc = mAt(input, r, c);
            const ifx_Complex_t history_rc = mAt(history, r, c);
            mAt(output, r, c) = ifx_complex_sub(input_rc, history_rc);
            mAt(history, r, c) = ifx_complex_add(
                ifx_complex_mul_real(input_rc, alpha),
                ifx_complex_mul_real(history_rc, (1 - alpha)));
        }
    }
}

//----------------------------------------------------------------------------

void ifx_2dmti_set_filter_coeff_r(ifx_2DMTI_R_t* handle,
                                  ifx_Float_t alpha_mti_filter)
{
    IFX_ERR_BRK_NULL(handle);
    IFX_ERR_BRK_ARGUMENT(alpha_mti_filter < 0 || alpha_mti_filter > 1);

    handle->alpha_MTI_filter = alpha_mti_filter;
}

//----------------------------------------------------------------------------

ifx_Float_t ifx_2dmti_get_filter_coeff_r(ifx_2DMTI_R_t* handle)
{
    IFX_ERR_BRV_NULL(handle, 0);

    return handle->alpha_MTI_filter;
}

//----------------------------------------------------------------------------

void ifx_2dmti_set_filter_coeff_c(ifx_2DMTI_C_t* handle,
                                  ifx_Float_t alpha_mti_filter)
{
    IFX_ERR_BRK_NULL(handle);
    IFX_ERR_BRK_ARGUMENT(alpha_mti_filter < 0 || alpha_mti_filter > 1);

    handle->alpha_MTI_filter = alpha_mti_filter;
}

//----------------------------------------------------------------------------

ifx_Float_t ifx_2dmti_get_filter_coeff_c(ifx_2DMTI_C_t* handle)
{
    IFX_ERR_BRV_NULL(handle, 0);

    return handle->alpha_MTI_filter;
}
