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

#include "ifxAlgo/MTI.h"

#include "ifxBase/Defines.h"
#include "ifxBase/Error.h"
#include "ifxBase/internal/Macros.h"
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
 * @brief Defines the structure for MTI filter.
 *        Use type ifx_MTI_t for this struct.
 */
struct ifx_MTI_s
{
    ifx_Float_t alpha;                /**< Decides the weight \f$\alpha\f$ of the MTI filter.*/
    ifx_Vector_R_t* spectrum_history; /**< A real vector container that stores the historical
                                           range spectrum data to be subtracted from the next
                                           incoming range spectrum data.*/
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

ifx_MTI_t* ifx_mti_create(ifx_Float_t alpha_mti_filter,
                          uint32_t spectrum_length)
{
    IFX_ERR_BRN_ARGUMENT(alpha_mti_filter < 0 || alpha_mti_filter > 1)
    IFX_ERR_BRN_ARGUMENT(spectrum_length == 0)

    ifx_MTI_t* h = ifx_mem_alloc(sizeof(struct ifx_MTI_s));
    IFX_ERR_BRN_MEMALLOC(h);

    h->alpha = alpha_mti_filter;
    h->spectrum_history = ifx_vec_create_r(spectrum_length);

    if (h->spectrum_history == NULL)
    {
        ifx_mti_destroy(h);
        return NULL;
    }

    return h;
}

//----------------------------------------------------------------------------

void ifx_mti_destroy(ifx_MTI_t* mti)
{
    if (mti == NULL)
    {
        return;
    }

    ifx_vec_destroy_r(mti->spectrum_history);
    ifx_mem_free(mti);
}

//----------------------------------------------------------------------------

void ifx_mti_run(ifx_MTI_t* mti,
                 const ifx_Vector_R_t* input,
                 ifx_Vector_R_t* output)
{
    IFX_ERR_BRK_NULL(mti);
    IFX_VEC_BRK_VALID(input);
    IFX_VEC_BRK_VALID(output);
    IFX_VEC_BRK_DIM(mti->spectrum_history, input);
    IFX_VEC_BRK_DIM(input, output);

    // for shorter names
    const ifx_Float_t alpha = mti->alpha;
    ifx_Vector_R_t* history = mti->spectrum_history;

    // output_n := input_n - history_n
    // history_n := (1-alpha)*history_{n-1} + alpha*input_n
    for (uint32_t j = 0; j < vLen(input); j++)
    {
        const ifx_Float_t input_j = vAt(input, j);
        const ifx_Float_t history_j = vAt(history, j);
        vAt(output, j) = input_j - history_j;
        // vAt(history, j) = (1 - alpha) * history_j + alpha*input_j;
        vAt(history, j) += alpha * vAt(output, j);
    }
}
