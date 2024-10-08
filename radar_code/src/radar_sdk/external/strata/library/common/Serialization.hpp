/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include "EndianConversion.hpp"
#include "serialization/SerializationSize.hpp"

#include <array>
#include <type_traits>


// put data into serial buffer


// GCC needs forward declarations of array functions for correct overload resolution

template <typename T, std::size_t M, std::size_t N>
inline uint8_t *hostToSerial(uint8_t *buf, const T (&first)[M][N]);

template <typename T, std::size_t N>
inline uint8_t *hostToSerial(uint8_t *buf, const T (&first)[N]);

template <typename T, std::size_t N>
inline uint8_t *hostToSerial(uint8_t *buf, const std::array<T, N> &first);


template <typename T>
inline typename std::enable_if<
    sizeof(T) == 1,
    uint8_t *>::type
hostToSerial(uint8_t *buf, T first)
{
    auto it = reinterpret_cast<const uint8_t *>(&first);
    *buf++  = *it;
    return buf;
}

template <typename T>
inline typename std::enable_if<
    (sizeof(T) > 1) &&
        std::is_integral<typename std::remove_reference<T>::type>::value &&
        std::is_signed<typename std::remove_reference<T>::type>::value,
    uint8_t *>::type
hostToSerial(uint8_t *buf, T first)
{
    return hostToSerial(buf, static_cast<typename std::make_unsigned<T>::type>(first));
}

template <typename T>
inline typename std::enable_if<
    std::is_floating_point<typename std::remove_reference<T>::type>::value,
    uint8_t *>::type
hostToSerial(uint8_t *buf, T first)
{
    auto it = reinterpret_cast<const uint8_t *>(&first);
    std::copy(it, it + sizeof(first), buf);
    return buf + sizeof(first);
}

template <typename T>
inline typename std::enable_if<
    std::is_same<typename std::remove_reference<T>::type, uint16_t>::value,
    uint8_t *>::type
hostToSerial(uint8_t *buf, T first)
{
    *(buf++) = first & 0xFF;
    *(buf++) = first >> 8;
    return buf;
}

template <typename T>
inline typename std::enable_if<
    std::is_same<typename std::remove_reference<T>::type, uint32_t>::value,
    uint8_t *>::type
hostToSerial(uint8_t *buf, T first)
{
    *(buf++) = first & 0xFF;
    *(buf++) = static_cast<uint8_t>(first >> 8);
    *(buf++) = static_cast<uint8_t>(first >> 16);
    *(buf++) = static_cast<uint8_t>(first >> 24);
    return buf;
}


// overload for types with .value() function
template <typename T>
inline typename std::enable_if<
    std::is_member_function_pointer<decltype(&T::value)>::value,
    uint8_t *>::type
hostToSerial(uint8_t *buf, T first)
{
    return hostToSerial(buf, first.value());
}


// overload for BatchType (or any type defining serialized_size and having members address and value)
template <typename T>
inline typename std::enable_if<
    T::serialized_sizeof() == sizeof(std::declval<T>().address) + sizeof(std::declval<T>().value),
    uint8_t *>::type
hostToSerial(uint8_t *buf, const T &first)
{
    buf = hostToSerial(buf, first.address);
    buf = hostToSerial(buf, first.value);
    return buf;
}


template <typename T>
inline typename std::enable_if<
    (is_little_endian<T>::value && std::is_arithmetic<T>::value) ||
        (sizeof(T) == 1),
    uint8_t *>::type
hostToSerial(uint8_t *buf, const T *first, const T *last)
{
    auto begin_it = reinterpret_cast<const uint8_t *>(first);
    auto end_it   = reinterpret_cast<const uint8_t *>(last);
    std::copy(begin_it, end_it, buf);
    return buf + (end_it - begin_it);
}

template <typename T>
inline typename std::enable_if<
    (is_big_endian<T>::value && (sizeof(T) > 1)) ||
        !std::is_arithmetic<T>::value,
    uint8_t *>::type
hostToSerial(uint8_t *buf, const T *first, const T *last)
{
    while (first < last)
    {
        buf = hostToSerial(buf, *first++);
    }
    return buf;
}


template <typename T, std::size_t M, std::size_t N>
inline uint8_t *hostToSerial(uint8_t *buf, const T (&first)[M][N])
{
    auto it = &first[0][0];
    return hostToSerial(buf, it, it + M * N);
}

template <typename T, std::size_t N>
inline uint8_t *hostToSerial(uint8_t *buf, const T (&first)[N])
{
    return hostToSerial(buf, std::begin(first), std::end(first));
}

template <typename T, std::size_t N>
inline uint8_t *hostToSerial(uint8_t *buf, const std::array<T, N> &first)
{
    return hostToSerial(buf, first.data(), first.data() + N);
}


// --------------------------------


// get data from serial buffer


// GCC needs forward declarations of array functions for correct overload resolution

template <typename T, std::size_t M, std::size_t N>
inline const uint8_t *serialToHost(const uint8_t *buf, T (&dest)[M][N]);

template <typename T, std::size_t N>
inline const uint8_t *serialToHost(const uint8_t *buf, T (&dest)[N]);

template <typename T, std::size_t N>
inline const uint8_t *serialToHost(const uint8_t *buf, std::array<T, N> &dest);


template <typename T>
inline typename std::enable_if<
    sizeof(T) == 1,
    T>::type
serialToHost(const uint8_t *buf)
{
    return *reinterpret_cast<const T *>(buf);
}

template <typename T>
inline typename std::enable_if<
    (sizeof(T) > 1) &&
        std::is_integral<T>::value &&
        std::is_signed<T>::value,
    T>::type
serialToHost(const uint8_t *buf)
{
    return static_cast<T>(serialToHost<typename std::make_unsigned<T>::type>(buf));
}

template <typename T>
inline typename std::enable_if<
    std::is_same<T, uint16_t>::value,
    T>::type
serialToHost(const uint8_t *buf)
{
    T value = *buf++;
    value |= static_cast<T>(*buf) << 8;
    return value;
}

template <typename T>
inline typename std::enable_if<
    std::is_same<T, uint32_t>::value,
    T>::type
serialToHost(const uint8_t *buf)
{
    T value = *buf++;
    value |= static_cast<T>(*buf++) << 8;
    value |= static_cast<T>(*buf++) << 16;
    value |= static_cast<T>(*buf) << 24;
    return value;
}

template <typename T>
inline typename std::enable_if<
    std::is_same<T, uint64_t>::value,
    T>::type
serialToHost(const uint8_t *buf)
{
    T value = *(buf++);
    value |= static_cast<T>(*buf++) << 8;
    value |= static_cast<T>(*buf++) << 16;
    value |= static_cast<T>(*buf++) << 24;
    value |= static_cast<T>(*buf++) << 32;
    value |= static_cast<T>(*buf++) << 40;
    value |= static_cast<T>(*buf++) << 48;
    value |= static_cast<T>(*buf) << 56;
    return value;
}

template <typename T>
inline typename std::enable_if<
    std::is_floating_point<T>::value,
    T>::type
serialToHost(const uint8_t *buf)
{
    T value;
    auto first = reinterpret_cast<uint8_t *>(&value);
    auto next  = buf + serialized_size<T>::value;
    std::copy(buf, next, first);
    return value;
}


template <typename T>
inline T serialToHost(const uint8_t *buf, const uint8_t *&next)
{
    next = buf + serialized_size<T>::value;
    return serialToHost<T>(buf);
}


template <typename T>
inline typename std::enable_if<
    std::is_fundamental<T>::value,
    const uint8_t *>::type
serialToHost(const uint8_t *buf, T &first)
{
    first     = serialToHost<T>(buf);
    auto next = buf + serialized_size<T>::value;
    return next;
}


template <typename T>
inline typename std::enable_if<
    (is_little_endian<T>::value && std::is_arithmetic<T>::value) ||
        (sizeof(T) == 1),
    const uint8_t *>::type
serialToHost(const uint8_t *buf, T *first, T *last)
{
    std::size_t count = last - first;
    auto next         = buf + count * serialized_size<T>::value;
    std::copy(buf, next, reinterpret_cast<uint8_t *>(first));
    return next;
}

template <typename T>
inline typename std::enable_if<
    (is_big_endian<T>::value && (sizeof(T) > 1)) ||
        !std::is_arithmetic<T>::value,
    const uint8_t *>::type
serialToHost(const uint8_t *buf, T *first, T *last)
{
    while (first < last)
    {
        *(first++) = serialToHost<T>(buf);
        buf += serialized_size<T>::value;
    }
    return buf;
}


template <typename T, std::size_t M, std::size_t N>
inline const uint8_t *serialToHost(const uint8_t *buf, T (&first)[M][N])
{
    auto it = &first[0][0];
    return serialToHost(buf, it, it + M * N);
}

template <typename T, std::size_t N>
inline const uint8_t *serialToHost(const uint8_t *buf, T (&first)[N])
{
    return serialToHost(buf, std::begin(first), std::end(first));
}

template <typename T, std::size_t N>
inline const uint8_t *serialToHost(const uint8_t *buf, std::array<T, N> &first)
{
    return serialToHost(buf, first.data(), first.data() + N);
}
