/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <cstdint>
#include <type_traits>


namespace endian
{
    template <typename T>
    inline typename std::enable_if<!std::is_integral<typename std::remove_reference<T>::type>::value || (sizeof(T) == 1), T>::type
    swap(T value)
    {
        return value;
    }

    template <typename T>
    inline typename std::enable_if<std::is_integral<typename std::remove_reference<T>::type>::value && (sizeof(T) == 2), T>::type
    swap(T value)
    {
        return static_cast<T>(
            (value << 8) |
            ((value >> 8) & 0xFF));
    }

    template <typename T>
    inline typename std::enable_if<std::is_integral<typename std::remove_reference<T>::type>::value && (sizeof(T) == 4), T>::type
    swap(T value)
    {
        return static_cast<T>(
            (value << 24) |
            ((value << 8) & 0x00FF0000) |
            ((value >> 8) & 0x0000FF00) |
            ((value >> 24) & 0x000000FF));
    }

    template <typename T>
    inline typename std::enable_if<std::is_floating_point<typename std::remove_reference<T>::type>::value, T>::type
    swap(T value)
    {
        // todo: add floating point endianness handling
        return value;
    }

    template <typename InputIt, typename OutputIt>
    inline void swap(InputIt first, InputIt last, OutputIt dest)
    {
        while (first < last)
        {
            *(dest++) = endian::swap(*(first++));
        }
    }

    template <typename InputIt>
    inline void swap(InputIt first, InputIt last)
    {
        endian::swap(first, last, first);
    }
}


template <typename T = void>
struct is_big_endian
{
    constexpr static bool value =
#ifdef TARGET_PLATFORM_BIG_ENDIAN
        true
#else
        false
#endif
        ;
};

template <typename T = void>
struct is_little_endian
{
    constexpr static bool value = !is_big_endian<T>::value;
};
