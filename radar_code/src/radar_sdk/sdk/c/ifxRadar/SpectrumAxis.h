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

/**
 * @file SpectrumAxis.h
 *
 * \brief \copybrief gr_sa
 *
 * For details refer to \ref gr_sa
 */

#ifndef IFX_RADAR_SPECTRUM_AXIS_H
#define IFX_RADAR_SPECTRUM_AXIS_H

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include "ifxAlgo/FFT.h"
#include "ifxBase/Math.h"
#include "ifxBase/Types.h"


#ifdef __cplusplus
extern "C"
{
#endif


/*
==============================================================================
   2. DEFINITIONS
==============================================================================
*/

/*
==============================================================================
   3. TYPES
==============================================================================
*/

/*
==============================================================================
   4. FUNCTION PROTOTYPES
==============================================================================
*/

/** @addtogroup gr_cat_Radar
 * @{
 */

/** @defgroup gr_sa Spectrum Axis
 * @brief API for Spectrum Axis used by Range spectrum, Range Doppler and Range Angle Map modules
 *
 * @{
 */


/**
 * @brief For FMCW radars, this method calculates distance or range in meters per FFT bin.
 *
 * For real or complex input FFT spectrum of FMCW modulated radar data, range axis is calculated based on
 * up-chirp bandwidth, samples per chirp and FFT size.
 *
 * @param [in]     fft_size    			Size of FFT in powers of 2 and less than 65536
 * @param [in]     samples_per_chirp     Length of input data used by FFT calculation
 * @param [in]     bandwidth_Hz			Chirp bandwidth in Hz
 *
 * @return   Return distance in meters per FFT spectrum bin. In case of an error, a zero is returned.
 *
 */
IFX_DLL_PUBLIC
ifx_Float_t ifx_spectrum_axis_calc_dist_per_bin(uint32_t fft_size,
                                                uint32_t samples_per_chirp,
                                                ifx_Float_t bandwidth_Hz);

/**
 * @brief For FMCW or Doppler radars, this method calculates speed per FFT bin.
 *
 * For real or complex input FFT spectrum of FMCW modulated radar data, range axis is calculated based on
 * center_rf_freq_Hz, up chirp time (for FMCW) or pulse repetition time (reciprocal of sampling frequency for Doppler modulation) and FFT size.
 *
 * @param [in]     fft_size    			Size of FFT in powers of 2 and less than 65536
 * @param [in]     center_freq_Hz        RF center frequency in Hz
 * @param [in]     pulse_repet_time_s	Up chirp time for FMCW modulation or reciprocal of sampling frequency in case
 *										of Doppler modulation
 *
 * @return   Return speed per FFT spectrum bin in meters per second. In case of an error, a zero is returned.
 *
 */
IFX_DLL_PUBLIC
ifx_Float_t ifx_spectrum_axis_calc_speed_per_bin(uint32_t fft_size,
                                                 ifx_Float_t center_freq_Hz,
                                                 ifx_Float_t pulse_repet_time_s);

/**
 * @brief For FMCW radars, this method calculates beat frequency per FFT bin.
 *
 * For real or complex input FFT spectrum of FMCW modulated radar data, range axis is calculated based on
 * up-chirp bandwidth, samples per chirp, up-chirp time and FFT size.
 *
 * @param [in]     fft_size    			Size of FFT in powers of 2 and less than 65536
 * @param [in]     samples_per_chirp     Length of input data used by FFT calculation
 * @param [in]     bandwidth_Hz			Up chirp bandwidth in Hz
 * @param [in]     chirptime_s			Up chirp time in seconds
 *
 * @return   Return beat frequency per FFT spectrum bin in Hz. In case of an error, a zero is returned.
 *
 */
IFX_DLL_PUBLIC
ifx_Float_t ifx_spectrum_axis_calc_beat_freq_per_bin(uint32_t fft_size,
                                                     uint32_t samples_per_chirp,
                                                     ifx_Float_t bandwidth_Hz,
                                                     ifx_Float_t chirptime_s);

/**
 * @brief For FMCW radars, this method calculates axis of spectrum as range in meters.
 *
 * For real or complex input FFT spectrum of FMCW modulated radar data, range axis is calculated based on
 * up-chirp bandwidth, samples per chirp and FFT size.
 *
 * @param [in]     fft_type    			Defined by \ref ifx_FFT_Type_t for real or complex input for FFT
 * @param [in]     fft_size    			Size of FFT in powers of 2 and less than 65536
 * @param [in]     samples_per_chirp    Length of input data used by FFT calculation
 * @param [in]     bandwidth_Hz			Chirp bandwidth in Hz
 *
 * @return   Return an axis struct defined by \ref ifx_Math_Axis_Spec_t. It contains minimum and maximum values along with step size.
 * In case of an error, this struct contains zeros for all fields.
 *
 */
IFX_DLL_PUBLIC
ifx_Math_Axis_Spec_t ifx_spectrum_axis_calc_range_axis(ifx_FFT_Type_t fft_type,
                                                       uint32_t fft_size,
                                                       uint32_t samples_per_chirp,
                                                       ifx_Float_t bandwidth_Hz);

/**
 * @brief For FMCW or pure Doppler radars, this method calculates axis of spectrum as speed in meters/sec.
 *
 * For real or complex input FFT spectrum of FMCW modulated radar data, speed axis is calculated based on
 * pulse repetition time, RF center frequency and FFT size.
 *
 * @param [in]     fft_type    			Defined by \ref ifx_FFT_Type_t for real or complex input for FFT
 * @param [in]     fft_size    			Size of FFT in powers of 2 and less than 65536
 * @param [in]     center_rf_freq_Hz    RF center frequency in Hz
 * @param [in]     pulse_repet_time_s	Pulse repetition time is (1 / Sampling Freq) for Doppler modulation.
 *										For FMCW, it is up chirp time
 *
 * @return    Return an axis struct defined by \ref ifx_Math_Axis_Spec_t. It contains minimum and maximum values along with step size.
 * In case of an error, this struct contains zeros for all fields.
 *
 */
IFX_DLL_PUBLIC
ifx_Math_Axis_Spec_t ifx_spectrum_axis_calc_speed_axis(ifx_FFT_Type_t fft_type,
                                                       uint32_t fft_size,
                                                       ifx_Float_t center_rf_freq_Hz,
                                                       ifx_Float_t pulse_repet_time_s);

/**
 * @brief For FMCW or pure Doppler radars, this method calculates axis of spectrum as sampling frequency in Hz.
 *
 * For real or complex input FFT spectrum of FMCW modulated radar data, speed axis is calculated based on
 * sampling frequency and FFT size.
 *
 * @param [in]     fft_type    			Defined by \ref ifx_FFT_Type_t for real or complex input for FFT
 * @param [in]     fft_size    			Size of FFT in powers of 2 and less than 65536
 * @param [in]     sampling_freq_Hz     Sampling frequency in Hz
 *
 * @return    Return an axis struct defined by \ref ifx_Math_Axis_Spec_t. It contains minimum and maximum values along with step size.
 * In case of an error, this struct contains zeros for all fields.
 *
 */
IFX_DLL_PUBLIC
ifx_Math_Axis_Spec_t ifx_spectrum_axis_calc_sampling_freq_axis(ifx_FFT_Type_t fft_type,
                                                               uint32_t fft_size,
                                                               ifx_Float_t sampling_freq_Hz);

/**
 * @brief For FMCW radars, this method calculates axis of spectrum as beat frequency in Hz.
 *
 * For real or complex input FFT spectrum of FMCW modulated radar data, range axis is calculated based on
 * up-chirp bandwidth, samples per chirp, FFT size and pulse repetition time.
 *
 * @param [in]     fft_type    			Defined by \ref ifx_FFT_Type_t for real or complex input for FFT
 * @param [in]     fft_size    			Size of FFT in powers of 2 and less than 65536
 * @param [in]     samples_per_chirp    Length of input data used by FFT calculation
 * @param [in]     bandwidth_Hz			Up-chirp bandwidth in Hz
 * @param [in]     chirptime_s			Up-chirp time in seconds
 *
 * @return    Return an axis struct defined by \ref ifx_Math_Axis_Spec_t. It contains minimum and maximum values along with step size.
 * In case of an error, this struct contains zeros for all fields.
 *
 */
IFX_DLL_PUBLIC
ifx_Math_Axis_Spec_t ifx_spectrum_axis_calc_beat_freq_axis(ifx_FFT_Type_t fft_type,
                                                           uint32_t fft_size,
                                                           uint32_t samples_per_chirp,
                                                           ifx_Float_t bandwidth_Hz,
                                                           ifx_Float_t chirptime_s);

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* IFX_RADAR_SPECTRUM_AXIS_H */
