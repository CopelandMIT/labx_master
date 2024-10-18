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
 * @file Signal.h
 *
 * \brief \copybrief gr_signal
 *
 * For details refer to \ref gr_signal
 */

#ifndef IFX_ALGO_FILTER_H
#define IFX_ALGO_FILTER_H

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include "ifxBase/Matrix.h"
#include "ifxBase/Types.h"
#include "ifxBase/Vector.h"


#ifdef __cplusplus
extern "C"
{
#endif


/** @addtogroup gr_cat_Algorithms
 * @{
 */

/** @defgroup gr_signal Signal Processing
 * @brief API for digital filtering operations.
 *
 * Supports mathematical and other operations such as creation and destruction of
 * filter objects, filtering operation and clearing filter state history.
 *
 * @{
 */

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
 * @brief Forward declaration structure for linear filter to operate on Real signal Vector
 */
typedef struct ifx_Filter_R_s ifx_Filter_R_t;

/**
 * @brief Forward declaration structure for hilbert object
 */
typedef struct ifx_Hilbert_R_s ifx_Hilbert_R_t;

/**
 * @brief Defines supported Window options.
 */
typedef enum
{
    IFX_BUTTERWORTH_LOWPASS = 0U,  /**< Butterworth low-pass filter */
    IFX_BUTTERWORTH_HIGHPASS = 1U, /**< Butterworth high-pass filter */
    IFX_BUTTERWORTH_BANDPASS = 2U, /**< Butterworth band-pass filter */
} ifx_Butterworth_Type_t;

typedef enum
{
    IFX_CORRELATE_SAME,
    IFX_CORRELATE_FULL
} ifx_Correlate_Type_t;

/*
==============================================================================
   4. FUNCTION PROTOTYPES
==============================================================================
*/

/**
 * @brief IIR FIR Linear Filter
 * Filters a data sequence, input, using a digital filter with M
 * numerator coefficients vector b, and N denominator coefficients vector a.
 * The initial states (past samples of input and output) are set to zero.
 * The filter is implemented as a direct II transposed structure.
 * The formula for calculating the nth sample of output
 * is expressed as
 * \f[
 * \mathrm{output}_n = \frac{1}{a_0} \left(\sum_{j=0}^{M-1} b_j \cdot \mathrm{input}_{n-j}
 *                     - \sum_{j=1}^{N-1}a_j \cdot \mathrm{output}_{n-j}\right)
 * \f]
 *
 * input and output may point to the same vector.
 *
 * @param [in]     filter    Filter structure which includes the following pointers
 *                              - b         vector : the numerator coefficient vector
 *                              - a         vector : the denominator coefficient vector
 *                                              If a[0] is not 1, then the other coefficients are normalized by it's value.
 *                              - state_b   vector : FIR state from the last filtering operation
 *                              - state_a   vector : IIR state from the last filtering operation
 * @param [in]     input     input signal vector
 * @param [out]    output    output vector(should be the same length as input)
 *
 */
IFX_DLL_PUBLIC
void ifx_signal_filt_run_r(ifx_Filter_R_t* filter,
                           const ifx_Vector_R_t* input,
                           ifx_Vector_R_t* output);

/**
 * @brief IIR FIR Linear Filter
 *
 * This functions works like \ref ifx_signal_filt_run_r except that it works
 * on matrices instead of vectors. The filter is applied to each row of the
 * input matrix and the result is written to the output matrix.
 *
 * The size of the initial state must be bigger than the number of rows of
 * input. Make sure to correctly set the size of the initial state using
 * \ref ifx_signal_filt_resize_r with argument at least the number of rows
 * of output.
 *
 * input and output may point to the same matrix.
 *
 * @param [in]     filter    Filter object.
 * @param [in]     input     input matrix
 * @param [out]    output    output matrix
 */
IFX_DLL_PUBLIC
void ifx_signal_filt_run_mat_r(ifx_Filter_R_t* filter, const ifx_Matrix_R_t* input, ifx_Matrix_R_t* output);

/**
 * @brief Allocate memory and initialize a filter object with numerator and
 * denominator coefficients. Resets filter states to zero.
 *
 * @param [in]     b         ifx_vector containing
 *                           numerator coefficients to be loaded to filter
 * @param [in]     a         ifx_vector containing
 *                           denominator coefficients to be loaded to filter
 *
 * @return Pointer to allocated and initialized real Filter structure
 *
 */
IFX_DLL_PUBLIC
ifx_Filter_R_t* ifx_signal_filt_create_r(const ifx_Vector_R_t* b, const ifx_Vector_R_t* a);

/**
 * @brief Resize the size of the initial state.
 *
 * Set the size of the initial state to size. size is the number of initial states.
 * The initial state will be initialized by 0.
 *
 * After calling this function, \ref ifx_signal_filt_run_mat_r can process input
 * matrices with at most size of rows.
 *
 * @param [in]     filter    Filter object.
 * @param [in]     size      Number of initial states.
 *
 */
IFX_DLL_PUBLIC
void ifx_signal_filt_resize_r(ifx_Filter_R_t* filter, uint32_t size);

/**
 * @brief Create a Butterworth filter
 *
 * This function creates a Butterworth filter. Following filter is created:
 *   - type=IFX_BUTTERWORTH_LOWPASS: low-pass filter with cutoff frequency cutoff_frequency1_Hz.
 *   - type=IFX_BUTTERWORTH_HIGHPASS: high-pass filter with cutoff frequency cutoff_frequency1_Hz.
 *   - type=IFX_BUTTERWORTH_BANDPASS: band-pass filter with cutoff frequencies cutoff_frequency1_Hz, cutoff_frequency2_Hz.
 *
 * Note that unless type=IFX_BUTTERWORTH_BANDPASS the argument cutoff_frequency2_Hz is ignored.
 *
 * The sampling frequency must be at least two-times the cutoff frequency.
 *
 * The returned filter object can be destroyed after usage using the function \ref ifx_signal_filt_destroy_r.
 *
 * The current implementation becomes numerically unstable with higher orders.
 * It is not recommended to use orders higher than 2.
 *
 * @param [in]   type                    type of Butterworth filter.
 * @param [in]   order                   order of Butterworth filter (must be positive).
 * @param [in]   sampling_frequency_Hz   sampling frequency in Hz (must be at least twice the cutoff frequency).
 * @param [in]   cutoff_frequency1_Hz    cutoff frequency in Hz.
 * @param [in]   cutoff_frequency2_Hz    cutoff frequency in Hz (only used for band-pass filter).
 * @return Pointer to allocated and initialized real Filter structure
 *
 * @return Pointer to allocated and initialized real Filter structure
 *
 */
IFX_DLL_PUBLIC
ifx_Filter_R_t* ifx_signal_filter_butterworth_create_r(ifx_Butterworth_Type_t type, uint32_t order, ifx_Float_t sampling_frequency_Hz, ifx_Float_t cutoff_frequency1_Hz, ifx_Float_t cutoff_frequency2_Hz);

/**
 * @brief Resets filter states maintaining the filter coefficients
 *
 *
 * @param [in,out] filter    filter stucture whose states are
 *                           to be reset.
 *
 */
IFX_DLL_PUBLIC
void ifx_signal_filt_reset_r(ifx_Filter_R_t* filter);

/**
 * @brief Frees the memory allocated for a Filter object \ref ifx_Filter_R_t
 *
 *
 * @param [in]     filter    filter stucture whose memory and members need to be
 *                           deallocated.
 *
 */
IFX_DLL_PUBLIC
void ifx_signal_filt_destroy_r(ifx_Filter_R_t* filter);

/**
 * @brief Cross-correlate two 1-dimensional arrays.
 *
 * Cross-correlate the two 1-dimensional arrays x and and y. The
 * result is written to z.
 *
 * If mode is \ref IFX_CORRELATE_FULL the output vector has the length
 * \f$\mathrm{len}(x)+\mathrm{len}(y)-1\f$ and is computed via
 * \f[
 * z_k = \sum_{l=0}^{\mathrm{len}(x)-1}
 *                     x_l \cdot y_{l + \mathrm{len}(y) - k - 1}
 * \f]
 * where \f$k=0,1,\dots,\mathrm{len}(x)+\mathrm{len}(y)-2\f$,
 * and \f$y_m\f$ is zero if the index is outside of the valid range.
 *
 * If mode is \ref IFX_CORRELATE_SAME the output is a centered version of
 * mode \ref IFX_CORRELATE_FULL with dimension \f$\mathrm{len}(x)\f$.
 *
 * @param [in]     x        First input vector
 * @param [in]     y        Second input vector
 * @param [out]    z        Vector of discrete linear cross-correlation of input1 and input2
 * @param [in]     mode     Mode indicating size of output
 *
 */
IFX_DLL_PUBLIC
void ifx_signal_correlate_r(const ifx_Vector_R_t* x, const ifx_Vector_R_t* y, ifx_Vector_R_t* z, ifx_Correlate_Type_t mode);

/**
 * @brief Generates a gaussian pulse vector.
 * Uses pulse configuration parameters \f$b_w\f$ (pulse bandwidth) and \f$f_c\f$(center frequency)
 * The output 'output' is the same size as 't'. The output is a sinusoid shaped by a
 * gaussian function envelope. The \f$\mathrm{bwr}\f$ or reference level at which fractional bandwidth
 * is calculated is fixed at -6 (dB). The elements of the output are populated according to
 * the following equation
 * \f[
 * \mathrm{output}_n = e^{-\mathrm{a} {t_n}^{2}} \cdot cos(2 \pi f_c t_n)
 * \f]
 * where
 * \f[
 * \mathrm{a} = \frac{-(\pi f_c b_w)^2}{4 \mathrm{ln}(10^{\frac{bwr}{20}})}
 * \f]
 *
 * @param [in]     t            time vector defined by \ref ifx_Vector_R_t
 *
 * @param [in]     fc           pulse center frequency in Hz
 *
 * @param [in]     bw           pulse bandwidth
 *
 * @param [out]    output       output vector defined by \ref ifx_Vector_R_t
 *                              contains gaussian pulse values.
 *                              (should be equal in length to 't' input time vector.)
 *
 */
IFX_DLL_PUBLIC
void ifx_signal_gaussianpulse_r(const ifx_Vector_R_t* t,
                                ifx_Float_t fc,
                                ifx_Float_t bw,
                                ifx_Vector_R_t* output);

/**
 * @brief Computes an analytical signal using the hilbert transform.
 * The imaginary component of the output analytical signal is computed using a discrete Hilbert transform filter
 * with a band pass (95%) frequency response. The coefficients of this filter
 * approximate the time domain response of a filter imparting a \f$\pm 90^{\circ} \f$ shift to each
 * frequency component of the input.
 * See also: https://en.wikipedia.org/wiki/Hilbert_transform#Discrete_Hilbert_transform. <br>
 * The elements of the filter are populated according to
 * the following equation
 * \f[
 * \mathrm{h[n]} \overset{\Delta}{=} \left\{ \begin{matrix} 0, \quad for \;\mathrm{n}\; even \\
 * \frac{2}{\pi n}, \;for\;\mathrm{n}\; odd \end{matrix}
 * \right.
 * \f]
 * This model has a range from \f$ -\infty \le\ n\le \infty \f$ and has to be limited.
 * A Hamming window is applied to this section in order to
 * reduce the side lobe artifacts of a rectangular window.
 * The imaginary part of the analytical signal is computed through convolution and selection of the output with
 * the zero phase as center.
 * The real part of the analytical signal is the input signal itself.
 *
 * @param [in]     hilbert_object   input object defined by \ref ifx_Hilbert_R_t
 *
 * @param [in]     input            input signal vector defined by \ref ifx_Vector_R_t
 *
 * @param [out]    output           output complex analytical signal vector defined by \ref ifx_Vector_C_t
 *
 */
IFX_DLL_PUBLIC
void ifx_signal_hilbert_run_c(ifx_Hilbert_R_t* hilbert_object,
                              const ifx_Vector_R_t* input,
                              ifx_Vector_C_t* output);

/**
 * @brief Creates a Hilbert Filter object \ref ifx_Filter_R_t
 *
 * The 'hilbert_order' parameter can be used to control how large a response we consider.
 * The applied range of the filter reduces to
 * \f$ -\mathrm{N} \le\ n\le \mathrm{N} \f$ where
 * \f[
 * \mathrm{N} = \left(hilbert\_order \cdot 2\right)-1
 * \f]
 * making the effective filter length
 * \f[
 * Filter\_length = \left(hilbert\_order \cdot 4\right)-1
 * \f]
 * The larger the order, the more accurate the output.
 *
 * @param [in]     hilbert_order    unique unsigned non zero values of hilbert filter.
 *                                 Results in filter length as shown above.
 *
 * @param [in]     signal_length    length of signal, helpful in reducing hilbert filter length. 0 for variable.
 *
 * @return Hilbert Filter object \ref ifx_Filter_R_t
 *
 */
IFX_DLL_PUBLIC
ifx_Hilbert_R_t* ifx_signal_hilbert_create_r(uint32_t hilbert_order, uint32_t signal_length);

/**
 * @brief Frees the memory allocated for a Hilbert Filter object \ref ifx_Filter_R_t
 *
 *
 * @param [in]     hilbert_object    hilbert stucture whose memory and members need to be
 *                                   deallocated.
 *
 */
IFX_DLL_PUBLIC
void ifx_signal_hilbert_destroy_r(ifx_Hilbert_R_t* hilbert_object);

/**
 * @brief computes Mean Absolute Error for complex vectors.
 * mean absolute error (MAE) is a measure of errors between paired observations expressing
 * the same phenomenon. It is useful optimized method in matching signal shapes.
 * See also: https://en.wikipedia.org/wiki/Mean_absolute_error.
 * the following equation is implemented
 * \f[
 * \mathrm{MAE} = \frac{1}{N} \left(\sum_{j=0}^{N-1} \left|x_j-y_j\right|\right)
 * \f]
 *
 * @param [in]     x            complex reference vector defined by \ref ifx_Vector_C_t
 *
 * @param [in]     y            complex vector defined by \ref ifx_Vector_C_t
 *
 * @return Floating point value denoting the MAE result
 *
 */
IFX_DLL_PUBLIC
ifx_Float_t ifx_signal_mean_abs_error_c(const ifx_Vector_C_t* x,
                                        const ifx_Vector_C_t* y);

/**
 * @brief Compute filter coefficients for Buttworth band-pass filter
 *
 * Compute coefficients of an Nth-order Butterworth band-pass filter. At the
 * cuttoff frequencies the signal is damped by -3dB.
 *
 * The filter coefficients can then be used to create a new filter using
 * @ref ifx_signal_filt_create_r. You can also use \ref ifx_signal_filter_butterworth_create_r
 * to create a Butterworth band-pass filter in one step.
 *
 * The cutoff frequencies and the sampling frequency must fulfil:
 * 0 < frequency_low_Hz < frequency_high_Hz < frequency_high_Hz/2
 *
 * The vectors a and b must be allocated by the caller. The length of a and b
 * must be order+1.
 *
 * The current implementation becomes numerically unstable with higher orders.
 * It is not recommended to use orders higher than 2.
 *
 * See also: https://docs.scipy.org/doc/scipy/reference/generated/scipy.signal.butter.html.
 *
 * @param [in]  order                   order of the Buttworth filter
 * @param [in]  sampling_frequency_Hz   sampling frequency in Hz
 * @param [in]  frequency_low_Hz        lower cutoff frequency in Hz
 * @param [in]  frequency_high_Hz       higher cutoff frequency in Hz
 * @param [out] b                       numerator polynomials of filter
 * @param [out] a                       denominator polynomials of filter
 *
 */
IFX_DLL_PUBLIC
void ifx_signal_butterworth_bandpass(uint32_t order,
                                     ifx_Float_t sampling_frequency_Hz,
                                     ifx_Float_t frequency_low_Hz,
                                     ifx_Float_t frequency_high_Hz,
                                     ifx_Vector_R_t* b,
                                     ifx_Vector_R_t* a);

/**
 * @brief Compute filter coefficients for Buttworth low-pass filter
 *
 * Compute coefficients of an Nth-order Butterworth low-pass filter. At the
 * cuttoff frequency the signal is damped by -3dB.
 *
 * The filter coefficients can then be used to create a new filter using
 * @ref ifx_signal_filt_create_r. You can also use \ref ifx_signal_filter_butterworth_create_r
 * to create a Butterworth low-pass filter in one step.
 *
 * The sampling frequency must be higher than twice the cutoff frequency:
 * sampling_frequency_Hz > 2*cutoff_frequency_Hz
 *
 * The vectors a and b must be allocated by the caller. The length of a and b
 * must be order+1.
 *
 * The current implementation becomes numerically unstable with higher orders.
 * It is not recommended to use orders higher than 2.
 *
 * See also: https://www.dsprelated.com/showarticle/1119.php and https://docs.scipy.org/doc/scipy/reference/generated/scipy.signal.butter.html.
 *
 * @param [in]  order                   order of the Buttworth filter
 * @param [in]  sampling_frequency_Hz   sampling frequency in Hz
 * @param [in]  cutoff_frequency_Hz     cutoff frequency in Hz
 * @param [out] b                       numerator polynomials of filter
 * @param [out] a                       denominator polynomials of filter
 *
 */
IFX_DLL_PUBLIC
void ifx_signal_butterworth_lowpass(uint32_t order, ifx_Float_t sampling_frequency_Hz, ifx_Float_t cutoff_frequency_Hz, ifx_Vector_R_t* b, ifx_Vector_R_t* a);

/**
 * @brief Compute filter coefficients for Buttworth high-pass filter
 *
 * Compute coefficients of an Nth-order Butterworth hig-hpass filter. At the
 * cutoff frequency the signal is damped by -3dB.
 *
 * The filter coefficients can then be used to create a new filter using
 * @ref ifx_signal_filt_create_r. You can also use \ref ifx_signal_filter_butterworth_create_r
 * to create a Butterworth high-pass filter in one step.
 *
 * The sampling frequency must be higher than twice the cutoff frequency:
 * sampling_frequency_Hz > 2*cutoff_frequency_Hz
 *
 * The vectors a and b must be allocated by the caller. The length of a and b
 * must be order+1.
 *
 * The current implementation becomes numerically unstable with higher orders.
 * It is not recommended to use orders higher than 2.
 *
 * See also: https://www.dsprelated.com/showarticle/1135.php and https://docs.scipy.org/doc/scipy/reference/generated/scipy.signal.butter.html.
 *
 * @param [in]  order                   order of the Buttworth filter
 * @param [in]  sampling_frequency_Hz   sampling frequency in Hz
 * @param [in]  cutoff_frequency_Hz     cutoff frequency in Hz
 * @param [out] b                       numerator polynomials of filter
 * @param [out] a                       denominator polynomials of filter
 *
 */
IFX_DLL_PUBLIC
void ifx_signal_butterworth_highpass(uint32_t order, ifx_Float_t sampling_frequency_Hz, ifx_Float_t cutoff_frequency_Hz, ifx_Vector_R_t* b, ifx_Vector_R_t* a);

/**
 * @brief Computes median filter on input vector and stores on output vector
 *
 * Median filter is computing each output element on index n as median on range
 * (n-median_size/2, n+median_size/2). In situation where there even median_size
 * take one element more before then after processed element.
 *
 * On cornels of input vector median windows is decreased to median_size/2.
 *
 * Number of input vector must be same as output.
 * @param [in]  input        data before filtration
 * @param [out] output       data after filtration
 * @param [in]  win_size  the window size of computed median (from how many elements one element is computed from)
 */
IFX_DLL_PUBLIC
void ifx_signal_filter_median(const ifx_Vector_R_t* input, ifx_Vector_R_t* output, uint32_t win_size);

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* IFX_ALGO_SIGNAL_H */
