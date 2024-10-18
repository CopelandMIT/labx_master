/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <utility>


namespace strata
{

    template <typename FunctionType>
    class finally_impl
    {
    public:
        finally_impl()                     = delete;
        finally_impl(const finally_impl &) = delete;
        finally_impl &operator=(const finally_impl &) = delete;
        finally_impl &operator=(finally_impl &&) = delete;

        finally_impl(const FunctionType &function) :
            m_function {function},
            m_active {true}
        {}

        finally_impl(FunctionType &&function) :
            m_function {std::move(function)},
            m_active {true}
        {}

        finally_impl(finally_impl &&other) :
            m_function {std::move(other.m_function)},
            m_active {other.m_active}
        {
            other.m_active = false;
        }

        ~finally_impl()
        {
            if (m_active)
            {
                m_function();
            }
        }

    private:
        FunctionType m_function;
        bool m_active;
    };

    template <typename FunctionType>
    finally_impl<typename std::decay<FunctionType>::type> finally(FunctionType &&function)
    {
        return finally_impl<typename std::decay<FunctionType>::type>(std::forward<FunctionType>(function));
    }

}
