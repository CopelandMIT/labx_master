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


// copy host buffer to little-endian buffer

template <typename InputIt, typename OutputIt>
inline typename std::enable_if<is_little_endian<InputIt>::value>::type
hostToLittle(InputIt first, InputIt last, OutputIt dest)
{
    std::copy(first, last, dest);
}

template <typename InputIt, typename OutputIt>
inline typename std::enable_if<is_big_endian<InputIt>::value>::type
hostToLittle(InputIt first, InputIt last, OutputIt dest)
{
    endian::swap(first, last, dest);
}


// transform host buffer into little-endian buffer

template <typename It>
inline typename std::enable_if<is_little_endian<It>::value>::type
hostToLittle(It, It)
{
}

template <typename It>
inline typename std::enable_if<is_big_endian<It>::value>::type
hostToLittle(It first, It last)
{
    endian::swap(first, last, first);
}


// transform host value into little-endian value

template <typename T>
inline typename std::enable_if<is_little_endian<T>::value, T>::type
hostToLittle(T first)
{
    return first;
}

template <typename T>
inline typename std::enable_if<is_big_endian<T>::value, T>::type
hostToLittle(T first)
{
    return endian::swap(first);
}


// inverse functions are just the same...

template <typename T>
inline T littleToHost(T value)
{
    return hostToLittle(value);
}

template <typename... Types>
inline void littleToHost(Types... args)
{
    hostToLittle(std::forward<Types>(args)...);
}
