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

#ifndef IFX_GUARDEDHANDLE_HPP
#define IFX_GUARDEDHANDLE_HPP

#include "../Exception.hpp"
#include "../internal/NonCopyable.hpp"


template <typename HandleT, void destroyHandleF(HandleT)>
class GuardedHandle
{
    HandleT m_handle;

public:
    NONCOPYABLE(GuardedHandle);
    GuardedHandle() = delete;

    GuardedHandle(HandleT handle) :
        m_handle(handle)
    {}

    ~GuardedHandle()
    {
        destroyHandleF(m_handle);
    }

    operator HandleT()
    {
        /* FIXME: Q: how do we want to handle dereferencing a nullptr. Throwing and exception, assert? */
        if (m_handle == nullptr)
            throw rdk::exception::argument_null();
        return m_handle;
    }
    operator HandleT() const
    {
        if (m_handle == nullptr)
            throw rdk::exception::argument_null();
        return m_handle;
    }
};

#endif  // IFX_GUARDEDHANDLE_HPP
