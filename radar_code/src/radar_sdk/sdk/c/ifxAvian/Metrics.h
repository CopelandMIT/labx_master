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
 * @file Metrics.h
 *
 * \brief \copybrief gr_avian_metrics
 *
 * For details refer to \ref gr_avian_metrics
 */

#ifndef IFX_AVIAN_METRICS_H
#define IFX_AVIAN_METRICS_H

#include "DeviceControl.h"
#include "ifxFmcw/MetricsFmcw.h"


#ifdef __cplusplus
extern "C"
{
#endif


/** @addtogroup gr_cat_Avian
 * @{
 */

/** @defgroup gr_avian_metrics Metrics
 * @brief API for Metrics
 *
 * The metrics provide a very easy way to configure the radar sensor in terms
 * of maximum range, maximum speed, range resolution, and speed
 * resolution that one wants to achieve. The metrics parameters in \ref ifx_Avian_Metrics_t can be converted
 * into a Avian device configuration \ref ifx_Avian_Config_t and vice versa.
 *
 * @{
 */

typedef ifx_Fmcw_Metrics_t ifx_Avian_Metrics_t;

/**
 * @brief Determines metrics parameters from Avian device configuration
 *
 * The function converts the radar configuration in config to a metrics
 * structure.
 *
 * @param [in]   avian       Avian device handle
 * @param [in]   config      Avian device configuration structure
 * @param [out]  metrics     Metrics structure
 */
IFX_DLL_PUBLIC
void ifx_avian_metrics_from_config(const ifx_Avian_Device_t* avian, const ifx_Avian_Config_t* config, ifx_Avian_Metrics_t* metrics);

/**
 * @brief Translates metrics parameters to Avian device configuration
 *
 * The function converts the metrics parameters in metrics to an Avian device
 * configuration. If round_to_power_of_2 is true, the computed values for
 * num_samples_per_chirp and num_chirps_per_frame will be rounded to the next
 * power of 2.
 *
 * Changes:
 * * Radar SDK 3.4: If metrics.center_frequency_Hz is set to 0 the center of the RF
 *   band (the middle of the minimum and maximum supported RF frequency) is used as
 *   center frequency. Prior to Radar SDK 3.4 61.25GHz the middle of the ISM band
 *   was used if for bandwidths of less than 500MHz and the center of the RF band
 *   otherwise.
 * This function is not filling the following parameters of device config structure and caller of this function shall
 * fill these values from existing device config.
 * - sample_rate_Hz
 * - tx_mask
 * - rx_mask
 * - tx_power_level
 * - lp_cutoff_Hz
 * - hp_cutoff_Hz
 * - if_gain_dB
 * - mimo_mode
 *
 * @param [in]  avian                   Avian device handle
 * @param [in]  metrics                 Metrics structure
 * @param [in]  round_to_power_of_2     If true num_samples_per_chirp and
                                        num_chirps_per_frame are rounded to
                                        the next power of 2.
 * @param [out] config                  Avian device configuration structure
 */
IFX_DLL_PUBLIC
void ifx_avian_metrics_to_config(const ifx_Avian_Device_t* avian, const ifx_Avian_Metrics_t* metrics, bool round_to_power_of_2,
                                 ifx_Avian_Config_t* config);

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* IFX_AVIAN_METRICS_H */
