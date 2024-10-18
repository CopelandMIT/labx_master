/**
 * @copyright 2020 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "BridgeProtocolI2c.hpp"
#include <platform/exception/EProtocol.hpp>
#include <platform/interfaces/IVendorCommands.hpp>
#include <universal/protocol/protocol_definitions.h>


BridgeProtocolI2c::BridgeProtocolI2c(IVendorCommands *commands) :
    m_commands(commands)
{
}

uint16_t BridgeProtocolI2c::getMaxTransfer() const
{
    return m_commands->getMaxTransfer();
}

void BridgeProtocolI2c::writeWithoutPrefix(uint16_t devAddr, uint16_t length, const uint8_t buffer[])
{
    m_commands->vendorWrite(REQ_I2C, devAddr, REQ_I2C_FLAG_TRANSACTION, length, buffer);
}

void BridgeProtocolI2c::writeWith8BitPrefix(uint16_t devAddr, uint8_t prefix, uint16_t length, const uint8_t buffer[])
{
    m_commands->vendorWrite(REQ_I2C, devAddr, REQ_I2C_FLAG_TRANSACTION | REQ_I2C_FLAG_WITH_REG_ADDR | prefix, length, buffer);
}

void BridgeProtocolI2c::writeWith16BitPrefix(uint16_t devAddr, uint16_t prefix, uint16_t length, const uint8_t buffer[])
{
    m_commands->vendorWrite(REQ_I2C_TRANSACTION_16, devAddr, prefix, length, buffer);
}

void BridgeProtocolI2c::readWithoutPrefix(uint16_t devAddr, uint16_t length, uint8_t buffer[])
{
    m_commands->vendorRead(REQ_I2C, devAddr, REQ_I2C_FLAG_TRANSACTION, length, buffer);
}

void BridgeProtocolI2c::readWith8BitPrefix(uint16_t devAddr, uint8_t prefix, uint16_t length, uint8_t buffer[])
{
    m_commands->vendorRead(REQ_I2C, devAddr, REQ_I2C_FLAG_TRANSACTION | REQ_I2C_FLAG_WITH_REG_ADDR | prefix, length, buffer);
}

void BridgeProtocolI2c::readWith16BitPrefix(uint16_t devAddr, uint16_t prefix, uint16_t length, uint8_t buffer[])
{
    m_commands->vendorRead(REQ_I2C_TRANSACTION_16, devAddr, prefix, length, buffer);
}

void BridgeProtocolI2c::configureBusSpeed(uint16_t devAddr, uint32_t speed)
{
    if (!speed)
    {
        throw EProtocol("I2C bus speed must not be zero");
    }

    const uint16_t wValue = (devAddr & 0xF000) | static_cast<uint16_t>((speed >> 15) & 0x0FFF);
    const uint16_t wIndex = static_cast<uint16_t>(speed & 0x7FFF);
    m_commands->vendorWrite(REQ_I2C, wValue, wIndex);
}

void BridgeProtocolI2c::clearBus(uint16_t devAddr)
{
    const uint16_t wValue = (devAddr & 0xF000);
    const uint16_t wIndex = 0;
    m_commands->vendorWrite(REQ_I2C, wValue, wIndex);
}

void BridgeProtocolI2c::pollForAck(uint16_t devAddr)
{
    m_commands->vendorWrite(REQ_I2C, devAddr, REQ_I2C_FLAG_TRANSACTION | REQ_I2C_FLAG_ACK_POLLING);
}
