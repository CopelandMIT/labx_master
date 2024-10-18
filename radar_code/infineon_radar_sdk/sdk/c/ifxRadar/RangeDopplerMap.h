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
 * \brief \copybrief gr_rdm
 *
 * For details refer to \ref gr_rdm
 */

#ifndef IFX_RADAR_RANGE_DOPPLER_MAP_H
#define IFX_RADAR_RANGE_DOPPLER_MAP_H

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
typedef struct ifx_RDM_s ifx_RDM_t;

/**
 * @brief Defines the structure for Range Doppler module related settings.
 */
typedef struct
{
    ifx_Float_t spect_threshold;             /**< Threshold is in always linear scale, should be greater than 1-e6.
                                                  Range spectrum output values below this are set to 1-e6 (-120dB).*/
    ifx_Math_Scale_Type_t output_scale_type; /**< Linear or dB scale for the output of range spectrum module.*/
    ifx_PPFFT_Config_t range_fft_config;     /**< Preprocessed FFT settings for range FFT e.g. mean removal, FFT settings.*/
    ifx_PPFFT_Config_t doppler_fft_config;   /**< Preprocessed FFT settings for Doppler FFT e.g. mean removal, FFT settings.*/
} ifx_RDM_Config_t;

/*
==============================================================================
   4. FUNCTION PROTOTYPES
==============================================================================
*/

/** @addtogroup gr_cat_Radar
 * @{
 */

/** @defgroup gr_rdm Range Doppler Map
 * @brief API for Range Doppler Map
 *
 * Range Doppler map is a 2D FFT transform on time domain data and the output of this
 * module is 2D matrix of real values (absolute magnitude of FFT transform). Here spectrum means,
 * real absolute amplitude values calculated from complex FFT output.
 * Range Doppler spectrum has two processing chains, one is over time domain chirp data (i.e. fast time) named as
 * range FFT computation, second processing chain is over all range FFT bins (i.e. slow time) named as Doppler FFT.
 * Afterwards, thresholding is applied on real 2D matrix.
 *
 * Block level signal processing modules looks like following;
 *
 * Raw Data => Range FFT => Matrix transpose => Doppler FFT => FFT Shift => Absolute => Thresholding => Scale conversion
 *
 * In order to compute range Doppler spectrum from real/complex IQ raw ADC input data, following signal processing
 * chain is applied;
 * - Range spectrum => mean removal (optional), windowing, zero padding, FFT transform
 * - Matrix transpose
 * - Doppler spectrum => mean removal (optional), windowing, zero padding, FFT transform
 * - Absolute amplitude spectrum on 2D matrix
 * - FFT shift to move DC bin to the center of the spectrum
 * - Thresholding (Values below threshold are clipped to threshold value)
 * - Scaling (to scale up or down the spectrum)
 *
 * Range Doppler spectrum output format:
 * - By default dB scale, Linear scale is also possible
 * - Rows of matrix: Range with 0 (first row) to Max (last row) of matrix. For real input, only positive half
 *   of spectrum is computed, thus range is only computed for positive half of spectrum
 * - Columns of matrix: Speed values are mapped with DC in center and positive half on right
 *   and negative on left
 *
 * If new device settings result into different samples per chirp or chirps per frame value, then old handle of Range Doppler
 * needs to be destroyed and create a new handle with updated values of samples per chirp and chirp per frame. Range FFT size and
 * Doppler FFT size cannot be less than samples per chirp and chirps per frame respectively and these FFT sizes should also not be
 * more than a factor of x4 i.e. with zero padding factor. For example if samples per chirp = 128, then valid range FFT size limits is
 * [128, 512], FFT larger than 512 is possible but will degrade the performance of range doppler result. Similarly if
 * chirps per frame = 64, then Doppler FFT size valid range is [64, 256].
 *
 * An algorithm explanation is also available at the \ref ssct_radarsdk_algorithms_rdm SDK documentation.
 *
 * @{
 */

/**
 * @brief Creates a range Doppler spectrum handle (object), based on the input parameters
 * for range spectrum and Doppler spectrum e.g.
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
 *  - Output of range Doppler spectrum module below this threshold is clipped to this threshold.
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
ifx_RDM_t* ifx_rdm_create(const ifx_RDM_Config_t* config);

/**
 * @brief Performs destruction of range Doppler spectrum handle (object) to clear internal states and memories
 *        used for range Doppler spectrum calculation.
 *
 * @param [in]     handle    A handle to the range Doppler processing object
 *
 */
IFX_DLL_PUBLIC
void ifx_rdm_destroy(ifx_RDM_t* handle);

/**
 * @brief Performs signal processing on a real input I or Q (e.g. mean removal, windowing, zero padding,
 *        FFT transform) and produces a real amplitude range Doppler spectrum as output.
 *
 * @param [in]     handle    A handle to the range Doppler processing object.
 * @param [in]     input     The real (i.e. either I or Q channel) time domain input data matrix,
 *                           with rows as chirps and columns as samples per chirp.
 * @param [out]    output    Output is always a real matrix containing absolute amplitude spectrum in linear or dB scale.
 *
 */
IFX_DLL_PUBLIC
void ifx_rdm_run_r(ifx_RDM_t* handle,
                   const ifx_Matrix_R_t* input,
                   ifx_Matrix_R_t* output);

/**
 * @brief Performs signal processing on a real input I or Q (e.g. mean removal, windowing, zero padding,
 *        FFT transform) and produces a complex amplitude range Doppler spectrum as output.
 *
 * @param [in]     handle    A handle to the range Doppler processing object.
 * @param [in]     input     The real (i.e. either I or Q channel) time domain input data matrix,
 *                           with rows as chirps and columns as samples per chirp.
 * @param [out]    output    Output is always a complex matrix containing absolute amplitude spectrum in linear or dB scale.
 *
 */
IFX_DLL_PUBLIC
void ifx_rdm_run_rc(ifx_RDM_t* handle,
                    const ifx_Matrix_R_t* input,
                    ifx_Matrix_C_t* output);
/**
 * @brief Performs signal processing on a complex input IQ (e.g. mean removal, windowing, zero padding,
 *        FFT transform) and produces a real amplitude range Doppler spectrum as output.
 *
 * @param [in]     handle    A handle to the range Doppler spectrum object.
 * @param [in]     input     The complex (i.e. both IQ channels) time domain input data matrix matrix,
 *                           with rows as chirps and columns as samples per chirp.
 * @param [out]    output    Output is always a complex matrix containing absolute amplitude spectrum in linear or dB scale.
 *
 */
IFX_DLL_PUBLIC
void ifx_rdm_run_c(ifx_RDM_t* handle,
                   const ifx_Matrix_C_t* input,
                   ifx_Matrix_C_t* output);

/**
 * @brief Performs signal processing on a complex input IQ (e.g. mean removal, windowing, zero padding,
 *        FFT transform) and produces a real amplitude range Doppler spectrum as output.
 *
 * @param [in]     handle    A handle to the range Doppler spectrum object
 * @param [in]     input     The complex (i.e. both IQ channels) time domain input data matrix matrix,
 *                           with rows as chirps and columns as samples per chirp.
 * @param [out]    output    Output is always a real matrix containing absolute amplitude spectrum in linear or dB scale.
 *
 */
IFX_DLL_PUBLIC
void ifx_rdm_run_cr(ifx_RDM_t* handle,
                    const ifx_Matrix_C_t* input,
                    ifx_Matrix_R_t* output);

/**
 * @brief Modifies the threshold value set within the range Doppler spectrum handle.
 *        Idea is to provide a runtime modification option to change threshold without destroy/create handle.
 *
 * @param [in]     handle    A handle to the range Doppler spectrum object.
 * @param [in]     threshold Threshold in linear scale. Range Doppler spectrum output values below this
 *                           are set to threshold value at the moment.
 */
IFX_DLL_PUBLIC
void ifx_rdm_set_threshold(ifx_RDM_t* handle,
                           ifx_Float_t threshold);

/**
 * @brief Returns the currently used threshold within range Doppler spectrum handle.
 *
 * @param [in]     handle    A handle to the range Doppler spectrum object.
 *
 * @return Current threshold value.
 *
 */
IFX_DLL_PUBLIC
ifx_Float_t ifx_rdm_get_threshold(const ifx_RDM_t* handle);

/**
 * @brief Configures at runtime, the range Doppler spectrum output to linear or dB scale in the handle.
 *
 * @param [in]     handle              A handle to the range Doppler spectrum object.
 * @param [in]     output_scale_type   Linear or dB are the possible options.
 *
 */
IFX_DLL_PUBLIC
void ifx_rdm_set_output_scale_type(ifx_RDM_t* handle,
                                   ifx_Math_Scale_Type_t output_scale_type);

/**
 * @brief Returns current range Doppler spectrum output scale i.e. linear or dB scale used within the handle.
 *
 * @param [in]     handle    A handle to the range Doppler spectrum object.
 *
 * @return Linear or dB are the possible options.
 *
 */
IFX_DLL_PUBLIC
ifx_Math_Scale_Type_t ifx_rdm_get_output_scale_type(const ifx_RDM_t* handle);

/**
 * @brief Facilitates to update range window used within range Doppler spectrum handle.
 *        For example, if range window type or its scale needs to be modified, one can update
 *        by passing the new window type or attenuation scale in window configuration
 *        structure defined by \ref ifx_Window_Config_t.
 *
 * @param [in]     config    Window settings with new gain value or window type but with same length
 *                           as passed during range Doppler handle creation.
 * @param [in]     handle    A handle to the range Doppler spectrum object.
 *
 */
IFX_DLL_PUBLIC
void ifx_rdm_set_range_window(const ifx_Window_Config_t* config,
                              ifx_RDM_t* handle);

/**
 * @brief Facilitates to update Doppler window used within range Doppler spectrum handle.
 *        For example, if Doppler window type or its scale needs to be modified, one can update
 *        by passing the new window type or attenuation scale in window configuration
 *        structure defined by \ref ifx_Window_Config_t.
 *
 * @param [in]     config    Window settings with new gain value or window type but with same length
 *                           as passed during range Doppler handle creation.
 * @param [in]     handle    A handle to the range Doppler spectrum object.
 *
 */
IFX_DLL_PUBLIC
void ifx_rdm_set_doppler_window(const ifx_Window_Config_t* config,
                                ifx_RDM_t* handle);

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* IFX_RADAR_RANGE_DOPPLER_MAP_H */
