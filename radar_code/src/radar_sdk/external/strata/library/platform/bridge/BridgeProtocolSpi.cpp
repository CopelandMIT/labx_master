/**
 * @copyright 2020 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "BridgeProtocolSpi.hpp"
#include <common/NarrowCast.hpp>
#include <platform/interfaces/IVendorCommands.hpp>
#include <universal/protocol/protocol_definitions.h>


BridgeProtocolSpi::BridgeProtocolSpi(IVendorCommands *commands) :
    m_commands(commands)
{
}

void BridgeProtocolSpi::configure(uint8_t devId, uint8_t flags, uint8_t wordSize, uint32_t speed)
{
    const uint16_t wValue = static_cast<uint16_t>(wordSize << 8) | devId;
    const uint16_t wIndex = static_cast<uint16_t>(flags);
    m_commands->vendorWrite(REQ_SPI, wValue, wIndex, sizeof(speed), &speed);
}

uint32_t BridgeProtocolSpi::getMaxTransfer() const
{
    return m_commands->getMaxTransfer();
}

void BridgeProtocolSpi::write(uint8_t devId, uint32_t count, const uint8_t buffer[], bool keepSel)
{
    const uint8_t bufferWidth       = sizeof(uint8_t);
    const uint16_t keepSelectedFlag = keepSel ? REQ_SPI_FLAG_KEEP_SELECTED : 0;
    const uint16_t wValue           = static_cast<uint16_t>(bufferWidth << 8) | devId;
    const uint16_t wIndex           = REQ_SPI_FLAG_TRANSACTION | keepSelectedFlag;
    const uint16_t wLength          = narrow_cast<uint16_t>(count * sizeof(uint8_t));
    m_commands->vendorWrite(REQ_SPI, wValue, wIndex, wLength, buffer);
}

void BridgeProtocolSpi::write(uint8_t devId, uint32_t count, const uint16_t buffer[], bool keepSel)
{
    const uint8_t bufferWidth       = sizeof(uint16_t);
    const uint16_t keepSelectedFlag = keepSel ? REQ_SPI_FLAG_KEEP_SELECTED : 0;
    const uint16_t wValue           = static_cast<uint16_t>(bufferWidth << 8) | devId;
    const uint16_t wIndex           = REQ_SPI_FLAG_TRANSACTION | keepSelectedFlag;
    const uint16_t wLength          = narrow_cast<uint16_t>(count * sizeof(uint16_t));
    m_commands->vendorWrite(REQ_SPI, wValue, wIndex, wLength, buffer);
}

void BridgeProtocolSpi::write(uint8_t devId, uint32_t count, const uint32_t buffer[], bool keepSel)
{
    const uint8_t bufferWidth       = sizeof(uint32_t);
    const uint16_t keepSelectedFlag = keepSel ? REQ_SPI_FLAG_KEEP_SELECTED : 0;
    const uint16_t wValue           = static_cast<uint16_t>(bufferWidth << 8) | devId;
    const uint16_t wIndex           = REQ_SPI_FLAG_TRANSACTION | keepSelectedFlag;
    const uint16_t wLength          = narrow_cast<uint16_t>(count * sizeof(uint32_t));
    m_commands->vendorWrite(REQ_SPI, wValue, wIndex, wLength, buffer);
}

void BridgeProtocolSpi::read(uint8_t devId, uint32_t count, uint8_t buffer[], bool keepSel)
{
    const uint8_t bufferWidth       = sizeof(uint8_t);
    const uint16_t keepSelectedFlag = keepSel ? REQ_SPI_FLAG_KEEP_SELECTED : 0;
    const uint16_t wValue           = static_cast<uint16_t>(bufferWidth << 8) | devId;
    const uint16_t wIndex           = REQ_SPI_FLAG_TRANSACTION | keepSelectedFlag;
    const uint16_t wLength          = narrow_cast<uint16_t>(count * sizeof(uint8_t));
    m_commands->vendorRead(REQ_SPI, wValue, wIndex, wLength, buffer);
}

void BridgeProtocolSpi::read(uint8_t devId, uint32_t count, uint16_t buffer[], bool keepSel)
{
    const uint8_t bufferWidth       = sizeof(uint16_t);
    const uint16_t keepSelectedFlag = keepSel ? REQ_SPI_FLAG_KEEP_SELECTED : 0;
    const uint16_t wValue           = static_cast<uint16_t>(bufferWidth << 8) | devId;
    const uint16_t wIndex           = REQ_SPI_FLAG_TRANSACTION | keepSelectedFlag;
    const uint16_t wLength          = narrow_cast<uint16_t>(count * sizeof(uint16_t));
    m_commands->vendorRead(REQ_SPI, wValue, wIndex, wLength, buffer);
}

void BridgeProtocolSpi::read(uint8_t devId, uint32_t count, uint32_t buffer[], bool keepSel)
{
    const uint8_t bufferWidth       = sizeof(uint32_t);
    const uint16_t keepSelectedFlag = keepSel ? REQ_SPI_FLAG_KEEP_SELECTED : 0;
    const uint16_t wValue           = static_cast<uint16_t>(bufferWidth << 8) | devId;
    const uint16_t wIndex           = REQ_SPI_FLAG_TRANSACTION | keepSelectedFlag;
    const uint16_t wLength          = narrow_cast<uint16_t>(count * sizeof(uint32_t));
    m_commands->vendorRead(REQ_SPI, wValue, wIndex, wLength, buffer);
}

void BridgeProtocolSpi::transfer(uint8_t devId, uint32_t count, const uint8_t bufWrite[], uint8_t bufRead[], bool keepSel)
{
    const uint8_t bufferWidth       = sizeof(uint8_t);
    const uint16_t keepSelectedFlag = keepSel ? REQ_SPI_FLAG_KEEP_SELECTED : 0;
    const uint16_t wValue           = static_cast<uint16_t>(bufferWidth << 8) | devId;
    const uint16_t wIndex           = REQ_SPI_FLAG_TRANSACTION | keepSelectedFlag;
    const uint16_t wLengthSend      = narrow_cast<uint16_t>(count * bufferWidth);
    uint16_t wLengthReceive         = wLengthSend;
    m_commands->vendorTransfer(REQ_SPI, wValue, wIndex, wLengthSend, bufWrite, wLengthReceive, bufRead);
}

void BridgeProtocolSpi::transfer(uint8_t devId, uint32_t count, const uint16_t bufWrite[], uint16_t bufRead[], bool keepSel)
{
    const uint8_t bufferWidth       = sizeof(uint16_t);
    const uint16_t keepSelectedFlag = keepSel ? REQ_SPI_FLAG_KEEP_SELECTED : 0;
    const uint16_t wValue           = static_cast<uint16_t>(bufferWidth << 8) | devId;
    const uint16_t wIndex           = REQ_SPI_FLAG_TRANSACTION | keepSelectedFlag;
    const uint16_t wLengthSend      = narrow_cast<uint16_t>(count * bufferWidth);
    uint16_t wLengthReceive         = wLengthSend;
    m_commands->vendorTransfer(REQ_SPI, wValue, wIndex, wLengthSend, bufWrite, wLengthReceive, bufRead);
}

void BridgeProtocolSpi::transfer(uint8_t devId, uint32_t count, const uint32_t bufWrite[], uint32_t bufRead[], bool keepSel)
{
    const uint8_t bufferWidth       = sizeof(uint32_t);
    const uint16_t keepSelectedFlag = keepSel ? REQ_SPI_FLAG_KEEP_SELECTED : 0;
    const uint16_t wValue           = static_cast<uint16_t>(bufferWidth << 8) | devId;
    const uint16_t wIndex           = REQ_SPI_FLAG_TRANSACTION | keepSelectedFlag;
    const uint16_t wLengthSend      = narrow_cast<uint16_t>(count * bufferWidth);
    uint16_t wLengthReceive         = wLengthSend;
    m_commands->vendorTransfer(REQ_SPI, wValue, wIndex, wLengthSend, bufWrite, wLengthReceive, bufRead);
}
