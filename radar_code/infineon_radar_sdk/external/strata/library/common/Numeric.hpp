/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <limits>


/*!
 * Clamps a given value to a specified narrower type
 */

template <typename R, typename T>
constexpr R clampValue(T value)
{
    return (value > std::numeric_limits<R>::max()) ? std::numeric_limits<R>::max() : static_cast<R>(value);
}
