/**
 * @copyright 2020 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "BridgeProtocolFlash.hpp"
#include <common/NarrowCast.hpp>
#include <platform/interfaces/IVendorCommands.hpp>
#include <universal/protocol/protocol_definitions.h>


BridgeProtocolFlash::BridgeProtocolFlash(IVendorCommands *commands) :
    m_commands(commands)
{
}

uint32_t BridgeProtocolFlash::getMaxTransfer() const
{
    return m_commands->getMaxTransfer();
}

void BridgeProtocolFlash::read(uint8_t devId, uint32_t address, uint32_t length, uint8_t buffer[])
{
    if (address & 0xFF000000)
    {
        throw ENarrowCast(24);
    }
    const uint16_t wValue = static_cast<uint16_t>((address & 0xFF) << 8) | devId;
    m_commands->vendorRead(REQ_FLASH_TRANSACTION, wValue, static_cast<uint16_t>(address >> 8), narrow_cast<uint16_t>(length), buffer);
}

void BridgeProtocolFlash::write(uint8_t devId, uint32_t address, uint32_t length, const uint8_t buffer[])
{
    if (address & 0xFF000000)
    {
        throw ENarrowCast(24);
    }
    const uint16_t wValue = static_cast<uint16_t>((address & 0xFF) << 8) | devId;
    m_commands->vendorWrite(REQ_FLASH_TRANSACTION, wValue, static_cast<uint16_t>(address >> 8), narrow_cast<uint16_t>(length), buffer);
}

void BridgeProtocolFlash::erase(uint8_t devId, uint32_t address)
{
    if (address & 0xFF000000)
    {
        throw ENarrowCast(24);
    }
    const uint16_t wValue = static_cast<uint16_t>((address & 0xFF) << 8) | devId;
    m_commands->vendorWrite(REQ_FLASH_ERASE_WIP, wValue, static_cast<uint16_t>(address >> 8));
}

uint8_t BridgeProtocolFlash::getStatus(uint8_t devId)
{
    uint8_t status;
    m_commands->vendorRead(REQ_FLASH_ERASE_WIP, devId, 0, sizeof(status), &status);
    return status;
}
