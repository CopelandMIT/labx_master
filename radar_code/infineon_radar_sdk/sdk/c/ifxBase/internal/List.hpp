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

#ifndef IFX_BASE_LIST_HPP
#define IFX_BASE_LIST_HPP

#include "../Error.h"
#include "../List.h"
#include "../Mem.h"

#include <vector>

/**
 * @brief This template allows to create an ifx_List_t from a std::vector
 *
 * This template allows to convert a C++ std::vector into a \ref ifx_List_t
 * list.
 *
 * If memory allocation fails, the template returns NULL and a \ref IFX_ERROR_MEMORY_ALLOCATION_FAILED
 * error is set.
 *
 * The caller is responsible to free the memory of the returned list by calling
 * \ref ifx_list_destroy.
 *
 * @param [in]  vector	a std::vector
 * @return		list	list of type \ref ifx_List_t
 */
template <class T>
ifx_List_t* ifx_list_from_vector(std::vector<T> vector)
{
    ifx_List_t* list = ifx_list_create(ifx_mem_free);
    IFX_ERR_BRV_MEMALLOC(list, nullptr);

    for (const auto& elem : vector)
    {
        auto* p = static_cast<T*>(ifx_mem_alloc(sizeof(T)));
        if (!p)
        {
            ifx_error_set(IFX_ERROR_MEMORY_ALLOCATION_FAILED);
            ifx_list_destroy(list);
            return nullptr;
        }

        *p = elem;

        if (!ifx_list_push_back(list, p))
        {
            ifx_list_destroy(list);
            return nullptr;
        }
    }

    return list;
}

#endif /* IFX_BASE_LIST_HPP */
