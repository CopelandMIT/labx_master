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

#include "MetricsFmcw.h"
#include "ifxBase/Math.h"

#include <cmath>

namespace {
inline uint32_t ceil_u32(float v)
{
    return static_cast<uint32_t>(std::ceil(v));
}

inline float get_sampling_bandwidth(const ifx_Fmcw_Sequence_Chirp_t& chirp)
{
    if (chirp.start_frequency_Hz > chirp.end_frequency_Hz)
    {
        return static_cast<float>(chirp.start_frequency_Hz - chirp.end_frequency_Hz);
    }
    else
    {
        return static_cast<float>(chirp.end_frequency_Hz - chirp.start_frequency_Hz);
    }
}
}  // namespace

void ifx_fmcw_metrics_from_sequence(const ifx_Fmcw_Sequence_Element_t* chirp_loop, ifx_Fmcw_Metrics_t* metrics)
{
    IFX_ERR_BRK_NULL(chirp_loop);
    IFX_ERR_BRK_ARGUMENT(chirp_loop->type != IFX_SEQ_LOOP);

    IFX_ERR_BRK_ARGUMENT((chirp_loop->loop.sub_sequence == nullptr) || (chirp_loop->loop.sub_sequence->type != IFX_SEQ_CHIRP));

    IFX_ERR_BRK_NULL(metrics);

    const auto& chirp = chirp_loop->loop.sub_sequence->chirp;
    const auto sampling_bandwidth_Hz = get_sampling_bandwidth(chirp);
    const float range_resolution_m = IFX_LIGHT_SPEED_MPS / (2.0f * sampling_bandwidth_Hz);
    const float max_range_m = chirp.num_samples * range_resolution_m / 2.0f;
    const auto center_frequency_Hz = (chirp.start_frequency_Hz + chirp.end_frequency_Hz) / 2.0;
    const auto max_speed_m_s = static_cast<float>(IFX_LIGHT_SPEED_MPS / (4.0 * center_frequency_Hz * chirp_loop->loop.repetition_time_s));
    const float speed_resolution_m_s = 2.0f * max_speed_m_s / chirp_loop->loop.num_repetitions;

    metrics->max_range_m = max_range_m;
    metrics->range_resolution_m = range_resolution_m;
    metrics->max_speed_m_s = max_speed_m_s;
    metrics->speed_resolution_m_s = speed_resolution_m_s;
    metrics->center_frequency_Hz = center_frequency_Hz;
}


void ifx_fmcw_sequence_from_metrics(const ifx_Fmcw_Metrics_t* metrics, bool round_to_power_of_2, ifx_Fmcw_Sequence_Element_t* chirp_loop)
{
    IFX_ERR_BRK_NULL(metrics);
    IFX_ERR_BRK_ARGUMENT(metrics->speed_resolution_m_s >= metrics->max_speed_m_s);

    IFX_ERR_BRK_NULL(chirp_loop);
    IFX_ERR_BRK_ARGUMENT(chirp_loop->type != IFX_SEQ_LOOP);

    auto* chirp = chirp_loop->loop.sub_sequence;
    IFX_ERR_BRK_ARGUMENT((chirp == nullptr) || (chirp->type != IFX_SEQ_CHIRP));

    auto num_samples_per_chirp = ceil_u32(2 * metrics->max_range_m / metrics->range_resolution_m);
    auto num_chirps_per_frame = ceil_u32(2 * metrics->max_speed_m_s / metrics->speed_resolution_m_s);
    if (round_to_power_of_2)
    {
        num_samples_per_chirp = ifx_math_round_up_power_of_2_uint32(num_samples_per_chirp);
        num_chirps_per_frame = ifx_math_round_up_power_of_2_uint32(num_chirps_per_frame);
    }

    const float sampling_bandwidth_Hz = IFX_LIGHT_SPEED_MPS / (2.0f * metrics->range_resolution_m);

    auto center_frequency_Hz = metrics->center_frequency_Hz;

    const auto chirp_repetition_time_s = static_cast<float>(IFX_LIGHT_SPEED_MPS / (4.0 * center_frequency_Hz * metrics->max_speed_m_s));

    const auto start_frequency_Hz = center_frequency_Hz - sampling_bandwidth_Hz / 2.0;
    const auto end_frequency_Hz = center_frequency_Hz + sampling_bandwidth_Hz / 2.0;

    chirp_loop->loop.num_repetitions = num_chirps_per_frame;
    chirp_loop->loop.repetition_time_s = chirp_repetition_time_s;

    chirp->chirp.start_frequency_Hz = start_frequency_Hz;
    chirp->chirp.end_frequency_Hz = end_frequency_Hz;
    chirp->chirp.num_samples = num_samples_per_chirp;
}
