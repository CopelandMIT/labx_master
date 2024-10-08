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

#ifndef IFX_MDA_HPP
#define IFX_MDA_HPP

#include <cstring>
#include <functional>


#include "../Mda.h"

/**
 * @brief Definition of iteration function.
 *
 * The function is called for each element of a multi-dimensional array.
 *
 * The first argument corresponds to the offset, the second argument contains
 * the indices.
 *
 * If the function returns true the iteration continues, otherwise it stops.
 */
using IterFunc = std::function<bool(size_t, const uint32_t[])>;

template <class MDA_TYPE>
static inline size_t mda_elements(const MDA_TYPE* mda)
{
    IFX_ERR_BRV_NULL(mda, 0);

    if (IFX_MDA_DIMENSIONS(mda) == 0)
        return 0;

    size_t elems = 1;
    for (uint32_t dim = 0; dim < IFX_MDA_DIMENSIONS(mda); dim++)
        elems *= IFX_MDA_SHAPE(mda)[dim];
    return elems;
}

/**
 * @brief Iterate over multi-dimensional array
 *
 * @tparam MDA_TYPE either ifx_Mda_R_t or ifx_Mda_C_t
 * @param mda Multi-dimensional array
 * @param f Iterator function
 * @return true if function iterated over all values
 * @return false if iteration was aborted (f returned false)
 */
template <class MDA_TYPE>
static inline bool iterate(const MDA_TYPE* mda, const IterFunc& f)
{
    if (mda_elements(mda) == 0)
        return true;

    const auto dimensions = IFX_MDA_DIMENSIONS(mda);
    const auto* stride = IFX_MDA_STRIDE(mda);

    uint32_t indices[IFX_MDA_MAX_DIM] = {0};
    for (size_t p = dimensions - 1;;)
    {
        const size_t offset = ifx_mda_offset(dimensions, stride, indices);
        bool carry_on = f(offset, indices);
        if (!carry_on)
            return false;

        indices[dimensions - 1]++;
        while (indices[p] == IFX_MDA_SHAPE(mda)[p])
        {
            indices[p] = 0;
            if (p == 0)
                return true;
            indices[--p]++;
            if (indices[p] != IFX_MDA_SHAPE(mda)[p])
                p = dimensions - 1;
        }
    }

    return true;
}

#endif  // IFX_MDA_HPP
