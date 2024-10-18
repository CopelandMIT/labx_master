/**
 * @copyright 2020 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "BridgeProtocolGpio.hpp"
#include <platform/interfaces/IVendorCommands.hpp>
#include <universal/protocol/protocol_definitions.h>


BridgeProtocolGpio::BridgeProtocolGpio(IVendorCommands *commands) :
    m_commands(commands)
{
}

void BridgeProtocolGpio::configurePin(uint16_t id, uint8_t flags)
{
    m_commands->vendorWrite(REQ_GPIO, (flags << 8) | 2, id);
}

void BridgeProtocolGpio::setPin(uint16_t id, bool state)
{
    m_commands->vendorWrite(REQ_GPIO, state ? 1 : 0, id);
}

bool BridgeProtocolGpio::getPin(uint16_t id)
{
    uint8_t buf;
    m_commands->vendorRead(REQ_GPIO, 0, id, sizeof(buf), &buf);
    return (buf != 0);
}

void BridgeProtocolGpio::configurePort(uint16_t port, uint8_t flags, uint32_t mask)
{
    m_commands->vendorWrite(REQ_PORT, (flags << 8) | 2, port, sizeof(mask), &mask);
}

void BridgeProtocolGpio::setPort(uint16_t port, uint32_t state, uint32_t mask)
{
    const uint32_t buf[] = {mask, state};
    m_commands->vendorWrite(REQ_PORT, 1, port, sizeof(buf), buf);
}

uint32_t BridgeProtocolGpio::getPort(uint16_t port)
{
    uint32_t buf;
    m_commands->vendorRead(REQ_PORT, 0, port, sizeof(buf), &buf);
    return buf;
}
