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

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "ifxAlgo/OSCFAR.h"

#include "ifxBase/Defines.h"
#include "ifxBase/Error.h"
#include "ifxBase/internal/Macros.h"
#include "ifxBase/Matrix.h"
#include "ifxBase/Mem.h"
#include "ifxBase/Vector.h"


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
 * @brief Defines the structure for OSCFAR module.
 *        Use type ifx_OSCFAR_s for this struct.
 */
struct ifx_OSCFAR_s
{
    uint8_t ref_win_len;         /**< Reference window length.*/
    uint16_t os_index;           /**< Ordered statistic metric (index).*/
    ifx_Float_t coarse_scalar;   /**< Used for coarse thresholding 2D feature map.*/
    ifx_Float_t alpha;           /**< Threshold factor.*/
    ifx_Matrix_R_t* sliding_win; /**< Sliding Window. */
    ifx_Vector_R_t* tmp_ref_vec; /**< ... */
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

/**
 * @brief Local compare function for qsort
 */
static int cmpfunc(const void* a, const void* b);

/*
==============================================================================
   6. LOCAL FUNCTIONS
==============================================================================
*/

static int cmpfunc(const void* a, const void* b)
{
    if (*(ifx_Float_t*)a > *(ifx_Float_t*)b)
    {
        return 1;
    }
    else if (*(ifx_Float_t*)a < *(ifx_Float_t*)b)
    {
        return -1;
    }
    else  // if( *(ifx_Float_t*)a == *(ifx_Float_t*)b )
    {
        return 0;
    }
}

/*
==============================================================================
   7. EXPORTED FUNCTIONS
==============================================================================
*/

ifx_OSCFAR_t* ifx_oscfar_create(const ifx_OSCFAR_Config_t* config)
{
    IFX_ERR_BRN_NULL(config);

    ifx_OSCFAR_t* h = ifx_mem_alloc(sizeof(struct ifx_OSCFAR_s));
    IFX_ERR_BRN_MEMALLOC(h);

    uint16_t ref_mat_size = 2 * config->win_rank - 1;
    uint16_t osarray_size = ref_mat_size * ref_mat_size
                            - (config->guard_band * 2 + 1) * (config->guard_band * 2 + 1);

    h->ref_win_len = config->win_rank - 1;
    h->os_index = (uint16_t)FLOOR(osarray_size * config->sample + (ifx_Float_t)0.5) - 1;
    h->coarse_scalar = config->coarse_scalar;
    h->alpha = osarray_size * (POW(config->pfa, -(ifx_Float_t)1 / osarray_size) - 1);

    uint16_t outer_row_index = (config->win_rank - 1) - (config->guard_band - 1);
    uint16_t outer_col_index = outer_row_index - 1;
    IFX_ERR_HANDLE_N(h->sliding_win = ifx_mat_create_r(ref_mat_size, ref_mat_size),
                     ifx_oscfar_destroy(h));

    for (uint16_t row = 0; row < ref_mat_size; ++row)
    {
        for (uint16_t col = 0; col < ref_mat_size; ++col)
        {
            if (row < outer_col_index
                || row > ref_mat_size - outer_row_index
                || col < outer_col_index
                || col > ref_mat_size - outer_row_index)
            {
                IFX_MAT_AT(h->sliding_win, row, col) = 1;
            }
        }
    }

    IFX_ERR_HANDLE_N(h->tmp_ref_vec = ifx_vec_create_r(ref_mat_size * ref_mat_size),
                     ifx_oscfar_destroy(h));

    return h;
}

//----------------------------------------------------------------------------

void ifx_oscfar_run(const ifx_OSCFAR_t* handle,
                    ifx_Matrix_R_t* feature2D,
                    ifx_Matrix_R_t* detector_output)
{
    IFX_ERR_BRK_NULL(handle);
    IFX_MAT_BRK_VALID(feature2D);
    IFX_MAT_BRK_VALID(detector_output);

    ifx_mat_clear_r(detector_output);

    ifx_Float_t input_mean = ifx_mat_mean_r(feature2D);
    ifx_Float_t coarse_threshold = handle->coarse_scalar * input_mean;

    for (uint32_t col = handle->ref_win_len + 1; col < mCols(feature2D) - handle->ref_win_len - 1; ++col)
    {
        for (uint32_t row = handle->ref_win_len + 1; row < mRows(feature2D) - handle->ref_win_len - 1; ++row)
        {
            if (IFX_MAT_AT(feature2D, row, col) > coarse_threshold)
            {
                // calculate handle->tmp_ref_vec
                uint32_t ref_row_idx = row - handle->ref_win_len;
                uint32_t ref_col_idx = col - handle->ref_win_len;

                for (uint32_t sliding_row = 0; sliding_row < mRows(handle->sliding_win); ++sliding_row)
                {
                    for (uint32_t sliding_col = 0; sliding_col < mCols(handle->sliding_win); ++sliding_col)
                    {
                        ifx_Float_t tmp_value = IFX_MAT_AT(handle->sliding_win, sliding_row, sliding_col)
                                                * IFX_MAT_AT(feature2D, (size_t)ref_row_idx + sliding_row, (size_t)ref_col_idx + sliding_col);
                        ifx_vec_setat_r(handle->tmp_ref_vec, sliding_row * mCols(handle->sliding_win) + sliding_col, tmp_value);
                    }
                }

                qsort(vDat(handle->tmp_ref_vec), vLen(handle->tmp_ref_vec), sizeof(ifx_Float_t), cmpfunc);
                ifx_Float_t os_threshold = handle->alpha * IFX_VEC_AT(handle->tmp_ref_vec, handle->os_index);

                if (IFX_MAT_AT(feature2D, row, col) < os_threshold)
                {
                    IFX_MAT_AT(feature2D, row, col) = 0;
                }
                else
                {
                    IFX_MAT_AT(detector_output, row, col) = IFX_MAT_AT(feature2D, row, col);
                }
            }
        }
    }
}

//----------------------------------------------------------------------------

void ifx_oscfar_destroy(ifx_OSCFAR_t* handle)
{
    if (handle == NULL)
    {
        return;
    }

    ifx_mat_destroy_r(handle->sliding_win);
    ifx_vec_destroy_r(handle->tmp_ref_vec);
    ifx_mem_free(handle);

    handle = NULL;
}
