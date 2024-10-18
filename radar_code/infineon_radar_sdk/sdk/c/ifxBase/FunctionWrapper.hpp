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

#pragma once

#include "Error.h"
#include "Exception.hpp"
#include <common/exception/EException.hpp>
#include <platform/exception/EConnection.hpp>

#include <limits>


namespace rdk {

template <typename Caller, typename Returner, typename... Args>
auto exception_handler(Caller caller, Returner returner, Args&&... args) -> decltype(caller(args...))
{
    try
    {
        return caller(std::forward<Args>(args)...);
    }
    catch (const rdk::exception::exception& e)
    {
        IFX_LOG_DEBUG("exception_handler - rdk::exception, \"%s\"", e.what());
        ifx_error_set(e.error_code());
    }
    catch (const EConnection& e)
    {
        (void)e;
        IFX_LOG_DEBUG("exception_handler - EConnection, \"%s\"", e.what());
        ifx_error_set(IFX_ERROR_COMMUNICATION_ERROR);
    }
    catch (const EException& e)
    {
        (void)e;
        IFX_LOG_DEBUG("exception_handler - EException, \"%s\"", e.what());
        ifx_error_set(IFX_ERROR_HOST);
    }
    catch (const std::bad_alloc&)
    {
        IFX_LOG_DEBUG("exception_handler - std::bad_alloc");
        ifx_error_set(IFX_ERROR_MEMORY_ALLOCATION_FAILED);
    }
    catch (const std::exception& e)
    {
        (void)e;
        IFX_LOG_DEBUG("exception_handler - std::exception, \"%s\"", e.what());
        ifx_error_set(IFX_ERROR);
    }
    catch (...)
    {
        IFX_LOG_DEBUG("exception_handler - unexpected exception");
        ifx_error_set(IFX_ERROR);
    }

    return returner();
}


// Comments on the following templates:
// - to match both the function parameters and the actual parameters, we add an rvalue reference to the actual parameters
// - we need identical overloads for const member function, otherwise we would not be able to deduce the type


// Execute callable "caller" and catch the exception if an exception occurs
//
// This template executes the callable "caller".
// If func throws an exception, the template catches the exception,
// sets the corresponding IFX error using ifx_error_set(), and returns default_return_value.
// If func doesn't throw an exception, the return value of func is returned.
// There is also an overload for void function that does not have a default_return_value.
//
// Example:
//     auto f = [&value](param1) -> bool {
//         return do_stuff(value, param1);
//     };
//
//     bool result = call_func(f, false, param1); // returns false if an exception occurs

template <typename ReturnType>
inline constexpr ReturnType default_returner()
{
    return ReturnType {};
}

template <>
inline constexpr void default_returner()
{
}

template <>
inline constexpr float default_returner()
{
    return std::numeric_limits<float>::quiet_NaN();
}

template <>
inline constexpr double default_returner()
{
    return std::numeric_limits<double>::quiet_NaN();
}

template <typename Caller, typename DefaultType, typename... Args>
auto call_func(Caller caller, DefaultType&& default_return_value, Args&&... args) -> decltype(caller(args...))
{
    auto value_returner = [default_return_value]() {
        return default_return_value;
    };
    return exception_handler(caller, value_returner, std::forward<Args>(args)...);
}

template <typename Caller, typename... Args>
auto call_func(Caller caller, Args&&... args) -> decltype(caller(args...))
{
    return exception_handler(caller, default_returner<decltype(caller(args...))>, std::forward<Args>(args)...);
}

template <typename Class>
inline void check_handle(Class* handle)
{
    if (!handle)
    {
        throw rdk::exception::argument_null();
    }
}

// Execute member function "func" and catch the exception if an exception occurs
//
// This template executes the member function "func" of the instance given by handle.
// If func throws an exception, the template catches the exception,
// sets the corresponding IFX error using ifx_error_set(), and returns default_return_value.
// If func doesn't throw an exception, the return value of func is returned.
// There is also an overload for void function that does not have a default_return_value.
//
// Example:
//     bool result = call_func(handle, &Class::set_config, false, param1); // returns false if an exception occurs
template <typename Return, typename Class, typename DefaultType, typename... Params, typename... Args>
auto call_func(Class* handle, Return (Class::*func)(Params...), DefaultType&& default_return_value, Args&&... args) -> decltype((handle->*func)(args...))
{
    auto caller = [handle, func](Args&&... args) {
        check_handle(handle);
        return (handle->*func)(std::forward<Args>(args)...);
    };
    return call_func(caller, std::forward<DefaultType>(default_return_value), std::forward<Args>(args)...);
}

template <typename Return, typename Class, typename DefaultType, typename... Params, typename... Args>
auto call_func(const Class* handle, Return (Class::*func)(Params...) const, DefaultType&& default_return_value, Args&&... args) -> decltype((handle->*func)(args...))
{
    auto caller = [handle, func](Args&&... args) {
        check_handle(handle);
        return (handle->*func)(std::forward<Args>(args)...);
    };
    return call_func(caller, std::forward<DefaultType>(default_return_value), std::forward<Args>(args)...);
}

template <typename Return, typename DefaultType, typename... Params, typename... Args>
auto call_func(Return (*func)(Params...), DefaultType&& default_return_value, Args&&... args) -> decltype(func(args...))
{
    auto caller = [func](Args&&... args) {
        return func(std::forward<Args>(args)...);
    };
    return call_func(caller, std::forward<DefaultType>(default_return_value), std::forward<Args>(args)...);
}

template <typename Return, typename Class, typename... Params, typename... Args>
auto call_func(Class* handle, Return (Class::*func)(Params...), Args&&... args) -> decltype((handle->*func)(args...))
{
    auto caller = [handle, func](Args&&... args) {
        check_handle(handle);
        return (handle->*func)(std::forward<Args>(args)...);
    };
    return call_func(caller, std::forward<Args>(args)...);
}

template <typename Return, typename Class, typename... Params, typename... Args>
auto call_func(const Class* handle, Return (Class::*func)(Params...) const, Args&&... args) -> decltype((handle->*func)(args...))
{
    auto caller = [handle, func](Args&&... args) {
        check_handle(handle);
        return (handle->*func)(std::forward<Args>(args)...);
    };
    return call_func(caller, std::forward<Args>(args)...);
}

template <typename Return, typename... Params, typename... Args>
auto call_func(Return (*func)(Params...), Args&&... args) -> decltype(func(args...))
{
    auto caller = [func](Args&&... args) {
        return func(std::forward<Args>(args)...);
    };
    return call_func(caller, std::forward<Args>(args)...);
}

}  // namespace rdk
