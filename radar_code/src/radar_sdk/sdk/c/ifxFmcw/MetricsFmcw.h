/* ===========================================================================
** Copyright (C) 2022 Infineon Technologies AG
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
 * @file MetricsFmcw.h
 *
 * \brief \copybrief gr_metrics_fmcw
 *
 * For details refer to \ref gr_metrics_fmcw
 */

#ifndef IFX_METRICS_FMCW_H
#define IFX_METRICS_FMCW_H

#include "DeviceFmcwTypes.h"
#include "ifxBase/Types.h"

#ifdef __cplusplus
extern "C"
{
#endif
/** @addtogroup gr_cat_Fmcw
 * @{
 */

/** @defgroup gr_metrics_fmcw MetricsFmcw
 * @brief API for MetricsFmcw
 *
 * The metrics provide a very easy way to configure the radar sensor in terms
 * of maximum range, maximum speed, range resolution, and speed
 * resolution that one wants to achieve. The metrics parameters in \ref ifx_Fmcw_Metrics_t can be converted
 * into a Fmcw device sequence \ref ifx_Fmcw_Sequence_Element_t and vice versa.
 *
 * @{
 */

typedef struct
{
    float range_resolution_m;   /**< The range resolution is the distance between two consecutive
                                     bins of the range transform. Note that even though zero
                                     padding before the range transform seems to increase this
                                     resolution, the true resolution does not change but depends
                                     only from the acquisition parameters. Zero padding is just
                                     interpolation! */
    float max_range_m;          /**< The bins of the range transform represent the range
                                     between 0m and this value. (If the time domain input data it
                                     is the range-max_range_m ... max_range_m.) */
    float max_speed_m_s;        /**< The bins of the Doppler transform represent the speed values
                                     between -max_speed_m_s and max_speed_m_s. */
    float speed_resolution_m_s; /**< The speed resolution is the distance between two consecutive
                                     bins of the Doppler transform. Note that even though zero
                                     padding before the speed transform seems to increase this
                                     resolution, the true resolution does not change but depends
                                     only from the acquisition parameters. Zero padding is just
                                     interpolation! */

    double center_frequency_Hz; /**< Center frequency of the FMCW chirp. If the value is set to 0
                                     the center of the RF band (middle of minimum and maximum
                                     supported RF frequency) will be used as center frequency. */
} ifx_Fmcw_Metrics_t;

/**
 * @brief Determines metrics parameters from Fmcw device sequence
 *
 * This function extracts metrics parameters from the provided chirp loop sequence.
 *
 * @param [in]   chirp_loop  Pointer to Fmcw device chirp_loop element
 * @param [out]  metrics     Pointer to Metrics structure
 */
IFX_DLL_PUBLIC
void ifx_fmcw_metrics_from_sequence(const ifx_Fmcw_Sequence_Element_t* chirp_loop, ifx_Fmcw_Metrics_t* metrics);

/**
 * @brief Determines chirp loop sequence out of metric parameters
 *
 * If round_to_power_of_2 is true, the computed values for
 * num_samples_per_chirp and num_chirps_per_frame will be rounded to the next
 * power of 2.
 *
 * This function is not filling the following parameters of device config structure and caller of this function shall
 * fill these values from existing device config.
 * - sample_rate_Hz
 * - tx_mask
 * - rx_mask
 * - tx_power_level
 * - lp_cutoff_Hz
 * - hp_cutoff_Hz
 * - if_gain_dB
 *
 * @param [in]  metrics                 Metrics structure
 * @param [in]  round_to_power_of_2     If true num_samples_per_chirp and
                                        num_chirps_per_frame are rounded to
                                        the next power of 2.
 * @param [out] chirp_loop              pointer to a loop sequence that
                                        contains a chirp as a subsequence
 */
IFX_DLL_PUBLIC
void ifx_fmcw_sequence_from_metrics(const ifx_Fmcw_Metrics_t* metrics, bool round_to_power_of_2, ifx_Fmcw_Sequence_Element_t* chirp_loop);

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* IFX_METRICS_FMCW_H */
