/**
 * @copyright 2020 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "LittleEndianReader.hpp"

LittleEndianReader::LittleEndianReader(const std::uint8_t *buffer, const std::uint32_t size) :
    m_buffer(buffer), m_size(size)
{
}

bool LittleEndianReader::read(size_t offset, bool &result) const
{
    std::uint8_t temp {};
    if (!read(offset, temp))
    {
        return false;
    }

    result = temp;
    return true;
}
