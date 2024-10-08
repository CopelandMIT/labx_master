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

#include "ifxBase/Complex.h"
#include "ifxBase/Cube.h"
#include "ifxBase/Defines.h"
#include "ifxBase/Error.h"
#include "ifxBase/Matrix.h"
#include "ifxBase/Mem.h"
#include "ifxBase/Vector.h"

#include "ifxRadar/DBF.h"

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
 * @brief Defines the structure for DBF module.
 *        Use type ifx_DBF_t for this struct.
 */
struct ifx_DBF_s
{
    ifx_Matrix_C_t* weights; /**< Weights.*/
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

static void init_weights(ifx_DBF_t* handle,
                         const ifx_DBF_Config_t* config);

/*
==============================================================================
   6. LOCAL FUNCTIONS
==============================================================================
*/

static void init_weights(ifx_DBF_t* handle,
                         const ifx_DBF_Config_t* config)
{
    ifx_Float_t exp_arg;
    ifx_Float_t weight_r;
    ifx_Float_t weight_i;
    ifx_Complex_t weight_rx1;
    ifx_Complex_t weight;

    const ifx_Float_t exp_arg_const = (2.0f * IFX_PI * config->d_by_lambda);

    const ifx_Float_t weight_scale = 1.0f / SQRT((ifx_Float_t)config->num_antennas);

    ifx_Float_t angle_step = (ifx_Float_t)(config->max_angle - config->min_angle) / (ifx_Float_t)(config->num_beams - 1);

    IFX_COMPLEX_SET(weight_rx1, weight_scale, 0);

    for (uint32_t beam = 0; beam < config->num_beams; beam++)
    {
        exp_arg = SIND(config->min_angle + angle_step * beam) * exp_arg_const;

        IFX_MAT_AT(handle->weights, 0, beam) = weight_rx1;

        for (int32_t ant = 1; ant < config->num_antennas; ant++)
        {
            exp_arg *= (ifx_Float_t)ant;

            SINCOS(exp_arg, &weight_i, &weight_r);  // e^(j*theta) = cos(theta) + jsin(theta)

            IFX_COMPLEX_SET(weight, (weight_r * weight_scale), (weight_i * weight_scale));

            IFX_MAT_AT(handle->weights, ant, beam) = weight;
        }
    }
}

/*
==============================================================================
   7. EXPORTED FUNCTIONS
==============================================================================
*/

ifx_DBF_t* ifx_dbf_create(const ifx_DBF_Config_t* config)
{
    IFX_ERR_BRN_NULL(config);

    ifx_DBF_t* h = ifx_mem_alloc(sizeof(struct ifx_DBF_s));
    IFX_ERR_BRN_MEMALLOC(h);

    h->weights = ifx_mat_create_c(config->num_antennas, config->num_beams);
    IFX_ERR_BRN_MEMALLOC(h->weights);

    init_weights(h, config);

    return h;
}

//----------------------------------------------------------------------------

void ifx_dbf_run_c(ifx_DBF_t* handle,
                   const ifx_Cube_C_t* rng_dopp_spectrum,
                   ifx_Cube_C_t* rng_dopp_image_beam)
{
    IFX_ERR_BRK_NULL(handle);
    IFX_CUBE_BRK_VALID(rng_dopp_spectrum);
    IFX_ERR_BRK_NULL(rng_dopp_image_beam);

    IFX_ERR_BRK_ARGUMENT(IFX_CUBE_ROWS(rng_dopp_spectrum) != IFX_CUBE_ROWS(rng_dopp_image_beam));
    IFX_ERR_BRK_ARGUMENT(IFX_CUBE_COLS(rng_dopp_spectrum) != IFX_CUBE_COLS(rng_dopp_image_beam));
    IFX_ERR_BRK_ARGUMENT(IFX_MAT_COLS(handle->weights) != IFX_CUBE_SLICES(rng_dopp_image_beam));

    ifx_Matrix_C_t rd_spec_view;
    ifx_Matrix_C_t rdi_beam_view;

    int num_antennas = IFX_MAT_ROWS(handle->weights);
    int num_beams = IFX_MAT_COLS(handle->weights);

    for (int beam = 0; beam < num_beams; beam++)
    {
        ifx_cube_get_slice_c(rng_dopp_image_beam, beam, &rdi_beam_view);  // set view to the output

        ifx_cube_get_slice_c(rng_dopp_spectrum, 0, &rd_spec_view);        // set view to the rx1 rng dopp spectrum

        ifx_mat_scale_c(&rd_spec_view, IFX_MAT_AT(handle->weights, (size_t)num_antennas - 1, beam), &rdi_beam_view);

        for (int ant = 1; ant < num_antennas; ant++)
        {
            ifx_cube_get_slice_c(rng_dopp_spectrum, ant, &rd_spec_view);  // set view to the next rx antenna rng dopp spectrum

            ifx_mat_mac_c(&rdi_beam_view, &rd_spec_view, IFX_MAT_AT(handle->weights, (size_t)ant - 1, beam), &rdi_beam_view);
        }
    }
}

//----------------------------------------------------------------------------

void ifx_dbf_destroy(ifx_DBF_t* handle)
{
    if (handle == NULL)
    {
        return;
    }

    ifx_mat_destroy_c(handle->weights);
    ifx_mem_free(handle);
}

//----------------------------------------------------------------------------

uint32_t ifx_dbf_get_beam_count(ifx_DBF_t* handle)
{
    IFX_ERR_BRV_NULL(handle, 0);

    return IFX_MAT_COLS(handle->weights);
}
