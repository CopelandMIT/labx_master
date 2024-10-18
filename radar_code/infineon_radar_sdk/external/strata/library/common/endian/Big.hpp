/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include "General.hpp"

#include <algorithm>


// copy host buffer to big-endian buffer

template <typename InputIt, typename OutputIt>
inline typename std::enable_if<is_big_endian<InputIt>::value>::type
hostToBig(InputIt first, InputIt last, OutputIt dest)
{
    std::copy(first, last, dest);
}

template <typename InputIt, typename OutputIt>
inline typename std::enable_if<is_little_endian<InputIt>::value>::type
hostToBig(InputIt first, InputIt last, OutputIt dest)
{
    endian::swap(first, last, dest);
}


// transform host buffer into big-endian buffer

template <typename It>
inline typename std::enable_if<is_big_endian<It>::value>::type
hostToBig(It, It)
{
}

template <typename It>
inline typename std::enable_if<is_little_endian<It>::value>::type
hostToBig(It first, It last)
{
    endian::swap(first, last, first);
}


// transform host value into big-endian value

template <typename T>
inline typename std::enable_if<is_big_endian<T>::value, T>::type
hostToBig(T first)
{
    return first;
}

template <typename T>
inline typename std::enable_if<is_little_endian<T>::value, T>::type
hostToBig(T first)
{
    return endian::swap(first);
}


// inverse functions are just the same...

template <typename T>
inline T bigToHost(T value)
{
    return hostToBig(value);
}

template <typename... Types>
inline void bigToHost(Types... args)
{
    hostToBig(std::forward<Types>(args)...);
}
