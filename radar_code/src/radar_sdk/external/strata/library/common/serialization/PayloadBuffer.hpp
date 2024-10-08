/**
 * @copyright 2020 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */
#pragma once

#include <common/Buffer.hpp>
#include <common/Numeric.hpp>
#include <common/Serialization.hpp>
#include <common/exception/EUninitialized.hpp>
#include <limits>


template <typename... Ts>
inline uint8_t *fill_impl(uint8_t *ptr, const Ts &...ts);

template <typename T, typename... Ts>
inline uint8_t *fill_impl(uint8_t *ptr, const T &t, const Ts &...ts)
{
    ptr = hostToSerial(ptr, t);
    return fill_impl(ptr, ts...);
}

template <>
inline uint8_t *fill_impl(uint8_t *ptr)
{
    return ptr;
}


/**
 *  @brief This class creates payload data based on the specified buffer.
 *  @note It should only be instantiated using make_payloadBuffer for
 *        automatic type deduction of count.
 *        Before accessing each chunk of data, update() has to be called.
 */
template <typename B, typename C, typename... Args>
class PayloadBuffer
{
public:
    static constexpr uint16_t elemSize = serialized_size<B>::value;
    static constexpr uint16_t argSize  = serialized_size<Args...>::value;

    PayloadBuffer() = delete;

    /**
     *  @param buf The buffer to be sent
     *  @param count The number of elements in the buffer
     *  @param maxTransfer The maximum chunk size of each payload transfer which
     *         depends on the transport mechanism implementation
     *         (set to zero if there is no limitation)
     */
    PayloadBuffer(const B *buf, C count, uint16_t maxTransfer, const Args &...args) :
        m_buf {buf},
        m_count {count},
        m_size {0}
    {
        if (maxTransfer == 0)
        {
            maxTransfer = std::numeric_limits<decltype(maxTransfer)>::max();
        }

        if (argSize > maxTransfer)
        {
            m_maxCount = 0;
        }
        else
        {
            m_maxCount = (maxTransfer - argSize) / elemSize;
            m_maxCount = std::min(m_maxCount, count);
        }

        if (m_maxCount == 0)
        {
            throw EUninitialized("Maximum length too small for even one element (or narrowing conversion not handled)");
        }
        m_payload.resize(m_maxCount * elemSize + argSize);

        fill(args...);
    }

    /**
     * @brief This updates the payload data to the next chunk.
     * @note the data can be accessed afterwards using data()
     * @return the amount of data in the chunk. This can also be accessed
     *         afterwards by calling size()
     */
    C update()
    {
        const auto wCount = std::min(m_count, m_maxCount);

        m_size   = static_cast<decltype(m_size)>(wCount * elemSize + argSize);
        auto ptr = m_payload.end() - m_size;
        ptr      = hostToSerial(ptr, m_buf, m_buf + wCount);
        if (ptr != m_payload.end() - argSize)
        {
            throw EUninitialized("Size mismatch in data initializtion");
        }
        m_buf += wCount;
        m_count -= wCount;

        return wCount;
    }

    /**
     * @brief Update the payload data with the specified values
     */
    inline void fill(const Args &...args)
    {
        auto ptr = fill_impl(m_payload.end() - argSize, args...);
        if (ptr != m_payload.end())
        {
            throw EUninitialized("Size mismatch in data initializtion");
        }
    }

    /**
     * @brief Get the data to write next
     * @return the pointer to the current payload data
     */
    const uint8_t *data() const
    {
        if (m_count && !m_size)
        {
            throw EUninitialized("Access before data initializtion");
        }
        return m_payload.end() - m_size;
    }

    /**
     * @brief Gets the size of buffer specified by data()
     * @return the size of the current payload data
     */
    uint16_t size() const
    {
        return m_size;
    }

private:
    const B *m_buf;
    C m_count, m_maxCount;
    uint16_t m_size;
    strata::buffer<uint8_t> m_payload;
};

/**
 *  @brief This class is a specialization for uin8_t
 *         which does not need an internal buffer
 *         when the payload doesn't need additional arguments
 */
template <typename C>
class PayloadBuffer<uint8_t, C>
{
public:
    using T = uint8_t;

    PayloadBuffer() = delete;
    PayloadBuffer(const T *buf, C count, uint16_t maxTransfer = 0) :
        m_buf {buf},
        m_count {count},
        m_size {0}
    {
        if (maxTransfer == 0)
        {
            maxTransfer = std::numeric_limits<decltype(maxTransfer)>::max();
        }
        if (maxTransfer > count)
        {
            m_maxCount = count;
        }
        else
        {
            m_maxCount = clampValue<decltype(m_maxCount)>(maxTransfer);
        }
    }

    /**
     * @brief This updates the payload data to the next chunk.
     * @note the data can be accessed afterwards using data()
     * @return the amount of data in the chunk. This can also be accessed
     *         afterwards by calling size()
     */
    C update()
    {
        const auto wCount = std::min(m_count, m_maxCount);

        m_data = m_buf;

        m_buf += wCount;
        m_count -= wCount;

        m_size = static_cast<decltype(m_size)>(wCount);
        return wCount;
    }

    /**
     * @brief Dummy function for consistent interface
     */
    inline void fill()
    {
    }

    /**
     * @brief Get the data to write next
     * @return the pointer to the current payload data
     */
    const uint8_t *data() const
    {
        if (m_count && !m_size)
        {
            throw EUninitialized("Access before data initializtion");
        }
        return m_data;
    }

    /**
     * @brief Gets the size of buffer specified by data()
     * @return the size of the current payload data
     */
    uint16_t size() const
    {
        return m_size;
    }

private:
    const T *m_buf;
    C m_count, m_maxCount;
    uint16_t m_size;
    const uint8_t *m_data;
};

/**
 *  @brief This function creates a payload data object based on the specified buffer.
 *  @param buf The buffer to be sent
 *  @param count The number of elements in the buffer
 *  @param maxTransfer The maximum chunk size of each payload transfer which
 *         depends on the transport mechanism implementation
 *         (set to zero if there is no limitation)
 */
template <typename B, typename C, typename... Args>
inline PayloadBuffer<B, C, Args...> make_payloadBuffer(const B *buf, C count, uint16_t maxTransfer, const Args &...args)
{
    return PayloadBuffer<B, C, Args...>(buf, count, maxTransfer, args...);
}


/**
 *  @brief This class creates payload data based on the variadic template types.
 *  @note It should only be instantiated using make_payloadParameters for
 *        automatic type deduction and cross-checking of the function signature.
 */
template <typename... Args>
class PayloadWriteParameters
{
public:
    PayloadWriteParameters() = delete;

    /**
     *  @param variadic arguments to be placed in payload
     */
    PayloadWriteParameters(const Args &...args)
    {
        fill(args...);
    }

    /**
     * @brief Update the payload data with the specified values
     */
    inline void fill(const Args &...args)
    {
        auto ptr = fill_impl(m_payload, args...);
        if (ptr != m_payload + size())
        {
            throw EUninitialized("Size mismatch in data initializtion");
        }
    }

    /**
     * @brief Get the data to write
     * @return the pointer to the payload data
     */
    const uint8_t *data() const
    {
        return m_payload;
    }

    /**
     * @brief Gets the size of buffer specified by data()
     * @return the size of the payload data
     */
    static constexpr uint16_t size()
    {
        return serialized_size<Args...>::value;
    }

private:
    uint8_t m_payload[size()];
    uint8_t *m_ptr;
};

/**
 *  @brief This function creates a payload data object based on the specified arguments.
 *         It also checks if the signature of the specified function matches.
 *  @param function signature to check
 *  @param variadic arguments to be placed in payload
 */
template <typename Class, typename Return, typename... Args>
inline PayloadWriteParameters<Args...> make_payloadWriteParameters(Return (Class::*)(Args...), const Args &...args)
{
    return PayloadWriteParameters<Args...>(args...);
}

template <typename... Args>
inline PayloadWriteParameters<Args...> make_payloadWriteParameters(const Args &...args)
{
    return PayloadWriteParameters<Args...>(args...);
}


/**
 *  @brief This class creates payload data based on the variadic template types.
 *  @note It should only be instantiated using make_PayloadReadParameters for
 *        automatic type deduction and cross-checking of the function signature.
 */
template <typename... Args>
class PayloadReadParameters
{
public:
    /**
     * @brief Update the payload data with the specified values
     */
    inline void unserialize(Args &...args)
    {
        auto ptr = unserialize_impl(m_payload, args...);
        if (ptr != m_payload + size())
        {
            throw EUninitialized("Size mismatch in data unserialization");
        }
    }

    /**
     * @brief Get the data to write
     * @return the pointer to the payload data
     */
    uint8_t *data()
    {
        return m_payload;
    }

    /**
     * @brief Gets the size of buffer specified by data()
     * @return the size of the payload data
     */
    static constexpr uint16_t size()
    {
        return serialized_size<Args...>::value;
    }

private:
    template <typename... Ts>
    inline const uint8_t *unserialize_impl(const uint8_t *buf, Ts &...ts);

    template <typename T, typename... Ts>
    inline const uint8_t *unserialize_impl(const uint8_t *buf, T &t, Ts &...ts)
    {
        buf = serialToHost(buf, t);
        return unserialize_impl(buf, ts...);
    }

    inline const uint8_t *unserialize_impl(const uint8_t *buf)
    {
        return buf;
    }

    uint8_t m_payload[size()];
    uint8_t *m_ptr;
};


/**
 *  @brief This function creates a payload data object based on the specified arguments.
 *         It also checks if the signature of the specified function matches.
 *  @param function signature to check
 *  @param variadic arguments to be placed in payload
 */
template <typename Class, typename Return, typename... Args>
inline PayloadReadParameters<Args...> make_PayloadReadParameters(Return (Class::*)(Args...), Args...)
{
    return PayloadReadParameters<Args...>();
}

template <typename... Args>
inline PayloadReadParameters<Args...> make_PayloadReadParameters(Args...)
{
    return PayloadReadParameters<Args...>();
}
