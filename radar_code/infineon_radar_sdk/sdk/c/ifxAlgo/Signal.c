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

#include <stdlib.h>
#include <string.h>  // for memmove

#include "ifxAlgo/Signal.h"
#include "ifxAlgo/Window.h"

#include "ifxBase/Complex.h"
#include "ifxBase/Matrix.h"
#include "ifxBase/Vector.h"

#include "ifxBase/Defines.h"
#include "ifxBase/Error.h"
#include "ifxBase/internal/Macros.h"
#include "ifxBase/Mem.h"


/*
==============================================================================
   2. LOCAL DEFINITIONS
==============================================================================
*/

// Reference Fractional Bandwidth for Gaussian Pulse fixed at -6dB
#define GAUSSPULSE_REF_FRAC_BW (-6)

// Modulo which ensures modified length of hilbert filter is in accordance with the implementation.
#define HILBERT_FIR_MODULO (3)

// setting when signal length is not fixed for hilbert filter
#define HILBERT_SIGNAL_LENGTH_VARIABLE (0)

// Default Order for Hilbert Transformation
#define HILBERT_ORDER_DEFAULT (23)

// Maximum Order for Hilbert Transformation
#define HILBERT_ORDER_MAX (50)

// Invalid Mean Absolute Error
#define MAE_INVALID (-1.)


/*
==============================================================================
   3. LOCAL TYPES
==============================================================================
*/

/**
 * @brief Defines the structure for real value filter object.
 *        Use type ifx_Filter_R_t for this struct.
 *        needs to be initialized with filter weights in the feed forward 'b'
 *        and feedback 'a' taps. The vector lengths for states are dependant
 *        on the length of weight vectors.
 */

struct ifx_Filter_R_s
{
    ifx_Vector_R_t* a;       /**< Vector containing feedback iir coefficients a, owned by the Filter object */
    ifx_Vector_R_t* b;       /**< Vector containing feedforward fir coefficients b, owned by the Filter object */
    ifx_Matrix_R_t* state_a; /**< Vector containing feedback states, owned by the Filter object */
    ifx_Matrix_R_t* state_b; /**< Vector containing feedforward states, owned by the Filter object */
    ifx_Float_t scale;       /**< Scaling factor for the filter coefficients derived by feedback tap a[0] */
};

/**
 * @brief Defines the structure for real value hilbert object.
 *        Use type ifx_Hilbert_R_t for this struct.
 *        needs to be initialized with Hilbert Taps,
 *        a zero vector for flushing. and a vector for flushed output.
 */
struct ifx_Hilbert_R_s
{
    ifx_Filter_R_t* hilbert_filter;
    ifx_Vector_R_t* flush_in_vector;
    ifx_Vector_R_t* flush_out_vector;
};

/*
==============================================================================
   4. LOCAL DATA
==============================================================================
*/

static const ifx_Complex_t complex_zero = IFX_COMPLEX_DEF(0, 0);
static const ifx_Complex_t complex_one = IFX_COMPLEX_DEF(1, 0);

/*
==============================================================================
   5. LOCAL FUNCTION PROTOTYPES
==============================================================================
*/
/**
 * @brief De-initializes a Filter object \ref ifx_Filter_R_t
 *
 *
 * @param [in,out] filter    pointer to filter stucture whose members have
 *                           to be de-initialized.
 *
 */
static void ifx_signal_filt_deinit_r(ifx_Filter_R_t* filter);

/**
 * @brief De-initializes a Hilbert object \ref ifx_Hilbert_R_t
 *
 *
 * @param [in,out] hilbert_obj    pointer to hilbert stucture whose members have
 *                               to be de-initialized.
 *
 */
static void ifx_hilbert_deinit_r(ifx_Hilbert_R_t* hilbert_obj);

/**
 * @brief  Convert poles and zeros to polynomials with coefficients an and bn
 *
 *
 * @param [in] c             pointer to \ref ifx_Vector_C_t with poles and zeros
 *
 * @param result_c [out]     pointer to \ref ifx_Vector_C_t with polynomials.
 */
static void calc_poly_c(const ifx_Vector_C_t* c, ifx_Vector_C_t* result_c);

/**
 * @brief  Convert poles and zeros to polynomials with coefficients an and bn
 *
 *
 * @param [in] c             pointer to \ref ifx_Vector_R_t with poles and zeros
 *
 * @param result_r [out]     pointer to \ref ifx_Vector_R_t with polynomials.
 */
// static void calc_poly_r(const ifx_Vector_R_t* c, ifx_Vector_R_t* result_r);

/**
 * @brief Compute coefficients of (1+a*x)^n
 *
 * The function computes the coefficients of the polynomial (1+a*x)^n
 * and writes them to result starting with the lowest order.
 *
 * Example:
 * (1+a*x)^2 = 1+2*a*x+a^2*x^2 => result = [1,2*a,2*a^2]
 * We implement this algorithm:
 * https://de.wikipedia.org/w/index.php?title=Binomialkoeffizient&oldid=206359200#Algorithmus_zur_effizienten_Berechnung
 *
 * @param [in] n            exponent
 * @param [in] a            factor
 * @param result [out]      pointer to \ref ifx_Vector_R_t with coefficients.
 */
static void calc_binom_vec(uint32_t n, ifx_Float_t a, ifx_Vector_R_t* result);

/*
==============================================================================
   6. LOCAL FUNCTIONS
==============================================================================
*/

static void ifx_signal_filt_deinit_r(ifx_Filter_R_t* filter)
{
    IFX_ERR_BRK_NULL(filter);

    filter->a = NULL;
    filter->b = NULL;
    filter->state_a = NULL;
    filter->state_b = NULL;
}

//----------------------------------------------------------------------------

static void ifx_hilbert_deinit_r(ifx_Hilbert_R_t* hilbert_obj)
{
    IFX_ERR_BRK_NULL(hilbert_obj);

    hilbert_obj->hilbert_filter = NULL;
    hilbert_obj->flush_in_vector = NULL;
    hilbert_obj->flush_out_vector = NULL;
}

//----------------------------------------------------------------------------

static void calc_poly_c(const ifx_Vector_C_t* c, ifx_Vector_C_t* result_c)
{
    IFX_ERR_BRK_NULL(c);
    IFX_ERR_BRK_COND(vLen(c) + 1 != vLen(result_c), IFX_ERROR_DIMENSION_MISMATCH);

    ifx_vec_setall_c(result_c, complex_zero);
    vAt(result_c, vLen(c)) = complex_one;

    for (uint32_t i = 0; i < vLen(c); ++i)
    {
        for (uint32_t j = vLen(c); j > 0; j--)
        {
            ifx_vec_setat_c(result_c,
                            vLen(c) - j,
                            ifx_complex_sub(vAt(result_c, vLen(c) - j + 1),
                                            ifx_complex_mul(vAt(c, i), vAt(result_c, vLen(c) - j))));
        }

        ifx_vec_setat_c(result_c, vLen(c), ifx_complex_mul_real(ifx_complex_mul(vAt(c, i), vAt(result_c, vLen(c))), -1));
    }

    ifx_vec_setat_c(result_c, 0, complex_one);
}

void correlate_same(const ifx_Vector_R_t* input, const ifx_Vector_R_t* vector, ifx_Vector_R_t* output)
{
    IFX_VEC_BRK_VALID(input);
    IFX_VEC_BRK_VALID(vector);
    IFX_VEC_BRK_VALID(output);

    // length of output must be equal to length of input
    IFX_VEC_BRK_DIM(input, output);
    // initialize output
    ifx_vec_setall_r(output, 0);

    // populate correlation values
    for (uint32_t i = 0; i < vLen(output); i++)
    {

        uint32_t corr_start_index = MAX(0, (int32_t)((vLen(vector) / 2) - i));                   // mid to start index of correlation vector
        uint32_t input_index = MAX(0, (int32_t)(i - (vLen(vector) / 2)));
        uint32_t corr_length = MIN(vLen(vector) - corr_start_index, vLen(input) - input_index);  // correlation limit

        vAt(output, i) = ifx_vec_dot2_r(vector, input, corr_start_index, input_index, corr_length);
    }
}


void correlate_full(const ifx_Vector_R_t* x, const ifx_Vector_R_t* y, ifx_Vector_R_t* z)
{
    IFX_VEC_BRK_VALID(x);
    IFX_VEC_BRK_VALID(y);
    IFX_VEC_BRK_VALID(z);

    const uint32_t len_x = vLen(x);
    const uint32_t len_y = vLen(y);

    uint32_t len_out = len_x + len_y - 1;
    IFX_ERR_BRK_COND(vLen(z) != len_out, IFX_ERROR_DIMENSION_MISMATCH);

    for (uint32_t k = 0; k < len_out; k++)
    {
        const uint32_t lstart = (k + 1) > len_y ? (k + 1 - len_y) : 0;
        const uint32_t lend = MIN(len_x, k + 1);
        const uint32_t len = lend - lstart;

        vAt(z, k) = ifx_vec_dot2_r(x, y, lstart, lstart + len_y - k - 1, len);
    }
}

//----------------------------------------------------------------------------
#if 0
static void calc_poly_r(const ifx_Vector_R_t* c, ifx_Vector_R_t* result_r)
{
   IFX_ERR_BRK_NULL(c);
   IFX_ERR_BRK_COND(vLen(c) + 1 != vLen(result_r), IFX_ERROR_DIMENSION_MISMATCH);

   ifx_vec_setall_r(result_r, 0);

   ifx_vec_setat_r(result_r, vLen(c), 1);

   for (uint32_t i = 0; i < vLen(c) ; ++i) {
      for (uint32_t j = vLen(len); j > 0; j--) {
         ifx_vec_setat_r( result_r,
                          vLen(c) - j,
                          vAt(result_r, vLen(c) - j + 1) - vAt(c,i) *  vAt(result_r, vLen(c) - j));
      }

      ifx_vec_setat_r( result_r, vLen(c), -1 * vAt(c,i) * vAt(result_r, vLen(c)));
   }

   ifx_vec_setat_r(result_r, 0, 1);
}
#endif

//----------------------------------------------------------------------------

static ifx_Vector_C_t* butterworth_poles(uint32_t order)
{
    ifx_Vector_C_t* poles = ifx_vec_create_c(order);
    if (poles == NULL)
        return NULL;

    for (uint32_t j = 0; j < order; j++)
    {
        const uint32_t k = j + 1;
        const ifx_Float_t theta = (2 * k - 1) * IFX_PI / (2 * order);

        const ifx_Complex_t z = IFX_COMPLEX_DEF(-SIN(theta), COS(theta));
        vAt(poles, j) = z;
    }

    return poles;
}

//----------------------------------------------------------------------------

/**
 * @brief Compute coefficients of (1+a*x)^n
 *
 * The function computes the coefficients of the polynomial (1+a*x)^n
 * and writes them to result starting with the lowest order.
 *
 * Example:
 * (1+a*x)^2 = 1+2*a*x+a^2*x^2 => result = [1,2*a,2*a^2]
 * We implement this algorithm:
 * https://de.wikipedia.org/w/index.php?title=Binomialkoeffizient&oldid=206359200#Algorithmus_zur_effizienten_Berechnung
 */
static void calc_binom_vec(uint32_t n, ifx_Float_t a, ifx_Vector_R_t* result)
{
    IFX_ERR_BRK_NULL(result);
    IFX_ERR_BRK_COND(n + 1 != vLen(result), IFX_ERROR_DIMENSION_MISMATCH);

    ifx_Float_t ak = a;  // a**k
    ifx_Float_t p = 1;
    vAt(result, 0) = p;

    for (uint32_t k = 1; k <= n; k++)
    {
        p *= (n + 1 - k) / (ifx_Float_t)k;
        vAt(result, k) = p * ak;
        ak *= a;
    }
}

//----------------------------------------------------------------------------

void filter_r(const ifx_Vector_R_t* input, ifx_Vector_R_t* output, ifx_Float_t scale, const ifx_Vector_R_t* a, const ifx_Vector_R_t* b, ifx_Vector_R_t* state_a, ifx_Vector_R_t* state_b)
{
    IFX_ERR_BRK_NULL(input)
    IFX_ERR_BRK_NULL(output);
    IFX_ERR_BRK_NULL(a);
    IFX_ERR_BRK_NULL(b);
    IFX_ERR_BRK_NULL(state_a);
    IFX_ERR_BRK_NULL(state_b);

    // length of output must be equal to length of input
    IFX_VEC_BRK_DIM(input, output);

    // populate filter output
    for (uint32_t i = 0; i < vLen(output); i++)
    {
        vAt(state_a, 0) = 0;
        vAt(state_b, 0) = vAt(input, i);

        // feed forward (FIR part) and update state vector
        // Compute scale*state_b[i]*b[i] for i=1,2,3,...,vLen(b)-1.
        vAt(state_a, 0) = ifx_vec_dot2_r(state_b, b, 1, 1, vLen(b) - 1) * scale;

        // The memmove is equivalent to this loop (but memmove is faster):
        // for (uint32_t j = (vLen(b) - 1); j > 0; j--)
        //    vAt(state_b, j) = vAt(state_b, j - 1);
        const ifx_Float_t* src = IFX_MDA_DATA(state_b);
        ifx_Float_t* dest = IFX_MDA_DATA(state_b) + 1;
        const size_t size = (vLen(b) - 1) * sizeof(ifx_Float_t);
        memmove(dest, src, size);

        vAt(state_a, 0) += vAt(input, i) * vAt(b, 0) * scale;

        // feedback (IIR part) and update state vector
        for (uint32_t j = (vLen(state_a) - 1); j > 0; j--)
        {
            vAt(state_a, 0) -= vAt(state_a, j) * vAt(a, j) * scale;
            vAt(state_a, j) = vAt(state_a, j - 1);
        }
        vAt(output, i) = vAt(state_a, 0);
    }
}

/*
==============================================================================
   7. EXPORTED FUNCTIONS
==============================================================================
*/

void ifx_signal_filt_run_r(ifx_Filter_R_t* filter,
                           const ifx_Vector_R_t* input,
                           ifx_Vector_R_t* output)
{
    IFX_ERR_BRK_NULL(filter);
    ifx_Vector_R_t state_a;
    ifx_Vector_R_t state_b;
    ifx_mat_get_rowview_r(filter->state_a, 0, &state_a);
    ifx_mat_get_rowview_r(filter->state_b, 0, &state_b);
    filter_r(input, output, filter->scale, filter->a, filter->b, &state_a, &state_b);
}

//----------------------------------------------------------------------------

void ifx_signal_filt_run_mat_r(ifx_Filter_R_t* filter, const ifx_Matrix_R_t* input, ifx_Matrix_R_t* output)
{
    IFX_ERR_BRK_NULL(filter);
    IFX_MAT_BRK_VALID(input);
    IFX_MAT_BRK_VALID(output);

    // dimensions of output must be equal to dimensions of input
    IFX_MAT_BRK_DIM(input, output);

    IFX_ERR_BRK_COND(mRows(input) > mRows(filter->state_a), IFX_ERROR_ARGUMENT_INVALID);

    const uint32_t rows = mRows(input);
    for (uint32_t row = 0; row < rows; row++)
    {
        ifx_Vector_R_t state_a = {0};
        ifx_Vector_R_t state_b = {0};
        ifx_Vector_R_t row_input = {0};
        ifx_Vector_R_t row_output = {0};

        ifx_mat_get_rowview_r(filter->state_a, row, &state_a);
        ifx_mat_get_rowview_r(filter->state_b, row, &state_b);
        ifx_mat_get_rowview_r(input, row, &row_input);
        ifx_mat_get_rowview_r(output, row, &row_output);

        filter_r(&row_input, &row_output, filter->scale, filter->a, filter->b, &state_a, &state_b);
    }
}

//----------------------------------------------------------------------------


void ifx_signal_filt_reset_r(ifx_Filter_R_t* filter)
{
    ifx_mat_clear_r(filter->state_a);
    ifx_mat_clear_r(filter->state_b);
}

//----------------------------------------------------------------------------

void ifx_signal_filt_resize_r(ifx_Filter_R_t* filter, uint32_t size)
{
    IFX_ERR_BRK_ARGUMENT(size == 0);

    ifx_Matrix_R_t* state_a = ifx_mat_create_r(size, mCols(filter->state_a));
    ifx_Matrix_R_t* state_b = ifx_mat_create_r(size, mCols(filter->state_b));
    if (state_a == NULL || state_b == NULL)
    {
        ifx_mat_destroy_r(state_a);
        ifx_mat_destroy_r(state_b);
        return;
    }

    ifx_mat_clear_r(state_a);
    ifx_mat_clear_r(state_b);

    ifx_mat_destroy_r(filter->state_a);
    ifx_mat_destroy_r(filter->state_b);

    filter->state_a = state_a;
    filter->state_b = state_b;
}

//----------------------------------------------------------------------------

ifx_Filter_R_t* ifx_signal_filt_create_r(const ifx_Vector_R_t* ff_coeff, const ifx_Vector_R_t* fb_coeff)
{
    IFX_VEC_BRV_VALID(ff_coeff, NULL);
    IFX_VEC_BRV_VALID(fb_coeff, NULL);

    ifx_Vector_R_t* fb_taps = NULL;
    ifx_Matrix_R_t* fb_state = NULL;
    ifx_Vector_R_t* ff_taps = NULL;
    ifx_Matrix_R_t* ff_state = NULL;

    ifx_Filter_R_t* filter = ifx_mem_alloc(sizeof(ifx_Filter_R_t));
    IFX_ERR_BRN_MEMALLOC(filter);

    fb_taps = ifx_vec_clone_r(fb_coeff);
    IFX_ERR_BRF_COND(!fb_taps, IFX_ERROR_MEMORY_ALLOCATION_FAILED);
    fb_state = ifx_mat_create_r(1, vLen(fb_coeff));
    IFX_ERR_BRF_COND(!fb_state, IFX_ERROR_MEMORY_ALLOCATION_FAILED);
    ff_taps = ifx_vec_clone_r(ff_coeff);
    IFX_ERR_BRF_COND(!ff_taps, IFX_ERROR_MEMORY_ALLOCATION_FAILED);
    ff_state = ifx_mat_create_r(1, vLen(ff_coeff));
    IFX_ERR_BRF_COND(!ff_state, IFX_ERROR_MEMORY_ALLOCATION_FAILED);

    filter->a = fb_taps;
    filter->b = ff_taps;
    filter->state_a = fb_state;
    filter->state_b = ff_state;

    // reset filter states
    ifx_signal_filt_reset_r(filter);

    filter->scale = (ifx_Float_t)1 / vAt(filter->a, 0);

    return filter;
fail:
    ifx_mem_free(filter);
    ifx_vec_destroy_r(fb_taps);
    ifx_mat_destroy_r(fb_state);
    ifx_vec_destroy_r(ff_taps);
    ifx_mat_destroy_r(ff_state);
    return NULL;
}

//----------------------------------------------------------------------------

ifx_Filter_R_t* ifx_signal_filter_butterworth_create_r(ifx_Butterworth_Type_t type, uint32_t order, ifx_Float_t sampling_frequency_Hz, ifx_Float_t cutoff_frequency1_Hz, ifx_Float_t cutoff_frequency2_Hz)
{
    ifx_Vector_R_t* a = NULL;
    ifx_Vector_R_t* b = NULL;

    switch (type)
    {
        case IFX_BUTTERWORTH_LOWPASS:
            a = ifx_vec_create_r(order + 1);
            b = ifx_vec_create_r(order + 1);
            ifx_signal_butterworth_lowpass(order, sampling_frequency_Hz, cutoff_frequency1_Hz, b, a);
            break;

        case IFX_BUTTERWORTH_HIGHPASS:
            a = ifx_vec_create_r(order + 1);
            b = ifx_vec_create_r(order + 1);
            ifx_signal_butterworth_highpass(order, sampling_frequency_Hz, cutoff_frequency1_Hz, b, a);
            break;

        case IFX_BUTTERWORTH_BANDPASS:
            a = ifx_vec_create_r(2 * order + 1);
            b = ifx_vec_create_r(2 * order + 1);
            ifx_signal_butterworth_bandpass(order, sampling_frequency_Hz, cutoff_frequency1_Hz, cutoff_frequency2_Hz, b, a);
            break;
    }

    ifx_Filter_R_t* filter = ifx_signal_filt_create_r(b, a);
    ifx_vec_destroy_r(a);
    ifx_vec_destroy_r(b);

    return filter;
}

//----------------------------------------------------------------------------

void ifx_signal_filt_destroy_r(ifx_Filter_R_t* filter)
{
    if (!filter)
    {
        return;
    }

    ifx_vec_destroy_r(filter->a);
    ifx_vec_destroy_r(filter->b);
    ifx_mat_destroy_r(filter->state_a);
    ifx_mat_destroy_r(filter->state_b);

    ifx_signal_filt_deinit_r(filter);
    ifx_mem_free(filter);
}

//----------------------------------------------------------------------------

void ifx_signal_correlate_r(const ifx_Vector_R_t* x, const ifx_Vector_R_t* y, ifx_Vector_R_t* z, ifx_Correlate_Type_t mode)
{
    switch (mode)
    {
        case IFX_CORRELATE_SAME:
            correlate_same(x, y, z);
            break;

        case IFX_CORRELATE_FULL:
            correlate_full(x, y, z);
            break;

        default:
            ifx_error_set(IFX_ERROR_ARGUMENT_INVALID);
    }
}

//----------------------------------------------------------------------------

void ifx_signal_gaussianpulse_r(const ifx_Vector_R_t* input,
                                ifx_Float_t centerfreq,
                                ifx_Float_t pulse_bw,
                                ifx_Vector_R_t* output)
{
    IFX_VEC_BRK_VALID(input);
    IFX_VEC_BRK_VALID(output);

    // length of output must be equal to length of input
    IFX_VEC_BRK_DIM(input, output);

    // Center frequency must be >=0.
    IFX_ERR_BRK_ARGUMENT(centerfreq < 0);

    // Fractional bandwidth of pulse must be > 0.
    IFX_ERR_BRK_ARGUMENT(pulse_bw <= 0);

    // compute factor 'alpha' determining gaussian pulse width
    ifx_Float_t alpha = -(POW(IFX_PI * centerfreq * pulse_bw, 2)
                          / (LOGN(10) * (ifx_Float_t)GAUSSPULSE_REF_FRAC_BW / (ifx_Float_t)5));

    // populate gaussian pulse vector corresponding to time vector and pulse configuration
    for (uint32_t i = 0; i < vLen(input); i++)
    {
        vAt(output, i) = EXP(-(alpha * POW(vAt(input, i), 2)))
                         * COS(2 * IFX_PI * centerfreq * vAt(input, i));
    }
}

//----------------------------------------------------------------------------

void ifx_signal_hilbert_destroy_r(ifx_Hilbert_R_t* hilbert_obj)
{
    if (!hilbert_obj)
        return;

    ifx_signal_filt_destroy_r(hilbert_obj->hilbert_filter);
    ifx_vec_destroy_r(hilbert_obj->flush_in_vector);
    ifx_vec_destroy_r(hilbert_obj->flush_out_vector);

    ifx_hilbert_deinit_r(hilbert_obj);
    ifx_mem_free(hilbert_obj);
}


//----------------------------------------------------------------------------

void ifx_signal_hilbert_filter_calc_r(ifx_Vector_R_t* hilbert_filter)
{
    IFX_ERR_BRK_NULL(hilbert_filter);

    ifx_vec_setall_r(hilbert_filter, 0.f);

    uint32_t centertap = vLen(hilbert_filter) >> 1;
    uint32_t order = (vLen(hilbert_filter) + 1) >> 2;

    for (uint32_t i = 0; i < order; i++)
    {
        uint32_t index = (i * 2) + 1;
        vAt(hilbert_filter, centertap + index) = 2 / (IFX_PI * index);
        vAt(hilbert_filter, centertap - index) = -(vAt(hilbert_filter, centertap + index));
    }

    // create window
    ifx_Window_Config_t hilbert_fir_window_config;
    hilbert_fir_window_config.type = IFX_WINDOW_HAMM;
    hilbert_fir_window_config.size = vLen(hilbert_filter);

    ifx_Vector_R_t* hilbert_fir_window = ifx_vec_create_r(vLen(hilbert_filter));
    IFX_ERR_BRK_MEMALLOC(hilbert_fir_window);

    ifx_window_init(&hilbert_fir_window_config, hilbert_fir_window);

    // apply window
    for (uint32_t i = 0; i < vLen(hilbert_filter); i++)
    {
        vAt(hilbert_filter, i) = vAt(hilbert_filter, i) * vAt(hilbert_fir_window, i);
    }
    // destroy window
    ifx_vec_destroy_r(hilbert_fir_window);
}

//----------------------------------------------------------------------------

ifx_Hilbert_R_t* ifx_signal_hilbert_create_r(uint32_t hilbert_order, uint32_t signal_length)
{
    ifx_Vector_R_t* hilbert_fir_coeffs = NULL;
    ifx_Filter_R_t* hilbert_filter = NULL;
    ifx_Vector_R_t* flush_in = NULL;
    ifx_Vector_R_t* flush_out = NULL;
    ifx_Hilbert_R_t* hilbert_object = NULL;

    // control the argument validity 'hilbert_order'
    IFX_ERR_BRF_COND((hilbert_order == 0), IFX_ERROR_ARGUMENT_INVALID);
    IFX_ERR_BRF_COND((hilbert_order > HILBERT_ORDER_MAX), IFX_ERROR_ARGUMENT_INVALID);

    hilbert_object = ifx_mem_alloc(sizeof(ifx_Hilbert_R_t));
    IFX_ERR_BRF_MEMALLOC(hilbert_object);

    // compute requested filter length
    uint32_t hilbert_filter_length = (hilbert_order * 4) - 1;

    // if signal_length known, cap length to maximum useful
    if (signal_length != HILBERT_SIGNAL_LENGTH_VARIABLE)
    {
        if (hilbert_filter_length > (signal_length * 2))
        {
            hilbert_filter_length = signal_length * 2;
            // to ensure the modified hilbert filter length has correct modulo i.e. 3
            hilbert_filter_length += HILBERT_FIR_MODULO - (hilbert_filter_length % 4);
        }
    }

    // compute filter coefficients
    hilbert_fir_coeffs = ifx_vec_create_r(hilbert_filter_length);
    IFX_ERR_BRF_MEMALLOC(hilbert_fir_coeffs);

    ifx_signal_hilbert_filter_calc_r(hilbert_fir_coeffs);

    // initialize filter as fir
    ifx_Float_t filter_coeff_a_data[] = {
        1.f,
    };
    ifx_Vector_R_t vector_fb_coeffs;
    ifx_vec_rawview_r(&vector_fb_coeffs, filter_coeff_a_data, 1, 1);

    // create filter object
    hilbert_filter = ifx_signal_filt_create_r(hilbert_fir_coeffs, &vector_fb_coeffs);
    ifx_vec_destroy_r(hilbert_fir_coeffs);
    IFX_ERR_BRF_MEMALLOC(hilbert_filter);
    flush_in = ifx_vec_create_r(vLen(hilbert_filter->b) >> 1);
    IFX_ERR_BRF_MEMALLOC(flush_in);
    flush_out = ifx_vec_create_r(vLen(flush_in));
    IFX_ERR_BRF_MEMALLOC(flush_out);
    ifx_vec_setall_r(flush_in, 0.f);

    hilbert_object->hilbert_filter = hilbert_filter;
    hilbert_object->flush_in_vector = flush_in;
    hilbert_object->flush_out_vector = flush_out;


    return (hilbert_object);
fail:
    ifx_vec_destroy_r(hilbert_fir_coeffs);
    ifx_signal_filt_destroy_r(hilbert_filter);
    ifx_vec_destroy_r(flush_in);
    ifx_vec_destroy_r(flush_out);
    ifx_signal_hilbert_destroy_r(hilbert_object);
    return (NULL);
}

//----------------------------------------------------------------------------

void ifx_signal_hilbert_run_c(ifx_Hilbert_R_t* hilbert_object,
                              const ifx_Vector_R_t* input,
                              ifx_Vector_C_t* output)
{
    IFX_ERR_BRK_NULL(hilbert_object);
    IFX_VEC_BRK_VALID(input);
    IFX_VEC_BRK_VALID(output);

    // length of output must be equal to length of input
    IFX_VEC_BRK_DIM(input, output);

    ifx_Vector_R_t* output_quad = NULL;

    // create quadrature (imaginary) component for output
    output_quad = ifx_vec_create_r(vLen(input));
    IFX_ERR_BRF_MEMALLOC(output_quad);

    // run filter
    ifx_signal_filt_run_r(hilbert_object->hilbert_filter, input, output_quad);

    // rearrange_output
    uint32_t write_index = 0;
    for (uint32_t read_index = (vLen(hilbert_object->hilbert_filter->b) >> 1); read_index < vLen(output_quad); read_index++)
    {
        vAt(output_quad, write_index++) = vAt(output_quad, read_index);
    }
    // flush filter

    ifx_signal_filt_run_r(hilbert_object->hilbert_filter, hilbert_object->flush_in_vector, hilbert_object->flush_out_vector);
    // copy flush values
    for (uint32_t read_index = 0; read_index < vLen(hilbert_object->flush_out_vector); read_index++)
    {
        vAt(output_quad, write_index++) = vAt(hilbert_object->flush_out_vector, read_index);
    }

    ifx_signal_filt_reset_r(hilbert_object->hilbert_filter);
    // create analytical signal
    ifx_vec_complex_c(input, output_quad, output);

fail:
    ifx_vec_destroy_r(output_quad);
}

//----------------------------------------------------------------------------

ifx_Float_t ifx_signal_mean_abs_error_c(const ifx_Vector_C_t* reference,
                                        const ifx_Vector_C_t* vector)
{
    ifx_Float_t mean_abs_error = MAE_INVALID;
    IFX_ERR_BRF_NULL(reference);
    IFX_ERR_BRF_NULL(vector);

    // lengths must be the same
    // IFX_VEC_BRF_DIM(reference, vector);
    IFX_ERR_BRF_COND(vLen(reference) != vLen(vector), -1);

    ifx_Float_t sum_abs_error = 0;
    for (uint32_t i = 0; i < vLen(vector); i++)
    {
        sum_abs_error += ifx_complex_abs((ifx_complex_sub(vAt(reference, i), vAt(vector, i))));
    }
    mean_abs_error = sum_abs_error / vLen(vector);
fail:
    return mean_abs_error;
}

//----------------------------------------------------------------------------

/* See https://docs.scipy.org/doc/scipy/reference/generated/scipy.signal.butter.html
 * and https://www.dsprelated.com/showarticle/1128.php.
 * Also, see https://en.wikipedia.org/wiki/Bilinear_transform on how to transform the
 * analogue filter to a digital one.
 */
void ifx_signal_butterworth_bandpass(uint32_t order,
                                     ifx_Float_t sampling_frequency_Hz,
                                     ifx_Float_t frequency_low_Hz,
                                     ifx_Float_t frequency_high_Hz,
                                     ifx_Vector_R_t* b_r,
                                     ifx_Vector_R_t* a_r)
{
    /* check input parameters */
    IFX_VEC_BRK_VALID(a_r);
    IFX_VEC_BRK_VALID(b_r);
    IFX_ERR_BRK_ARGUMENT(order == 0);
    IFX_ERR_BRK_COND(vLen(a_r) != (2 * order + 1), IFX_ERROR_DIMENSION_MISMATCH);
    IFX_ERR_BRK_COND(vLen(b_r) != (2 * order + 1), IFX_ERROR_DIMENSION_MISMATCH);
    IFX_ERR_BRK_ARGUMENT(frequency_low_Hz <= 0 || frequency_low_Hz >= frequency_high_Hz || (2 * frequency_high_Hz) >= sampling_frequency_Hz);

    ifx_Vector_C_t* pa_c = NULL;
    ifx_Vector_C_t* p_c = NULL;
    ifx_Vector_C_t* a_c = NULL;
    ifx_Vector_C_t* p_prime_c = NULL;

    /* step 1:
     * Compute the poles of the analogue filter
     */
    p_prime_c = butterworth_poles(order);
    IFX_ERR_BRF_MEMALLOC(p_prime_c);

    // Step 2
    const ifx_Float_t F1 = sampling_frequency_Hz / IFX_PI * TAN(IFX_PI * frequency_low_Hz / sampling_frequency_Hz);
    const ifx_Float_t F2 = sampling_frequency_Hz / IFX_PI * TAN(IFX_PI * frequency_high_Hz / sampling_frequency_Hz);
    ifx_Float_t F0 = SQRT(F1 * F2);

    // Step 3
    const ifx_Float_t bw = F2 - F1;
    const ifx_Float_t alpha = bw / (2 * F0);

    pa_c = ifx_vec_create_c(2 * order);
    IFX_ERR_BRF_MEMALLOC(pa_c);

    for (uint32_t k = 0; k < order; k++)
    {
        const ifx_Complex_t one_i_c = IFX_COMPLEX_DEF(0, 1);       // 1j
        const ifx_Complex_t neg_one_i_c = IFX_COMPLEX_DEF(0, -1);  // -1j

        ifx_Complex_t pk_prime_c = vAt(p_prime_c, k);
        ifx_Complex_t x_c = ifx_complex_mul_real(pk_prime_c, alpha);
        ifx_Complex_t x2_c = ifx_complex_mul(x_c, x_c);
        ifx_Complex_t root = ifx_complex_sqrt(ifx_complex_sub(complex_one, x2_c));

        vAt(pa_c, 2 * k + 0) = ifx_complex_add(x_c, ifx_complex_mul(one_i_c, root));
        vAt(pa_c, 2 * k + 1) = ifx_complex_add(x_c, ifx_complex_mul(neg_one_i_c, root));
    }
    ifx_vec_scale_cr(pa_c, 2 * IFX_PI * F0, pa_c);

    // Step 4
    p_c = ifx_vec_create_c(2 * order);
    IFX_ERR_BRF_MEMALLOC(p_c);

    for (uint32_t j = 0; j < 2 * order; j++)
    {
        ifx_Complex_t x = ifx_complex_div_real(vAt(pa_c, j), 2 * sampling_frequency_Hz);
        vAt(p_c, j) = ifx_complex_div(ifx_complex_add(complex_one, x), ifx_complex_sub(complex_one, x));
    }

    // Step 5
    // b_r are the coefficients of the polynomial
    // (1-z)^order * (1+z)^order = (1-z*z)^order
    {
        ifx_vec_setall_r(b_r, 0);

        // create a vector view with every second element of b_r
        ifx_Vector_R_t view;
        ifx_vec_view_r(&view, b_r, 0, order + 1, 2);
        calc_binom_vec(order, -1, &view);
    }

    // Step 6
    a_c = ifx_vec_create_c(2 * order + 1);
    IFX_ERR_BRF_MEMALLOC(a_c);
    calc_poly_c(p_c, a_c);
    IFX_ERR_BRF_COND(!a_c, IFX_ERROR_INTERNAL);

    for (uint32_t i = 0; i < vLen(a_r); i++)
        vAt(a_r, i) = IFX_COMPLEX_REAL(vAt(a_c, i));

    // Step 7: Scaling factor
    {
        const ifx_Float_t f0 = SQRT(frequency_low_Hz * frequency_high_Hz);
        const ifx_Float_t theta = 2 * IFX_PI * f0 / sampling_frequency_Hz;
        const ifx_Complex_t z = IFX_COMPLEX_DEF(COS(theta), -SIN(theta));

        // numerator: (1+z)^order * (1-z)^order = ((1+z)*(1-z))^order
        const ifx_Complex_t opz = ifx_complex_add(complex_one, z);  // 1+z ("one minus z")
        const ifx_Complex_t omz = ifx_complex_sub(complex_one, z);  // 1-z ("one plus z")
        const ifx_Complex_t numerator_c = ifx_complex_pow(ifx_complex_mul(omz, opz), order);

        // denominator = (z-p_0)*(z-p_1)*...*(z-p_2N)
        ifx_Complex_t denominator_c = complex_one;
        for (uint32_t i = 0; i < vLen(p_c); i++)
            denominator_c = ifx_complex_mul(denominator_c, ifx_complex_sub(z, vAt(p_c, i)));

        const ifx_Float_t K = ifx_complex_abs(ifx_complex_div(numerator_c, denominator_c));
        IFX_ERR_BRF_COND(K == 0.0, IFX_ERROR_INTERNAL);

        ifx_vec_scale_r(b_r, 1 / K, b_r);
    }

fail:
    ifx_vec_destroy_c(p_prime_c);
    ifx_vec_destroy_c(pa_c);
    ifx_vec_destroy_c(p_c);
    ifx_vec_destroy_c(a_c);
}

static void butterworth_lowhighpass(uint32_t order, ifx_Float_t sampling_frequency_Hz, ifx_Float_t cutoff_frequency_Hz, bool is_highpass, ifx_Vector_R_t* b, ifx_Vector_R_t* a)
{
    /* See https://docs.scipy.org/doc/scipy/reference/generated/scipy.signal.butter.html,
     * https://www.dsprelated.com/showarticle/1135.php (high-pass) and https://www.dsprelated.com/showarticle/1119.php (low-pass).
     * Also, see https://en.wikipedia.org/wiki/Bilinear_transform on how to transform the
     * analogue filter to a digital one.
     */

    /* check input parameters */
    IFX_VEC_BRK_VALID(a);
    IFX_VEC_BRK_VALID(b);
    IFX_ERR_BRK_ARGUMENT(order == 0);
    IFX_ERR_BRK_COND(vLen(a) != (order + 1), IFX_ERROR_DIMENSION_MISMATCH);
    IFX_ERR_BRK_COND(vLen(b) != (order + 1), IFX_ERROR_DIMENSION_MISMATCH);
    IFX_ERR_BRK_ARGUMENT(sampling_frequency_Hz <= 0 || cutoff_frequency_Hz <= 0 || (2 * cutoff_frequency_Hz) >= sampling_frequency_Hz);

    ifx_Vector_C_t* ac = NULL;
    ifx_Vector_C_t* poles = NULL;
    ifx_Vector_C_t* p = NULL;

    /* step 1:
     * Compute the poles of the analogue filter
     */
    poles = butterworth_poles(order);
    IFX_ERR_BRF_MEMALLOC(poles);

    /* step 2:
     * Given the -3 dB discrete frequency cutoff_frequency_Hz of the digital
     * filter, find the corresponding frequency of the analogue filter
     * See also: https://en.wikipedia.org/wiki/Bilinear_transform
     */
    const ifx_Float_t Fc = sampling_frequency_Hz / IFX_PI * TAN(IFX_PI * cutoff_frequency_Hz / sampling_frequency_Hz);

    /* step 3:
     * Transform the normalized analogue poles to analog highpass poles.
     */
    if (is_highpass)
    {
        /* poles[j] = 2*pi*Fc*poles[j] */
        ifx_vec_scale_cr(poles, 2 * IFX_PI * Fc, poles);
    }
    else /* lowpass */
    {
        const ifx_Complex_t c = IFX_COMPLEX_DEF(2 * IFX_PI * Fc, 0);

        /* poles[j] = 2*pi*Fc/poles[j] */
        for (uint32_t j = 0; j < vLen(poles); j++)
            vAt(poles, j) = ifx_complex_div(c, vAt(poles, j));
    }

    /* step 4:
     * Transform the poles from the s-plane to the z-plane using the bilinear
     * transform.
     * See also: https://en.wikipedia.org/wiki/Bilinear_transform
     */
    p = ifx_vec_create_c(order);
    IFX_ERR_BRF_MEMALLOC(p);

    for (uint32_t j = 0; j < vLen(poles); j++)
    {
        /* poles[j] / (2*sampling_frequency) */
        const ifx_Complex_t z = ifx_complex_div_real(vAt(poles, j), 2 * sampling_frequency_Hz);

        /* 1+z */
        const ifx_Complex_t numerator = ifx_complex_add_real(z, 1);

        /* 1-z */
        const ifx_Complex_t denominator = ifx_complex_add_real(ifx_complex_mul_real(z, -1), 1);

        /* p = (1 + poles / (2 * sampling_frequency_Hz)) / (1 - poles / (2 * sampling_frequency_Hz)) */
        vAt(p, j) = ifx_complex_div(numerator, denominator);
    }

    /* step 5:
     * Add order of zeros at z=-1 (low-pass) or z=1 (high-pass).
     * The transfer function H(z) then looks like:
     *    H(z) = K * (z+1)^N / ((z-p_0)*(z-p_1)*...(z-p_{N-1})) (low-pass)
     * or
     *    H(z) = K * (z-1)^N / ((z-p_0)*(z-p_1)*...(z-p_{N-1})) (high-pass)
     * where N corresponds to the order. K is a normalization constant that
     * we will determine in step 7.
     *
     * We directly compute the polynomial coefficients of (1+z)^N (low-pass) or
     * (1-z)^N (high-pass). The coefficients of this polynomial correspond
     * to the coefficients of b. Also, the coefficients are just the
     * binomial coefficients (N,k) for k=0,1,2,...,N:
     *      (1+z)^N = (N,0)z^0 + (N,1)z^1 + (N,2)z^2 ... + (N,N)z^N
     * where (N,k) denotes the binomial coefficient ("N choose k").
     */
    if (is_highpass)
        calc_binom_vec(order, -1, b);  // compute coefficients of (1-z)^N
    else
        calc_binom_vec(order, +1, b);  // compute coefficients of (1+z)^N

    /* step 6:
     * Convert poles and zeros to polynomials with coefficients a.
     */
    ac = ifx_vec_create_c(order + 1);
    IFX_ERR_BRF_MEMALLOC(ac);
    calc_poly_c(p, ac);

    for (uint32_t j = 0; j < vLen(ac); j++)
        vAt(a, j) = IFX_COMPLEX_REAL(vAt(ac, j));

    /* step 7:
     * Normalize the coefficients of b such that the transfer function H(z)=1
     * for z=1 (low-pass) or z=-1 (high-pass).
     * We use the formulae from step 5:
     *
     *    K = H(z) * (z-p_0)*(z-p_1)*...(z-p_{N-1}) / (z+1)^N   (low-pass, z=+1)
     * or
     *    K = H(z) * (z-p_0)*(z-p_1)*...(z-p_{N-1}) / (z-1)^N   (high-pass, z=-1)
     * or
     * It can be seen that the denominator is 2^N (low-pass) or (-2)^N (high-pass).
     */
    {
        ifx_Complex_t z;
        if (is_highpass)
            IFX_COMPLEX_SET(z, -1, 0);
        else
            IFX_COMPLEX_SET(z, 1, 0);

        ifx_Complex_t product = complex_one;
        for (uint32_t j = 0; j < vLen(p); j++)
            product = ifx_complex_mul(product, ifx_complex_sub(z, vAt(p, j)));

        ifx_Float_t K = IFX_COMPLEX_REAL(product) / POW(2, (ifx_Float_t)order);
        if (is_highpass && order % 2)
            K = -K;

        // scale b by K
        ifx_vec_scale_r(b, K, b);
    }

fail:
    ifx_vec_destroy_c(p);
    ifx_vec_destroy_c(poles);
    ifx_vec_destroy_c(ac);
}

void ifx_signal_butterworth_lowpass(uint32_t order, ifx_Float_t sampling_frequency_Hz, ifx_Float_t cutoff_frequency_Hz, ifx_Vector_R_t* a, ifx_Vector_R_t* b)
{
    butterworth_lowhighpass(order, sampling_frequency_Hz, cutoff_frequency_Hz, false, a, b);
}

void ifx_signal_butterworth_highpass(uint32_t order, ifx_Float_t sampling_frequency_Hz, ifx_Float_t cutoff_frequency_Hz, ifx_Vector_R_t* a, ifx_Vector_R_t* b)
{
    butterworth_lowhighpass(order, sampling_frequency_Hz, cutoff_frequency_Hz, true, a, b);
}

//----------------------------------------------------------------------------

void ifx_signal_filter_median(const ifx_Vector_R_t* input, ifx_Vector_R_t* output, uint32_t win_size)
{
    IFX_VEC_BRK_VALID(input);
    IFX_VEC_BRK_VALID(output);
    IFX_ERR_BRK_ARGUMENT(win_size == 0);
    IFX_ERR_BRK_COND(vLen(input) != vLen(output), IFX_ERROR_DIMENSION_MISMATCH);

    win_size = MIN(win_size, vLen(input) * 2);  // 2x len there is max for median
    const uint32_t len = vLen(input);
    const uint32_t win_len_left = win_size / 2;
    const uint32_t win_len_right = win_size - win_len_left;

    for (uint32_t i = 0; i < len; i++)
    {
        const uint32_t start = (uint32_t)(MAX(0, (int32_t)i - (int32_t)win_len_left));
        const uint32_t end = MIN(i + win_len_right, len);
        // Range in math notation: [start, end)
        const uint32_t median_size = end - start;
        vAt(output, i) = ifx_vec_median_range_r(input, start, median_size);
    }
}
