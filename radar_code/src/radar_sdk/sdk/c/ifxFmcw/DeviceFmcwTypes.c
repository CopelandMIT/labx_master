/* ===========================================================================
** Copyright (C) 2021 - 2022 Infineon Technologies AG
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

#include "DeviceFmcwTypes.h"
#include "ifxBase/Mem.h"


/*
==============================================================================
   5. LOCAL FUNCTION PROTOTYPES
==============================================================================
*/

ifx_Fmcw_Sequence_Element_t* ifx_fmcw_create_simple_sequence(ifx_Fmcw_Simple_Sequence_Config_t* config)
{
    IFX_ERR_BRN_NULL(config);
    ifx_Fmcw_Sequence_Element_t* frame_loop = ifx_mem_alloc(sizeof(ifx_Fmcw_Sequence_Element_t));
    IFX_ERR_BRN_MEMALLOC(frame_loop);
    ifx_Fmcw_Sequence_Element_t* chirp_loop = ifx_mem_alloc(sizeof(ifx_Fmcw_Sequence_Element_t));
    IFX_ERR_BRN_MEMALLOC(chirp_loop);
    ifx_Fmcw_Sequence_Element_t* chirp = ifx_mem_alloc(sizeof(ifx_Fmcw_Sequence_Element_t));
    IFX_ERR_BRN_MEMALLOC(chirp);

    frame_loop->type = IFX_SEQ_LOOP;
    frame_loop->next_element = NULL;
    frame_loop->loop.sub_sequence = chirp_loop;
    frame_loop->loop.num_repetitions = 0;
    frame_loop->loop.repetition_time_s = config->frame_repetition_time_s;

    chirp_loop->type = IFX_SEQ_LOOP;
    chirp_loop->next_element = NULL;
    chirp_loop->loop.sub_sequence = chirp;
    chirp_loop->loop.num_repetitions = config->num_chirps;
    chirp_loop->loop.repetition_time_s = config->chirp_repetition_time_s;

    chirp->type = IFX_SEQ_CHIRP;
    chirp->next_element = NULL;
    chirp->chirp = config->chirp;

    /*
     * For TDM MIMO a chirp is added for each active TX antenna. First the
     * existing chirp is modified to use only the first active TX, then a chirp
     * for each remaining TX antenna is added.
     */
    if (config->tdm_mimo && (config->chirp.tx_mask != 0))
    {
        // Search for the first active TX antenna.
        uint8_t overall_mask = config->chirp.tx_mask;
        uint8_t current_mask = 1;
        while ((overall_mask & current_mask) == 0)
            current_mask <<= 1;

        // Activate only first TX antenna in first chirp.
        chirp->chirp.tx_mask = current_mask;
        overall_mask &= ~current_mask;

        // Add new chirp for each active TX antenna.
        for (; overall_mask != 0; current_mask <<= 1)
        {
            if ((overall_mask & current_mask) == 0)
                continue;

            ifx_Fmcw_Sequence_Element_t* new_chirp = ifx_mem_alloc(sizeof(ifx_Fmcw_Sequence_Element_t));
            if (new_chirp == NULL)
            {
                ifx_fmcw_destroy_sequence(frame_loop);
                ifx_error_set(IFX_ERROR_MEMORY_ALLOCATION_FAILED);
                return NULL;
            }

            new_chirp->type = IFX_SEQ_CHIRP;
            new_chirp->next_element = NULL;
            new_chirp->chirp = config->chirp;

            new_chirp->chirp.tx_mask = current_mask;
            overall_mask &= ~current_mask;

            chirp->next_element = new_chirp;
            chirp = new_chirp;
        }
    }

    return frame_loop;
}

// NOLINTNEXTLINE(misc-no-recursion)
void ifx_fmcw_destroy_sequence(ifx_Fmcw_Sequence_Element_t* sequence)
{
    if (sequence == NULL)
    {
        return;
    }

    if (sequence->type == IFX_SEQ_LOOP)
    {
        ifx_fmcw_destroy_sequence(sequence->loop.sub_sequence);
    }
    ifx_fmcw_destroy_sequence(sequence->next_element);
    ifx_mem_free(sequence);
}

ifx_Fmcw_Sequence_Element_t* ifx_fmcw_create_sequence_element(ifx_Fmcw_Element_Type type)
{
    ifx_Fmcw_Sequence_Element_t* element = ifx_mem_alloc(sizeof(ifx_Fmcw_Sequence_Element_t));
    IFX_ERR_BRN_MEMALLOC(element);

    element->type = type;
    element->next_element = NULL;
    if (element->type == IFX_SEQ_LOOP)
    {
        element->loop.sub_sequence = NULL;
    }
    return element;
}
