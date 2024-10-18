/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <type_traits>


///
/// This is the helper implementation to determine the size of a type.
/// Its default implementation is only enabled for fundamental types.
/// For other classes, including POD structs, there has to be a specialization.
///
template <typename T, typename Enabled = void>
struct serialized_size_impl;


template <typename T>
struct serialized_size_impl<T, typename std::enable_if<
                                   std::is_arithmetic<typename std::remove_reference<T>::type>::value>::type>
{
    static constexpr size_t value = sizeof(T);
};

template <typename T>
struct serialized_size_impl<T, typename std::enable_if<
                                   std::is_array<typename std::remove_reference<T>::type>::value>::type>
{
    static_assert(std::extent<typename std::remove_reference<T>::type>::value > 0, "deduction of array extent failed");
    static constexpr size_t value = serialized_size_impl<typename std::remove_extent<typename std::remove_reference<T>::type>::type>::value * std::extent<typename std::remove_reference<T>::type>::value;
};

template <typename T>
struct serialized_size_impl<T, typename std::enable_if<
                                   std::is_pointer<T>::value>::type>
{
    static constexpr size_t value = serialized_size_impl<std::remove_pointer<T>>::value;
};


template <typename T>
struct serialized_size_impl<T, typename std::enable_if<
                                   std::is_lvalue_reference<decltype(std::declval<T>().value())>::value && std::is_const<typename std::remove_reference<decltype(std::declval<T>().value())>::type>::value>::type>
{
    static constexpr size_t value = serialized_size_impl<decltype(std::declval<T>().value())>::value;
};


//#include <platform/interfaces/access/IMemory.hpp>
//template <template <typename, typename> class IMemory, typename A, typename B>
//struct serialized_size_impl<typename IMemory<A, B>::BatchType, typename std::enable_if<
//                                                           true  // std::is_same<A, B>::value
//                                                           >::type>
//{
//    static constexpr size_t value = sizeof(A) + sizeof(B);
//};

template <typename T>
struct serialized_size_impl<T, typename std::enable_if<
                                   std::is_same<size_t, decltype(T::serialized_sizeof())>::value>::type>
{
    static constexpr size_t value = T().serialized_sizeof();
};

///
/// This template determines the size of a variadic parameter pack.
/// It recursively calls the helper implementations for each type.
/// Its default implementation is only enabled for integral types.
/// For other classes, including POD structs, there has to be a specialization.
///
template <typename T = void, typename... Types>
struct serialized_size
{
    static constexpr size_t value = serialized_size<T>::value + serialized_size<Types...>::value;
};

template <typename T>
struct serialized_size<T>
{
    static constexpr size_t value = serialized_size_impl<T>::value;
};

template <>
struct serialized_size<void>
{
    static constexpr size_t value = 0;
};
