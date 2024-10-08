/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "BridgeLibUsb.hpp"
#include "LibUsbHelper.hpp"
#include <common/Buffer.hpp>
#include <common/Logger.hpp>
#include <common/Serialization.hpp>
#include <common/Time.hpp>
#include <platform/exception/EBridgeData.hpp>
#include <platform/exception/EConnection.hpp>
#include <platform/exception/EProtocol.hpp>
#include <platform/exception/EProtocolFunction.hpp>
#include <platform/frames/DebugFrame.hpp>
#include <platform/frames/ErrorFrame.hpp>
#include <universal/protocol/protocol_definitions.h>

#include <cstring>


//#define BRIDGE_LIBUSB_DATA_DEBUG


namespace
{
    constexpr bool setLocalTimestamp = false;

    constexpr const uint16_t frameHeaderSize   = 6;
    constexpr const uint32_t timestampSize     = sizeof(uint64_t);
    constexpr const uint32_t bufferPrefixSize  = sizeof(uint64_t);
    constexpr const uint32_t bufferPrefixStart = bufferPrefixSize - frameHeaderSize;

    constexpr const uint16_t controlTimeout = 1000;
    constexpr const uint16_t dataTimeout    = 200;
    constexpr const uint16_t dumpTimeout    = 10;

    constexpr const int defaultInterface       = 0;
    constexpr const unsigned char dataEndpoint = LIBUSB_DATA_ENDPOINT;
}


BridgeLibUsb::BridgeLibUsb(libusb_device *device, int fd) :
    m_protocol(this),
    m_context {LibUsbHelper::defaultContext},
    m_device {device},
    m_fd {fd},
    m_deviceHandle {nullptr}
{
    if (m_fd && m_device)
    {
        throw EConnection("BridgeLibUsb - cannot specify device and file descriptor at the same time");
    }
    if (!m_fd && !m_device)
    {
        throw EConnection("BridgeLibUsb - either device or file descriptor has to be specified");
    }

    // with an optional file descriptor to use for connection,
    // we want to disable device discovery
    const bool disableDeviceDiscovery  = (m_fd != 0);
    libusb_context **useDefaultContext = NULL;
    LibUsbHelper::init(useDefaultContext, disableDeviceDiscovery);

    //libusb_set_option(m_context, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_DEBUG);

    BridgeLibUsb::openConnection();
}

BridgeLibUsb::~BridgeLibUsb()
{
    BridgeLibUsb::closeConnection();

    libusb_exit(m_context);
}

bool BridgeLibUsb::isConnected()
{
    return m_deviceHandle != nullptr;
}

void BridgeLibUsb::openConnection()
{
    if (m_deviceHandle)
    {
        return;
    }

    m_packetCounter = 0;

    // if we pass in an optional file descriptor, it will be used to connect
    if (m_fd)
    {
        LibUsbHelper::open(m_context, m_fd, &m_deviceHandle);
    }
    else
    {
        LibUsbHelper::open(m_device, &m_deviceHandle);
    }

    const auto ret = libusb_claim_interface(m_deviceHandle, defaultInterface);
    if (ret != LIBUSB_SUCCESS)
    {
        libusb_close(m_deviceHandle);
        m_deviceHandle = nullptr;
        throw EConnection("BridgeLibUsb::openConnection - libusb_claim_interface() failed", ret);
    }
}

void BridgeLibUsb::closeConnection()
{
    BridgeLibUsb::stopStreaming();

    if (!m_deviceHandle)
    {
        return;
    }

    libusb_release_interface(m_deviceHandle, defaultInterface);
    libusb_close(m_deviceHandle);
    m_deviceHandle = nullptr;
}

IBridgeData *BridgeLibUsb::getIBridgeData()
{
    return this;
}

void BridgeLibUsb::setFrameBufferSize(uint32_t size)
{
    // allocate enough buffer memory for header to avoid memory copying
    m_framePool.setFrameBufferSize(bufferPrefixSize + size + timestampSize);
}

void BridgeLibUsb::setFramePoolCount(uint16_t count)
{
    m_framePool.setFrameCount(count);
}

IBridgeControl *BridgeLibUsb::getIBridgeControl()
{
    return &m_protocol;
}

void BridgeLibUsb::startStreaming()
{
    if (isBridgeDataStarted())
    {
        return;
    }

    if (!m_framePool.initialized())
    {
        throw EBridgeData("Calling startData() without frame pool being initialized");
    }
    startBridgeData();
    m_dataThread = std::thread(&BridgeLibUsb::dataThreadFunction, this);
}

void BridgeLibUsb::stopStreaming()
{
    if (!isBridgeDataStarted())
    {
        return;
    }
    stopBridgeData();
    m_dataThread.join();
}

void BridgeLibUsb::setDefaultTimeout()
{
}

uint16_t BridgeLibUsb::getMaxTransfer() const
{
    return m_maxPayload;
}

void BridgeLibUsb::checkStatus()
{
    uint8_t errorInfo[4];
    const uint8_t &bStatus = errorInfo[1];

    controlEndpointRead(VENDOR_REQ_READ, REQ_BOARD_INFO, REQ_BOARD_INFO_ERROR_INFO_WVALUE, REQ_BOARD_INFO_ERROR_INFO_LAST_ERROR_WINDEX, sizeof(errorInfo), errorInfo);
    if (bStatus)
    {
        throw EProtocolFunction(bStatus);
    }
}

void BridgeLibUsb::controlEndpointWrite(uint8_t bmReqType, uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength, const uint8_t buffer[])
{
    const int ret = libusb_control_transfer(m_deviceHandle, bmReqType, bRequest, wValue, wIndex, const_cast<uint8_t *>(buffer), wLength, controlTimeout);
    if (ret < 0)
    {
        throw EConnection("BridgeLibUsb::controlEndpointWrite - libusb_control_transfer() failed", ret);
    }
    if (ret != wLength)
    {
        throw EConnection("BridgeLibUsb::controlEndpointWrite - libusb_control_transfer() incomplete", (ret << 16) | wLength);
    }
}

uint16_t BridgeLibUsb::controlEndpointRead(uint8_t bmReqType, uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength, uint8_t buffer[])
{
    const int ret = libusb_control_transfer(m_deviceHandle, bmReqType, bRequest, wValue, wIndex, buffer, wLength, controlTimeout);
    if (ret < 0)
    {
        throw EConnection("BridgeLibUsb::controlEndpointRead - libusb_control_transfer() failed", ret);
    }
    return static_cast<uint16_t>(ret);
}

uint16_t BridgeLibUsb::controlEndpointReadChecked(uint8_t bmReqType, uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength, uint8_t buffer[])
{
    try
    {
        return controlEndpointRead(bmReqType, bRequest, wValue, wIndex, wLength, buffer);
    }
    catch (const EConnection &e)
    {
        if (e.code() == LIBUSB_ERROR_PIPE)  // FW signaled error for IN request (STALL)
        {
            // function failed, so read error code
            checkStatus();
            // we only reach here if we received E_SUCCESS after USB indicated an error, so something doesn't work as intended
            throw EProtocol("USB signaled an error, but the error code returned by the protocol is E_SUCCESS");
        }
        else
        {
            throw;
        }
    }
}

void BridgeLibUsb::vendorWrite(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength, const uint8_t buffer[])
{
    std::lock_guard<std::mutex> lock(m_lock);

    controlEndpointWrite(VENDOR_REQ_WRITE, bRequest, wValue, wIndex, wLength, buffer);
    checkStatus();
}

void BridgeLibUsb::vendorRead(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength, uint8_t buffer[])
{
    std::lock_guard<std::mutex> lock(m_lock);

    const auto receivedLength = controlEndpointReadChecked(VENDOR_REQ_READ, bRequest, wValue, wIndex, wLength, buffer);
    if (receivedLength != wLength)
    {
        throw EConnection("BridgeLibUsb::vendorRead - controlEndpointReadChecked() incomplete", (receivedLength << 16) | wLength);
    }
}

void BridgeLibUsb::vendorTransfer(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLengthSend, const uint8_t bufferSend[], uint16_t &wLengthReceive, uint8_t bufferReceive[])
{
    std::lock_guard<std::mutex> lock(m_lock);

    controlEndpointWrite(VENDOR_REQ_TRANSFER, bRequest, wValue, wIndex, wLengthSend, bufferSend);
    wLengthReceive = controlEndpointReadChecked(VENDOR_REQ_TRANSFER_2, bRequest, wValue, wIndex, wLengthReceive, bufferReceive);
}

bool BridgeLibUsb::dumpPacket()
{
    try
    {
        stdext::buffer<uint8_t> buf(LIBUSB_MAX_DATA_LENGTH);
        return (LibUsbHelper::readBulk(m_deviceHandle, (LIBUSB_ENDPOINT_IN | dataEndpoint), buf.data(), LIBUSB_MAX_DATA_LENGTH, dumpTimeout) > 0);
    }
    catch (const EProtocol &)
    {
        return false;
    }
}

void BridgeLibUsb::dataThreadFunction()
{
    IFrame *frame = nullptr;
    uint8_t *bufBegin, *bufEnd;
    uint8_t *buf;

    bool firstFrame         = true;
    uint64_t epochTimestamp = 0;
    uint8_t virtualChannel  = 0;

    // in our frame buffer, we overwrite the last bytes of the previous packet with the header of the new one.
    // so we have to restore them after reading a new packet, but we don't have to copy the whole payload every time.
    uint8_t backup[frameHeaderSize];

    while (isBridgeDataStarted())
    {
        if (!frame)
        {
            // try to dequeue frame to read data into
            frame = m_framePool.dequeueFrame();
            if (!frame)
            {
                queueFrame(ErrorFrame::create(DataError_FramePoolDepleted, VIRTUAL_CHANNEL_UNDEFINED));

                // try to discard one packet and try again
                if (dumpPacket())
                {
                    LOG(DEBUG) << "Data read thread - dumped packet";
                    m_packetCounter++;
                }
                continue;
            }

            // prepare frame buffer variables
            bufBegin = frame->getBuffer() + bufferPrefixStart;
            bufEnd   = bufBegin + frame->getBufferSize();
            buf      = bufBegin;  // this will point to (frameHeaderSize) bytes before the end of the current data!
        }

        while (isBridgeDataStarted())
        {
            // fill buffer with frame from multiple packets
            try
            {
                const auto remainingSize = static_cast<int>(bufEnd - buf);
                const auto readSize      = (remainingSize > m_maxPacketSize) ? m_maxPacketSize : remainingSize;
                const auto returnedSize  = LibUsbHelper::readBulk(m_deviceHandle, (LIBUSB_ENDPOINT_IN | dataEndpoint), buf, readSize, dataTimeout);

                if (returnedSize == 0)
                {
                    // no packet available, continue while loop
                    continue;
                }

                if (returnedSize < frameHeaderSize)
                {
                    LOG(DEBUG) << "Data read thread - Packet header incomplete";
                    continue;
                }

                const auto bmPktType = serialToHost<uint8_t>(buf);
                if ((bmPktType & 0xF0) != DATA_FRAME_PACKET)
                {
                    LOG(DEBUG) << "Data read thread - Packet type error: 0x" << std::hex << static_cast<int>(bmPktType);
                    continue;
                }

                const auto bChannel = serialToHost<uint8_t>(buf + 1);
                if (bmPktType & DATA_FRAME_FLAG_FIRST)
                {
                    if (setLocalTimestamp)
                    {
                        epochTimestamp = getEpochTime();
                    }
                    virtualChannel = bChannel;
                }

                const auto wLength = serialToHost<uint16_t>(buf + 4);
                if (returnedSize != frameHeaderSize + wLength)
                {
                    if (remainingSize < frameHeaderSize + wLength)
                    {
                        queueFrame(ErrorFrame::create(DataError_FrameSizeExceeded, bChannel));
                        LOG(DEBUG) << "Data read thread - Frame buffer insufficient - " << wLength + frameHeaderSize - remainingSize << " bytes discarded";
                    }
                    else
                    {
                        LOG(DEBUG) << "Data read thread - Packet length wrong: " << returnedSize << "; expected: " << (frameHeaderSize + wLength);
                    }
                    continue;
                }

                const auto wCounter = serialToHost<uint16_t>(buf + 2);
                if (firstFrame)
                {
#ifdef BRIDGE_LIBUSB_DATA_DEBUG
                    if (wCounter != m_packetCounter)
                    {
                        LOG(DEBUG) << "Data read thread - First frame packet counter reset: received = 0x" << std::hex << wCounter << " , current = 0x" << m_packetCounter;
                    }
#endif
                    firstFrame      = false;
                    m_packetCounter = wCounter + 1;
                }
                else if (wCounter != m_packetCounter)
                {
                    LOG(INFO) << "Data read thread - Packet loss";
#ifdef BRIDGE_LIBUSB_DATA_DEBUG
                    LOG(DEBUG) << "    counter mismatch: received = 0x" << std::hex << wCounter << " , current = 0x" << m_packetCounter;
#endif
                    m_packetCounter = wCounter + 1;

                    queueFrame(ErrorFrame::create(DataError_FrameDropped, bChannel));

                    if (!(bmPktType & DATA_FRAME_FLAG_FIRST))
                    {
                        // if this was a follow-up frame, discard the whole already received part
                        buf = bufBegin;

#ifdef BRIDGE_LIBUSB_DATA_DEBUG
                        LOG(DEBUG) << "Data read thread - discarding current frame";
#endif
                        continue;
                    }
                }
                else
                {
                    m_packetCounter++;
                }

                if (bmPktType & DATA_FRAME_FLAG_FIRST)
                {
                    if (buf != bufBegin)
                    {
                        // we already started receiving a frame, but now a new frame starts
                        // copy received payload to the beginning of buffer, to try to continue with new frame
                        std::copy(buf + frameHeaderSize, buf + frameHeaderSize + wLength, bufBegin);
                        buf = bufBegin;  // continue normally for a single/first packet
#ifdef BRIDGE_LIBUSB_DATA_DEBUG
                        LOG(DEBUG) << "Data read thread - previous frame incomplete: wCounter = 0x" << std::hex << wCounter;
#endif
                    }
                }
                else
                {
                    if (buf == bufBegin)
                    {
                        // we expected a new frame, but we received a follow-up packet
#ifdef BRIDGE_LIBUSB_DATA_DEBUG
                        LOG(DEBUG) << "Data read thread - discarding unexpected follow-up packet";
#endif
                        continue;  // don't do anything with the received packet and start over
                    }

                    if (virtualChannel != bChannel)
                    {
#ifdef BRIDGE_LIBUSB_DATA_DEBUG
                        LOG(DEBUG) << "Data read thread - Channel mismatch: received = 0x" << std::hex << static_cast<int>(bChannel) << " , expected = 0x" << static_cast<int>(virtualChannel);
#endif
                        continue;  // don't do anything with the received packet and start over
                    }

                    // restore backup of previous packet, since we overwrote it with the beginning of the current packet
                    std::copy(backup, backup + frameHeaderSize, buf);
                }

                buf += wLength;

                if (bmPktType & DATA_FRAME_FLAG_LAST)
                {
                    if (bmPktType & DATA_FRAME_FLAG_TIMESTAMP)
                    {
                        buf -= sizeof(epochTimestamp);
                        if (!setLocalTimestamp)
                        {
                            serialToHost(buf + frameHeaderSize, epochTimestamp);
                        }
                    }
                    else if (!setLocalTimestamp)
                    {
                        epochTimestamp = 0;
                    }

                    if (bmPktType & DATA_FRAME_FLAG_ERROR)
                    {
                        uint32_t code;
                        const auto errorFrameLength = sizeof(code) + ((bmPktType & DATA_FRAME_FLAG_TIMESTAMP) ? sizeof(epochTimestamp) : 0);
                        if (wLength == errorFrameLength)
                        {
                            buf -= sizeof(code);
                            serialToHost(buf + frameHeaderSize, code);
                            queueFrame(ErrorFrame::create(code, bChannel, epochTimestamp));
                        }
                        else
                        {
                            buf -= wLength;
                            DebugFrame::log(buf + frameHeaderSize, wLength, epochTimestamp);
                        }
                        buf = bufBegin;
                    }
                    else
                    {
                        frame->setDataOffset(bufferPrefixSize);
                        frame->setDataSize(static_cast<uint32_t>(buf - bufBegin));
                        frame->setVirtualChannel(virtualChannel);
                        frame->setTimestamp(epochTimestamp);

                        queueFrame(frame);
                        frame = nullptr;
                    }

                    break;
                }
                else
                {
                    // save backup of current packet, since it will be overwritten by next packet
                    std::copy(buf, buf + frameHeaderSize, backup);
                }
            }
            catch (const std::exception &e)
            {
                queueFrame(ErrorFrame::create(DataError_LowLevelError, VIRTUAL_CHANNEL_UNDEFINED));
                LOG(DEBUG) << "Data read thread - " << e.what();
            }
        }
    }

    // if we own a dequeued frame buffer, make sure we return it
    if (frame)
    {
        m_framePool.queueFrame(frame);
    }
}
