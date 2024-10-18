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
 * @file RangeDopplerMap.h
 *
 * \brief \copybrief gr_doppler_spect
 *
 * For details refer to \ref gr_doppler_spect
 */

#ifndef IFX_RADAR_DOPPLER_SPECTROGRAM_H
#define IFX_RADAR_DOPPLER_SPECTROGRAM_H

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include "ifxAlgo/PreprocessedFFT.h"

#include "ifxBase/Matrix.h"
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

/**
 * @brief A handle for an instance of Range Doppler Map module, see RangeDopplerMap.h.
 */
typedef struct ifx_DopplerSpectrogram_s ifx_DopplerSpectrogram_t;

/**
 * @brief Defines the structure for Range Doppler module related settings.
 */
typedef struct
{
    uint32_t num_samples;                  /**< Samples per chirp for FMCW radars, for Doppler radars its samples per frame */
    uint32_t chirps_per_frame;             /**< For doppler radars, this shall be 1 */
    ifx_Float_t spect_threshold;           /**< Threshold is in always linear scale, should be greater than 1-e6.
                                                Range spectrum output values below this are set to 1-e6 (-120dB).*/
    ifx_PPFFT_Config_t doppler_fft_config; /**< Preprocessed FFT settings for Doppler FFT e.g. mean removal, FFT settings.*/
} ifx_DopplerSpectrogram_Config_t;

/*
==============================================================================
   4. FUNCTION PROTOTYPES
==============================================================================
*/

/** @addtogroup gr_cat_Radar
 * @{
 */

/** @defgroup gr_doppler_spect Range Doppler Map
 * @brief API for Doppler Spectrogram
 *
 * It is a 2D FFT transform on time domain data for FMCW radars and 1D FFT for Doppler radars. The output of this
 * module is 2D matrix of real values (absolute magnitude of FFT transform over time of 100 frames). Here spectrum means,
 * real absolute amplitude values calculated from complex FFT output.
 * Doppler Spectrogram has two processing chains for FMCW radars, one is over time domain chirp data (i.e. fast time) named as
 * range FFT computation, second processing chain is over the range FFT bin with maximum value of absolute spectrum (i.e. slow time) named as Doppler FFT.
 * Afterwards, threshold is applied on real Vector and placed in a 2D matrix according to the time scale i.e. most recent frame as first row.
 *
 * Block level signal processing modules for FMCW radars looks like following;
 *
 * Raw Data Matrix => Range FFT for all chirps => Doppler FFT over maximum magnitude range bin => FFT Shift => Absolute => Threshold => Scale conversion
 *
 * Block level signal processing modules for Doppler radars looks like following;
 *
 * Raw Data Vector => Doppler FFT only => FFT Shift => Absolute => Threshold => Scale conversion
 *
 * @{
 */

/**
 * @brief Creates a Doppler spectrogram handle (object), based on the input parameters
 * for range spectrum and Doppler spectrum. Module will ignore Range Spectrum parameters for Doppler radars as
 * range spectrum is not calculated for Doppler radars.
 * Range spectrum:
 *  - samples per chirp (defines zero padding for FFT)
 *  - mean removal flag
 *  - window type, length and attenuation
 *  - FFT type (\ref ifx_FFT_Type_t)
 *  - FFT size
 * Doppler spectrum:
 *  - number of chirps (defines zero padding for FFT)
 *  - mean removal flag
 *  - window type, length and attenuation
 *  - FFT type (always IFX_FFT_TYPE_C2C)
 *  - FFT size
 * Threshold:
 *  - Output of Doppler spectrogram module below this threshold is clipped to this threshold.
 * Scale type:
 *  - convert output to dB scale or keep linear scale.
 *
 * @param [in]     config    Contains configuration for range/Doppler FFT, e.g. mean removal,
 *                           window settings, FFT type and size.
 *
 * @return Handle to the newly created instance or NULL in case of failure.
 *
 */
IFX_DLL_PUBLIC
ifx_DopplerSpectrogram_t* ifx_doppler_spectrogram_create(const ifx_DopplerSpectrogram_Config_t* config);

/**
 * @brief Performs destruction of Doppler spectrogram handle (object) to clear internal states and memories
 *        used for Doppler spectrogram calculation.
 *
 * @param [in]     handle    A handle to the doppler spectrogram processing object
 *
 */
IFX_DLL_PUBLIC
void ifx_doppler_spectrogram_destroy(ifx_DopplerSpectrogram_t* handle);

/**
 * @brief Performs signal processing on a real input I or Q (e.g. mean removal, windowing, zero padding,
 *        FFT transform) and produces a real amplitude Doppler spectrogram as output.
 *
 * @param [in]     handle    A handle to the doppler spectrogram processing object.
 * @param [in]     input     The real (i.e. either I or Q channel) time domain input data matrix,
 *                           with rows as chirps and columns as samples per chirp.
 * @param [out]    output    Output is always a real matrix containing absolute amplitude spectrum in linear or dB scale.
 *
 */
IFX_DLL_PUBLIC
void ifx_doppler_spectrogram_run_r(ifx_DopplerSpectrogram_t* handle, const ifx_Matrix_R_t* input, ifx_Matrix_R_t* output);

/**
 * @brief Performs signal processing on a complex input IQ (e.g. mean removal, windowing, zero padding,
 *        FFT transform) and produces a real amplitude Doppler spectrogram as output.
 *
 * @param [in]     handle    A handle to the doppler spectrogram processing object
 * @param [in]     input     The complex (i.e. both IQ channels) time domain input data vector
 * @param [out]    output    Output is always a real matrix containing absolute amplitude spectrum in linear or dB scale.
 *
 */
IFX_DLL_PUBLIC
void ifx_doppler_spectrogram_run_cr(ifx_DopplerSpectrogram_t* handle, const ifx_Vector_C_t* input, ifx_Matrix_R_t* output);

/**
 * @brief Modifies the threshold value set within the Doppler spectrogram handle.
 *        Idea is to provide a runtime modification option to change threshold without destroy/create handle.
 *
 * @param [in]     handle    A handle to the doppler spectrogram processing object.
 * @param [in]     threshold Threshold in linear scale. Doppler spectrogram output values below this
 *                           are set to threshold value at the moment.
 */
IFX_DLL_PUBLIC
void ifx_doppler_spectrogram_set_threshold(ifx_DopplerSpectrogram_t* handle, ifx_Float_t threshold);

/**
 * @brief Returns the currently used threshold within Doppler spectrogram handle.
 *
 * @param [in]     handle    A handle to the doppler spectrogram processing object.
 *
 * @return Current threshold value.
 *
 */
IFX_DLL_PUBLIC
ifx_Float_t ifx_doppler_spectrogram_get_threshold(const ifx_DopplerSpectrogram_t* handle);

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* IFX_RADAR_DOPPLER_SPECTROGRAM_H */
