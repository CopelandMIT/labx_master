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
 * @file Vector.h
 *
 * \brief \copybrief gr_vector
 *
 * For details refer to \ref gr_vector
 */

#ifndef IFX_BASE_VECTOR_H
#define IFX_BASE_VECTOR_H

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include "Mda.h"
#include "Types.h"


#ifdef __cplusplus
extern "C"
{
#endif


/*
==============================================================================
   2. DEFINITIONS
==============================================================================
*/

// Access macros -------------------------------------------------------------
#define IFX_VEC_STRIDE(v)      (IFX_MDA_STRIDE(v)[0])
#define IFX_VEC_OFFSET(v, idx) IFX_MDA_OFFSET(v, idx)
#define IFX_VEC_LEN(v)         (IFX_MDA_SHAPE(v)[0])
#define IFX_VEC_DAT(v)         IFX_MDA_DATA(v)

/** @brief Access vector element
 *
 * The macro can be used to set and get elements of a vector, for example:
 * @code
 * foo = IFX_VEC_AT(cube, idx);
 * IFX_VEC_AT(cube, idx) = bar;
 * @endcode
 *
 * The macro works with both real (\ref ifx_Vector_R_t) and complex (\ref ifx_Vector_C_t) vectors.
 */
#define IFX_VEC_AT(v, idx) IFX_MDA_AT(v, idx)

// Condition check macro adaptations for Vector module -----------------------
#define IFX_VEC_BRK_DIM(v1, v2)    IFX_ERR_BRK_COND(IFX_VEC_LEN(v1) != IFX_VEC_LEN(v2), IFX_ERROR_DIMENSION_MISMATCH)
#define IFX_VEC_BRV_DIM(v1, v2, a) IFX_ERR_BRV_COND(IFX_VEC_LEN(v1) != IFX_VEC_LEN(v2), IFX_ERROR_DIMENSION_MISMATCH, a)

#define IFX_VEC_BRK_MINSIZE(v, minsize) IFX_ERR_BRK_COND(IFX_VEC_LEN(v) < (minsize), IFX_ERROR_DIMENSION_MISMATCH)

#define IFX_VEC_BRK_DIM_GT(vsmall, v) IFX_ERR_BRK_COND(vLen(vsmall) > vLen(v), IFX_ERROR_DIMENSION_MISMATCH)

#define IFX_VEC_BRK_VEC_BOUNDS(v, idx) IFX_ERR_BRK_COND((idx) >= IFX_VEC_LEN(v), IFX_ERROR_ARGUMENT_OUT_OF_BOUNDS)
#define IFX_VEC_BRF_VEC_BOUNDS(v, idx) IFX_ERR_BRF_COND((idx) >= IFX_VEC_LEN(v), IFX_ERROR_ARGUMENT_OUT_OF_BOUNDS)

#define IFX_VEC_BRK_VALID(m)                                                        \
    do                                                                              \
    {                                                                               \
        IFX_ERR_BRK_NULL(m);                                                        \
        IFX_ERR_BRK_COND(IFX_MDA_DIMENSIONS(m) != 1, IFX_ERROR_DIMENSION_MISMATCH); \
        IFX_ERR_BRK_ARGUMENT(vDat(m) == NULL);                                      \
    } while (0)
#define IFX_VEC_BRV_VALID(m, r)                                                        \
    do                                                                                 \
    {                                                                                  \
        IFX_ERR_BRV_NULL(m, r);                                                        \
        IFX_ERR_BRV_COND(IFX_MDA_DIMENSIONS(m) != 1, IFX_ERROR_DIMENSION_MISMATCH, r); \
        IFX_ERR_BRV_ARGUMENT(vDat(m) == NULL, r);                                      \
    } while (0)

/*
==============================================================================
   3. TYPES
==============================================================================
*/

/**
 * @brief Forward declaration structure to operate on Real Vector.
 */
typedef ifx_Mda_R_t ifx_Vector_R_t;

/**
 * @brief Forward declaration structure to operate on Complex Vector.
 */
typedef ifx_Mda_C_t ifx_Vector_C_t;

/**
 * @brief Defines supported Vector sorting order options.
 */
typedef enum
{
    IFX_SORT_ASCENDING = 0, /**< Sorting in Ascending order */
    IFX_SORT_DESCENDING     /**< Sorting in Descending order */
} ifx_Vector_Sort_Order_t;

/*
==============================================================================
   4. FUNCTION PROTOTYPES
==============================================================================
*/

/** @addtogroup gr_cat_SDK_base
 * @{
 */

/** @defgroup gr_vector Vector
 * @brief API for operations on Vector data structures
 *
 * Supports mathematical and other operations such as creation and destruction of
 * vectors, or printing vector samples onto a file.
 *
 * @{
 */

/**
 * @brief Assigns raw data to the ifx_Vector_R_t structure.
 * Example usage:
 *
 *   Given \ref ifx_Vector_R_t out_vector, dimension of the vector length,
 *   stride between consecutive elements in data array
 *   and real data in array arr_data of size length, one can assign:
 * @code
 *     ifx_vec_rawview_r(&out_vector, arr_data, length, stride);
 * @endcode
 *   e.g. for vector of length 3 one can assign:
 * @code
 *     ifx_Float_t arr_data[3] = {1, 2, 3};
 *     ifx_vec_rawview_r(&out_vector, arr_data, 3, 1);
 * @endcode
 *
 * @param [in,out] vector    Pointer to the memory containing array defined by \ref ifx_Vector_R_t
 * @param [in]     d         Data pointer to assign the vector
 * @param [in]     length    Number of elements
 * @param [in]     stride    Address difference for consecutive elements
 *
 */
IFX_DLL_PUBLIC
void ifx_vec_rawview_r(ifx_Vector_R_t* vector,
                       ifx_Float_t* d,
                       uint32_t length,
                       uint32_t stride);

/**
 * @brief Assigns raw data to the ifx_Vector_C_t structure.
 * Example usage:
 *
 *   Given \ref ifx_Vector_C_t out_vector, dimension of the vector length
 *   and a complex data in array arr_data of size length, one can assign:
 * @code
 *     ifx_vec_rawview_c(&out_vector, arr_data, length, stride);
 * @endcode
 *   e.g. for vector of length 3 one can assign:
 * @code
 *     ifx_Complex_t arr_data[3] = {{1,1}, {2,2}, {3,3}};
 *     ifx_vec_rawview_c(&out_vector, arr_data, 3, 1);
 * @endcode
 *
 * @param [in,out] vector    Pointer to the memory containing array defined by \ref ifx_Vector_C_t
 * @param [in]     d         Data pointer to assign the vector
 * @param [in]     length    Number of elements
 * @param [in]     stride    Address difference for consecutive elements
 *
 */
IFX_DLL_PUBLIC
void ifx_vec_rawview_c(ifx_Vector_C_t* vector,
                       ifx_Complex_t* d,
                       uint32_t length,
                       uint32_t stride);

/**
 * @brief Assigns real data from source vector to the destination vector.
 * Example usage:
 *
 *   Given \ref ifx_Vector_R_t dest_vector and \ref ifx_Vector_R_t source_vector,
 *   dimension of the dest_vector length and
 *   offset for source_matrix and spacing/stride between elements one can assign:
 * @code
 *     ifx_vec_view_r(&dest_vector, &source_vector, offset, length, spacing);
 * @endcode
 *   e.g. for taking a view of the second half of the vector:
 * @code
 *     ifx_Float_t arr_data[8] = {1, 2, 3, 4, 5, 6, 7, 8};
 *     ifx_vec_rawview_r(&source_vector, arr_data, 8, 1);
 *
 *     ifx_vec_view_r(&dest_vector, &source_vector, 4, 4, 1);
 * @endcode
 *     will give a vector with values:
 * @code
 *     [5, 6, 7, 8]
 * @endcode
 *
 * @param [in,out] vector    Pointer to the memory containing array defined by \ref ifx_Vector_R_t
 * @param [in]     source    Pointer to the memory containing array defined by \ref ifx_Vector_R_t
 * @param [in]     offset    Offset of elements to take from source
 * @param [in]     length    Number of elements
 * @param [in]     spacing   Stride between consecutive elements in the output vector
 *
 */
IFX_DLL_PUBLIC
void ifx_vec_view_r(ifx_Vector_R_t* vector,
                    ifx_Vector_R_t* source,
                    uint32_t offset,
                    uint32_t length,
                    uint32_t spacing);

/**
 * @brief Assigns complex data from source vector to the destination vector.
 * Example usage:
 *
 *   Given \ref ifx_Vector_C_t dest_vector and \ref ifx_Vector_C_t source_vector,
 *   dimension of the dest_vector length and
 *   offset for source_matrix and spacing/stride between elements one can assign:
 * @code
 *     ifx_vec_view_c(&dest_vector, &source_vector, offset, length, spacing);
 * @endcode
 *   e.g. for taking a view of the second half of the vector:
 * @code
 *     ifx_Complex_t arr_data[4] = {{1, 1}, {2, 2}, {3, 3}, {4, 4}};
 *     ifx_vec_rawview_c(&source_vector, arr_data, 4, 1);
 *
 *     ifx_vec_view_c(&dest_vector, &source_vector, 2, 2, 1);
 * @endcode
 *     will give a vector with values:
 * @code
 *     [{3, 3}, {4, 4}]
 * @endcode
 *
 * @param [in,out] vector    Pointer to the memory containing array defined by \ref ifx_Vector_C_t
 * @param [in]     source    Pointer to the memory containing array defined by \ref ifx_Vector_C_t
 * @param [in]     offset    Offset of elements to take from source
 * @param [in]     length    Number of elements
 * @param [in]     spacing   Stride between consecutive elements in the output vector
 *
 */
IFX_DLL_PUBLIC
void ifx_vec_view_c(ifx_Vector_C_t* vector,
                    ifx_Vector_C_t* source,
                    uint32_t offset,
                    uint32_t length,
                    uint32_t spacing);

/**
 * @brief Allocates memory for a real vector for a specified number of
 *        elements and initializes it to zero.
 *
 * @param [in]     length    Number of elements in the array
 *
 * @return Pointer to allocated and initialized real vector structure or NULL if allocation failed.
 *
 */
IFX_DLL_PUBLIC
ifx_Vector_R_t* ifx_vec_create_r(uint32_t length);

/**
 * @brief Allocates memory for a real vector for a specified number of
 *        elements and initializes it to zero.
 *
 * @param [in]     length    Number of elements in the array
 *
 * @return Pointer to allocated and initialized complex vector structure or NULL if allocation failed.
 *
 */
IFX_DLL_PUBLIC
ifx_Vector_C_t* ifx_vec_create_c(uint32_t length);

/**
 * @brief Clones a real vector \ref ifx_Vector_R_t
 *
 * @param [in]     vector    Pointer to the memory containing array defined by \ref ifx_Vector_R_t
 *
 * @return Cloned real data array \ref ifx_Vector_R_t
 *
 */
IFX_DLL_PUBLIC
ifx_Vector_R_t* ifx_vec_clone_r(const ifx_Vector_R_t* vector);

/**
 * @brief Clones a complex vector array \ref ifx_Vector_C_t
 *
 * @param [in]     vector    Pointer to the memory containing array defined by \ref ifx_Vector_C_t
 *
 * @return Cloned complex data array \ref ifx_Vector_C_t
 *
 */
IFX_DLL_PUBLIC
ifx_Vector_C_t* ifx_vec_clone_c(const ifx_Vector_C_t* vector);

/**
 * @brief Frees the memory allocated for a real vector for a specified
 *        number of elements.
 *
 * @param [in,out] vector    Pointer to the memory containing array defined by \ref ifx_Vector_R_t
 *
 */
IFX_DLL_PUBLIC
void ifx_vec_destroy_r(ifx_Vector_R_t* vector);

/**
 * @brief Frees the memory allocated for a real vector for a specified number of
 *        elements.
 *
 * @param [in,out] vector    Pointer to the memory containing array defined by \ref ifx_Vector_C_t
 *
 */
IFX_DLL_PUBLIC
void ifx_vec_destroy_c(ifx_Vector_C_t* vector);

/**
 * @brief Blits elements of a given real data array, to a new created real data array
 *        with user defined length.
 *
 * @param [in]     vector              Pointer to the memory containing array defined by \ref ifx_Vector_R_t
 * @param [in]     offset              ...
 * @param [in]     length              ...
 * @param [in]     target_offset       ...
 * @param [out]    target              ...
 *
 */
IFX_DLL_PUBLIC
void ifx_vec_blit_r(const ifx_Vector_R_t* vector,
                    uint32_t offset,
                    uint32_t length,
                    uint32_t target_offset,
                    ifx_Vector_R_t* target);

/**
 * @brief Blits elements of a given complex data array, to a new created complex data array
 *        with user defined length.
 *
 * @param [in]     vector              Pointer to the memory containing array defined by \ref ifx_Vector_C_t
 * @param [in]     offset              ...
 * @param [in]     length              ...
 * @param [in]     target_offset       ...
 * @param [out]    target              ...
 *
 */
IFX_DLL_PUBLIC
void ifx_vec_blit_c(const ifx_Vector_C_t* vector,
                    uint32_t offset,
                    uint32_t length,
                    uint32_t target_offset,
                    ifx_Vector_C_t* target);
/**
 * @brief Copies the elements from a real data source array to a real data destination array.
 *
 * @param [in]     vector    Pointer to the memory containing source array defined by \ref ifx_Vector_R_t
 * @param [out]    target    Pointer to the memory containing destination array defined by \ref ifx_Vector_R_t
 *
 */
IFX_DLL_PUBLIC
void ifx_vec_copy_r(const ifx_Vector_R_t* vector,
                    ifx_Vector_R_t* target);

/**
 * @brief Copies the elements from a complex vector source array to a complex vector destination array.
 *
 * @param [in]     vector    Pointer to the memory containing source array defined by \ref ifx_Vector_C_t
 * @param [out]    target    Pointer to the memory containing destination array defined by \ref ifx_Vector_C_t
 *
 */
IFX_DLL_PUBLIC
void ifx_vec_copy_c(const ifx_Vector_C_t* vector,
                    ifx_Vector_C_t* target);

/**
 * @brief Performs rotation on a real vector elements by factor and elements shifted out are fed back to the
 *        vector like a circular ring.
 *        Input and output memories should be of same type and size, else error code is returned.
 *
 * @param [in]     input     Real array on which shift operation needs to be performed
 * @param [in]     shift     Rotation factor (+ive for rotate clockwise, -ive not supported yet)
 *                           elements shifted by this factor in a circular ring manner
 * @param [out]    output    Rotated vector by a given rotation factor
 *
 */
IFX_DLL_PUBLIC
void ifx_vec_copyshift_r(const ifx_Vector_R_t* input,
                         uint32_t shift,
                         ifx_Vector_R_t* output);
/**
 * @brief Performs rotation on a complex vector elements by factor and elements shifted out are fed back to the
 *        vector like a circular ring.
 *        Input and output memories should be of same type and size, else error code is returned.
 *
 * @param [in]     input     Real array on which shift operation needs to be performed
 * @param [in]     shift     Rotation factor (+ive for rotate clockwise, -ive not supported yet)
 *                           elements shifted by this factor in a circular ring manner
 * @param [out]    output    Rotated vector by a given rotation factor
 *
 */
IFX_DLL_PUBLIC
void ifx_vec_copyshift_c(const ifx_Vector_C_t* input,
                         uint32_t shift,
                         ifx_Vector_C_t* output);

/**
 * @brief Performs rotation on a real vector elements by factor
 * and elements shifted out are fed back to the vector like a circular ring.
 * If shift value is 0, the vector data is not touched.
 *
 * Example usage:
 *
 *   Given \ref ifx_Vector_R_t vector and demanded shift one can assign:
 * @code
 *     ifx_vec_shift_r(&vector, shift);
 * @endcode
 *   e.g. for shifting a vector by 2:
 * @code
 *     ifx_Float_t arr_data[6] = {1, 2, 3, 4, 5, 6};
 *     ifx_vec_rawview_r(&source_vector, arr_data, 6, 1);
 *
 *     ifx_vec_shift_r(&vector, 2);
 * @endcode
 *     the vector will have values:
 * @code
 *     [3, 4, 5, 6, 1, 2]
 * @endcode
 *
 * @param [in,out] vector    Pointer to the memory containing array defined by \ref ifx_Vector_R_t
 * @param [in]     shift     Number of vector elements to be shifted
 *
 */
IFX_DLL_PUBLIC
void ifx_vec_shift_r(ifx_Vector_R_t* vector,
                     uint32_t shift);

/**
 * @brief Performs rotation on a complex vector elements by factor
 * and elements shifted out are fed back to the vector like a circular ring.
 * If shift value is 0, the vector data is not touched.
 *
 * Example usage:
 *
 *   Given \ref ifx_Vector_C_t vector and demanded shift one can assign:
 * @code
 *     ifx_vec_shift_c(&vector, shift);
 * @endcode
 *   e.g. for shifting a vector by 1:
 * @code
 *     ifx_Complex_t arr_data[4] = {1,1}, {2,2}, {3,3}, {4,4};
 *     ifx_vec_rawview_c(&source_vector, arr_data, 4, 1);
 *
 *     ifx_vec_shift_c(&vector, 1);
 * @endcode
 *     the vector will have values:
 * @code
 *     [{2,2}, {3,3}, {4,4}, {1,1}]
 * @endcode
 *
 * @param [in,out] vector    Pointer to the memory containing array defined by \ref ifx_Vector_C_t
 * @param [in]     shift     Number of vector elements to be shifted
 *
 */
IFX_DLL_PUBLIC
void ifx_vec_shift_c(ifx_Vector_C_t* vector,
                     uint32_t shift);

/**
 * @brief Sets a real user's defined value for all real vector elements.
 *
 * @param [in,out] vector    Pointer to the memory containing array defined by \ref ifx_Vector_R_t
 * @param [in]     value     User real defined value defined by \see ifx_Float_t
 *
 */
IFX_DLL_PUBLIC
void ifx_vec_setall_r(ifx_Vector_R_t* vector,
                      ifx_Float_t value);

/**
 * @brief Sets a complex user's defined value for all complex vector elements.
 *
 * @param [in,out] vector    Pointer to the memory containing array defined by \ref ifx_Vector_C_t
 * @param [in]     value     User complex defined value defined by \ref ifx_Complex_t
 *
 */
IFX_DLL_PUBLIC
void ifx_vec_setall_c(ifx_Vector_C_t* vector,
                      ifx_Complex_t value);

/**
 * @brief Initializes a Complex Vector using two real vectors.
 *
 * @param [in]     input_real   User complex defined value defined by \ref ifx_Vector_R_t
 * @param [in]     input_imag   User complex defined value defined by \ref ifx_Vector_R_t
 * @param [out]    output       User complex defined value defined by \ref ifx_Vector_C_t
 *
 */
IFX_DLL_PUBLIC
void ifx_vec_complex_c(const ifx_Vector_R_t* input_real,
                       const ifx_Vector_R_t* input_imag,
                       ifx_Vector_C_t* output);

/**
 * @brief Set all values in given range to same given value
 *
 * @param [in,out] vector    Pointer to the memory containing array defined by \ref ifx_Vector_R_t
 * @param [in]     offset    Start position of replacement
 * @param [in]     length    Number of elements in input vector to replace
 * @param [in]     value     Replacement value
 *
 */
IFX_DLL_PUBLIC
void ifx_vec_set_range_r(ifx_Vector_R_t* vector,
                         uint32_t offset,
                         uint32_t length,
                         ifx_Float_t value);

/**
 * @brief Set all values in given range to same given value
 *
 * @param [in,out] vector    Pointer to the memory containing array defined by \ref ifx_Vector_C_t
 * @param [in]     offset    Start position of replacement
 * @param [in]     length    Number of elements in input vector to replace
 * @param [in]     value     Replacement value
 *
 */
IFX_DLL_PUBLIC
void ifx_vec_set_range_c(ifx_Vector_C_t* vector,
                         uint32_t offset,
                         uint32_t length,
                         ifx_Complex_t value);

/**
 * @brief Sets a real user defined value at a given index in a real vector.
 *
 * @param [in,out] vector    Pointer to the memory containing array defined by \ref ifx_Vector_R_t.
 * @param [in]     idx       Location where the value is to be set.
 * @param [in]     value     User real defined value defined by \see ifx_Float_t.
 *
 */
IFX_DLL_PUBLIC
void ifx_vec_setat_r(ifx_Vector_R_t* vector,
                     uint32_t idx,
                     ifx_Float_t value);

/**
 * @brief Sets a real user defined value at a given index in a complex vector.
 *
 * @param [in,out] vector    Pointer to the memory containing array defined by \ref ifx_Vector_C_t.
 * @param [in]     idx       Location where the value is to be set.
 * @param [in]     value     User complex defined value defined by \ref ifx_Complex_t.
 *
 */
IFX_DLL_PUBLIC
void ifx_vec_setat_c(ifx_Vector_C_t* vector,
                     uint32_t idx,
                     ifx_Complex_t value);

/**
 * @brief Computes the sum of values of a real vector.
 *
 * @param [in]     vector    Pointer to the memory containing array defined by \ref ifx_Vector_R_t.
 *
 * @return Sum of all elements of the passed input vector.
 *
 */
IFX_DLL_PUBLIC
ifx_Float_t ifx_vec_sum_r(const ifx_Vector_R_t* vector);

/**
 * @brief Computes the sum of values of a complex vector.
 *
 * @param [in]     vector    Pointer to the memory containing array defined by \ref ifx_Vector_C_t.
 *
 * @return Sum of all elements of the passed input vector.
 *
 */
IFX_DLL_PUBLIC
ifx_Complex_t ifx_vec_sum_c(const ifx_Vector_C_t* vector);

/**
 * @brief Computes the sum of squared values of a given real vector.
 *
 * @param [in]     vector    Pointer to the memory containing array defined by \ref ifx_Vector_R_t.
 *
 * @return Sum of squared values of the passed input vector.
 *
 */
IFX_DLL_PUBLIC
ifx_Float_t ifx_vec_sqsum_r(const ifx_Vector_R_t* vector);

/**
 * @brief Computes the sum of squared values of a given complex vector.
 *
 * @param [in]     vector    Pointer to the memory containing array defined by \ref ifx_Vector_C_t.
 *
 * @return Sum of squared values of the passed input vector.
 *
 */
IFX_DLL_PUBLIC
ifx_Float_t ifx_vec_sqsum_c(const ifx_Vector_C_t* vector);

/**
 * @brief Returns the biggest absolute value of a given real vector.
 *
 * @param [in]     vector    Pointer to the memory containing array defined by \ref ifx_Vector_R_t.
 *
 * @return Maximum absolute value of the passed input vector.
 *
 */
IFX_DLL_PUBLIC
ifx_Float_t ifx_vec_maxabs_r(const ifx_Vector_R_t* vector);

/**
 * @brief Returns the smallest absolute value of a given real vector.
 *
 * @param [in]     vector    Pointer to the memory containing array defined by \ref ifx_Vector_R_t.
 *
 * @return Minimum absolute value of the passed input vector.
 *
 */
IFX_DLL_PUBLIC
ifx_Float_t ifx_vec_minabs_r(const ifx_Vector_R_t* vector);

/**
 * @brief Returns the index of maximum value of a given vector.
 *
 * @param [in]     vector    Pointer to the memory containing array defined by \ref ifx_Vector_R_t.
 *
 * @return Index of maximum value of the passed input vector.
 *
 */
IFX_DLL_PUBLIC
uint32_t ifx_vec_max_idx_r(const ifx_Vector_R_t* vector);

/**
 * @brief Returns the index of minimum value of a given vector.
 *
 * @param [in]     vector    Pointer to the memory containing array defined by \ref ifx_Vector_R_t.
 *
 * @return Index of minimum value of the passed input vector.
 *
 */
IFX_DLL_PUBLIC
uint32_t ifx_vec_min_idx_r(const ifx_Vector_R_t* vector);

/**
 * @brief Returns the index of maximum absolute value of a given complex vector.
 *
 * @param [in]     vector    Pointer to the memory containing array defined by \ref ifx_Vector_C_t.
 *
 * @return Index of Maximum absolute value of the passed input vector.
 *
 */
IFX_DLL_PUBLIC
ifx_Float_t ifx_vec_maxabs_c(const ifx_Vector_C_t* vector);

/**
 * @brief Returns the maximum absolute value of a given complex vector.
 *
 * @param [in]     vector    Pointer to the memory containing array defined by \ref ifx_Vector_C_t.
 *
 * @return Maximum absolute value of the passed input vector.
 *
 */
IFX_DLL_PUBLIC
uint32_t ifx_vec_max_idx_c(const ifx_Vector_C_t* vector);

/**
 * @brief Computes element wise addition of two real vectors and stores it
 *        in result vector. This function allows in-place calculation, that
 *        means a input vector can also be the output vector at the same time
 *        if the input vector can be overwritten.
 *
 * @param [in]     v1        Pointer to operand 1, a real vector of type
 *                           \ref ifx_Vector_R_t.
 * @param [in]     v2        Pointer to operand 2, a real vector of type
 *                           \ref ifx_Vector_R_t.
 * @param [out]    result    Pointer to output vector the sum is stored to.
 *                           Can be one of the input vectors v1 or v2.
 */
IFX_DLL_PUBLIC
void ifx_vec_add_r(const ifx_Vector_R_t* v1,
                   const ifx_Vector_R_t* v2,
                   ifx_Vector_R_t* result);
/**
 * @brief Computes element wise addition of two complex vectors and stores it
 *        in result vector. This function allows in-place calculation, that
 *        means a input vector can also be the output vector at the same time
 *        if the input vector can be overwritten.
 *
 * @param [in]     v1        Pointer to operand 1, a real vector of type
 *                           \ref ifx_Vector_C_t.
 * @param [in]     v2        Pointer to operand 2, a real vector of type
 *                           \ref ifx_Vector_C_t.
 * @param [out]    result    Pointer to output vector the sum is stored to.
 *                           Can be one of the input vectors v1 or v2.
 */
IFX_DLL_PUBLIC
void ifx_vec_add_c(const ifx_Vector_C_t* v1,
                   const ifx_Vector_C_t* v2,
                   ifx_Vector_C_t* result);
/**
 * @brief Computes element wise real vector subtraction result = v1 - v2.
 *
 * @param [in]     v1        Pointer to left hand operand \ref ifx_Vector_R_t
 *                           on which element wise subtraction is applied to.
 * @param [in]     v2        Pointer to right hand operand \ref ifx_Vector_R_t
 *                           which elements are subtracted from v1.
 * @param [out]    result    Pointer to result vector \ref ifx_Vector_R_t
 *                           containing the results of element wise subtraction.
 */
IFX_DLL_PUBLIC
void ifx_vec_sub_r(const ifx_Vector_R_t* v1,
                   const ifx_Vector_R_t* v2,
                   ifx_Vector_R_t* result);
/**
 * @brief Computes element wise real vector subtraction result = v1 - v2.
 *
 * @param [in]     v1        Pointer to left hand operand \ref ifx_Vector_C_t
 *                           on which element wise subtraction is applied to.
 * @param [in]     v2        Pointer to right hand operand \ref ifx_Vector_C_t
 *                           which elements are subtracted from v1.
 * @param [out]    result    Pointer to result vector \ref ifx_Vector_C_t
 *                           containing the results of element wise subtraction.
 */
IFX_DLL_PUBLIC
void ifx_vec_sub_c(const ifx_Vector_C_t* v1,
                   const ifx_Vector_C_t* v2,
                   ifx_Vector_C_t* result);
/**
 * @brief Computes element wise multiplication two real vectors.
 *
 * @param [in]     v1        Pointer to first operand \ref ifx_Vector_R_t
 *                           on which element wise multiplication is applied to.
 * @param [in]     v2        Pointer to first operand \ref ifx_Vector_R_t
 *                           on which element wise multiplication is applied to.
 * @param [out]    result    Pointer to result vector \ref ifx_Vector_R_t
 *                           containing the results of element wise multiplication.
 */
IFX_DLL_PUBLIC
void ifx_vec_mul_r(const ifx_Vector_R_t* v1,
                   const ifx_Vector_R_t* v2,
                   ifx_Vector_R_t* result);

/**
 * @brief Computes element wise multiplication two complex vectors.
 *
 * @param [in]     v1        Pointer to first operand \ref ifx_Vector_C_t
 *                           on which element wise multiplication is applied to.
 * @param [in]     v2        Pointer to first operand \ref ifx_Vector_C_t
 *                           on which element wise multiplication is applied to.
 * @param [out]    result    Pointer to result vector \ref ifx_Vector_C_t
 *                           containing the results of element wise multiplication.
 */
IFX_DLL_PUBLIC
void ifx_vec_mul_c(const ifx_Vector_C_t* v1,
                   const ifx_Vector_C_t* v2,
                   ifx_Vector_C_t* result);

/**
 * @brief Computes element wise multiplication two (complex & real) vectors.
 *
 * @param [in]     v1        Pointer to first operand \ref ifx_Vector_C_t
 *                           on which element wise multiplication is applied to.
 * @param [in]     v2        Pointer to first operand \ref ifx_Vector_R_t
 *                           on which element wise multiplication is applied to.
 * @param [out]    result    Pointer to result vector \ref ifx_Vector_C_t
 *                           containing the results of element wise multiplication.
 */
IFX_DLL_PUBLIC
void ifx_vec_mul_cr(const ifx_Vector_C_t* v1,
                    const ifx_Vector_R_t* v2,
                    ifx_Vector_C_t* result);

/**
 * @brief Computes absolute array (modulus) from Real input data. Absolute = |data|
 *
 * @param [in]     input     Pointer to data memory defined by \ref ifx_Vector_R_t
 *                           on which Absolute operation is to be performed.
 * @param [out]    output    Pointer to data memory defined by \ref ifx_Vector_R_t
 *                           containing the absolute values.
 */
IFX_DLL_PUBLIC
void ifx_vec_abs_r(const ifx_Vector_R_t* input,
                   ifx_Vector_R_t* output);
/**
 * @brief Computes absolute array from Complex input data. Absolute = sqrt(real^2 + imag^2)
 *
 * @param [in]     input     Pointer to data memory defined by \ref ifx_Vector_C_t
 *                           on which Absolute operation is to be performed.
 * @param [out]    output    Pointer to data memory defined by \ref ifx_Vector_R_t
 *                           containing the absolute values.
 */
IFX_DLL_PUBLIC
void ifx_vec_abs_c(const ifx_Vector_C_t* input,
                   ifx_Vector_R_t* output);

/**
 * @brief flips real vector.
 * populates vector 'output' with a reverse of the order of the elements in vector 'input'
 *
 * @param [in]     input     Pointer to data memory defined by \ref ifx_Vector_R_t
 *                           which is to be flipped.
 * @param [out]    output    Pointer to data memory defined by \ref ifx_Vector_R_t
 *                           containing the output values.
 */
IFX_DLL_PUBLIC
void ifx_vec_flip_r(const ifx_Vector_R_t* input,
                    ifx_Vector_R_t* output);

/**
 * @brief Removes a scalar value from each sample of a real vector.
 *
 * @param [in]     input               Pointer to data memory defined by \ref ifx_Vector_R_t
 *                                     from which the given scalar is to be subtracted.
 * @param [in]     scalar_value        Floating point scalar.
 * @param [out]    output              Pointer to data memory containing the result of scalar subtraction.
 *                                     In-place operation may be achieved by passing the same pointer for
 *                                     both input and output.
 */
IFX_DLL_PUBLIC
void ifx_vec_sub_rs(const ifx_Vector_R_t* input,
                    ifx_Float_t scalar_value,
                    ifx_Vector_R_t* output);

/**
 * @brief Removes a scalar value from each sample of a complex vector.
 *
 * @param [in]     input               Pointer to data memory defined by \ref ifx_Vector_C_t
 *                                     from which the given scalar is to be subtracted.
 * @param [in]     scalar_value        Floating point scalar.
 * @param [out]    output              Pointer to data memory containing the result of scalar subtraction.
 *                                     In-place operation may be achieved by passing the same pointer for
 *                                     both input and output.
 */
IFX_DLL_PUBLIC
void ifx_vec_sub_cs(const ifx_Vector_C_t* input,
                    ifx_Complex_t scalar_value,
                    ifx_Vector_C_t* output);

/**
 * @brief Computes multiplication of a given real vector by a scalar.
 *
 * @param [in]     input     Pointer to data memory defined by \ref ifx_Vector_R_t.
 * @param [in]     scale     Value by which each element in input array gets multiplied.
 * @param [out]    output    Pointer to the result vector \ref ifx_Vector_R_t.
 *
 */
IFX_DLL_PUBLIC
void ifx_vec_scale_r(const ifx_Vector_R_t* input,
                     ifx_Float_t scale,
                     ifx_Vector_R_t* output);

/**
 * @brief Applies linear complex value scaling to a real vector.
 *
 * Math operation implemented by this method: Z = a * X
 *
 * @param [in]     input     Real vector on which scaling is to be applied (X in above equation).
 * @param [in]     scale     Complex floating point values of scalar (a in above equation).
 * @param [out]    output    Complex vector where the result of scaling will be stored (Z in above equation).
 *                           It must be allocated and of the same dimensions as the given input vector. Can be in-place.
 *
 */
IFX_DLL_PUBLIC
void ifx_vec_scale_rc(const ifx_Vector_R_t* input,
                      ifx_Complex_t scale,
                      ifx_Vector_C_t* output);

/**
 * @brief Computes multiplication of a given complex vector by a scalar.
 *
 * @param [in]     input     Pointer to data memory defined by \ref ifx_Vector_C_t.
 * @param [in]     scale     Value by which each element in input array gets multiplied.
 * @param [out]    output    Pointer to the result vector \ref ifx_Vector_C_t.
 *
 */
IFX_DLL_PUBLIC
void ifx_vec_scale_c(const ifx_Vector_C_t* input,
                     ifx_Complex_t scale,
                     ifx_Vector_C_t* output);

/**
 * @brief Applies linear real value scaling to a complex vector.
 *
 * Math operation implemented by this method: Z = a * X
 *
 * @param [in]     input     Complex vector on which scaling is to be applied (X in above equation).
 * @param [in]     scale     Real floating point values of scalar (a in above equation).
 * @param [out]    output    Complex vector where the result of scaling will be stored (Z in above equation).
 *                           It must be allocated and of the same dimensions as the given input vector. Can be in-place.
 *
 */
IFX_DLL_PUBLIC
void ifx_vec_scale_cr(const ifx_Vector_C_t* input,
                      ifx_Float_t scale,
                      ifx_Vector_C_t* output);

/**
 * @brief Computes the euclidean distance between two n-D vectors v1 and v2 (both being of the same size).
 *
 * @param [in]     v1        Pointer to data memory defined by \ref ifx_Vector_R_t
 *                           containing initial points for euclidean distance calculation.
 * @param [in]     v2        Pointer to data memory defined by \ref ifx_Vector_R_t
 *                           containing terminal points for euclidean distance calculation.
 *
 * @return Euclidean distance between two input vectors v1 and v2
 *
 */
IFX_DLL_PUBLIC
ifx_Float_t ifx_vec_distance_r(const ifx_Vector_R_t* v1,
                               const ifx_Vector_R_t* v2);

/**
 * @brief Applies multiply accumulate (MAC) operation on real vectors.
 *
 * Math operation implemented by this method: result = a + b * scale
 *
 * @param [in]     v1        Real vector to be added i.e. "a" in above equation.
 * @param [in]     v2        Real vector to be scaled i.e. "b" in above equation.
 * @param [in]     scale     Real floating point values of scalar.
 * @param [out]    result    Real vector where the result of scaling will be stored.
 *                           It must be allocated and of the same dimensions as the given input vector. Can be in-place.
 *
 */
IFX_DLL_PUBLIC
void ifx_vec_mac_r(const ifx_Vector_R_t* v1,
                   const ifx_Vector_R_t* v2,
                   ifx_Float_t scale,
                   ifx_Vector_R_t* result);

/**
 * @brief Applies multiply accumulate (MAC) operation on complex vectors.
 *
 * Math operation implemented by this method: result = a + b * scale
 *
 * @param [in]     v1        Complex vector to be added i.e. "a" in above equation
 * @param [in]     v2        Complex vector to be scaled i.e. "b" in above equation
 * @param [in]     scale     Complex floating point values of scalar
 *
 * @param [out]    result    Complex vector where the result of scaling will be stored.
 *                           It must be allocated and of the same dimensions as the given input vector. Can be in-place.
 *
 */
IFX_DLL_PUBLIC
void ifx_vec_mac_c(const ifx_Vector_C_t* v1,
                   const ifx_Vector_C_t* v2,
                   ifx_Complex_t scale,
                   ifx_Vector_C_t* result);

/**
 * @brief Computes the arithmetic mean of a given real vector.
 *
 * @param [in]     vector    Pointer to the memory containing array defined by \ref ifx_Vector_R_t
 *                           from which mean is calculated.
 *
 * @return Mean value of the passed input vector.
 *
 */
IFX_DLL_PUBLIC
ifx_Float_t ifx_vec_mean_r(const ifx_Vector_R_t* vector);

/**
 * @brief Computes the arithmetic mean of a given complex vector.
 *
 * @param [in]     vector    Pointer to the memory containing array defined by \ref ifx_Vector_C_t
 *                           from which mean is calculated.
 *
 * @return Mean value of the passed input vector.
 *
 */
IFX_DLL_PUBLIC
ifx_Complex_t ifx_vec_mean_c(const ifx_Vector_C_t* vector);

/**
 * @brief Returns the maximum value of a real vector.
 *
 * @param [in]     vector    Pointer to data memory defined by \ref ifx_Vector_R_t
 *                           from which max value is extracted.
 *
 * @return Maximum value of the passed input real vector.
 *
 */
IFX_DLL_PUBLIC
ifx_Float_t ifx_vec_max_r(const ifx_Vector_R_t* vector);

/**
 * @brief Computes the variance of a real vector.
 *
 * @param [in]     vector    Pointer to data memory defined by \ref ifx_Vector_R_t
 *                           from which variance is calculated.
 *
 * @return Variance value of the passed input real vector.
 *
 */
IFX_DLL_PUBLIC
ifx_Float_t ifx_vec_var_r(const ifx_Vector_R_t* vector);

/**
 * @brief Finds local maxima with at least threshold value,
 *        and returns the indices at which the peaks occur.
 *
 * [peaks,idx] = findpeaks(vector,'MinPeakHeight',threshold);
 *
 * @param [in]     vector              Pointer to data memory defined by \ref ifx_Vector_R_t.
 * @param [in]     threshold           Min Peak Height Threshold.
 * @param [in]     num_maxima          Max number of local maxima to be identified.
 * @param [out]    maxima_idxs         Pointer to local maxima indices array.
 *
 * @return Number of identified local maxima of the passed input real vector.
 *
 */
IFX_DLL_PUBLIC
uint32_t ifx_vec_local_maxima(const ifx_Vector_R_t* vector,
                              ifx_Float_t threshold,
                              uint32_t num_maxima,
                              uint32_t* maxima_idxs);


/**
 * @brief Clears all elements of real vector defined by \ref ifx_Vector_R_t.
 *
 * @param [in]     vector    Pointer to real vector to be cleared.
 *
 */
IFX_DLL_PUBLIC
void ifx_vec_clear_r(ifx_Vector_R_t* vector);

/**
 * @brief Clears all elements of complex vector defined by \ref ifx_Vector_C_t.
 *
 * @param [in]     vector    Pointer to complex vector to be cleared.
 *
 */
IFX_DLL_PUBLIC
void ifx_vec_clear_c(ifx_Vector_C_t* vector);

/**
 * @brief Populates vector with evenly spaced numbers over a specified interval.
 * Uses inputs 'start','end' and the length of the 'output' vector to populate
 * it with equally spaced values. The starting value is 'start' and the last value is
 * 'end - delta'. The increment/decrement 'delta' is derived as
 * \f[
 * \mathrm{delta} = \frac{(\mathrm{end-start})}{length(\mathrm{output})}
 * \f]
 *
 * @param [in]     start    starting value of linear space
 * @param [in]     end      limiting value of linear space
 * @param [out]    output   Pointer to output vector defined by \ref ifx_Vector_R_t
 *
 */
IFX_DLL_PUBLIC
void ifx_vec_linspace_r(ifx_Float_t start,
                        ifx_Float_t end,
                        ifx_Vector_R_t* output);


/**
 * @brief Computes median
 *
 * Median is defined as value lying in midpoint of values that where previously sorted.
 * If the midpoint is betwean of two values the mean of them is taken as result.
 *
 * This algorithm is complexity is nlog(n) efficient for smaller tables rather
 * hundreds not thousends, it is also not efficient by that could cause worst cases n^2 complexity.
 * Algorithm was not randomise to avoid cornel of specifically sorted data not to add additional
 * overhead for usage in filters.
 *
 * Used algorithm is doing finding of median in place without creating any additional arrays.
 *
 * @param [in]     input     input data
 * @param [in]     offset    start position where fining median
 * @param [in]     length    number of elements from offset that will be taken into
 *                           consideration during median fininding
 * @retval         NaN       if 0 elements on input or error
 * @retval         median    otherwise
 */
IFX_DLL_PUBLIC
ifx_Float_t ifx_vec_median_range_r(const ifx_Vector_R_t* input, uint32_t offset, uint32_t length);

/**
 * @brief Computes median
 *
 * This function is generalization of /ref ::ifx_vec_median_range_r
 * @param [in]     input     input data
 * @retval         NaN       if 0 elements on input or error
 * @retval         median    otherwise
 */
IFX_DLL_PUBLIC
ifx_Float_t ifx_vec_median_r(const ifx_Vector_R_t* input);

/**
 * @brief Compute dot product between two real vectors
 *
 * Compute the dot product between vector v1 and vector v2. Both vectors must
 * have the same length. The dot product is defined as
 * \f$x \cdot y = \sum_j x_j y_j\f$.
 *
 * @param [in]     v1     first vector
 * @param [in]     v2     second vector
 * @retval         dot product beween v1 and v2
 */
IFX_DLL_PUBLIC
ifx_Float_t ifx_vec_dot_r(const ifx_Vector_R_t* v1, const ifx_Vector_R_t* v2);

/**
 * @brief Compute dot product between two real vectors
 *
 * This is a more general version of \ref ifx_vec_dot_r. The dot product is
 * computed as \f$\sum_{j=0}^{\mathrm{len}-1} x_{\alpha+j} y_{\beta+j}\f$, where
 * \f$x\f$ corresponds to v1, \f$y\f$ corresponds to v2, and \f$\alpha,\beta\f$
 * correspond to offset_v1 and offset_v2, respectively.
 *
 * @param [in]     v1           first vector
 * @param [in]     v2           second vector
 * @param [in]     offset_v1    offset of v1
 * @param [in]     offset_v2    offset of v2
 * @param [in]     len          number of elements
 * @retval         dot product beween v1 and v2
 */
IFX_DLL_PUBLIC
ifx_Float_t ifx_vec_dot2_r(const ifx_Vector_R_t* v1, const ifx_Vector_R_t* v2, uint32_t offset_v1, uint32_t offset_v2, uint32_t len);


/**
 * @brief Operates on real value vectors, to perform log base 10 from standard math.
 * If any of the input real values are negative, the output would be a NaN as per the C99 standard.
 *
 * @param [in]     input     Vector of real values as an input
 * @param [out]    output    Vector of real values as output, could be same as input for in-place operation
 *
 */
IFX_DLL_PUBLIC
void ifx_vec_log10_r(const ifx_Vector_R_t* input,
                     ifx_Vector_R_t* output);

/**
 * @brief Operates on complex value vectors, to perform log base 10 of complex numbers from standard math.
 *
 * @param [in]     input     Vector of complex values as an input
 * @param [out]    output    Vector of complex values as output, could be same as input for in-place operation
 *
 */
IFX_DLL_PUBLIC
void ifx_vec_log10_c(const ifx_Vector_C_t* input,
                     ifx_Vector_C_t* output);

/**
 * @brief Operates on real vector arrays, to convert from linear to dB scale.
 *
 * @param [in]     input     Vector of real values as an input
 * @param [in]     scale     For voltage this should be 20 i.e. 20xlog10() and for power 10 i.e. 10xlog10()
 *                           However, this is a generic math function, so scale can be any desired non zero float value
 * @param [out]    output    Vector of real values as output, could be same as input for in-place operation
 *
 */
IFX_DLL_PUBLIC
void ifx_vec_linear_to_dB(const ifx_Vector_R_t* input,
                          ifx_Float_t scale,
                          ifx_Vector_R_t* output);

/**
 * @brief Operates on real vector arrays, to convert from dB to linear scale.
 *
 * @param [in]     input     Vector of real values as an input
 * @param [in]     scale     For voltage this should be 20 i.e. 10^(A/20) and for power 10 i.e. 10^(A/10)
 * @param [out]    output    Vector of real values as output, could be same as input for in-place operation
 *
 */
IFX_DLL_PUBLIC
void ifx_vec_dB_to_linear(const ifx_Vector_R_t* input,
                          ifx_Float_t scale,
                          ifx_Vector_R_t* output);

/**
 * @brief Operates on real vector arrays, to calculate difference deviation in linear scale.
 *
 * Difference deviation is the standard deviation calculated on the difference vector (v2 - v1).
 * Output is a linear value, that can be used to observe the deviation in two vectors.
 *
 * @param [in]     v1     first vector of real values to be subtracted
 * @param [in]     v2     second vector of real values from which the other vectors is subtracted
 *
 * @retval         Difference deviation calculated from difference of two real vectors.
 */
IFX_DLL_PUBLIC
ifx_Float_t ifx_vec_difference_deviation_r(const ifx_Vector_R_t* v1, const ifx_Vector_R_t* v2);

/**
 * @brief Computes squared norm of complex vector.
 *
 * For each element in the vector input compute the square of the absolute
 * value and save it in output: \f$\mathrm{output}_j = |\mathrm{input}_j|^2\f$
 *
 * @param [in]     input     Complex input vector.
 * @param [out]    output    Square of norm of vector input.
 */
IFX_DLL_PUBLIC
void ifx_vec_squared_norm_c(const ifx_Vector_C_t* input, ifx_Vector_R_t* output);

/**
 * @brief Convert a vector containing squared absolute of spectrum to dB
 *
 * Given the squared norm of the spectrum, convert it to dB.
 *
 * The function is equivalent to:
 *   1. Taking the square root of all elements of vec.
 *   2. Clipping all values smaller than threshold to CLIPPING_VALUE.
 *   3. Converting all value to dB using scale.
 *
 * @param [in,out]  vec         squared norm of spectrum
 * @param [in]      scale       scale factor
 * @param [in]      threshold   threshold for clipping
 */
IFX_DLL_PUBLIC
void ifx_vec_spectrum2_to_db(ifx_Vector_R_t* vec, ifx_Float_t scale, ifx_Float_t threshold);

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* IFX_BASE_VECTOR_H */
