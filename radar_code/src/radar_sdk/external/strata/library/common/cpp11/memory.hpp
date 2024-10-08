#pragma once

#include <memory>

/*!
 * Custom implementation of std::make_unique() to avoid requiring C++14.
 */
#if (defined(_MSC_VER) && (_MSC_VER < 1800)) || (!defined(_MSC_VER) && (__cplusplus < 201402L))
namespace std  // NOLINT
{
    template <typename T, typename... Args>
    inline std::unique_ptr<T> make_unique(Args &&...args)
    {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }
}
#endif
