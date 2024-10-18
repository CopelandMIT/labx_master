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

/**
 * @file Mda.h
 *
 * \brief \copybrief gr_mda
 *
 * For details refer to \ref gr_mda
 */

#ifndef IFX_MDA_H
#define IFX_MDA_H

#include "Types.h"


#ifdef __cplusplus
#include <cassert>
#include <initializer_list>
#include <tuple>

extern "C"
{
#else
#include <assert.h>
#endif


/** @addtogroup gr_cat_SDK_base
 * @{
 */

/** @defgroup gr_mda Multi-dimensional arrays
 * @brief API for multi-dimensional arrays
 *
 * Supports creating, accessing, and destroying of multi-dimensional arrays.
 *
 * @section sect_mda The multi-dimensional array
 *
 * A multi-dimensional array is a container of items of the same type and the same size. Two containers are
 * available: \ref ifx_Mda_R_t for real values with data type \ref ifx_Float_t, and \ref ifx_Mda_C_t for complex
 * values of data type \ref ifx_Complex_t.
 *
 * The number of dimensions is defined by the members \ref ifx_Mda_R_t.dimensions and \ref ifx_Mda_C_t.dimensions. The
 * allowed indices for each dimension is defined by the member shape. For example, a matrix of dimension \f$M \times N\f$
 * is a multi-dimensional array with dimensions=2, and shape=(M,N).
 *
 * The maximum number of dimensions supported is defined by \ref IFX_MDA_MAX_DIM.
 *
 * @section sect_mda_creating_deleting Creating and deleting arrays
 *
 * Multi-dimensional arrays can either be created using the macros \ref IFX_MDA_CREATE_R and \ref IFX_MDA_CREATE_C,
 * or using the functions \ref ifx_mda_create_r and \ref ifx_mda_create_c. The macros are more convenient than
 * the functions.
 *
 * Creating an array allocates memory. The functions \ref ifx_mda_destroy_r and
 * \ref ifx_mda_destroy_c free the memory allocated a multi-dimensional array.
 *
 * For instance, the following snippet allocates memory for a real \f$5 \times 4\f$
 * matrix and frees the memory again:
 * @code {.C}
 * ifx_Mda_R_t* matrix = IFX_MDA_CREATE_R(5,4);
 * ifx_mda_destroy_r(matrix);
 * @endcode
 * Here is the same snippet but using the functions instead of the macros:
 * @code {.C}
 * const uint32_t dimensions = 2;
 * const uint32_t shape[] = {5, 4};
 * ifx_Mda_R_t* matrix = ifx_mda_create_r(dimensions, shape);
 * ifx_mda_destroy_r(matrix);
 * @endcode
 *
 * @section sect_mda_indexing Indexing
 *
 * You can access individual elements of a multi-dimensional array using the macro \ref IFX_MDA_AT. The macro
 * works for both real and complex arrays. Here is an example to create a \f$4 \times 4\f$ identity matrix:
 * @code {.C}
 * ifx_Mda_R_t* matrix = IFX_MDA_CREATE_R(4,4);
 * for(uint32_t r = 0; r < IFX_MDA_SHAPE(matrix)[0]; r++)
 * {
 *     for(uint32_t c = 0; c < IFX_MDA_SHAPE(matrix)[1]; c++)
 *     {
 *         if(r == c)
 *             IFX_MDA_AT(matrix, r, c) = 1; // diagonal element
 *         else
 *             IFX_MDA_AT(matrix, r, c) = 0; // off-diagonal element
 *     }
 * }
 * ifx_mda_destroy_r(matrix);
 * @endcode
 *
 * Giving less indices than dimensions is undefined behavior. If more indices than
 * dimensions are given only the first dimensions indices are used. Accessing elements
 * out of bounds is undefined behavior.
 *
 * @section sect_mda_views Views
 *
 * It is possible to create views. Views allow a different interpretation of the same data. For instance,
 * it is possible to fix an index for a specific dimension.
 *
 * Views are handled similar to the views in Python's numpy library. Here are a few examples:
 * @code {.C}
 * ifx_Mda_R_t* arr = IFX_MDA_CREATE_R(9,7,5);
 *
 * // First and last dimension are taken completely; the second dimension only consists of the indices 1,3;
 * // corresponds to numpy's: arr[:,1:5:2,:]
 * ifx_Mda_R_t view1;
 * IFX_MDA_VIEW_R(&view1, arr, IFX_MDA_SLICE_FULL(), IFX_MDA_SLICE(1,5,2), IFX_MDA_SLICE_FULL());
 *
 * // Fix first dimension to 1;
 * // corresponds to numpy's: arr[1,:,:]
 * ifx_Mda_R_t view2;
 * IFX_MDA_VIEW_R(&view2, arr, IFX_MDA_INDEX(1), IFX_MDA_SLICE_FULL(), IFX_MDA_SLICE_FULL());
 *
 * ifx_mda_destroy_r(arr);
 * @endcode
 *
 * As long as a view is still in use the original array must not be destroyed.
 *
 * @section sect_mda_memory_layout Internal memory layout
 *
 * A multi-dimensional array is internally a contiguous one-dimensional array. The
 * ranges in which the indices of the different dimensions can vary is specified by the array shape. The
 * computation of a tuple of indices to the offset of the internal one-dimensional array is
 * done via strided a indexing scheme. The offset in bytes for indices \f$(n_0, n_1, \dots, n_{N-1})\f$
 * is computed using
 * \f[
 * n_\mathrm{offset} = \sum_{j=0}^{N-1} n_j s_j
 * \f]
 * where \f$s_j\f$ denotes the stride for dimension \f$j\f$, and \f$N\f$ denotes the number of dimensions.
 *
 * While multiple striding schemes exist, this library uses row-major order (also called C order), see for instance
 * <a href="https://en.wikipedia.org/wiki/Row-_and_column-major_order">Wikipedia</a>. In row-major order
 * the matrix
 * \f[
 * M = \left(\begin{array}{ccc}
 *   1 & 2 & 3 \\
 *   4 & 5 & 6 \\
 *   7 & 8 & 9
 * \end{array}\right)
 * \f]
 * corresponds to the array \f$(1,2,3,4,5,6,7,8,9)\f$.
 *
 * @{
 */

/**
 * @brief Maximum number of dimensions supported.
 */
#define IFX_MDA_MAX_DIM 8

/**
 * @brief Internal macro to get the number of elements in a variadic macro
 */
#ifdef __cplusplus
/**
 * @brief Internal macro to get the number of elements in a variadic macro
 */
#define IFX_MDA_NUMARGS(...) std::tuple_size<decltype(std::make_tuple(__VA_ARGS__))>::value

/**
 * @brief Internal macro to convert variadic macro to array
 */
#define IFX_MDA_TO_ARRAY(type, ...) std::initializer_list<type> {__VA_ARGS__}.begin()
#else
/**
 * @brief Internal macro to get the number of elements in a variadic macro
 */
#define IFX_MDA_NUMARGS(...)        (sizeof((uint32_t[]) {__VA_ARGS__}) / sizeof(uint32_t))

/**
 * @brief Internal macro to convert variadic macro to array
 */
#define IFX_MDA_TO_ARRAY(type, ...) ((const uint32_t[]) {__VA_ARGS__})
#endif

/**
 * @brief Mask in member flags for owns_data
 */
#define IFX_MDA_FLAG_OWNS_DATA 1

typedef struct
{
    /** Number of dimensions */
    uint32_t dimensions;

    /** Pointer to memory containing data values */
    ifx_Float_t* data;

    /** Shape of array */
    uint32_t shape[IFX_MDA_MAX_DIM];

    /** Strides */
    size_t stride[IFX_MDA_MAX_DIM];

    /** Flags */
    uint32_t flags;
} ifx_Mda_R_t;

typedef struct
{
    /** Number of dimensions */
    uint32_t dimensions;

    /** Pointer to memory containing data values */
    ifx_Complex_t* data;

    /** Shape; first dimensions elements must be positive, subsequent elements must be 0 */
    uint32_t shape[IFX_MDA_MAX_DIM];

    /** Strides; first dimensions elements must be positive, subsequent elements must be 0 */
    size_t stride[IFX_MDA_MAX_DIM];

    /** Flags */
    uint32_t flags;
} ifx_Mda_C_t;

/**
 * @brief Define a slice
 *
 * This is equivalent to Python's slicing format start:stop:step, e.g., array[start:stop:step].
 *
 * If stop=step=0, the interpretation is fixing a value for the dimension.
 *
 * If start=stop=0 and step=1, the interpretation is to take the full documentation, equivalent to
 * Python's :.
 */
typedef struct
{
    uint32_t start; /**< start value of slice */
    uint32_t stop;  /**< end value of slice */
    uint32_t step;  /**< step */
} ifx_mda_slice_t;

/**
 * @brief Macro that evaluates true if both arrays have the same shape.
 */
#define IFX_MDA_SAME_SHAPE(a, b) (memcmp(IFX_MDA_SHAPE(a), IFX_MDA_SHAPE(b), sizeof(IFX_MDA_SHAPE(a))) == 0)

/**
 * @brief Returns offset for array arr and given indices.
 */
#define IFX_MDA_OFFSET(arr, ...) ifx_mda_offset(IFX_MDA_DIMENSIONS(arr), IFX_MDA_STRIDE(arr), IFX_MDA_TO_ARRAY(uint32_t, __VA_ARGS__))

/**
 * @brief Access element of array arr with given indices.
 */
#define IFX_MDA_AT(arr, ...) ((arr)->data[IFX_MDA_OFFSET((arr), __VA_ARGS__)])

/**
 * @brief Get number of dimensions.
 */
#define IFX_MDA_DIMENSIONS(arr) ((arr)->dimensions)

/**
 * @brief Get array with strides.
 */
#define IFX_MDA_STRIDE(arr) ((arr)->stride)

/**
 * @brief Get array with shape.
 */
#define IFX_MDA_SHAPE(arr) ((arr)->shape)

/**
 * @brief Get pointer to internal one-dimensional array.
 */
#define IFX_MDA_DATA(arr) ((arr)->data)

/**
 * @brief Get value of flags.
 */
#define IFX_MDA_FLAGS(arr) ((arr)->flags)

/**
 * @brief True if array has ownership of data.
 */
#define IFX_MDA_OWNS_DATA(arr) (((arr)->flags) & IFX_MDA_FLAG_OWNS_DATA)

/**
 * @brief Compute offset for array with given indices.
 *
 * In most cases it is more convenient to use the macro \ref IFX_MDA_AT.
 *
 * @param dimensions Number of dimensions.
 * @param stride Array with stride; must have at least dimensions of elements.
 * @param indices Array of elements; must have at least dimensions of elements.
 * @return offset Offset in internal one-dimensional memory.
 */
static inline size_t ifx_mda_offset(const uint32_t dimensions, const size_t* stride, const uint32_t* indices)
{
    size_t offset = 0;
    for (uint32_t i = 0; i < dimensions; i++)
        offset += stride[i] * indices[i];

    return offset;
}

/**
 * @brief Create multi-dimensional array with real values.
 *
 * The arguments specify the shape. For instance, to create a multi-dimensional array
 * of shape (3,4,5):
 * @code {.C}
 * ifx_Mda_R_t* array = IFX_MDA_CREATE_R(3,4,5);
 * @endcode
 *
 * \warning The macro evaluates the shape twice. For this reason side effects
 * must be avoided. For example, in the line
 * \code {.C}
 * ifx_Mda_R_t* array = IFX_MDA_CREATE_R(3,4,i++);
 * \endcode
 * the variable `i` will be incremented twice.
 *
 * On failure the returned pointer is NULL.
 */
#define IFX_MDA_CREATE_R(...) ifx_mda_create_r(IFX_MDA_NUMARGS(__VA_ARGS__), IFX_MDA_TO_ARRAY(uint32_t, __VA_ARGS__))

/**
 * @brief Create multi-dimensional array with complex values.
 *
 * The arguments specify the shape. For instance, to create a multi-dimensional array
 * of shape (3,4,5):
 * @code {.C}
 * ifx_Mda_C_t* array = IFX_MDA_CREATE_C(3,4,5);
 * @endcode
 *
 * \warning The macro evaluates the shape twice. For this reason side effects
 * must be avoided. For example, in the line
 * \code {.C}
 * ifx_Mda_C_t* array = IFX_MDA_CREATE_C(3,4,i++);
 * \endcode
 * the variable `i` will be incremented twice.
 *
 * On failure the returned pointer is NULL.
 */
#define IFX_MDA_CREATE_C(...) ifx_mda_create_c(IFX_MDA_NUMARGS(__VA_ARGS__), IFX_MDA_TO_ARRAY(uint32_t, __VA_ARGS__))

/**
 * @brief Use full dimension.
 *
 * Corresponds to
 * @code {.Python}
 * arr[..., :, ...]
 * @endcode in Python.
 */
#define IFX_MDA_SLICE_FULL() \
    {                        \
        0, 0, 1              \
    }

/**
 * @brief Fix dimension to ind.
 *
 * Corresponds to
 * @code {.Python}
 * arr[..., ind, ....]
 * @endcode
 * in Python.
 */
#define IFX_MDA_INDEX(ind) \
    {                      \
        (ind), 0, 0        \
    }

/**
 * @brief Specify slice.
 *
 * Corresponds to
 * @code {.Python}
 arr[..., start:stop:step, ....]
 * @endcode
 * in Python.
 */
#define IFX_MDA_SLICE(start, stop, step) \
    {                                    \
        (start), (stop), (step)          \
    }

/**
 * @brief Create real view.
 *
 * See section \ref sect_mda_views for more details.
 */
#define IFX_MDA_VIEW_R(view, orig, ...)                                  \
    do                                                                   \
    {                                                                    \
        const ifx_mda_slice_t slices_[] = {__VA_ARGS__};                 \
        const size_t num_slices_ = sizeof(slices_) / sizeof(slices_[0]); \
        ifx_mda_view_r((view), (orig), num_slices_, slices_);            \
    } while (0)

/**
 * @brief Create complex view.
 *
 * See section \ref sect_mda_views for more details.
 */
#define IFX_MDA_VIEW_C(view, orig, ...)                                  \
    do                                                                   \
    {                                                                    \
        const ifx_mda_slice_t slices_[] = {__VA_ARGS__};                 \
        const size_t num_slices_ = sizeof(slices_) / sizeof(slices_[0]); \
        ifx_mda_view_c((view), (orig), num_slices_, slices_);            \
    } while (0)

/**
 * @brief Create real multi-dimensional array.
 *
 * Typically it is more convenient to use the macro \ref IFX_MDA_CREATE_R.
 *
 * @param dimensions Number of dimensions.
 * @param shape Array with shape; must have at least dimensions of elements.
 * @return array    Newly created array.
 */
IFX_DLL_PUBLIC ifx_Mda_R_t* ifx_mda_create_r(uint32_t dimensions, const uint32_t shape[]);

/**
 * @brief Create complex multi-dimensional array.
 *
 * Typically it is more convenient to use the macro \ref IFX_MDA_CREATE_C.
 *
 * @param dimensions Number of dimensions.
 * @param shape Array with shape; must have at least dimensions of elements.
 * @return array    Newly created array.
 */
IFX_DLL_PUBLIC ifx_Mda_C_t* ifx_mda_create_c(uint32_t dimensions, const uint32_t shape[]);

/**
 * @brief Destroy real array.
 *
 * Free memory allocated for array mda.
 *
 * If mda is NULL, no operation is performed.
 *
 * @param mda Pointer to array
 */
IFX_DLL_PUBLIC void ifx_mda_destroy_r(ifx_Mda_R_t* mda);

/**
 * @brief Destroy complex array.
 *
 * Free memory allocated for array mda.
 *
 * If mda is NULL, no operation is performed.
 *
 * @param mda Pointer to array
 */
IFX_DLL_PUBLIC void ifx_mda_destroy_c(ifx_Mda_C_t* mda);

/**
 * @brief Create real view.
 *
 * @param view Pointer to view.
 * @param orig Original multi-dimensional array.
 * @param num_slices Number of elements of slices.
 * @param slices Array of slices.
 */
IFX_DLL_PUBLIC void ifx_mda_view_r(ifx_Mda_R_t* view, const ifx_Mda_R_t* orig, size_t num_slices, const ifx_mda_slice_t slices[]);

/**
 * @brief Create complex view.
 *
 * @param view Pointer to view.
 * @param orig Original multi-dimensional array.
 * @param num_slices Number of elements of slices.
 * @param slices Array of slices.
 */
IFX_DLL_PUBLIC void ifx_mda_view_c(ifx_Mda_C_t* view, const ifx_Mda_C_t* orig, size_t num_slices, const ifx_mda_slice_t slices[]);

/**
 * @brief Return true if memory is contiguous.
 *
 * @param mda array
 * @return true if contiguous
 * @return false otherwise
 */
IFX_DLL_PUBLIC bool ifx_mda_is_contiguous_r(const ifx_Mda_R_t* mda);

/**
 * @brief Return true if memory is contiguous.
 *
 * @param mda array
 * @return true if contiguous
 * @return false otherwise
 */
IFX_DLL_PUBLIC bool ifx_mda_is_contiguous_c(const ifx_Mda_C_t* mda);

/**
 * @brief Return number of elements of array.
 *
 * @param mda array
 * @return elements
 */
IFX_DLL_PUBLIC size_t ifx_mda_elements_r(const ifx_Mda_R_t* mda);

/**
 * @brief Return number of elements of array.
 *
 * @param mda array
 * @return elements
 */
IFX_DLL_PUBLIC size_t ifx_mda_elements_c(const ifx_Mda_C_t* mda);

/**
 * @brief Set all elements in array to value.
 *
 * @param mda array
 * @param value value
 */
IFX_DLL_PUBLIC void ifx_mda_setall_r(ifx_Mda_R_t* mda, ifx_Float_t value);

/**
 * @brief Set all elements in array to value.
 *
 * @param mda array
 * @param value value
 */
IFX_DLL_PUBLIC void ifx_mda_setall_c(ifx_Mda_C_t* mda, ifx_Complex_t value);

/**
 * @brief Copy array src to dest.
 *
 * src and dest must have the same shapes.
 *
 * @param src  source array
 * @param dest destination array
 */
IFX_DLL_PUBLIC void ifx_mda_copy_r(const ifx_Mda_R_t* src, ifx_Mda_R_t* dest);

/**
 * @brief Copy array src to dest.
 *
 * src and dest must have the same shapes.
 *
 * @param src  source array
 * @param dest destination array
 */
IFX_DLL_PUBLIC void ifx_mda_copy_c(const ifx_Mda_C_t* src, ifx_Mda_C_t* dest);

/**
 * @brief Create copy of array.
 *
 * Return a copy of the array mda. The caller is responsible to free the
 * returned array by calling \ref ifx_mda_destroy_r.
 *
 * @param mda array
 * @return copy copy of array mda
 */
IFX_DLL_PUBLIC ifx_Mda_R_t* ifx_mda_clone_r(const ifx_Mda_R_t* mda);

/**
 * @brief Create copy of array.
 *
 * Return a copy of the array mda. The caller is responsible to free the
 * returned array by calling \ref ifx_mda_destroy_c.
 *
 * @param mda array
 * @return copy copy of array mda
 */
IFX_DLL_PUBLIC ifx_Mda_C_t* ifx_mda_clone_c(const ifx_Mda_C_t* mda);

/**
 * @brief Create a raw view of a real muti-dimensional array.
 *
 * Initialize the multi-dimensional array mda with the given parameters.
 *
 * @param [out] mda         multi-dimensional array
 * @param [in] data         pointer to data
 * @param [in] dimensions   number of dimensions
 * @param [in] shape        array with shape (must have at least dimensions of elements)
 * @param [in] stride       array with stride (must have at least dimensions of elements)
 * @param [in] flags        flags
 */
IFX_DLL_PUBLIC void ifx_mda_rawview_r(ifx_Mda_R_t* mda, ifx_Float_t* data, uint32_t dimensions, const uint32_t* shape, const size_t* stride, uint32_t flags);

/**
 * @brief Create a raw view of a complex muti-dimensional array.
 *
 * Initialize the multi-dimensional array mda with the given parameters.
 *
 * @param [out] mda         multi-dimensional array
 * @param [in] data         pointer to data
 * @param [in] dimensions   number of dimensions
 * @param [in] shape        array with shape (must have at least dimensions of elements)
 * @param [in] stride       array with stride (must have at least dimensions of elements)
 * @param [in] flags        flags
 */
IFX_DLL_PUBLIC void ifx_mda_rawview_c(ifx_Mda_C_t* mda, ifx_Complex_t* data, uint32_t dimensions, const uint32_t* shape, const size_t* stride, uint32_t flags);

/**
 * @brief Clear multi-dimensional array.
 *
 * The function will set all elements to 0.
 *
 * @param mda         multi-dimensional array
 */
IFX_DLL_PUBLIC void ifx_mda_clear_r(ifx_Mda_R_t* mda);

/**
 * @brief Clear multi-dimensional array.
 *
 * The function will set all elements to 0+0j.
 *
 * @param mda         multi-dimensional array
 */
IFX_DLL_PUBLIC void ifx_mda_clear_c(ifx_Mda_C_t* mda);

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* IFX_MDA_H */
