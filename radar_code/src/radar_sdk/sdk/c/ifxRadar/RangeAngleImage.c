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

#include <stdlib.h> /* for qsort */

#include "ifxAlgo/2DMTI.h"

#include "ifxBase/Cube.h"
#include "ifxBase/Defines.h"
#include "ifxBase/Error.h"
#include "ifxBase/internal/Macros.h"
#include "ifxBase/Matrix.h"
#include "ifxBase/Mem.h"
#include "ifxBase/Vector.h"

#include "ifxAlgo/2DMTI.h"
#include "ifxRadar/RangeAngleImage.h"


/*
==============================================================================
   2. LOCAL DEFINITIONS
==============================================================================
*/

#define MAX_NUM_OF_IMAGES (64U)

#define MAX_NUM_ANTENNA_ARRAYS (16U)

#define USE_TEMP_MATRIX 1

/*
==============================================================================
   3. LOCAL TYPES
==============================================================================
*/

/**
 * @brief Defines the structure for Range Angle Image module processing.
 *        Use type ifx_RAI_t for this struct.
 */
struct ifx_RAI_s
{
    ifx_RDM_t* rdm_handle;            /**< Range doppler map handle for all rx antennas.*/
    ifx_2DMTI_C_t** mti_handle_array; /**< 2D MTI filter coefficient.*/
    ifx_DBF_t* dbf_handle;            /**< Digital beamforming module handle.*/
    uint32_t num_of_images;           /**< Number of images (responses) for Range Angle Image.*/
    uint32_t num_antenna_array;       /**< Number of virtual antennas.*/
    ifx_Cube_C_t* rdm_cube;           /**< 2D complex range doppler maps over rx antennas as a cube.*/
    ifx_Cube_C_t* rx_spectrum_cube;   /**< ... */
    ifx_Cube_C_t* dbf_cube;           /**< 2D complex DBF over rx antennas as a cube.*/
    ifx_Vector_R_t* snr_vec;          /**< SNR over doppler slices.*/
#ifdef USE_TEMP_MATRIX
    ifx_Matrix_R_t* temp_matrix;      /**< Scratch buffer to calculate SNR.*/
#endif
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

static void calculate_snr(ifx_RAI_t* handle);

static int cmpfunc(const void* a, const void* b);

/*
==============================================================================
   6. LOCAL FUNCTIONS
==============================================================================
*/

int float_compare(void* h, uint32_t ia, uint32_t ib)
{
    ifx_Float_t a = ((ifx_Float_t*)h)[ia];
    ifx_Float_t b = ((ifx_Float_t*)h)[ib];
    if (a > b)
        return 1;
    if (a < b)
        return -1;
    return 0;
}

static void ssort(void* handle,
                  uint32_t count,
                  int (*compare)(void* handle, uint32_t ia, uint32_t ib),
                  ifx_Vector_Sort_Order_t order,
                  uint32_t* array)
{
    uint32_t gaps[] = {701, 301, 132, 57, 23, 10, 4, 1};  // tbd
    uint32_t num_gaps = sizeof(gaps) / sizeof(gaps[0]);
    int ord = order == IFX_SORT_DESCENDING ? -1 : 1;

    for (uint32_t i = 0; i < count; i++)
    {
        array[i] = i;
    }

    for (uint32_t g = 0; g < num_gaps; g++)
    {
        uint32_t gap = gaps[g];

        if (gap > count)
        {
            continue;
        }

        for (uint32_t i = gap; i < count; i++)
        {
            uint32_t temp = array[i];
            uint32_t j;

            for (j = i; j >= gap && (compare(handle, array[j - gap], temp) == ord); j -= gap)
            {
                array[j] = array[j - gap];
            }

            array[j] = temp;
        }
    }
}

#ifdef USE_TEMP_MATRIX
static void calculate_snr(ifx_RAI_t* handle)
{
    ifx_Float_t signal_power;
    ifx_Float_t variance;

    for (uint32_t idx_doppler = 0; idx_doppler < cCols(handle->dbf_cube); ++idx_doppler)
    {
        ifx_cube_col_abs_r(handle->dbf_cube, idx_doppler, handle->temp_matrix);

        signal_power = ifx_mat_max_r(handle->temp_matrix);

        signal_power *= signal_power;

        variance = ifx_mat_var_r(handle->temp_matrix);

        IFX_VEC_AT(handle->snr_vec, idx_doppler) = signal_power / variance;
    }
}
#else
static void calculate_snr(ifx_RAI_t* handle)
{
    // c corresponds to the doppler index
    for (uint32_t c = 0; c < cCols(handle->dbf_cube); c++)
    {
        // this is used to compute max_{r,s} |DBF_{r,c,s}|
        ifx_Float_t max_abs_elem = 0;

        // The variance is computed using Welford's online algorithm, see
        // https://en.wikipedia.org/w/index.php?title=Algorithms_for_calculating_variance&oldid=928348206#Welford's_online_algorithm
        // for more information.

        // index for Welford's algorithm
        uint32_t n = 0;

        // sample mean of the first n samples
        ifx_Float_t mean_n = 0;

        // sum of squares of differences from the current mean (see link above
        // for more information)
        ifx_Float_t M2_n = 0;

        // compute SNR value for current index doppler (fixed column)
        for (uint32_t r = 0; r < cRows(handle->dbf_cube); r++)
        {
            for (uint32_t s = 0; s < cSlices(handle->dbf_cube); s++)
            {
                // xn = |DBF_{r,c,s}|
                const ifx_Float_t xn = ifx_complex_abs(cAt(handle->dbf_cube, r, c, s));

                // max_abs_elem = max_{r,s} |DBF_{r,c,s}|
                if (xn > max_abs_elem)
                    max_abs_elem = xn;

                // compute average and mean using Welford's algorithm
                n++;

                // save last values (mean_{n-1}, M2_{n-1})
                ifx_Float_t mean_nm1 = mean_n;
                ifx_Float_t M2_nm1 = M2_n;

                // update mean and M2n
                mean_n = mean_nm1 + (xn - mean_nm1) / n;
                M2_n = M2_nm1 + (xn - mean_nm1) * (xn - mean_n);
            }
        }

        // compute variance
        ifx_Float_t variance = M2_n / n;

        const ifx_Float_t signal_power = max_abs_elem * max_abs_elem;
        IFX_VEC_AT(handle->snr_vec, c) = signal_power / variance;
    }
}
#endif

//----------------------------------------------------------------------------

static int cmpfunc(const void* a, const void* b)
{
    if (*(ifx_Float_t*)a > *(ifx_Float_t*)b)
        return -1;
    else if (*(ifx_Float_t*)a < *(ifx_Float_t*)b)
        return 1;
    else  // if( *(ifx_Float_t*)a == *(ifx_Float_t*)b )
        return 0;
}

/*
==============================================================================
   7. EXPORTED FUNCTIONS
==============================================================================
*/

ifx_RAI_t* ifx_rai_create(ifx_RAI_Config_t* config)
{
    IFX_ERR_BRN_NULL(config);
    IFX_ERR_BRV_ARGUMENT(config->num_of_images > MAX_NUM_OF_IMAGES, NULL);
    IFX_ERR_BRV_ARGUMENT(config->num_antenna_array > MAX_NUM_ANTENNA_ARRAYS || config->num_antenna_array == 0, NULL);

    ifx_RAI_t* h = ifx_mem_alloc(sizeof(struct ifx_RAI_s));
    IFX_ERR_BRN_MEMALLOC(h);

    //----------------------- Range Doppler Map Handle -----------------------
    IFX_ERR_HANDLE_N(h->rdm_handle = ifx_rdm_create(&config->rdm_config),
                     ifx_rai_destroy(h));

    uint32_t range_fft_size = config->rdm_config.range_fft_config.fft_size;
    uint32_t doppler_fft_size = config->rdm_config.doppler_fft_config.fft_size;

    if (config->rdm_config.range_fft_config.fft_type == IFX_FFT_TYPE_R2C)
    {
        range_fft_size /= 2;
    }

    IFX_ERR_HANDLE_N(h->rdm_cube = ifx_cube_create_c(range_fft_size, doppler_fft_size, config->num_antenna_array),
                     ifx_rai_destroy(h));
    IFX_ERR_HANDLE_N(h->rx_spectrum_cube = ifx_cube_create_c(range_fft_size, doppler_fft_size, config->num_antenna_array),
                     ifx_rai_destroy(h));

    //----------------------- 2D MTI Handle ----------------------------------
    h->mti_handle_array = ifx_mem_alloc(sizeof(ifx_2DMTI_C_t*) * config->num_antenna_array);
    IFX_ERR_BRN_MEMALLOC(h);

    for (uint32_t i = 0; i < config->num_antenna_array; ++i)
    {
        IFX_ERR_HANDLE_N(h->mti_handle_array[i] = ifx_2dmti_create_c(config->alpha_mti_filter,
                                                                     range_fft_size, doppler_fft_size),
                         ifx_rai_destroy(h));
    }

    //----------------------- DBF Handle -------------------------------------
    IFX_ERR_HANDLE_N(h->dbf_handle = ifx_dbf_create(&config->dbf_config),
                     ifx_rai_destroy(h));

    IFX_ERR_HANDLE_N(h->dbf_cube = ifx_cube_create_c(range_fft_size, doppler_fft_size, config->dbf_config.num_beams),
                     ifx_rai_destroy(h));

    //----------------------- Internal Scratch Buffers -----------------------
    IFX_ERR_HANDLE_N(h->snr_vec = ifx_vec_create_r(config->rdm_config.doppler_fft_config.fft_size),
                     ifx_rai_destroy(h));

#ifdef USE_TEMP_MATRIX
    IFX_ERR_HANDLE_N(h->temp_matrix = ifx_mat_create_r(range_fft_size, config->dbf_config.num_beams),
                     ifx_rai_destroy(h));
#endif

    h->num_of_images = config->num_of_images;
    h->num_antenna_array = config->num_antenna_array;

    return h;
}

//----------------------------------------------------------------------------

void ifx_rai_destroy(ifx_RAI_t* handle)
{
    if (handle == NULL)
    {
        return;
    }

#ifdef USE_TEMP_MATRIX
    ifx_mat_destroy_r(handle->temp_matrix);
#endif
    ifx_vec_destroy_r(handle->snr_vec);

    ifx_cube_destroy_c(handle->dbf_cube);
    ifx_cube_destroy_c(handle->rdm_cube);
    ifx_cube_destroy_c(handle->rx_spectrum_cube);

    ifx_dbf_destroy(handle->dbf_handle);
    ifx_rdm_destroy(handle->rdm_handle);

    for (uint32_t i = 0; i < handle->num_antenna_array; ++i)
    {
        ifx_2dmti_destroy_c(handle->mti_handle_array[i]);
    }

    ifx_mem_free(handle->mti_handle_array);

    ifx_mem_free(handle);
}

//----------------------------------------------------------------------------

void ifx_rai_run_r(ifx_RAI_t* handle,
                   const ifx_Cube_R_t* input,
                   ifx_Cube_R_t* output)
{
    IFX_ERR_BRK_NULL(handle);
    IFX_CUBE_BRK_VALID(input);
    IFX_CUBE_BRK_VALID(output);

    // rdm_view: range_fft_size x doppler_fft_size
    ifx_Matrix_C_t rdm_view = {0};

    for (uint32_t rx = 0; rx < handle->num_antenna_array; ++rx)
    {
        // rawdata_view: num_chirps_per_frame x num_samples_per_frame
        ifx_Matrix_R_t rawdata_view = {0};

        // rx_spectrum_view: range_fft_size x doppler_fft_size
        ifx_Matrix_C_t rx_spectrum_view = {0};

        ifx_cube_get_row_r(input, rx, &rawdata_view);           // set view to the rx antenna for raw data matrix

        ifx_cube_get_slice_c(handle->rdm_cube, rx, &rdm_view);  // set view to the rx antenna for range doppler map

        ifx_cube_get_slice_c(handle->rx_spectrum_cube, rx, &rx_spectrum_view);

        ifx_rdm_run_rc(handle->rdm_handle, &rawdata_view, &rdm_view);

        ifx_2dmti_run_c(handle->mti_handle_array[rx], &rdm_view, &rx_spectrum_view);
    }

    ifx_dbf_run_c(handle->dbf_handle, handle->rx_spectrum_cube, handle->dbf_cube);

    calculate_snr(handle);

    // doppler FFT size
    uint32_t* snr_sorted_idx = ifx_mem_alloc(mCols(&rdm_view) * sizeof(uint32_t));
    IFX_ERR_BRK_MEMALLOC(snr_sorted_idx);

    ssort(vDat(handle->snr_vec), vLen(handle->snr_vec), float_compare, IFX_SORT_DESCENDING, snr_sorted_idx);

    for (uint32_t image = 0; image < handle->num_of_images; ++image)
    {
        uint32_t dopp_idx = snr_sorted_idx[image];

        // output: num_images (rows) x num_samples_per_frame (cols) x num_beams (slices)
        // Get a view for constant row; rai_view num_samples_per_frame x num_beams
        ifx_Matrix_R_t rai_view = {0};
        ifx_cube_get_row_r(output, image, &rai_view);

        ifx_cube_col_abs_r(handle->dbf_cube, dopp_idx, &rai_view);
    }

    ifx_mem_free(snr_sorted_idx);

    qsort(vDat(handle->snr_vec), vLen(handle->snr_vec), sizeof(ifx_Float_t), cmpfunc);
}

//----------------------------------------------------------------------------

ifx_Vector_R_t* ifx_rai_get_snr(ifx_RAI_t* handle)
{
    IFX_ERR_BRV_NULL(handle, NULL);
    return handle->snr_vec;
}

//----------------------------------------------------------------------------

ifx_Cube_C_t* ifx_rai_get_rx_spectrum(ifx_RAI_t* handle)
{
    IFX_ERR_BRV_NULL(handle, NULL);
    return handle->rx_spectrum_cube;
}

//----------------------------------------------------------------------------

ifx_Cube_C_t* ifx_rai_get_range_doppler(ifx_RAI_t* handle)
{
    IFX_ERR_BRV_NULL(handle, NULL);
    return handle->rdm_cube;
}
