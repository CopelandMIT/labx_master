/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "BridgeWiggler.hpp"

#include <common/Logger.hpp>
#include <platform/exception/EConnection.hpp>


BridgeWiggler::BridgeWiggler(uint8_t systemIndex) :
    m_protocol(this),
    m_systemIndex {systemIndex}
{
    BridgeWiggler::openConnection();
}

BridgeWiggler::~BridgeWiggler()
{
    BridgeWiggler::closeConnection();
}

bool BridgeWiggler::isConnected()
{
    return m_debugger.isOpened();
}

void BridgeWiggler::openConnection()
{
    m_debugger.open(m_systemIndex);
}

void BridgeWiggler::closeConnection()
{
    m_debugger.close();
}

IBridgeControl *BridgeWiggler::getIBridgeControl()
{
    return &m_protocol;
}

IBridgeData *BridgeWiggler::getIBridgeData()
{
    return this;
}

void BridgeWiggler::startStreaming()
{
}

void BridgeWiggler::stopStreaming()
{
}

void BridgeWiggler::setFrameBufferSize(uint32_t /*size*/)
{
}

void BridgeWiggler::setFramePoolCount(uint16_t /*count*/)
{
}

uint16_t BridgeWiggler::getMaxTransfer() const
{
    const auto maxTransfer = m_debugger.getMaxPayload() - m_commandHeaderSize;
    if (maxTransfer > UINT16_MAX)
    {
        return UINT16_MAX;
    }
    else
    {
        return static_cast<uint16_t>(maxTransfer);
    }
}

void BridgeWiggler::vendorWrite(uint8_t /*bRequest*/, uint16_t /*wValue*/, uint16_t /*wIndex*/, uint16_t /*wLength*/, const uint8_t /*buffer*/[])
{
    //    uint8_t packet[m_commandSize + crcSize];
    //    packet[0] = VENDOR_REQ_WRITE;
    //    packet[1] = bRequest;
    //    packet[2] = static_cast<uint8_t>(wValue);
    //    packet[3] = static_cast<uint8_t>(wValue >> 8);
    //    packet[4] = static_cast<uint8_t>(wIndex);
    //    packet[5] = static_cast<uint8_t>(wIndex >> 8);
    //    packet[6] = static_cast<uint8_t>(wLength);
    //    packet[7] = static_cast<uint8_t>(wLength >> 8);

    //    uint16_t crc = Crc16CcittFalse(packet, 8);
    //    if (wLength)
    //    {
    //        crc = Crc16CcittFalse(buffer, wLength, crc);
    //    }
    //    packet[m_commandSize] = static_cast<uint8_t>(crc >> 8);
    //    packet[m_commandSize + 1] = static_cast<uint8_t>(crc);

    //    std::lock_guard<std::mutex> lock(m_lock);
    //    clearFifo();

    //    if (wLength)
    //    {
    //        writeBuffer(packet, m_commandSize);
    //        writeBuffer(buffer, wLength);
    //        writeBuffer(&packet[8], crcSize);
    //    }
    //    else
    //    {
    //        writeBuffer(packet, m_commandSize + crcSize);
    //    }

    //    readBuffer(packet, responseSize + crcSize, defaultTimeout);
    //    crc = Crc16CcittFalse(packet, responseSize + crcSize);
    //    if (crc)
    //    {
    //        throw EProtocol("Write command response CRC error", crc);
    //    }
    //    const uint8_t &bmResType = packet[0];
    //    const uint8_t &bStatus = packet[1];
    //    if ((bmResType != VENDOR_REQ_WRITE) || bStatus)
    //    {
    //        throw EProtocol("Write command response status error", (bmResType << 16) | bStatus);
    //    }
}

void BridgeWiggler::vendorRead(uint8_t /*bRequest*/, uint16_t /*wValue*/, uint16_t /*wIndex*/, uint16_t /*wLength*/, uint8_t /*buffer*/[])
{
    //    uint8_t packet[m_commandSize + crcSize];
    //    packet[0] = VENDOR_REQ_READ;
    //    packet[1] = bRequest;
    //    packet[2] = static_cast<uint8_t>(wValue);
    //    packet[3] = static_cast<uint8_t>(wValue >> 8);
    //    packet[4] = static_cast<uint8_t>(wIndex);
    //    packet[5] = static_cast<uint8_t>(wIndex >> 8);
    //    packet[6] = static_cast<uint8_t>(wLength);
    //    packet[7] = static_cast<uint8_t>(wLength >> 8);

    //    uint16_t crc = Crc16CcittFalse(packet, m_commandSize);
    //    packet[m_commandSize] = static_cast<uint8_t>(crc >> 8);
    //    packet[m_commandSize + 1] = static_cast<uint8_t>(crc);

    //    std::lock_guard<std::mutex> lock(m_lock);
    //    clearFifo();

    //    writeBuffer(packet, m_commandSize + crcSize);

    //    readBuffer(packet, responseSize, defaultTimeout);
    //    const uint8_t &bmResType = packet[0];
    //    const uint8_t &bStatus = packet[1];
    //    if ((bmResType != VENDOR_REQ_READ) || bStatus)
    //    {
    //        throw EProtocol("Read command response status error", (bmResType << 16) | bStatus);
    //    }
    //    const uint16_t responseLength = (packet[3] << 8) | packet[2];
    //    if (responseLength != wLength)
    //    {
    //        throw EProtocol("Read command response length error", (responseLength << 16) | wLength);
    //    }
    //    readBuffer(buffer, wLength, defaultTimeout);
    //    readBuffer(&packet[responseSize], crcSize, defaultTimeout);

    //    // check CRC last to avoid several timeouts in case of error (error will anyways be detected from potentially corrupted response data first)
    //    crc = Crc16CcittFalse(packet, responseSize);
    //    crc = Crc16CcittFalse(buffer, wLength, crc);
    //    crc = Crc16CcittFalse(&packet[responseSize], crcSize, crc);
    //    if (crc)
    //    {
    //        throw EProtocol("Read command response CRC error", crc);
    //    }
}

void BridgeWiggler::vendorTransfer(uint8_t /*bRequest*/, uint16_t /*wValue*/, uint16_t /*wIndex*/, uint16_t /*wLengthSend*/, const uint8_t /*bufferSend*/[], uint16_t & /*wLengthReceive*/, uint8_t /*bufferReceive*/[])
{
    //To be implemented
}
