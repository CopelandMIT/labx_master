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

#include <cstdlib>
#include <cstring>

#include "ifxBase/Error.h"
#include "ifxRadar/SpectrumAxis.h"

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
ifx_Float_t ifx_spectrum_axis_calc_dist_per_bin(const uint32_t fft_size,
                                                const uint32_t samples_per_chirp,
                                                const ifx_Float_t bandwidth_Hz)
{
    if (bandwidth_Hz <= 0 || fft_size == 0 || samples_per_chirp == 0)
    {
        ifx_error_set(IFX_ERROR_ARGUMENT_OUT_OF_BOUNDS);
        return 0;
    }


    ifx_Float_t dist_per_bin = IFX_LIGHT_SPEED_MPS / (2 * bandwidth_Hz * static_cast<ifx_Float_t>(fft_size) / samples_per_chirp);

    return dist_per_bin;
}

//----------------------------------------------------------------------------

ifx_Float_t ifx_spectrum_axis_calc_speed_per_bin(const uint32_t fft_size,
                                                 const ifx_Float_t center_freq_Hz,
                                                 const ifx_Float_t pulse_repet_time_s)
{
    if (center_freq_Hz <= 0 || fft_size == 0 || pulse_repet_time_s <= 0)
    {
        ifx_error_set(IFX_ERROR_ARGUMENT_OUT_OF_BOUNDS);
        return 0;
    }

    ifx_Float_t fd_max = 1.0f / (2 * pulse_repet_time_s);

    ifx_Float_t hz_to_m_per_s = (IFX_LIGHT_SPEED_MPS / center_freq_Hz) / 2;

    return ((fd_max / (static_cast<ifx_Float_t>(fft_size) / 2)) * hz_to_m_per_s);
}

//----------------------------------------------------------------------------

ifx_Float_t ifx_spectrum_axis_calc_beat_freq_per_bin(const uint32_t fft_size,
                                                     const uint32_t samples_per_chirp,
                                                     const ifx_Float_t bandwidth_Hz,
                                                     const ifx_Float_t chirptime_s)
{
    if (bandwidth_Hz <= 0 || fft_size == 0 || chirptime_s <= 0 || samples_per_chirp == 0)
    {
        ifx_error_set(IFX_ERROR_ARGUMENT_OUT_OF_BOUNDS);
        return 0;
    }

    ifx_Float_t dist_per_bin = ifx_spectrum_axis_calc_dist_per_bin(fft_size, samples_per_chirp, bandwidth_Hz);

    ifx_Float_t beat_freq_per_meter = (bandwidth_Hz / chirptime_s) * (2.0f / IFX_LIGHT_SPEED_MPS);

    return (beat_freq_per_meter * dist_per_bin);
}

//----------------------------------------------------------------------------

ifx_Math_Axis_Spec_t ifx_spectrum_axis_calc_range_axis(const ifx_FFT_Type_t fft_type,
                                                       const uint32_t fft_size,
                                                       const uint32_t samples_per_chirp,
                                                       const ifx_Float_t bandwidth_Hz)
{
    ifx_Math_Axis_Spec_t range_axis_m = {0, 0, 0};

    IFX_ERR_BRV_COND(fft_size == 0, IFX_ERROR_ARGUMENT_OUT_OF_BOUNDS, range_axis_m);
    IFX_ERR_BRV_COND(samples_per_chirp == 0, IFX_ERROR_ARGUMENT_OUT_OF_BOUNDS, range_axis_m);
    IFX_ERR_BRV_COND(bandwidth_Hz <= 0, IFX_ERROR_ARGUMENT_OUT_OF_BOUNDS, range_axis_m);

    ifx_Float_t dist_per_bin = ifx_spectrum_axis_calc_dist_per_bin(fft_size, samples_per_chirp, bandwidth_Hz);
    uint32_t fft_bins = (fft_type == IFX_FFT_TYPE_R2C) ? (fft_size / 2 - 1) : fft_size - 1;

    range_axis_m.min_value = 0;
    range_axis_m.max_value = dist_per_bin * fft_bins;
    range_axis_m.value_bin_per_step = dist_per_bin;

    return range_axis_m;
}

//----------------------------------------------------------------------------

ifx_Math_Axis_Spec_t ifx_spectrum_axis_calc_speed_axis(const ifx_FFT_Type_t fft_type,
                                                       const uint32_t fft_size,
                                                       const ifx_Float_t center_rf_freq_Hz,
                                                       const ifx_Float_t pulse_repet_time_s)
{
    ifx_Math_Axis_Spec_t speed_axis_mps = {0, 0, 0};

    IFX_ERR_BRV_COND(fft_size == 0, IFX_ERROR_ARGUMENT_OUT_OF_BOUNDS, speed_axis_mps);
    IFX_ERR_BRV_COND(center_rf_freq_Hz <= 0, IFX_ERROR_ARGUMENT_OUT_OF_BOUNDS, speed_axis_mps);
    IFX_ERR_BRV_COND(pulse_repet_time_s <= 0, IFX_ERROR_ARGUMENT_OUT_OF_BOUNDS, speed_axis_mps);

    ifx_Float_t speed_per_bin = ifx_spectrum_axis_calc_speed_per_bin(fft_size, center_rf_freq_Hz, pulse_repet_time_s);

    speed_axis_mps.min_value = (fft_type == IFX_FFT_TYPE_R2C) ? 0 : (-speed_per_bin * (static_cast<ifx_Float_t>(fft_size) / 2));
    speed_axis_mps.max_value = speed_per_bin * (static_cast<ifx_Float_t>(fft_size) / 2 - 1);
    speed_axis_mps.value_bin_per_step = speed_per_bin;

    return speed_axis_mps;
}

//----------------------------------------------------------------------------

ifx_Math_Axis_Spec_t ifx_spectrum_axis_calc_sampling_freq_axis(const ifx_FFT_Type_t fft_type,
                                                               const uint32_t fft_size,
                                                               const ifx_Float_t sampling_freq_Hz)
{
    ifx_Math_Axis_Spec_t freq_axis_Hz = {0, 0, 0};

    IFX_ERR_BRV_COND(fft_size == 0, IFX_ERROR_ARGUMENT_OUT_OF_BOUNDS, freq_axis_Hz);
    IFX_ERR_BRV_COND(sampling_freq_Hz <= 0, IFX_ERROR_ARGUMENT_OUT_OF_BOUNDS, freq_axis_Hz);

    ifx_Float_t freq_per_bin = sampling_freq_Hz / fft_size;

    freq_axis_Hz.min_value = (fft_type == IFX_FFT_TYPE_R2C) ? 0 : (-freq_per_bin * fft_size / 2);
    freq_axis_Hz.max_value = freq_per_bin * (static_cast<ifx_Float_t>(fft_size) / 2 - 1);
    freq_axis_Hz.value_bin_per_step = freq_per_bin;

    return freq_axis_Hz;
}

//----------------------------------------------------------------------------

ifx_Math_Axis_Spec_t ifx_spectrum_axis_calc_beat_freq_axis(const ifx_FFT_Type_t fft_type,
                                                           const uint32_t fft_size,
                                                           const uint32_t samples_per_chirp,
                                                           const ifx_Float_t bandwidth_Hz,
                                                           const ifx_Float_t chirptime_s)
{
    ifx_Math_Axis_Spec_t beat_freq_axis_Hz = {0, 0, 0};

    IFX_ERR_BRV_COND(fft_size == 0, IFX_ERROR_ARGUMENT_OUT_OF_BOUNDS, beat_freq_axis_Hz);
    IFX_ERR_BRV_COND(samples_per_chirp == 0, IFX_ERROR_ARGUMENT_OUT_OF_BOUNDS, beat_freq_axis_Hz);
    IFX_ERR_BRV_COND(bandwidth_Hz <= 0, IFX_ERROR_ARGUMENT_OUT_OF_BOUNDS, beat_freq_axis_Hz);
    IFX_ERR_BRV_COND(chirptime_s <= 0, IFX_ERROR_ARGUMENT_OUT_OF_BOUNDS, beat_freq_axis_Hz);

    ifx_Float_t beat_freq_per_bin = ifx_spectrum_axis_calc_beat_freq_per_bin(fft_size, samples_per_chirp, bandwidth_Hz, chirptime_s);

    uint32_t fft_bins = (fft_type == IFX_FFT_TYPE_R2C) ? (fft_size / 2 - 1) : fft_size - 1;

    beat_freq_axis_Hz.min_value = 0;
    beat_freq_axis_Hz.max_value = beat_freq_per_bin * fft_bins;
    beat_freq_axis_Hz.value_bin_per_step = beat_freq_per_bin;

    return beat_freq_axis_Hz;
}
