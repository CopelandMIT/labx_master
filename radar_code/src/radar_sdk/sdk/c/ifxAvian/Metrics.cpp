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

#include "Metrics.h"


void ifx_avian_metrics_from_config(const ifx_Avian_Device_t* avian, const ifx_Avian_Config_t* config, ifx_Avian_Metrics_t* metrics)
{
    IFX_ERR_BRK_NULL(avian)
    IFX_ERR_BRK_NULL(config);
    IFX_ERR_BRK_NULL(metrics);

    ifx_Fmcw_Sequence_Element_t chirp_element;
    chirp_element.type = IFX_SEQ_CHIRP;
    chirp_element.next_element = nullptr;

    ifx_Fmcw_Sequence_Element_t chirp_loop_element;
    chirp_loop_element.type = IFX_SEQ_LOOP;
    chirp_loop_element.next_element = nullptr;

    auto& chirp_loop = chirp_loop_element.loop;
    chirp_loop.sub_sequence = &chirp_element;
    chirp_loop.num_repetitions = config->num_chirps_per_frame;
    chirp_loop.repetition_time_s = config->chirp_repetition_time_s;

    auto& chirp = chirp_element.chirp;
    chirp.sample_rate_Hz = static_cast<float>(config->sample_rate_Hz);
    chirp.rx_mask = config->rx_mask;
    chirp.tx_mask = config->tx_mask;
    chirp.tx_power_level = config->tx_power_level;
    chirp.if_gain_dB = config->if_gain_dB;
    chirp.start_frequency_Hz = static_cast<double>(config->start_frequency_Hz);
    chirp.end_frequency_Hz = static_cast<double>(config->end_frequency_Hz);
    chirp.num_samples = config->num_samples_per_chirp;
    chirp.hp_cutoff_Hz = config->hp_cutoff_Hz;
    chirp.lp_cutoff_Hz = config->aaf_cutoff_Hz;

    ifx_fmcw_metrics_from_sequence(&chirp_loop_element, metrics);
}

void ifx_avian_metrics_to_config(const ifx_Avian_Device_t* avian, const ifx_Avian_Metrics_t* metrics, bool round_to_power_of_2,
                                 ifx_Avian_Config_t* config)
{
    IFX_ERR_BRK_NULL(avian);
    IFX_ERR_BRK_NULL(metrics);
    IFX_ERR_BRK_NULL(config);
    IFX_ERR_BRK_BIGGER_INCL(metrics->speed_resolution_m_s, metrics->max_speed_m_s);

    ifx_Fmcw_Sequence_Element_t chirp_element;
    chirp_element.type = IFX_SEQ_CHIRP;
    chirp_element.next_element = nullptr;

    ifx_Fmcw_Sequence_Element_t chirp_loop_element;
    chirp_loop_element.type = IFX_SEQ_LOOP;
    chirp_loop_element.next_element = nullptr;
    chirp_loop_element.loop.sub_sequence = &chirp_element;

    ifx_fmcw_sequence_from_metrics(metrics, round_to_power_of_2, &chirp_loop_element);

    const auto& chirp_loop = chirp_loop_element.loop;
    const auto& chirp = chirp_element.chirp;

    config->start_frequency_Hz = static_cast<uint64_t>(chirp.start_frequency_Hz);
    config->end_frequency_Hz = static_cast<uint64_t>(chirp.end_frequency_Hz);
    config->num_samples_per_chirp = chirp.num_samples;

    config->chirp_repetition_time_s = chirp_loop.repetition_time_s;
    config->num_chirps_per_frame = chirp_loop.num_repetitions;
}
