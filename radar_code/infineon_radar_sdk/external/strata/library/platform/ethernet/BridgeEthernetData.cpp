/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "BridgeEthernetData.hpp"

#include <array>
#include <common/Logger.hpp>
#include <common/Serialization.hpp>
#include <common/Time.hpp>
#include <platform/exception/EBridgeData.hpp>
#include <platform/exception/EProtocol.hpp>
#include <platform/frames/DebugFrame.hpp>
#include <platform/frames/ErrorFrame.hpp>
#include <universal/protocol/protocol_definitions.h>


//#define BRIDGE_ETHERNET_DATA_DEBUG


namespace
{
    constexpr bool setLocalTimestamp = false;

    constexpr const uint16_t frameHeaderSize   = 6;
    constexpr const uint32_t timestampSize     = sizeof(uint64_t);
    constexpr const uint32_t bufferPrefixSize  = sizeof(uint64_t);
    constexpr const uint32_t bufferPrefixStart = bufferPrefixSize - frameHeaderSize;

    constexpr const uint16_t dataPort   = 55056;
    constexpr const int inputBufferSize = 4 * 1024 * 1024;

    constexpr const uint16_t defaultTimeout = 1000;
}


BridgeEthernetData::BridgeEthernetData(ISocket &socket, ipAddress_t ipAddr) :
    m_socket(socket),
    m_ipAddr {ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]}
{
    openConnection();
}

BridgeEthernetData::~BridgeEthernetData()
{
    closeConnection();
}

void BridgeEthernetData::openConnection()
{
    m_packetCounter = 0;
    m_socket.open(0, dataPort, m_ipAddr, defaultTimeout);
    m_socket.setInputBufferSize(inputBufferSize);
    m_socket.send(nullptr, 0);  // let the board know where to send the data to (anyways, this pipecleaner is needed for receiving to work)
}

void BridgeEthernetData::closeConnection()
{
    BridgeEthernetData::stopStreaming();
    m_socket.close();
}

void BridgeEthernetData::setFrameBufferSize(uint32_t size)
{
    // allocate enough buffer memory for header to avoid memory copying
    m_framePool.setFrameBufferSize(bufferPrefixSize + size + timestampSize);
}

void BridgeEthernetData::setFramePoolCount(uint16_t count)
{
    m_framePool.setFrameCount(count);
}

void BridgeEthernetData::startStreaming()
{
    if (isBridgeDataStarted())
    {
        return;
    }

    if (!m_socket.isOpened())
    {
        throw EBridgeData("Calling startData() without being connected");
    }
    if (!m_framePool.initialized())
    {
        throw EBridgeData("Calling startData() without frame pool being initialized");
    }

    cleanupStreaming();
    startBridgeData();

    switch (m_socket.getMode())
    {
        case ISocket::Mode::Datagram:
            m_dataThread = std::thread(&BridgeEthernetData::dataThreadFunctionDatagrams, this);
            break;
        case ISocket::Mode::Stream:
            m_dataThread = std::thread(&BridgeEthernetData::dataThreadFunctionStreaming, this);
            break;
    }
}

void BridgeEthernetData::stopStreaming()
{
    if (!isBridgeDataStarted())
    {
        return;
    }

    stopBridgeData();
    m_dataThread.join();
}

void BridgeEthernetData::cleanupStreaming()
{
    const auto expiry = std::chrono::steady_clock::now() + std::chrono::milliseconds(10);
    while (m_socket.dumpPacket())
    {
        if (std::chrono::steady_clock::now() > expiry)
        {
            LOG(DEBUG) << "BridgeEthernetData() - cleanupStreaming() timed out";
            break;
        }
    }
}

void BridgeEthernetData::dataThreadFunctionDatagrams()
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
                if (m_socket.dumpPacket())
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
                const auto remainingSize    = bufEnd - buf;
                const uint16_t readSize     = (remainingSize > m_socket.maxPayload()) ? m_socket.maxPayload() : static_cast<uint16_t>(remainingSize);
                const uint16_t returnedSize = m_socket.receive(buf, readSize);
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
#ifdef BRIDGE_ETHERNET_DATA_DEBUG
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
#ifdef BRIDGE_ETHERNET_DATA_DEBUG
                    LOG(DEBUG) << "     counter mismatch: received = 0x" << std::hex << wCounter << " , expected = 0x" << m_packetCounter;
#endif
                    m_packetCounter = wCounter + 1;

                    queueFrame(ErrorFrame::create(DataError_FrameDropped, bChannel));

                    if (!(bmPktType & DATA_FRAME_FLAG_FIRST))
                    {
                        // if this was a follow-up frame, discard the whole already received part
                        buf = bufBegin;

#ifdef BRIDGE_ETHERNET_DATA_DEBUG
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
#ifdef BRIDGE_ETHERNET_DATA_DEBUG
                        LOG(DEBUG) << "Data read thread - previous frame incomplete: wCounter = 0x" << std::hex << wCounter;
#endif
                    }
                }
                else
                {
                    if (buf == bufBegin)
                    {
                        // we expected a new frame, but we received a follow-up packet
#ifdef BRIDGE_ETHERNET_DATA_DEBUG
                        LOG(DEBUG) << "Data read thread - discarding unexpected follow-up packet";
#endif
                        continue;  // don't do anything with the received packet and start over
                    }

                    if (virtualChannel != bChannel)
                    {
#ifdef BRIDGE_ETHERNET_DATA_DEBUG
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

void BridgeEthernetData::dataThreadFunctionStreaming()
{
    uint8_t header[frameHeaderSize];
    IFrame *frame   = nullptr;
    State state     = WaitForFrameStart;
    bool firstFrame = true;

    try
    {
        while (isBridgeDataStarted())
        {
            // 1. retrieve the frame header
            if (!receive(header, frameHeaderSize))
                break;

            // 2. parse the header and change the state in the state machine
            const auto bmPktType = serialToHost<uint8_t>(&header[0]);
            const auto wCounter  = serialToHost<uint16_t>(&header[2]);
            const auto bChannel  = serialToHost<uint8_t>(&header[1]);
            const auto wLength   = serialToHost<uint16_t>(&header[4]);
            if (!(bmPktType & DATA_FRAME_PACKET))
            {
                // With TCP receiving an unknown packet would probably be a programming error.
                throw EBridgeData("Unknown packet type received.");
            }
            const bool counterOk = checkCounter(firstFrame, wCounter, m_packetCounter, bChannel);
            m_packetCounter      = wCounter + 1;

            // 3. Get a frame buffer from the pool, if there is none drop the frame that is currently received.
            //    That way we gain time without filling up the OS buffers and causing a backlog in the firmware.
            if (!frame)
            {
                frame = m_framePool.dequeueFrame();
                if (!frame)
                {
                    // No frame available in the buffer, drop the frame that is currently transmitted.
                    state = DropFrame;
                }
            }

            // 4. Decide based on the state what to do next.
            switch (state)
            {
                case WaitForFrameStart:
                {
                    if (bmPktType & DATA_FRAME_FLAG_FIRST)
                    {
                        state = handleFirstPacket(frame, bmPktType, bChannel, wLength);
                    }
                    else
                    {
                        state = DropFrame;
                        dropPayload(wLength);
                    }

                    break;
                }
                case WaitForMiddleOrEnd:
                {
                    if (bmPktType & DATA_FRAME_FLAG_FIRST)
                    {
                        state = handleFirstPacket(frame, bmPktType, bChannel, wLength);
                    }
                    else
                    {
                        if (counterOk && frame->getVirtualChannel() == bChannel)
                        {
                            state = receivePayload(frame, wLength, bmPktType);
                        }
                        else
                        {
                            state = DropFrame;
                            dropPayload(wLength);
                        }
                    }

                    break;
                }

                case DropFrame:
                {
                    if (frame && (bmPktType & DATA_FRAME_FLAG_FIRST))
                        state = handleFirstPacket(frame, bmPktType, bChannel, wLength);
                    else
                    {
                        dropPayload(wLength);
                    }
                }
            }
        }
    }
    catch (const std::exception &e)
    {
        LOG(ERROR) << "An exception happened in the streaming data retrieval function, maybe lost connection: " << e.what();
    }

    // if we own a dequeued frame buffer, make sure we return it
    if (frame)
    {
        m_framePool.queueFrame(frame);
    }
}

bool BridgeEthernetData::checkCounter(bool &firstFrame, uint16_t actualCounter, uint16_t expectedCounter, uint8_t channel)
{
    if (firstFrame)
    {
        firstFrame = false;
    }
    else if (actualCounter != expectedCounter)
    {
        LOG(INFO) << "Data read thread - Packet loss";
#ifdef BRIDGE_ETHERNET_DATA_DEBUG
        LOG(DEBUG) << "    Packet loss, counter mismatch: received = 0x" << std::hex << actualCounter << " , current = 0x" << expectedCounter;
#endif
        queueFrame(ErrorFrame::create(DataError_FrameDropped, channel));

        return false;
    }

    return true;
}

BridgeEthernetData::State BridgeEthernetData::receivePayload(IFrame *&frame, uint16_t length, uint8_t bmPktType)
{
    if (length + frame->getDataSize() > frame->getBufferSize())
    {
        throw EBridgeData("Frame too large");
    }

    if (!receive(frame->getData() + frame->getDataSize(), length))
        return WaitForFrameStart;

    const bool hasTimeStamp      = bmPktType & DATA_FRAME_FLAG_TIMESTAMP;
    constexpr auto timeStampSize = sizeof(frame->getTimestamp());
    frame->setDataSize(frame->getDataSize() + length - (hasTimeStamp ? timeStampSize : 0));

    if (bmPktType & DATA_FRAME_FLAG_LAST)
    {
        if ((bmPktType & DATA_FRAME_FLAG_TIMESTAMP) && !setLocalTimestamp)
        {
            uint64_t epochTimestamp = 0;
            serialToHost(frame->getData() + frame->getDataSize(), epochTimestamp);
            frame->setTimestamp(epochTimestamp);
        }

        if (bmPktType & DATA_FRAME_FLAG_ERROR)
        {
            uint32_t code;
            const auto errorFrameLength = sizeof(code) + (hasTimeStamp ? timeStampSize : 0);
            if (length == errorFrameLength)
            {
                code = serialToHost<uint32_t>(frame->getData());
                queueFrame(ErrorFrame::create(code, frame->getVirtualChannel(), frame->getTimestamp()));
            }
            else
            {
                DebugFrame::log(frame->getData(), frame->getDataSize(), frame->getTimestamp());
            }
        }
        else
        {
            queueFrame(frame);
            frame = nullptr;
        }
        return WaitForFrameStart;
    }
    else
    {
        return WaitForMiddleOrEnd;
    }
}

bool BridgeEthernetData::receive(uint8_t *buffer, uint16_t length)
{
    uint16_t bytesRetrieved = 0U;
    while (isBridgeDataStarted() && bytesRetrieved < length)
    {
        bytesRetrieved += m_socket.receive(buffer + bytesRetrieved, length - bytesRetrieved);
    }

    return bytesRetrieved == length;
}

void BridgeEthernetData::dropPayload(uint16_t length)
{
    std::array<uint8_t, 512> dropBuffer;

    while (isBridgeDataStarted() && length > 0)
    {
        const uint16_t bytesToRetrieve = std::min(static_cast<uint16_t>(dropBuffer.size()), length);
        length -= m_socket.receive(dropBuffer.data(), bytesToRetrieve);
    }
}

BridgeEthernetData::State BridgeEthernetData::handleFirstPacket(IFrame *&frame, uint8_t bmPktType, uint8_t bChannel, uint16_t wLength)
{
    // Reset everything of the frame in case it is re-used
    frame->setVirtualChannel(bChannel);
    frame->setDataSize(0);
    frame->setTimestamp(setLocalTimestamp ? getEpochTime() : 0);

    return receivePayload(frame, wLength, bmPktType);
}
