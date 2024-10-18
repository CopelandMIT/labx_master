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

#include <stdlib.h>
#include <string.h>

#include "ifxBase/Error.h"
#include "ifxBase/internal/Macros.h"
#include "ifxBase/Mem.h"
#include "ifxBase/Vector.h"

#include "ifxRadar/PeakSearch.h"

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
 *
 * @brief Defines the structure for Peak Search module.
 *        Use type struct ifx_Peak_Search_t for this struct.
 *
 * The peak search module works on an input data set, typically a vector of
 * real numbers 'y' (e.g. amplitude) representing a certain quantity 'x' (e.g a range spectrum).
 * The run function of this module (\ref ifx_peak_search_run) identifies peaks
 * in 'y' represented by the data set. This structure specifies certain
 * parameters based on which the peaks are identified in the input data set, and also
 * the result of the peak search.
 *
 */
struct ifx_Peak_Search_s
{
    ifx_Float_t value_per_bin;     /**< This is the value quantity 'x' represented by each bin (or each sample) of the
                                        input data set e.g., if the input data set is a range spectrum, then
                                        this value gives the range represented by each sample of the data set.*/
    ifx_Float_t search_zone_start; /**< This specifies the start value of a section of the input data where the peak
                                        search needs to be made e.g., if the data set is a range spectrum
                                        this value specifies the starting range value of the search zone.*/
    ifx_Float_t search_zone_end;   /**< This specifies the end value of a section of the input data where the peak
                                        search needs to be made e.g, if the data set is a range spectrum
                                        this value specifies the end range value of the search zone.*/
    ifx_Float_t threshold_factor;  /**< The peak search incorporates an adaptive threshold value of 'y' below
                                        which any peaks are ignored. This factor is multiplied with the mean value
                                        of the entire input data set to get an adaptive threshold value.*/
    ifx_Float_t threshold_offset;  /**< This value is added to the value obtained by multiplying the threshold_factor with
                                        the mean of the data set to get the final 'y' threshold.*/
    uint32_t max_num_peaks;        /**< This decides the max number of peaks to be identified in the active zone.*/
    uint32_t peak_count;           /**< This gives the number of peaks identified.*/
    uint32_t* peak_idx;            /**< This gives the indices of the peaks identified in the input data set as a vector.
                                        The size of this vector is equal to peak_count.*/
    ifx_Float_t* peak_val;         /**< This gives the values of the peaks identified in the input data set as a vector.
                                        The size of this vector is equal to peak_count.*/
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
 * @brief Returns the threshold object
 *
 * @param [in]     data_set  ...
 * @param [in]     factor    ...
 * @param [in]     offset    ...
 *
 * @return Threshold value
 */
static ifx_Float_t get_threshold(const ifx_Vector_R_t* data_set,
                                 ifx_Float_t factor,
                                 ifx_Float_t offset);

/**
 * @brief Resets the peak search handle
 *
 * @param [in]     handle    A handle to the peak search object
 */
static void reset_handle(ifx_Peak_Search_t* handle);

/*
==============================================================================
   6. LOCAL FUNCTIONS
==============================================================================
*/

static ifx_Float_t get_threshold(const ifx_Vector_R_t* data_set,
                                 ifx_Float_t factor,
                                 ifx_Float_t offset)
{
    ifx_Float_t threshold = ifx_vec_sum_r(data_set);

    threshold *= factor / (ifx_Float_t)vLen(data_set);
    threshold += offset;

    return threshold;
}

//----------------------------------------------------------------------------

static void reset_handle(ifx_Peak_Search_t* handle)
{
    handle->peak_count = 0;
    memset(handle->peak_idx, 0, sizeof(uint32_t) * handle->max_num_peaks);
    memset(handle->peak_val, 0, sizeof(ifx_Float_t) * handle->max_num_peaks);
}

/*
==============================================================================
   7. EXPORTED FUNCTIONS
==============================================================================
*/

ifx_Peak_Search_t* ifx_peak_search_create(const ifx_Peak_Search_Config_t* config)
{
    IFX_ERR_BRN_ARGUMENT(config->value_per_bin <= 0);
    IFX_ERR_BRN_ARGUMENT(config->search_zone_start <= 0);
    IFX_ERR_BRN_ARGUMENT(config->search_zone_end <= 0 || config->search_zone_end < config->search_zone_start);
    IFX_ERR_BRN_ARGUMENT(config->max_num_peaks == 0);

    ifx_Peak_Search_t* h = ifx_mem_alloc(sizeof(struct ifx_Peak_Search_s));
    IFX_ERR_BRN_MEMALLOC(h);

    h->value_per_bin = config->value_per_bin;
    h->search_zone_start = config->search_zone_start;
    h->search_zone_end = config->search_zone_end;
    h->threshold_factor = config->threshold_factor;
    h->threshold_offset = config->threshold_offset;
    h->max_num_peaks = config->max_num_peaks;

    h->peak_idx = ifx_mem_alloc(sizeof(uint32_t) * config->max_num_peaks);
    h->peak_val = ifx_mem_alloc(sizeof(ifx_Float_t) * config->max_num_peaks);

    reset_handle(h);
    return h;
}

//----------------------------------------------------------------------------

void ifx_peak_search_destroy(ifx_Peak_Search_t* handle)
{
    if (handle == NULL)
    {
        return;
    }

    ifx_mem_free(handle->peak_idx);
    ifx_mem_free(handle->peak_val);
    ifx_mem_free(handle);
}

//----------------------------------------------------------------------------

void ifx_peak_search_run(ifx_Peak_Search_t* handle,
                         const ifx_Vector_R_t* data_set,
                         ifx_Peak_Search_Result_t* result)
{
    IFX_ERR_BRK_NULL(handle);
    IFX_VEC_BRK_VALID(data_set);
    IFX_ERR_BRK_NULL(result);

    // data_set length must be minimum 5 because -2/+2 neighbor checking
    if (vLen(data_set) < 5)
    {
        return;
    }

    reset_handle(handle);

    ifx_Float_t threshold = get_threshold(data_set,
                                          handle->threshold_factor,
                                          handle->threshold_offset);

    for (uint32_t n = 2; n < vLen(data_set) - 2; n++)
    {
        const ifx_Float_t fp = vAt(data_set, n);
        const ifx_Float_t fl = vAt(data_set, n - 1);
        const ifx_Float_t fl2 = vAt(data_set, n - 2);
        const ifx_Float_t fr = vAt(data_set, n + 1);
        const ifx_Float_t fr2 = vAt(data_set, n + 2);

        if (fp >= threshold && fp >= fl2 && fp >= fl && fp > fr && fp > fr2)
        {
            ifx_Float_t cur_value = n * handle->value_per_bin;

            if (cur_value >= handle->search_zone_start
                && cur_value <= handle->search_zone_end)
            {
                handle->peak_idx[handle->peak_count] = n;
                handle->peak_val[handle->peak_count] = fp;
                ++handle->peak_count;

                if (handle->peak_count >= handle->max_num_peaks)
                {
                    break;
                }
            }
        }
    }

    result->peak_count = handle->peak_count;
    result->index = handle->peak_idx;
}
