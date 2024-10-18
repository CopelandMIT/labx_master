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

#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "ifxBase/Complex.h"
#include "ifxBase/Cube.h"
#include "ifxBase/Defines.h"
#include "ifxBase/Error.h"
#include "ifxBase/internal/Macros.h"
#include "ifxBase/LA.h"
#include "ifxBase/Matrix.h"
#include "ifxBase/Mem.h"
#include "ifxBase/Vector.h"

#include "ifxRadar/AngleCapon.h"

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
 * @brief Defines the structure for Angle Capon module.
 *        Use type ifx_AngleCapon_t for this struct.
 */
struct ifx_AngleCapon_s
{
    uint8_t num_virtual_antennas;           /**< Virtual number of antennas.*/
    uint8_t num_beams;                      /**< Number of beams.*/
    uint8_t selected_rx;                    /**< Select the best Rx channel for choosing proper Doppler index.*/
    ifx_Float_t phase_offset_degrees;       /**< Phase offset compensation between used Rx antennas.*/
    uint16_t neighbouring_bins;             /**< Neighbouring bins.*/
    uint16_t num_chirps;                    /**< Number of chirps per frame.*/
    ifx_Matrix_C_t* weights;                /**< Weights */
    ifx_Matrix_C_t* range_pulse_matrix;     /**< Range pulse matrix.*/
    ifx_Vector_C_t* range_pulse_scalar;     /**< Range pulse scalar matrix.*/
    ifx_Matrix_C_t* range_pulse_covariance; /**< Range pulse covariance matrix.*/
    ifx_Vector_R_t* angle_vector;           /**< Angle vector covering the radar FoV.*/
    ifx_Matrix_C_t* inverse;                /**< ... */
    ifx_Vector_C_t* tmp_vec;                /**< ... */
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

static void init_angle_vector(const ifx_AngleCapon_Config_t* config,
                              ifx_Vector_R_t* angle_vector);


static void init_range_pulse_scalar(const ifx_AngleCapon_Config_t* config,
                                    ifx_Vector_C_t* range_pulse_scalar);

/**
 * We need think about how to: 1. make use of this weight matrix or vector for
 * both AngleCapon and DBF module; 2. make use of "for iAngle = 1:numBeams" from the Matlab
 * code and only use one vector to simplify the calculation if possible.
 * For the moment, this straightforward implementation is copied from DBF module. @endinternal */
static void init_weights(ifx_Matrix_C_t* weights,
                         const ifx_AngleCapon_Config_t* config);

static uint32_t find_doppler_idx(const ifx_Matrix_C_t* rx_channel,
                                 uint16_t range_idx,
                                 uint16_t num_chirps,
                                 uint16_t neighboring_bins);

/*
==============================================================================
   6. LOCAL FUNCTIONS
==============================================================================
*/

static void init_angle_vector(const ifx_AngleCapon_Config_t* config,
                              ifx_Vector_R_t* angle_vector)
{
    const ifx_Float_t step = (config->max_angle_degrees - config->min_angle_degrees) / (vLen(angle_vector) - 1);

    for (uint32_t idx = 0; idx < vLen(angle_vector); ++idx)
    {
        ifx_vec_setat_r(angle_vector, idx, config->min_angle_degrees + step * idx);
    }
}

//----------------------------------------------------------------------------

static void init_range_pulse_scalar(const ifx_AngleCapon_Config_t* config,
                                    ifx_Vector_C_t* range_pulse_scalar)
{
    ifx_Float_t exp_arg = -2 * IFX_PI * config->d_by_lambda * SIND(config->phase_offset_degrees);
    ifx_Float_t scalar_r = 0.0;
    ifx_Float_t scalar_i = 0.0;
    ifx_Complex_t scalar;

    for (uint32_t idx = 0; idx < vLen(range_pulse_scalar); ++idx)
    {
        SINCOS(exp_arg * idx, &scalar_i, &scalar_r);
        IFX_COMPLEX_SET(scalar, scalar_r, scalar_i);
        ifx_vec_setat_c(range_pulse_scalar, idx, scalar);
    }
}

//----------------------------------------------------------------------------

static void init_weights(ifx_Matrix_C_t* weights,
                         const ifx_AngleCapon_Config_t* config)
{
    // weights(1,:) = (1/sqrt(numAntennas));
    // for iAngle = 1:numBeams
    //     for iAntenna = 2:numAntennas
    //         weights(iAntenna,iAngle) = (exp(1j*2*pi*(iAntenna-1)*d_by_lambda*sind(angleVector(iAngle)))/sqrt(numAntennas));
    //     end
    // end

    ifx_Float_t exp_arg;
    ifx_Float_t weight_r;
    ifx_Float_t weight_i;
    ifx_Complex_t weight;

    const ifx_Float_t exp_arg_const = (2 * IFX_PI * config->d_by_lambda);
    const ifx_Float_t weight_scale = 1.0f / SQRT((ifx_Float_t)config->num_virtual_antennas);
    ifx_Float_t angle_step = (config->max_angle_degrees - config->min_angle_degrees) / (config->num_beams - 1);
    for (int beam = 0; beam < config->num_beams; beam++)
    {
        exp_arg = SIND(config->min_angle_degrees + angle_step * beam) * exp_arg_const;

        for (int ant = 0; ant < config->num_virtual_antennas; ant++)
        {
            SINCOS(exp_arg * ant, &weight_i, &weight_r);
            IFX_COMPLEX_SET(weight, (weight_r * weight_scale), (weight_i * weight_scale));
            mAt(weights, ant, beam) = weight;
        }
    }
}

//----------------------------------------------------------------------------

static uint32_t find_doppler_idx(const ifx_Matrix_C_t* rx_channel,
                                 uint16_t range_idx,
                                 uint16_t num_chirps,
                                 uint16_t neighboring_bins)
{
    uint32_t doppler_idx = 0;
    ifx_Float_t maximum = -1;  // maximum will never be negative.

    for (uint32_t col = 0; col < mCols(rx_channel); ++col)
    {
        ifx_Float_t abs_value = ifx_complex_abs(IFX_MAT_AT(rx_channel, range_idx, col));

        if (abs_value > maximum)
        {
            maximum = abs_value;
            doppler_idx = col;
        }
    }

    if (doppler_idx + neighboring_bins >= num_chirps)
        doppler_idx = num_chirps - neighboring_bins - 1;
    else if (doppler_idx < neighboring_bins)
        doppler_idx = neighboring_bins;

    return doppler_idx;
}

/*
==============================================================================
   7. EXPORTED FUNCTIONS
==============================================================================
*/

ifx_AngleCapon_t* ifx_anglecapon_create(const ifx_AngleCapon_Config_t* config)
{
    IFX_ERR_BRN_NULL(config);

    ifx_AngleCapon_t* h = ifx_mem_alloc(sizeof(struct ifx_AngleCapon_s));
    IFX_ERR_BRN_MEMALLOC(h);

    h->num_virtual_antennas = config->num_virtual_antennas;
    h->num_beams = config->num_beams;
    h->selected_rx = config->selected_rx;
    h->phase_offset_degrees = config->phase_offset_degrees;
    h->neighbouring_bins = (config->range_win_size - 1) / 2;
    h->num_chirps = config->chirps_per_frame;

    IFX_ERR_HANDLE_N(h->weights = ifx_mat_create_c(config->num_virtual_antennas, config->num_beams),
                     ifx_anglecapon_destroy(h));
    init_weights(h->weights, config);

    IFX_ERR_HANDLE_N(h->range_pulse_matrix = ifx_mat_create_c(config->num_virtual_antennas, config->range_win_size),
                     ifx_anglecapon_destroy(h));

    IFX_ERR_HANDLE_N(h->range_pulse_scalar = ifx_vec_create_c(config->num_virtual_antennas),
                     ifx_anglecapon_destroy(h));
    init_range_pulse_scalar(config, h->range_pulse_scalar);

    IFX_ERR_HANDLE_N(h->angle_vector = ifx_vec_create_r(config->num_beams),
                     ifx_anglecapon_destroy(h));
    init_angle_vector(config, h->angle_vector);

    IFX_ERR_HANDLE_N(h->range_pulse_covariance = ifx_mat_create_c(config->num_virtual_antennas, config->num_virtual_antennas),
                     ifx_anglecapon_destroy(h));

    IFX_ERR_HANDLE_N(h->inverse = ifx_mat_create_c(config->num_virtual_antennas, config->num_virtual_antennas),
                     ifx_anglecapon_destroy(h));

    IFX_ERR_HANDLE_N(h->tmp_vec = ifx_vec_create_c(config->num_virtual_antennas),
                     ifx_anglecapon_destroy(h));

    return h;
}

//----------------------------------------------------------------------------

ifx_Float_t ifx_anglecapon_run(const ifx_AngleCapon_t* handle,
                               uint32_t range_bin,
                               const ifx_Cube_C_t* rx_spectrum)
{
    IFX_ERR_BRV_NULL(handle, IFX_NAN);
    IFX_CUBE_BRV_VALID(rx_spectrum, IFX_NAN);

    ifx_Matrix_C_t rx_channel;
    ifx_cube_get_slice_c(rx_spectrum, handle->selected_rx, &rx_channel);
    uint32_t doppler_idx = find_doppler_idx(&rx_channel,
                                            range_bin,
                                            handle->num_chirps,
                                            handle->neighbouring_bins);

    for (uint8_t ant_idx = 0; ant_idx < handle->num_virtual_antennas; ++ant_idx)
    {
        ifx_Matrix_C_t tmp_matrix;
        ifx_Matrix_C_t lens;
        ifx_Matrix_C_t range_pulse_row;
        ifx_cube_get_slice_c(rx_spectrum, ant_idx, &tmp_matrix);
        ifx_mat_view_c(&lens, &tmp_matrix, range_bin, doppler_idx - handle->neighbouring_bins, 1, handle->neighbouring_bins * 2 + 1);
        ifx_mat_view_c(&range_pulse_row, handle->range_pulse_matrix, ant_idx, 0, 1, mCols(handle->range_pulse_matrix));
        ifx_mat_scale_c(&lens, IFX_VEC_AT(handle->range_pulse_scalar, ant_idx), &range_pulse_row);
    }

    // Calculate covariance_matrix = range_pulse_matrix*(range_pulse_matrix Transpose)
    ifx_mat_abct_c(handle->range_pulse_matrix, handle->range_pulse_matrix, handle->range_pulse_covariance);

    // Find out the maximum value inside vector capon_result and its index
    ifx_Float_t min_value = FLT_MAX;  // all elements from capon_result are not negative
    ifx_Float_t angle = vAt(handle->angle_vector, 0);
    for (uint32_t idx = 0; idx < handle->num_beams; ++idx)
    {
        ifx_la_invert_c(handle->range_pulse_covariance, handle->inverse);
        ifx_Vector_C_t vec_weight;
        ifx_mat_get_colview_c(handle->weights, idx, &vec_weight);
        ifx_mat_mul_cv(handle->inverse, &vec_weight, handle->tmp_vec);

        ifx_Complex_t sum = IFX_COMPLEX_DEF(0, 0);
        for (uint32_t idxvec = 0; idxvec < vLen(handle->tmp_vec); ++idxvec)
        {
            ifx_Complex_t local_weight = IFX_COMPLEX_DEF(IFX_COMPLEX_REAL(vAt(&vec_weight, idxvec)), -IFX_COMPLEX_IMAG(vAt(&vec_weight, idxvec)));
            sum = ifx_complex_add(sum, ifx_complex_mul(local_weight, vAt(handle->tmp_vec, idxvec)));
        }

        ifx_Float_t value = ifx_complex_abs(sum);
        if (min_value > value)
        {
            angle = vAt(handle->angle_vector, idx);
            min_value = value;
        }
    }

    return angle;
}

//----------------------------------------------------------------------------

void ifx_anglecapon_destroy(ifx_AngleCapon_t* handle)
{
    if (handle == NULL)
    {
        return;
    }

    ifx_mat_destroy_c(handle->weights);
    ifx_mat_destroy_c(handle->range_pulse_matrix);
    ifx_vec_destroy_c(handle->range_pulse_scalar);
    ifx_vec_destroy_r(handle->angle_vector);
    ifx_mat_destroy_c(handle->range_pulse_covariance);
    ifx_mat_destroy_c(handle->inverse);
    ifx_vec_destroy_c(handle->tmp_vec);
    ifx_mem_free(handle);

    handle = NULL;
}
