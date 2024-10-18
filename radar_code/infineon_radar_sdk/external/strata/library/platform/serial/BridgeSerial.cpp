/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "BridgeSerial.hpp"

#include <common/Buffer.hpp>
#include <common/Finally.hpp>
#include <common/Logger.hpp>
#include <common/Serialization.hpp>
#include <common/Time.hpp>
#include <common/crc/Crc16.hpp>
#include <platform/exception/EBridgeData.hpp>
#include <platform/exception/EProtocol.hpp>
#include <platform/exception/EProtocolFunction.hpp>
#include <platform/frames/DebugFrame.hpp>
#include <platform/frames/ErrorFrame.hpp>


//#define BRIDGE_SERIAL_DATA_DEBUG


namespace
{
    constexpr bool setLocalTimestamp = false;

    constexpr const uint16_t packetStartSize = 4;
    constexpr const uint16_t packetCrcSize   = 2;
    constexpr const uint16_t frameHeaderSize = 6;
    constexpr const uint32_t timestampSize   = sizeof(uint64_t);

    constexpr const uint32_t defaultBaudrate = 921600;
    constexpr const uint16_t portTimeout     = 100;

    constexpr const std::chrono::milliseconds enumerateTimeout(portTimeout);
    constexpr const std::chrono::milliseconds defaultTimeout(1000);
}


BridgeSerial::BridgeSerial(const char port[]) :
    m_protocol(this),
    m_portName {port}
{
    BridgeSerial::openConnection();
}

BridgeSerial::~BridgeSerial()
{
    BridgeSerial::closeConnection();
}

bool BridgeSerial::isConnected()
{
    return m_port.isOpened();
}

void BridgeSerial::openConnection()
{
    m_packetCounter = 0;
    m_commandActive = false;
    m_resynchronize = false;

    m_cachedPacket = None;

    m_port.open(m_portName.c_str(), defaultBaudrate, portTimeout);
    m_port.clearInputBuffer();  // if the previous connection was not closed gracefully, there might be stale data left

    m_timeout = enumerateTimeout;
}

void BridgeSerial::closeConnection()
{
    BridgeSerial::stopStreaming();
    m_port.close();
}

IBridgeData *BridgeSerial::getIBridgeData()
{
    return this;
}

void BridgeSerial::setFrameBufferSize(uint32_t size)
{
    m_framePool.setFrameBufferSize(size + timestampSize);
}

void BridgeSerial::setFramePoolCount(uint16_t count)
{
    m_framePool.setFrameCount(count);
}

IBridgeControl *BridgeSerial::getIBridgeControl()
{
    return &m_protocol;
}

void BridgeSerial::startStreaming()
{
    if (isBridgeDataStarted())
    {
        return;
    }

    if (!m_port.isOpened())
    {
        throw EBridgeData("Calling startData() without being connected");
    }
    if (!m_framePool.initialized())
    {
        throw EBridgeData("Calling startData() without frame pool being initialized");
    }

    if (m_resynchronize)
    {
        std::unique_lock<std::mutex> lock(m_lock);
        m_cv.wait(lock, [this] { return (!m_commandActive); });
        if (m_resynchronize)  // check again if it has been cleared in the mean time
        {
            m_port.clearInputBuffer();
            m_resynchronize = false;
        }
    }
    startBridgeData();

    m_dataThread = std::thread(&BridgeSerial::dataThreadFunction, this);
}

void BridgeSerial::stopStreaming()
{
    if (!isBridgeDataStarted())
    {
        return;
    }

    stopBridgeData();

    {
        std::lock_guard<std::mutex> lock(m_lock);  // lock to update CV without race condition
    }
    m_cv.notify_all();
    m_dataThread.join();
}

void BridgeSerial::setDefaultTimeout()
{
    m_timeout = defaultTimeout;
}

uint16_t BridgeSerial::getMaxTransfer() const
{
    return m_maxPayload - m_commandHeaderSize - packetCrcSize;
}

bool BridgeSerial::readPacketStart(uint8_t buffer[], PacketType type, bool discardOther)
{
    if (m_cachedPacket == type)
    {
        std::copy(std::begin(m_packetStartCache), std::end(m_packetStartCache), buffer);

        // lock to update CV without race condition is already acquired by caller
        m_cachedPacket = None;
        m_cv.notify_one();

        return true;
    }
    else if (m_cachedPacket == None)
    {
        const uint16_t returnedSize = m_port.receive(buffer, packetStartSize);
        if (!returnedSize)
        {
            return false;
        }
        else if (returnedSize < packetStartSize)
        {
            throw EProtocol("readPacketStart() - incomplete", returnedSize);
        }
        else
        {
            PacketType currentPacket;
            const auto &bmPktType = buffer[0];

            if ((bmPktType & 0xF0) == DATA_FRAME_PACKET)
            {
                currentPacket = Data;
            }
            else
            {
                switch (bmPktType)
                {
#ifdef STRATA_LEGACY_PROTOCOL_3
                    case VENDOR_REQ_READ_LEGACY:
                    case VENDOR_REQ_WRITE_LEGACY:
                    case VENDOR_REQ_TRANSFER_LEGACY:
#endif
                    case VENDOR_REQ_READ:
                    case VENDOR_REQ_WRITE:
                    case VENDOR_REQ_TRANSFER:
                        currentPacket = Control;
                        break;
                    default:
                        m_resynchronize = true;
                        // or implement synchronization recovery mechanism
                        throw EProtocol("readPacketStart() - unknown packet type, synchronization lost!", (buffer[0] << 24) | (buffer[1] << 24) | (buffer[2] << 8) | buffer[3]);
                }
            }

            if (currentPacket == type)
            {
                return true;
            }

            std::copy(buffer, buffer + packetStartSize, m_packetStartCache);
            m_cachedPacket = currentPacket;
        }
    }
    else if (discardOther)
    {
        uint16_t wLength;
        if (m_cachedPacket == Data)
        {
            // data is not started, but we received a data packet, so we just dump it...
            m_port.receive(reinterpret_cast<uint8_t *>(&wLength), 2);
            littleToHost(wLength);
#ifdef BRIDGE_SERIAL_DATA_DEBUG
            LOG(DEBUG) << "BridgeSerial::readPacketStart() - discarding cached data packet";
#endif
        }
        else  // Control
        {
            // control response received, but no command is active, so we just dump it...
            wLength = serialToHost<uint16_t>(m_packetStartCache + 2);
#ifdef BRIDGE_SERIAL_DATA_DEBUG
            LOG(DEBUG) << "BridgeSerial::readPacketStart() - discarding cached control packet";
#endif
        }

        strata::buffer<uint8_t> dump(wLength + packetCrcSize);
        m_port.receive(dump.data(), wLength + packetCrcSize);

        m_cachedPacket = None;
    }

    // lock to update CV without race condition is already acquired by caller
    m_cv.notify_one();
    return false;
}

void BridgeSerial::dumpRemainder(uint16_t wLength)
{
    strata::buffer<uint8_t> dump(wLength + packetCrcSize);
    m_port.receive(dump.data(), wLength + packetCrcSize);  // try to not lose the snyc by reading the remaining data packet
}


#ifdef STRATA_LEGACY_PROTOCOL_3
    // this has to be done after the defines are used in the case statement in readPacketStart()
    #undef VENDOR_REQ_WRITE
    #undef VENDOR_REQ_READ
    #undef VENDOR_REQ_TRANSFER

    #define VENDOR_REQ_WRITE    m_protocol.vendorReqWrite
    #define VENDOR_REQ_READ     m_protocol.vendorReqRead
    #define VENDOR_REQ_TRANSFER m_protocol.vendorReqTransfer
#endif


void BridgeSerial::sendRequest(uint8_t bmReqType, uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength, const uint8_t buffer[])
{
    uint8_t packet[m_commandHeaderSize + packetCrcSize] = {
        bmReqType,
        bRequest,
        static_cast<uint8_t>(wValue),
        static_cast<uint8_t>(wValue >> 8),
        static_cast<uint8_t>(wIndex),
        static_cast<uint8_t>(wIndex >> 8),
        static_cast<uint8_t>(wLength),
        static_cast<uint8_t>(wLength >> 8),
    };

    if (bmReqType == VENDOR_REQ_READ)
    {
        wLength = 0;
    }

    uint16_t crc = Crc16CcittFalse(packet, m_commandHeaderSize);
    if (wLength)
    {
        crc = Crc16CcittFalse(buffer, wLength, crc);
    }
    packet[m_commandHeaderSize]     = static_cast<uint8_t>(crc >> 8);
    packet[m_commandHeaderSize + 1] = static_cast<uint8_t>(crc);

    if (m_resynchronize && !isBridgeDataStarted())
    {
        m_commandActive = true;  // block access to port already here
        m_port.clearInputBuffer();
    }

    if (wLength)
    {
        m_port.send(packet, m_commandHeaderSize);
        m_port.send(buffer, wLength);
        m_port.send(&packet[m_commandHeaderSize], packetCrcSize);
    }
    else
    {
        m_port.send(packet, m_commandHeaderSize + packetCrcSize);
    }
    m_commandActive = true;
}

void BridgeSerial::receiveResponse(uint8_t bmReqType, uint8_t bRequest, uint16_t &wLength, uint8_t buffer[])
{
    const auto maxLength = wLength;
    uint8_t packet[m_responseHeaderSize + packetCrcSize];
    {
        std::unique_lock<std::mutex> lock(m_lock);
        auto endCommand = strata::finally([this] {
            m_commandActive = false;
            m_cv.notify_one();
        });

        m_resynchronize       = true;  // if we exit this block with an error, this has to be set
        uint16_t returnedSize = 0;
        const auto expiry     = std::chrono::steady_clock::now() + m_timeout;
        do
        {
            m_cv.wait(lock, [this] { return (m_cachedPacket == Control) || !isBridgeDataStarted(); });
            if (readPacketStart(packet, Control, !isBridgeDataStarted()))
            {
                returnedSize = packetStartSize;  // no further read necessary, since (m_responseHeaderSize == packetStartSize)
                break;
            }
        } while (std::chrono::steady_clock::now() < expiry);
        if (returnedSize < m_responseHeaderSize)
        {
            throw EProtocol("Request response header not received", returnedSize);
        }

        wLength = serialToHost<uint16_t>(&packet[2]);
        if (wLength > maxLength)
        {
            throw EProtocol("Request response too long for buffer", (wLength << 16) | (bmReqType << 8) | bRequest);
        }
        if (wLength != 0)
        {
            if (m_port.receive(buffer, wLength) != wLength)
            {
                throw EProtocol("Request response payload not completely received");
            }
        }
        if (m_port.receive(&packet[m_responseHeaderSize], packetCrcSize) != packetCrcSize)
        {
            throw EProtocol("Request response CRC not completely received");
        }
        m_resynchronize = false;  // if we make it here, we are synchronous
    }

    const uint8_t &bmResType = packet[0];
    const uint8_t &bStatus   = packet[1];

    uint16_t crc = Crc16CcittFalse(packet, m_responseHeaderSize);
    if (wLength != 0)
    {
        crc = Crc16CcittFalse(buffer, wLength, crc);
    }
    crc = Crc16CcittFalse(&packet[m_responseHeaderSize], packetCrcSize, crc);
    if (crc)
    {
        throw EProtocol("Request response CRC error", (crc << 16) | (bmResType << 8) | bRequest);
    }
    if (bmResType != bmReqType)
    {
        throw EProtocol("Request response type error", (bmResType << 8) | bmReqType);
    }
    if (bStatus)
    {
#ifdef STRATA_LEGACY_PROTOCOL_3
        if (bStatus == STATUS_REQUEST_TYPE_INVALID)
        {
            m_resynchronize = true;
        }
#endif
        throw EProtocolFunction(bStatus);
    }
    if (bmReqType == VENDOR_REQ_READ)
    {
        if (wLength != maxLength)
        {
            throw EProtocol("Read request response length error", (wLength << 16) | maxLength);
        }
    }
}

void BridgeSerial::vendorWrite(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength, const uint8_t buffer[])
{
    std::lock_guard<std::mutex> lock(m_commandLock);

    sendRequest(VENDOR_REQ_WRITE, bRequest, wValue, wIndex, wLength, buffer);
    wLength = 0;
    receiveResponse(VENDOR_REQ_WRITE, bRequest, wLength, nullptr);
}

void BridgeSerial::vendorRead(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength, uint8_t buffer[])
{
    std::lock_guard<std::mutex> lock(m_commandLock);

    sendRequest(VENDOR_REQ_READ, bRequest, wValue, wIndex, wLength, nullptr);
    receiveResponse(VENDOR_REQ_READ, bRequest, wLength, buffer);
}

void BridgeSerial::vendorTransfer(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLengthSend, const uint8_t bufferSend[], uint16_t &wLengthReceive, uint8_t bufferReceive[])
{
    std::lock_guard<std::mutex> lock(m_commandLock);

    sendRequest(VENDOR_REQ_TRANSFER, bRequest, wValue, wIndex, wLengthSend, bufferSend);
    receiveResponse(VENDOR_REQ_TRANSFER, bRequest, wLengthReceive, bufferReceive);
}

void BridgeSerial::dataThreadFunction()
{
    bool firstFrame = true;
    IFrame *frame   = nullptr;
    uint64_t epochTimestamp;

    // the following initializations are only to avoid false positive warnings of uninitialized variable
    uint8_t virtualChannel = 0;
    uint8_t *bufBegin      = nullptr;
    uint8_t *bufEnd        = nullptr;
    uint8_t *buf           = nullptr;

    // fill frame buffer from multiple data packets
    while (isBridgeDataStarted())
    {
        // read one data packet
        std::unique_lock<std::mutex> lock(m_lock);
        m_cv.wait(lock, [this] { return ((m_cachedPacket != Control) || !m_commandActive); });

        try
        {
            uint8_t packetHeader[frameHeaderSize];
            uint16_t returnedSize = 0;
            if (readPacketStart(packetHeader, Data, !m_commandActive))
            {
                returnedSize = packetStartSize + m_port.receive(packetHeader + packetStartSize, frameHeaderSize - packetStartSize);
            }
            if (returnedSize < frameHeaderSize)
            {
                // no data packet available, continue while loop
#ifdef BRIDGE_SERIAL_DATA_DEBUG
                if (returnedSize)
                {
                    LOG(DEBUG) << "Data read thread - Packet header incomplete";
                }
#endif
                continue;
            }

            const auto bmPktType = serialToHost<uint8_t>(packetHeader + 0);
            if ((bmPktType & 0xF0) != DATA_FRAME_PACKET)
            {
                {
#ifdef BRIDGE_SERIAL_DATA_DEBUG
                    LOG(DEBUG) << "Data read thread - Packet type error: 0x" << std::hex << static_cast<int>(bmPktType);
#endif
                    continue;
                }
            }
            const auto bChannel = serialToHost<uint8_t>(packetHeader + 1);
            const auto wLength  = serialToHost<uint16_t>(packetHeader + 4);

            const auto wCounter = serialToHost<uint16_t>(packetHeader + 2);
            if (firstFrame)
            {
#ifdef BRIDGE_SERIAL_DATA_DEBUG
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
#ifdef BRIDGE_SERIAL_DATA_DEBUG
                LOG(DEBUG) << "    counter mismatch: received = 0x" << std::hex << wCounter << " , expected = 0x" << m_packetCounter;
#endif
                m_packetCounter = wCounter + 1;

                queueFrame(ErrorFrame::create(DataError_FrameDropped, bChannel));

                if (!(bmPktType & DATA_FRAME_FLAG_FIRST))
                {
                    // if this was a follow-up frame, discard the whole already received part
#ifdef BRIDGE_SERIAL_DATA_DEBUG
                    LOG(DEBUG) << "Data read thread - discarding current frame";
#endif
                    dumpRemainder(wLength);
                    continue;
                }
            }
            else
            {
                m_packetCounter++;
            }

            if (!frame)
            {
                // try to dequeue frame to read data into
                frame = m_framePool.dequeueFrame();

                if (frame)
                {
                    // prepare frame buffer variables
                    bufBegin = frame->getBuffer();
                    bufEnd   = bufBegin + frame->getBufferSize();
                    buf      = bufBegin;
                }
            }
            const auto remainingSize = bufEnd - buf;
            if (!frame || (wLength > remainingSize))
            {
                dumpRemainder(wLength);

                if (frame == nullptr)
                {
                    queueFrame(ErrorFrame::create(DataError_FramePoolDepleted, VIRTUAL_CHANNEL_UNDEFINED));
#ifdef BRIDGE_SERIAL_DATA_DEBUG
                    LOG(DEBUG) << "Data read thread - frame pool depleted, dumping packet";
#endif
                }
                else
                {
                    queueFrame(ErrorFrame::create(DataError_FrameSizeExceeded, bChannel));
#ifdef BRIDGE_SERIAL_DATA_DEBUG
                    LOG(DEBUG) << "Data read thread - frame size exceeded, dumping packet";
#endif
                }
                continue;  // restart loop which will again try to dequeue frame buffer
            }

            if (bmPktType & DATA_FRAME_FLAG_FIRST)
            {
                if (setLocalTimestamp)
                {
                    epochTimestamp = getEpochTime();
                }
                virtualChannel = bChannel;

                if (buf != bufBegin)
                {
                    // we already started receiving a frame, but now a new frame starts
#ifdef BRIDGE_SERIAL_DATA_DEBUG
                    LOG(DEBUG) << "Data read thread - previous frame incomplete";
#endif
                    buf = bufBegin;  // continue normally for a single/first packet
                }
            }

            if (m_port.receive(buf, wLength) != wLength)
            {
#ifdef BRIDGE_SERIAL_DATA_DEBUG
                LOG(DEBUG) << "Data read thread - Packet payload incomplete";
#endif
                buf = bufBegin;  // the frame is corrupted, so reset everyting for the next frame start
                queueFrame(ErrorFrame::create(DataError_FrameDropped, bChannel));
                continue;
            }

            uint8_t packetCrc[packetCrcSize];
            if (m_port.receive(packetCrc, packetCrcSize) != packetCrcSize)
            {
#ifdef BRIDGE_SERIAL_DATA_DEBUG
                LOG(DEBUG) << "Data read thread - Packet CRC incomplete";
#endif
                buf = bufBegin;  // the frame is corrupted, so reset everyting for the next frame start
                queueFrame(ErrorFrame::create(DataError_FrameDropped, bChannel));
                continue;
            }

            // we do not need to read from the serial port for this data packet any more
            lock.unlock();

            uint16_t crc = Crc16CcittFalse(packetHeader, frameHeaderSize);
            crc          = Crc16CcittFalse(buf, wLength, crc);
            crc          = Crc16CcittFalse(packetCrc, packetCrcSize, crc);
            if (crc)
            {
#ifdef BRIDGE_SERIAL_DATA_DEBUG
                LOG(DEBUG) << "Data read thread - Packet CRC error: 0x" << std::hex << crc;
#endif
                buf = bufBegin;  // the frame is corrupted, so reset everyting for the next frame start
                queueFrame(ErrorFrame::create(DataError_FrameDropped, bChannel));
                continue;
            }

            if (!(bmPktType & DATA_FRAME_FLAG_FIRST))
            {
                if (buf == bufBegin)
                {
                    // we expected a new frame, but we received a follow-up packet
#ifdef BRIDGE_SERIAL_DATA_DEBUG
                    LOG(DEBUG) << "Data read thread - discarding unexpected follow-up packet";
#endif
                    continue;  // don't do anything with the received packet and start over
                }

                if (virtualChannel != bChannel)
                {
#ifdef BRIDGE_SERIAL_DATA_DEBUG
                    LOG(DEBUG) << "Data read thread - Channel mismatch: received = 0x" << std::hex << static_cast<int>(bChannel) << " , expected = 0x" << static_cast<int>(virtualChannel);
#endif
                    buf = bufBegin;  // the frame might be corrupted, so reset everyting for the next frame start
                    queueFrame(ErrorFrame::create(DataError_FrameDropped, bChannel));
                    continue;        // don't do anything with the received packet and start over
                }
            }

            buf += wLength;

            if (bmPktType & DATA_FRAME_FLAG_LAST)
            {
                if (bmPktType & DATA_FRAME_FLAG_TIMESTAMP)
                {
                    buf -= sizeof(epochTimestamp);
                    if (!setLocalTimestamp)
                    {
                        serialToHost(buf, epochTimestamp);
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
                        serialToHost(buf - sizeof(code), code);
                        queueFrame(ErrorFrame::create(code, bChannel, epochTimestamp));
                    }
                    else
                    {
                        DebugFrame::log(buf - wLength, wLength, epochTimestamp);
                    }
                    buf = bufBegin;
                }
                else
                {
                    frame->setDataOffset(0);
                    frame->setDataSize(static_cast<uint32_t>(buf - bufBegin));
                    frame->setVirtualChannel(virtualChannel);
                    frame->setTimestamp(epochTimestamp);

                    queueFrame(frame);
                    frame = nullptr;
                }
            }
        }
        catch (const std::exception &e)
        {
            queueFrame(ErrorFrame::create(DataError_LowLevelError, VIRTUAL_CHANNEL_UNDEFINED));
            LOG(DEBUG) << "Data read thread - " << e.what();
        }
    }

    // when stopping the data transfer, check if we still have a frame buffer dequeued
    if (frame)
    {
        m_framePool.queueFrame(frame);
    }
}
