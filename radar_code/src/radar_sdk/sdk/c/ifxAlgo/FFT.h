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
 * @file FFT.h
 *
 * \brief \copybrief gr_fft
 *
 * For details refer to \ref gr_fft
 */

#ifndef IFX_ALGO_FFT_H
#define IFX_ALGO_FFT_H

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include "ifxBase/Types.h"
#include "ifxBase/Vector.h"


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
 * @brief A handle for an instance of FFT processing module, see FFT.h.
 */
typedef struct ifx_FFT_s ifx_FFT_t;

/**
 * @brief Defines supported FFT Types.
 */
typedef enum
{
    IFX_FFT_TYPE_R2C = 1U, /**< Input is real and FFT output is complex.*/
    IFX_FFT_TYPE_C2C = 2U  /**< Input is complex and FFT output is complex.*/
} ifx_FFT_Type_t;

/*
==============================================================================
   4. FUNCTION PROTOTYPES
==============================================================================
*/

/** @addtogroup gr_cat_Algorithms
 * @{
 */

/** @defgroup gr_fft FFT
 * @brief API for Fast Fourier Transform (FFT)
 *
 * Supports one-dimensional FFT for real and complex input signals.
 *
 * The Fourier transformation is an integral transform that composes a
 * continuous function depending on space or time into functions depending on
 * spatial or temporal frequency.  The transform for a function \f$f(x)\f$ is
 * defined as
 * \f[
 * \hat{f}(y) = \int_{-\infty}^\infty f(x) e^{- 2\pi i \, x\cdot y} \,\mathrm{d}x
 * \f]
 * where \f$\hat{f}\f$ denotes the Fourier transform of \f$f\f$. The transform
 * is effectively a change of basis and expresses the original function \f$f\f$
 * as a superposition of plane waves. It is a useful tool in signal processing
 * for example to detect periodic components of a signal. Also note that
 * other definitions of the Fourier transformation with different prefactors
 * exist.
 *
 * The Discrete Fourier Transform (DFT) corresponds to the Fourier transform
 * for a discrete signal \f$a_j\f$ \f$(j=0,1,\dots,N-1)\f$:
 * \f[
 * \hat{a}_k = \sum_{j=0}^{N-1} e^{-2\pi i \cdot \frac{jk}{N}} \cdot a_j \quad (k=0,1,\dots,N-1)
 * \f]
 *
 * For real input signals \f$a_k = a_k^*\f$ (\f$a_k^*\f$ denotes the complex conjugate of \f$a_k\f$)
 * the DFT contains only \f$N/2+1\f$ independent values \f$\hat{a}_0, \hat{a}_1, \dots, \hat{a}_{N/2}\f$
 * as \f$\hat{a}_{N-k} = \hat{a}^*_k\f$. Note that the DC component
 * \f$\hat{a}_0\f$ and the component \f$\hat{a}_{N/2}\f$ are both real.
 *
 * The DFT can be computed effectively by means of the Fast Fourier Transform
 * (FFT) algorithm with \f$\mathcal{O}(N \log N)\f$ effort for an input
 * signal of length \f$N\f$.
 *
 * More information can be found in standard textbooks or on Wikipedia:
 *   - https://en.wikipedia.org/wiki/Discrete_Fourier_transform
 *   - https://en.wikipedia.org/wiki/Fast_Fourier_transform
 *
 * @{
 */

/**
 * @brief Creates an FFT object
 *
 * Create an FFT object. fft_type must be either \ref IFX_FFT_TYPE_R2C if
 * the input signal is real or \ref IFX_FFT_TYPE_C2C for a complex input
 * signal.
 *
 * fft_size must be a power of 2, and 4 <= fft_size <= 65536.
 *
 * @param [in]     fft_type  FFT type, see \ref ifx_FFT_Type_t.
 * @param [in]     fft_size  FFT size \f$N\f$
 *
 * @return fft  FFT object
 */
IFX_DLL_PUBLIC
ifx_FFT_t* ifx_fft_create(ifx_FFT_Type_t fft_type, uint32_t fft_size);

/**
 * @brief Destroys FFT object
 *
 * Destroys the FFT object and frees the allocated memory.
 *
 * @param [in]     handle    FFT object
 */
IFX_DLL_PUBLIC
void ifx_fft_destroy(ifx_FFT_t* handle);

/**
 * @brief Performs FFT transform on a real input
 *
 * Performs a discrete Fourier transform on real input data using the Fast
 * Fourier Transform (FFT) algorithm. The FFT size \f$N\f$ was specified in
 * \ref ifx_fft_create.
 *
 * If the length of the input vector is smaller than \f$N\f$ the input vector
 * is internally zero padded. If the length of the input vector is larger than
 * \f$N\f$ only the first \f$N\f$ elements are used and all other elements
 * of the vector are ignored.
 *
 * If the length of the output vector is at least \f$N\f$ the full \f$N\f$
 * complex frequency samples are written to output.
 *
 * If the length of the output vector is at least \f$N/2+1\f$ only \f$N/2+1\f$
 * complex frequency samples are written to output.
 *
 * If the length of the output vector is \f$N/2\f$ only \f$N/2\f$ complex
 * frequency samples are written to output.
 *
 * The first element of output (index 0) contains the DC component and has
 * imaginary part 0.
 *
 * If the length of the output vector is smaller than \f$N/2\f$ an error is
 * set.
 *
 * @param [in]     handle    FFT object
 * @param [in]     input     Real input samples
 * @param [out]    output    Complex output vector
 */
IFX_DLL_PUBLIC
void ifx_fft_run_rc(ifx_FFT_t* handle,
                    const ifx_Vector_R_t* input,
                    ifx_Vector_C_t* output);

/**
 * @brief Performs FFT transform on a complex input
 *
 * Performs a discrete Fourier transform using the Fast Fourier Transform (FFT)
 * algorithm. The FFT size \f$N\f$ was specified in \ref ifx_fft_create.
 *
 * If the length of the input vector is smaller than \f$N\f$, the input vector
 * is internally zero padded. If the length of the input vector is larger than
 * \f$N\f$ only the first \f$N\f$ elements are used and all other elements of
 * the vector are ignored.
 *
 * The FFT output has \f$N\f$ complex elements and hence \f$N\f$ elements are written
 * to the vector output. The vector output must have at least length \f$N\f$.
 *
 * @param [in]     handle    A handle to the FFT object
 * @param [in]     input     Complex input vector
 * @param [out]    output    Complex output vector (length of \f$N\f$ or larger)
 */
IFX_DLL_PUBLIC
void ifx_fft_run_c(ifx_FFT_t* handle,
                   const ifx_Vector_C_t* input,
                   ifx_Vector_C_t* output);

/**
 * @brief Performs shift on a FFT amplitude spectrum (real values) to bring DC bin in
 *        the center of spectrum, positive bins on right side and negative bins on left side.
 *        Input and output memories should be of same type and size.
 *
 * @param [in]     input     Real array on which shift operation needs to be performed
 * @param [out]    output    DC bin shifted to the center of the output array
 *
 */
IFX_DLL_PUBLIC
void ifx_fft_shift_r(const ifx_Vector_R_t* input,
                     ifx_Vector_R_t* output);

/**
 * @brief Performs shift on a FFT spectrum complex to bring DC bin in the center of spectrum,
 *        positive bins on right side and negative bins on left side.
 *        Input and output memories should be of same type and size.
 *
 * @param [in]     input     Complex array on which shift operation needs to be performed
 * @param [out]    output    DC bin shifted to the center of the output array
 *
 */
IFX_DLL_PUBLIC
void ifx_fft_shift_c(const ifx_Vector_C_t* input,
                     ifx_Vector_C_t* output);

/**
 * @brief Returns the FFT size within current FFT handle.
 *
 * @param [in]     handle    A handle to the FFT object
 *
 * @return FFT size configured in the FFT handle.
 */
IFX_DLL_PUBLIC
uint32_t ifx_fft_get_fft_size(const ifx_FFT_t* handle);

/**
 * @brief Returns the FFT type within current FFT handle.
 *
 * @param [in]     handle    A handle to the FFT object
 *
 * @return FFT type configured in the FFT handle defined by \ref ifx_FFT_Type_t.
 *
 */
IFX_DLL_PUBLIC
ifx_FFT_Type_t ifx_fft_get_fft_type(const ifx_FFT_t* handle);

/**
 * @brief Perform FFT on raw pointers
 * @param [in]     handle    A handle to the FFT object
 * @param [in]     in        Pointer to array of floats, size must be the configured FFT size.
 * @param [out]    out       Pointer to output array of complex floats, size is half the configured FFT size.
 */
IFX_DLL_PUBLIC
void ifx_fft_raw_rc(ifx_FFT_t* handle, const ifx_Float_t* in, ifx_Complex_t* out);

/**
 * @brief Perform FFT on raw pointers
 * @param [in]     handle    A handle to the FFT object
 * @param [in]     in        Pointer to array of complex floats, size must be the configured FFT size.
 * @param [out]    out       Pointer to output array of complex floats, size must be the configured FFT size.
 */
IFX_DLL_PUBLIC
void ifx_fft_raw_c(ifx_FFT_t* handle, const ifx_Complex_t* in, ifx_Complex_t* out);

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* IFX_ALGO_FFT_H */
