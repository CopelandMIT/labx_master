/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <thread>


constexpr auto defaultStep = std::chrono::steady_clock::duration::zero();  // just yield to other threads by default


template <typename Condition, typename Rep, typename Period, typename PeriodStep = decltype(defaultStep)::period>
inline bool waitFor(Condition condition, std::chrono::duration<Rep, Period> timeout, std::chrono::duration<Rep, PeriodStep> step = defaultStep)
{
    const auto expiry = std::chrono::steady_clock::now() + timeout;

    while (!condition())
    {
        if (std::chrono::steady_clock::now() > expiry)
        {
            return false;
        }
        std::this_thread::sleep_for(step);
    }

    return true;
}
