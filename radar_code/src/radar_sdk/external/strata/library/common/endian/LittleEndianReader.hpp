/**
 * @copyright 2020 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include "common/endian/Little.hpp"
#include <cstdint>
#include <cstring>

///
/// \brief LittleEndianReader is a small helper class to read data stored in little endian
///
class LittleEndianReader
{
public:
    ///
    /// \param buffer to read the data from, is stored in little endian
    /// \param size of the buffer
    ///
    explicit LittleEndianReader(const std::uint8_t *buffer, const std::uint32_t size);


    ///
    /// \brief read reads the variable of type T at offset in buffer and stores it in result
    /// \param offset in buffer where to start reading T
    /// \param[out] result
    /// \retval true on success
    /// \retval false when the buffer is not large enough to read the result
    ///
    template <typename T>
    bool read(size_t offset, T &result) const
    {
        if (!m_buffer || offset + sizeof(T) > m_size)
        {
            return false;
        }

        T temp {};
        std::memcpy(&temp, m_buffer + offset, sizeof(temp));
        result = littleToHost(temp);

        return true;
    }

    ///
    /// \brief read is an overload to specifically handle booleans.
    ///
    bool read(size_t offset, bool &result) const;

private:
    const std::uint8_t *m_buffer;
    const std::uint32_t m_size;
};
