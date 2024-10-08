/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <chrono>


template <typename Unit = std::chrono::microseconds>
inline std::chrono::system_clock::rep getEpochTime()
{
    return std::chrono::duration_cast<Unit>(std::chrono::system_clock::now().time_since_epoch()).count();
}


template <typename Unit = std::chrono::microseconds>
inline std::chrono::steady_clock::rep getElapsedTime(std::chrono::steady_clock::time_point since)
{
    return std::chrono::duration_cast<Unit>(std::chrono::steady_clock::now() - since).count();
}
