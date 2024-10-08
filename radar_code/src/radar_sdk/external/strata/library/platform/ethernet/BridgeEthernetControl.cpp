/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "BridgeEthernetControl.hpp"

#include <common/Buffer.hpp>
#include <common/Logger.hpp>
#include <common/Serialization.hpp>
#include <common/crc/Crc16.hpp>
#include <platform/exception/EProtocol.hpp>
#include <platform/exception/EProtocolFunction.hpp>


#ifdef STRATA_LEGACY_PROTOCOL_3
    #undef VENDOR_REQ_WRITE
    #undef VENDOR_REQ_READ
    #undef VENDOR_REQ_TRANSFER

    #define VENDOR_REQ_WRITE    m_protocol.vendorReqWrite
    #define VENDOR_REQ_READ     m_protocol.vendorReqRead
    #define VENDOR_REQ_TRANSFER m_protocol.vendorReqTransfer
#endif


namespace
{
    constexpr const uint16_t controlPort = 55055;

    constexpr const uint16_t defaultTimeout = 1000;
}


BridgeEthernetControl::BridgeEthernetControl(ISocket &socket, ipAddress_t ipAddr) :
    m_protocol(this),
    m_socket(socket),
    m_ipAddr {ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]}
{
    openConnection();
}

BridgeEthernetControl::~BridgeEthernetControl()
{
    closeConnection();
}

IBridgeControl *BridgeEthernetControl::getIBridgeControl()
{
    return &m_protocol;
}

void BridgeEthernetControl::openConnection()
{
    m_socket.open(0, controlPort, m_ipAddr, defaultTimeout);
    m_commandError = false;
}

void BridgeEthernetControl::closeConnection()
{
    m_socket.close();
}

void BridgeEthernetControl::setDefaultTimeout()
{
    m_socket.setTimeout(defaultTimeout);
}

uint16_t BridgeEthernetControl::getMaxTransfer() const
{
    return m_socket.maxPayload() - m_commandHeaderSize;
}

void BridgeEthernetControl::sendRequest(uint8_t *packet, uint16_t sendSize, uint8_t bmReqType, uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength, const uint8_t buffer[])
{
    if ((bmReqType == VENDOR_REQ_WRITE) || (bmReqType == VENDOR_REQ_TRANSFER))
    {
        if (sendSize > m_socket.maxPayload())
        {
            throw EProtocol("Request length is too long, the maximum is ", getMaxTransfer());
        }
        std::copy(buffer, buffer + wLength, &packet[8]);
    }
    else if (bmReqType == VENDOR_REQ_READ)
    {
        static const auto receiveSize = m_responseHeaderSize + wLength;
        if (receiveSize > m_socket.maxPayload())
        {
            throw EProtocol("Read request length is too long, the maximum is ", getMaxTransfer());
        }
    }
    else
    {
        throw EProtocol("Invalid bmReqType", bmReqType);
    }

    packet[0] = bmReqType;
    packet[1] = bRequest;
    packet[2] = static_cast<uint8_t>(wValue);
    packet[3] = static_cast<uint8_t>(wValue >> 8);
    packet[4] = static_cast<uint8_t>(wIndex);
    packet[5] = static_cast<uint8_t>(wIndex >> 8);
    packet[6] = static_cast<uint8_t>(wLength);
    packet[7] = static_cast<uint8_t>(wLength >> 8);

    if (m_commandError)
    {
        m_socket.dumpPacket();
        m_commandError = false;
    }

    m_socket.send(packet, sendSize);
}

void BridgeEthernetControl::receiveResponse(uint8_t *packet, uint16_t receiveSize, uint8_t bmReqType, uint8_t bRequest, uint16_t &wLength, uint8_t buffer[])
{
    uint16_t receivedLength;
    if (m_socket.getMode() == ISocket::Mode::Stream)
    {
        // First only receive the header in stream mode
        receivedLength = receiveStream(packet, m_responseHeaderSize);
    }
    else
    {
        // Receive the complete packet in datagram mode
        receivedLength = receiveDatagram(packet, receiveSize);
    }
    if (receivedLength < m_responseHeaderSize)
    {
        m_commandError = true;
        throw EProtocol("No / incomplete response header received", bRequest);
    }

    const uint8_t &bmResType = packet[0];
    const uint8_t &bStatus   = packet[1];

    if (bmResType != bmReqType)
    {
        throw EProtocol("Request response type error", (bmResType << 8) | bmReqType);
    }
    if (bStatus)
    {
        throw EProtocolFunction(bStatus);
    }

    const auto maxLength = wLength;
    wLength              = serialToHost<uint16_t>(&packet[2]);
    if (bmReqType == VENDOR_REQ_READ)
    {
        if (wLength != maxLength)
        {
            throw EProtocol("Request response length error", (wLength << 16) | (bmReqType << 8) | bRequest);
        }
    }
    if (wLength > maxLength)
    {
        throw EProtocol("Request response too long for buffer", (wLength << 16) | (bmReqType << 8) | bRequest);
    }

    uint16_t receivedPayloadLength;
    if (m_socket.getMode() == ISocket::Mode::Stream)
    {
        // Receive payload directly into destination buffer in stream mode
        receivedPayloadLength = receiveStream(buffer, wLength);
    }
    else
    {
        receivedPayloadLength = receivedLength - m_responseHeaderSize;
        // Copy already received data into destination buffer in datagram mode
        const auto payload = &packet[m_responseHeaderSize];
        std::copy(payload, payload + receivedPayloadLength, buffer);
    }
    if (receivedPayloadLength != wLength)
    {
        m_commandError = true;
        throw EProtocol("No / incomplete response payload received", bRequest);
    }
}

uint16_t BridgeEthernetControl::receiveStream(uint8_t *buffer, uint16_t receiveSize)
{
    // In stream mode the data can be fragmented. We have to re-assemble it
    uint16_t received = 0;
    while (receiveSize != 0)
    {
        uint16_t returnedLength = m_socket.receive(buffer + received, receiveSize);
        if (returnedLength == 0)
        {
            // No more data available
            return received;
        }
        received += returnedLength;
        receiveSize -= returnedLength;
    }
    return received;
}

uint16_t BridgeEthernetControl::receiveDatagram(uint8_t *buffer, uint16_t receiveSize)
{
    // In datagram mode the data should be received in one chunk
    return m_socket.receive(buffer, receiveSize);
}

void BridgeEthernetControl::vendorWrite(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength, const uint8_t buffer[])
{
    std::lock_guard<std::mutex> lock(m_lock);

    const uint16_t sendSize    = m_commandHeaderSize + wLength;
    const uint16_t receiveSize = m_responseHeaderSize;
    strata::buffer<uint8_t> packet(std::max(sendSize, receiveSize));

    sendRequest(packet.data(), sendSize, VENDOR_REQ_WRITE, bRequest, wValue, wIndex, wLength, buffer);
    wLength = 0;
    receiveResponse(packet.data(), receiveSize, VENDOR_REQ_WRITE, bRequest, wLength, nullptr);
}

void BridgeEthernetControl::vendorRead(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength, uint8_t buffer[])
{
    std::lock_guard<std::mutex> lock(m_lock);

    const uint16_t sendSize    = m_commandHeaderSize;
    const uint16_t receiveSize = m_responseHeaderSize + wLength;
    strata::buffer<uint8_t> packet(std::max(sendSize, receiveSize));

    sendRequest(packet.data(), sendSize, VENDOR_REQ_READ, bRequest, wValue, wIndex, wLength, nullptr);
    receiveResponse(packet.data(), receiveSize, VENDOR_REQ_READ, bRequest, wLength, buffer);
}

void BridgeEthernetControl::vendorTransfer(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLengthSend, const uint8_t bufferSend[], uint16_t &wLengthReceive, uint8_t bufferReceive[])
{
    std::lock_guard<std::mutex> lock(m_lock);

    const uint16_t sendSize    = m_commandHeaderSize + wLengthSend;
    const uint16_t receiveSize = m_responseHeaderSize + wLengthReceive;
    strata::buffer<uint8_t> packet(std::max(sendSize, receiveSize));

    sendRequest(packet.data(), sendSize, VENDOR_REQ_TRANSFER, bRequest, wValue, wIndex, wLengthSend, bufferSend);
    receiveResponse(packet.data(), receiveSize, VENDOR_REQ_TRANSFER, bRequest, wLengthReceive, bufferReceive);
}
