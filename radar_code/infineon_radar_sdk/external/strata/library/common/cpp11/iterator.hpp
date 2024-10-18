#pragma once

#include <iterator>

/*!
 * Custom implementation of std::size() to avoid requiring C++17.
 *
 * The C++17 standard says:
 * "The following macro names shall be defined by the implementation:
 *  __cplusplus The integer literal 201703L."
 *
 * If _MSC_VER is defined we are using MSVC. In that case define the template
 * if the version is older than MSVC 2015.
 *
 * If _MSC_VER is not defined we are not using MSVC. In that case define the
 * template if the C++ version is older than C++17.
 */
#if (defined(_MSC_VER) && (_MSC_VER < 1900)) || (!defined(_MSC_VER) && (__cplusplus < 201703L))
namespace std  // NOLINT
{
    template <typename T, std::size_t N>
    inline constexpr std::size_t size(const T (&)[N]) noexcept
    {
        return N;
    }
}
#endif
