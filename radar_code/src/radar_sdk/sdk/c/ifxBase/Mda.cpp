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

#include <cstring>
#include <functional>

#include "Complex.h"
#include "Error.h"
#include "internal/Mda.hpp"
#include "Mda.h"
#include "Mem.h"

/**
 * @brief Compute a*b and check for overflows.
 *
 * Compute multiplication a*b. If an overflow occurs, *overflow is set to true.
 * Otherwise overflow is not written to.
 *
 * @param[in] a 1st multiplicand
 * @param[in] b 2nd multiplicand
 * @param[out] overflow true on overflow, otherwise overflow is not written
 * @return product a*b
 */
static inline size_t mul_ovf(const size_t a, const size_t b, bool* overflow)
{
    const size_t x = a * b;
    if (a != 0 && x / a != b)
        *overflow = true;
    return x;
}

/**
 * @brief Compute required size for data
 *
 * @param[in] dimensions Number of dimensions
 * @param[in] shape Array with shape; must have dimensions of elements
 * @param[in] size_element Size in bytes of one element
 * @param[out] overflow true on overflow, otherwise false
 * @return data size
 */
static inline size_t compute_data_size(const uint32_t dimensions, const uint32_t shape[], const size_t size_element, bool* overflow)
{
    *overflow = false;
    if (dimensions == 0)
        return 0;

    size_t size = 1;
    for (uint32_t dim = 0; dim < dimensions; dim++)
        size = mul_ovf(size, shape[dim], overflow);

    return mul_ovf(size, size_element, overflow);
}

template <class MDA_TYPE>
static inline MDA_TYPE* mda_create(const uint32_t dimensions, const uint32_t shape[])
{
    IFX_ERR_BRV_NULL(shape, nullptr);
    IFX_ERR_BRV_COND(dimensions > IFX_MDA_MAX_DIM, IFX_ERROR_ARGUMENT_OUT_OF_BOUNDS, nullptr);

    // Allocate memory for structure; it is important that the memory is initialized with zeros
    auto* mda = static_cast<MDA_TYPE*>(ifx_mem_calloc(sizeof(MDA_TYPE), 1));
    IFX_ERR_BRV_MEMALLOC(mda, nullptr);

    using dtype = decltype(IFX_MDA_DATA(mda)[0]);  // type of element, i.e. ifx_Float_t or ifx_Complex_t
    using dtype_p = decltype(mda->data);           // pointer type of element, i.e. ifx_Float_t* or ifx_Complex_t*

    // Compute requireed size for data
    bool overflow = false;
    const size_t data_size = compute_data_size(dimensions, shape, sizeof(dtype), &overflow);
    if (overflow)
    {
        ifx_mem_free(mda);
        ifx_error_set(IFX_ERROR_MEMORY_ALLOCATION_FAILED);
        return nullptr;
    }

    // Allocate memory for data
    IFX_MDA_DATA(mda) = static_cast<dtype_p>(ifx_mem_aligned_alloc(data_size, IFX_MEMORY_ALIGNMENT));
    if (!mda->data)
    {
        ifx_mem_free(mda);
        ifx_error_set(IFX_ERROR_MEMORY_ALLOCATION_FAILED);
        return nullptr;
    }

    // Initialize dimensions
    IFX_MDA_DIMENSIONS(mda) = dimensions;

    // Copy shape
    std::memcpy(IFX_MDA_SHAPE(mda), shape, sizeof(uint32_t) * dimensions);

    // The object owns the memory of data
    IFX_MDA_FLAGS(mda) |= IFX_MDA_FLAG_OWNS_DATA;

    // Initialize stride
    {
        size_t offset = 1;
        for (uint32_t dim = 0; dim < dimensions; dim++)
        {
            const uint32_t k = dimensions - 1 - dim;
            IFX_MDA_STRIDE(mda)[k] = offset;
            offset *= IFX_MDA_SHAPE(mda)[k];
        }
    }

    return mda;
}

ifx_Mda_R_t* ifx_mda_create_r(const uint32_t dimensions, const uint32_t shape[])
{
    return mda_create<ifx_Mda_R_t>(dimensions, shape);
}

ifx_Mda_C_t* ifx_mda_create_c(const uint32_t dimensions, const uint32_t shape[])
{
    return mda_create<ifx_Mda_C_t>(dimensions, shape);
}

template <class MDA_TYPE>
static inline void ifx_mda_destroy(MDA_TYPE mda)
{
    if (!mda)
        return;

    if (IFX_MDA_OWNS_DATA(mda))
        ifx_mem_aligned_free(IFX_MDA_DATA(mda));

    ifx_mem_free(mda);
}

void ifx_mda_destroy_r(ifx_Mda_R_t* mda)
{
    ifx_mda_destroy(mda);
}

void ifx_mda_destroy_c(ifx_Mda_C_t* mda)
{
    ifx_mda_destroy(mda);
}

template <class MDA_TYPE>
static inline void mda_view(MDA_TYPE* view, const MDA_TYPE* orig, const size_t num_slices, const ifx_mda_slice_t slices[])
{
    IFX_ERR_BRK_NULL(view);
    IFX_ERR_BRK_NULL(orig);
    IFX_ERR_BRK_NULL(slices);

    // initialize view with zeros
    std::memset(view, 0, sizeof(MDA_TYPE));

    IFX_ERR_BRK_COND(num_slices != IFX_MDA_DIMENSIONS(orig), IFX_ERROR_DIMENSION_MISMATCH);

    // check
    for (uint32_t dim = 0; dim < IFX_MDA_DIMENSIONS(orig); dim++)
    {
        const uint32_t start = slices[dim].start;
        const uint32_t stop = slices[dim].stop;
        const uint32_t step = slices[dim].step;

        if (stop == 0 && step == 0)
            continue;  // equivalent to index
        if (start == 0 && stop == 0 && step == 1)
            continue;  // equivalent to :

        if (stop <= start || step == 0)
        {
            ifx_error_set(IFX_ERROR_DIMENSION_MISMATCH);
            return;
        }

        if (stop > IFX_MDA_SHAPE(orig)[dim])
        {
            ifx_error_set(IFX_ERROR_DIMENSION_MISMATCH);
            return;
        }
    }

    // data
    {
        uint32_t indices[IFX_MDA_MAX_DIM] = {0};
        for (uint32_t dim = 0; dim < IFX_MDA_DIMENSIONS(orig); dim++)
        {
            indices[dim] = slices[dim].start;
        }

        const auto dimensions = IFX_MDA_DIMENSIONS(orig);
        const auto* stride = IFX_MDA_STRIDE(orig);
        IFX_MDA_DATA(view) = IFX_MDA_DATA(orig) + ifx_mda_offset(dimensions, stride, indices);
    }

    IFX_MDA_FLAGS(view) &= ~IFX_MDA_FLAG_OWNS_DATA;

    // stride and shape
    // set stride and and shape to match with the view
    {
        uint32_t dimensions = 0;
        for (uint32_t dim = 0; dim < IFX_MDA_DIMENSIONS(orig); dim++)
        {
            uint32_t start = slices[dim].start;
            uint32_t stop = slices[dim].stop;
            uint32_t step = slices[dim].step;

            if (start == 0 && stop == 0 && step == 1)
            {
                start = 0;
                stop = IFX_MDA_SHAPE(orig)[dim];
                step = 1;
            }

            if (!(stop == 0 && step == 0))
            {
                IFX_MDA_SHAPE(view)
                [dimensions] = 1 + (stop - start - 1) / step;
                IFX_MDA_STRIDE(view)[dimensions] = IFX_MDA_STRIDE(orig)[dim] * step;
                dimensions++;
            }
        }

        IFX_MDA_DIMENSIONS(view) = dimensions;
    }
}

void ifx_mda_view_r(ifx_Mda_R_t* view, const ifx_Mda_R_t* orig, const size_t num_slices, const ifx_mda_slice_t slices[])
{
    mda_view(view, orig, num_slices, slices);
}

void ifx_mda_view_c(ifx_Mda_C_t* view, const ifx_Mda_C_t* orig, const size_t num_slices, const ifx_mda_slice_t slices[])
{
    mda_view(view, orig, num_slices, slices);
}

template <class MDA_TYPE>
static inline bool mda_is_contiguous(const MDA_TYPE* mda)
{
    IFX_ERR_BRV_NULL(mda, false);

    const uint32_t dimensions = IFX_MDA_DIMENSIONS(mda);

    size_t p = 1;
    for (uint32_t dim = 0; dim < dimensions; dim++)
    {
        const uint32_t j = dimensions - 1 - dim;
        if (IFX_MDA_STRIDE(mda)[j] != p)
            return false;

        p *= IFX_MDA_SHAPE(mda)[j];
    }

    return true;
}

bool ifx_mda_is_contiguous_r(const ifx_Mda_R_t* mda)
{
    return mda_is_contiguous(mda);
}

bool ifx_mda_is_contiguous_c(const ifx_Mda_C_t* mda)
{
    return mda_is_contiguous(mda);
}

size_t ifx_mda_elements_r(const ifx_Mda_R_t* mda)
{
    return mda_elements(mda);
}

size_t ifx_mda_elements_c(const ifx_Mda_C_t* mda)
{
    return mda_elements(mda);
}

template <class MDA_TYPE, class DTYPE>
static inline void mda_setall(MDA_TYPE* mda, const DTYPE value)
{
    IFX_ERR_BRK_NULL(mda);

    const auto f = [mda, value](size_t offset, const uint32_t* /* indices */) {
        IFX_MDA_DATA(mda)
        [offset] = value;
        return true;
    };

    iterate(mda, f);
}

void ifx_mda_setall_r(ifx_Mda_R_t* mda, const ifx_Float_t value)
{
    mda_setall(mda, value);
}

void ifx_mda_setall_c(ifx_Mda_C_t* mda, const ifx_Complex_t value)
{
    mda_setall(mda, value);
}

template <class MDA_TYPE>
static inline void mda_copy(const MDA_TYPE* src, MDA_TYPE* dest)
{
    IFX_ERR_BRK_NULL(src);
    IFX_ERR_BRK_NULL(dest);
    IFX_ERR_BRK_COND(!IFX_MDA_SAME_SHAPE(src, dest), IFX_ERROR_DIMENSION_MISMATCH);

    const IterFunc f = [src, dest](size_t offset, const uint32_t* /* indices */) {
        IFX_MDA_DATA(dest)[offset] = IFX_MDA_DATA(src)[offset];
        return true;
    };

    iterate(src, f);
}

void ifx_mda_copy_r(const ifx_Mda_R_t* src, ifx_Mda_R_t* dest)
{
    mda_copy(src, dest);
}

void ifx_mda_copy_c(const ifx_Mda_C_t* src, ifx_Mda_C_t* dest)
{
    mda_copy(src, dest);
}

template <class MDA_TYPE>
MDA_TYPE* mda_clone(const MDA_TYPE* mda)
{
    IFX_ERR_BRV_NULL(mda, nullptr);

    auto* clone = mda_create<MDA_TYPE>(IFX_MDA_DIMENSIONS(mda), IFX_MDA_SHAPE(mda));
    if (!clone)
        return nullptr;

    mda_copy(mda, clone);

    return clone;
}

ifx_Mda_R_t* ifx_mda_clone_r(const ifx_Mda_R_t* mda)
{
    return mda_clone(mda);
}

ifx_Mda_C_t* ifx_mda_clone_c(const ifx_Mda_C_t* mda)
{
    return mda_clone(mda);
}

template <class MDA_TYPE, class DTYPE>
void mda_rawview(MDA_TYPE* mda, DTYPE* data, uint32_t dimensions, const uint32_t* shape, const size_t* stride, uint32_t flags)
{
    IFX_ERR_BRK_NULL(mda);
    IFX_ERR_BRK_NULL(data);
    IFX_ERR_BRK_NULL(shape);
    IFX_ERR_BRK_NULL(stride);
    IFX_ERR_BRK_COND(dimensions == 0 || dimensions > IFX_MDA_MAX_DIM, IFX_ERROR_ARGUMENT_INVALID);

    // dimensions
    IFX_MDA_DIMENSIONS(mda) = dimensions;

    // data
    IFX_MDA_DATA(mda) = data;

    // shape
    std::memset(IFX_MDA_SHAPE(mda), 0, sizeof(IFX_MDA_SHAPE(mda)));
    std::memcpy(IFX_MDA_SHAPE(mda), shape, dimensions * sizeof(uint32_t));

    // stride
    std::memset(IFX_MDA_STRIDE(mda), 0, sizeof(IFX_MDA_STRIDE(mda)));
    std::memcpy(IFX_MDA_STRIDE(mda), stride, dimensions * sizeof(size_t));

    // flags
    IFX_MDA_FLAGS(mda) = flags;
}

void ifx_mda_rawview_r(ifx_Mda_R_t* mda, ifx_Float_t* data, uint32_t dimensions, const uint32_t* shape, const size_t* stride, uint32_t flags)
{
    return mda_rawview(mda, data, dimensions, shape, stride, flags);
}

void ifx_mda_rawview_c(ifx_Mda_C_t* mda, ifx_Complex_t* data, uint32_t dimensions, const uint32_t* shape, const size_t* stride, uint32_t flags)
{
    return mda_rawview(mda, data, dimensions, shape, stride, flags);
}

template <class MDA_TYPE, class DTYPE>
void mda_clear(MDA_TYPE* mda, DTYPE zero)
{
    IFX_ERR_BRK_NULL(mda);

    auto* data = IFX_MDA_DATA(mda);

    if (mda_is_contiguous(mda))
    {
        const size_t elements = mda_elements(mda);
        const size_t size_element = sizeof(data[0]);
        std::memset(IFX_MDA_DATA(mda), 0, elements * size_element);
    }
    else
        mda_setall(mda, zero);
}

void ifx_mda_clear_r(ifx_Mda_R_t* mda)
{
    mda_clear(mda, 0.f);
}

void ifx_mda_clear_c(ifx_Mda_C_t* mda)
{
    const ifx_Complex_t zero = IFX_COMPLEX_DEF(0, 0);
    mda_clear(mda, zero);
}
