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

#include "List.h"
#include "Error.h"
#include "internal/NonCopyable.hpp"

#include <vector>

/*
==============================================================================
   3. LOCAL TYPES
==============================================================================
*/

struct ifxList
{
private:
    void (*m_destructor)(void*) = nullptr;
    std::vector<void*> m_vector;

public:
    NONCOPYABLE(ifxList);
    ifxList() = delete;
    ifxList(void (*destructor)(void*)) :
        m_destructor(destructor)
    {}

    ~ifxList()
    {
        if (m_destructor)
        {
            for (void* p : m_vector)
                m_destructor(p);
        }
    }

    size_t size() const
    {
        return m_vector.size();
    }

    void push_back(void* v)
    {
        return m_vector.push_back(v);
    }

    void* get(size_t index) const
    {
        if (index < m_vector.size())
            return m_vector[index];
        else
        {
            ifx_error_set(IFX_ERROR_ARGUMENT_OUT_OF_BOUNDS);
            return nullptr;
        }
    }
};

/*
==============================================================================
   7. EXPORTED FUNCTIONS
==============================================================================
*/

ifx_List_t* ifx_list_create(void destructor(void*))
{
    auto* list = new (std::nothrow) ifxList(destructor);
    IFX_ERR_BRV_NULL(list, nullptr);
    return list;
}

void ifx_list_destroy(ifx_List_t* list)
{
    delete list;
}

size_t ifx_list_size(const ifx_List_t* list)
{
    return list->size();
}

bool ifx_list_push_back(ifx_List_t* list, void* elem)
{
    try
    {
        list->push_back(elem);
        return true;
    }
    catch (const std::bad_alloc&)
    {
        ifx_error_set(IFX_ERROR_MEMORY_ALLOCATION_FAILED);
        return false;
    }
}

void* ifx_list_get(const ifx_List_t* list, size_t index)
{
    return list->get(index);
}
