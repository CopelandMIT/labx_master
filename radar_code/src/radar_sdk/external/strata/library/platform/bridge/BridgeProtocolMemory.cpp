/**
 * @copyright 2020 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "BridgeProtocolMemory.hpp"
#include <platform/interfaces/IVendorCommands.hpp>
#include <universal/protocol/protocol_definitions.h>


BridgeProtocolMemory::BridgeProtocolMemory(IVendorCommands *commands) :
    m_commands(commands)
{
}

uint32_t BridgeProtocolMemory::read(uint32_t address)
{
    uint32_t value;
    read(address, 1, &value);
    return value;
}

void BridgeProtocolMemory::write(uint32_t address, uint32_t value)
{
    write(address, 1, &value);
}

void BridgeProtocolMemory::write(uint32_t address, uint32_t count, const uint32_t data[])
{
    const uint8_t bRequest  = REQ_MEMORY;
    const uint16_t maxCount = m_commands->getMaxTransfer() / sizeof(*data);

    while (count > 0)
    {
        const uint16_t wCount  = (count > maxCount) ? maxCount : static_cast<uint16_t>(count);
        const uint16_t wLength = wCount * sizeof(*data);

        const uint16_t wValue = static_cast<uint16_t>(address);
        const uint16_t wIndex = static_cast<uint16_t>(address >> 16);
        m_commands->vendorWrite(bRequest, wValue, wIndex, wLength, data);

        address += wLength;
        data += wCount;
        count -= wCount;
    }
}

void BridgeProtocolMemory::read(uint32_t address, uint32_t count, uint32_t data[])
{
    const uint8_t bRequest  = REQ_MEMORY;
    const uint16_t maxCount = m_commands->getMaxTransfer() / sizeof(*data);

    while (count > 0)
    {
        const uint16_t wCount  = (count > maxCount) ? maxCount : static_cast<uint16_t>(count);
        const uint16_t wLength = wCount * sizeof(*data);

        const uint16_t wValue = static_cast<uint16_t>(address);
        const uint16_t wIndex = static_cast<uint16_t>(address >> 16);
        m_commands->vendorRead(bRequest, wValue, wIndex, wLength, data);

        address += wLength;
        data += wCount;
        count -= wCount;
    }
}
