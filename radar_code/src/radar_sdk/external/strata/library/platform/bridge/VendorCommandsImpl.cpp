/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "VendorCommandsImpl.hpp"

#include <common/Serialization.hpp>
#include <universal/protocol/protocol_definitions.h>


VendorCommandsImpl::VendorCommandsImpl() :
    m_protocolVersion {0}
{
}

void VendorCommandsImpl::readVersionInfo(uint16_t *buffer, std::size_t count)
{
    const uint16_t wLength = static_cast<uint16_t>(sizeof(*buffer) * count);

    vendorRead(REQ_BOARD_INFO, REQ_BOARD_INFO_VERSION_INFO_WVALUE, 0, wLength, buffer);
    littleToHost(buffer, buffer + count);

    m_protocolVersion = (buffer[4] << 16) | buffer[5];
}

uint32_t VendorCommandsImpl::getProtocolVersion() const
{
    return m_protocolVersion;
}
