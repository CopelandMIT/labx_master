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
 * @file Math.h
 *
 * \brief \copybrief gr_math
 *
 * For details refer to \ref gr_math
 */

#ifndef IFX_BASE_MATH_H
#define IFX_BASE_MATH_H

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/
#include "Types.h"
#include "Vector.h"


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
 * @brief Forward declaration structure for Math Axis Spec.
 */
typedef struct ifx_Math_Axis_Spec_s ifx_Math_Axis_Spec_t;

/**
 * @brief Defines supported Scale Types.
 */
typedef enum
{
    IFX_SCALE_TYPE_LINEAR = 0U,         /**< By default scale is linear for all modules.*/
    IFX_SCALE_TYPE_DECIBEL_10LOG = 10U, /**< Scale is in dB = 10xlog10().*/
    IFX_SCALE_TYPE_DECIBEL_20LOG = 20U  /**< Scale is in dB = 20xlog10().*/
} ifx_Math_Scale_Type_t;

/**
 * @brief Defines the structure for semantics of an axis that represents a physical quantity.
 *
 * The axis can be along a vector (e.g., Frequency axis of a vector representing FFT values)
 * or even along one of the dimensions of a matrix (e.g., Range axis or the Speed
 * axis of a matrix representing a range-doppler map).
 * Structures of this type can be used to readily describe / visualize / plot a physical
 * quantity without additional calculations in a GUI.
 */
struct ifx_Math_Axis_Spec_s
{
    ifx_Float_t min_value;          /**< Minimum value depicted in the axis (can be negative).*/
    ifx_Float_t max_value;          /**< Maximum value depicted in the axis.*/
    ifx_Float_t value_bin_per_step; /**< Value bin represented by each step (sample) in the axis.*/
};

/*
==============================================================================
   4. FUNCTION PROTOTYPES
==============================================================================
*/

/** @addtogroup gr_cat_SDK_base
 * @{
 */

/** @defgroup gr_math Math
 * @brief API for Mathematical operations
 *
 * Supports mathematical operations such as
 * mean calculation, scalar removal from an array, element wise
 * addition, subtraction and multiplication of arrays, scalar
 * multiplication of an array, absolute value calculation, maximum
 * value in an array.
 *
 * @{
 */

/**
 * @brief Computes the maximum value in a real array along with the corresponding index.
 *
 * @param [in]     input     Pointer to data memory defined by \ref ifx_Vector_R_t,
 *                           whose maximum value is to be computed
 * @param [out]    max_idx   Index corresponding to the maximum value (only set if not NULL)
 *
 * @return Maximum value in the given array
 *
 */
IFX_DLL_PUBLIC
ifx_Float_t ifx_math_find_max(const ifx_Vector_R_t* input,
                              uint32_t* max_idx);

/**
 * @brief Operates on \ref ifx_Vector_R_t arrays, to clip values less than given threshold.
 *
 * if input(n) < threshold
 *    output(n) = clip_value
 * else
 *    output(n) = input(n)
 *
 * @param [in]     input               Pointer to data memory defined by \ref ifx_Vector_R_t
 * @param [in]     threshold           Real value, must be greater than or equal to zero
 * @param [in]     clip_value          Real value, to be set if input values are greater than threshold
 * @param [out]    output              Pointer to output memory, could be same as input for in-place operation
 *
 */
IFX_DLL_PUBLIC
void ifx_math_vec_clip_lt_threshold_r(const ifx_Vector_R_t* input,
                                      ifx_Float_t threshold,
                                      ifx_Float_t clip_value,
                                      ifx_Vector_R_t* output);

/**
 * @brief Operates on \ref ifx_Vector_R_t arrays, to clip values greater than given threshold.
 *
 * if input(n) > threshold
 *    output(n) = clip_value
 * else
 *    output(n) = input(n)
 *
 * @param [in]     input               Pointer to data memory defined by \ref ifx_Vector_R_t
 * @param [in]     threshold           Real value, must be greater than or equal to zero
 * @param [in]     clip_value          Real value, to be set if input values are greater than threshold
 * @param [out]    output              Pointer to output memory, could be same as input for in-place operation
 *
 */
IFX_DLL_PUBLIC
void ifx_math_vec_clip_gt_threshold_r(const ifx_Vector_R_t* input,
                                      ifx_Float_t threshold,
                                      ifx_Float_t clip_value,
                                      ifx_Vector_R_t* output);

/**
 * @brief Operates on real scalar, to convert from linear to dB scale.
 * If any of the input real values are negative, the output would be a NaN as per the C99 standard.
 *
 * @param [in]     input     Real value as input
 * @param [in]     scale     For voltage this should be 20 i.e. 20xlog10() and for power 10 i.e. 10xlog10()
 *                           However, this is a generic math function, so scale can be any desired non zero float value
 *
 * @return dB value if input as per scale specified
 */
IFX_DLL_PUBLIC
ifx_Float_t ifx_math_linear_to_db(ifx_Float_t input,
                                  ifx_Float_t scale);

/**
 * @brief Operates on real scalar, to convert from dB to Linear scale.
 *
 * @param [in]     input     Real value as input
 * @param [in]     scale     For voltage this should be 20 i.e. 10^(A/20) and for power 10 i.e. 10^(A/10)
 *
 * @return Real value as output
 */
IFX_DLL_PUBLIC
ifx_Float_t ifx_math_db_to_linear(ifx_Float_t input,
                                  ifx_Float_t scale);

/**
 * @brief Round uint32 up to the next power of 2
 *
 * The function returns x rounded up to the next power of 2.
 *
 * The returned value y is a power of 2, is y>=x, and y<2*x.
 *
 * For x=0 the function returns 0.
 *
 * @param [in] x    input value
 * @retval y        x rounded up to the next power of 2
 */
IFX_DLL_PUBLIC
uint32_t ifx_math_round_up_power_of_2_uint32(uint32_t x);

/**
 * @brief Checks if power of two
 *
 * The function returns true if n is power of two.
 *
 * 0 is not considered as a power of 2, i.e., for n=0 the function returns 0.
 *
 * @param [in] n    input value
 * @retval         1         if n is power of 2
 * @retval         0         otherwise
 */
IFX_DLL_PUBLIC
int ifx_math_ispower_of_2(uint32_t n);

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* IFX_BASE_MATH_H */
