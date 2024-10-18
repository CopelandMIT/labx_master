/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "EnumeratorEthernet.hpp"

#include <common/Logger.hpp>
#include <common/Serialization.hpp>
#include <platform/ethernet/BoardDescriptorEthernet.hpp>
#include <platform/exception/EConnection.hpp>
#include <platform/exception/EProtocol.hpp>
#include <platform/templates/identifyBoardFunction.hpp>
#include <universal/protocol/protocol_definitions.h>


namespace
{
    const std::vector<remoteInfo_t> defaultBroadcastAddresses {
        {{169, 254, 255, 255}, 55055},
        {{169, 254, 1, 255}, 55055},
    };

    constexpr const uint16_t timeout     = 200;
    constexpr const uint16_t timeoutStep = 100;

    constexpr const uint16_t m_commandHeaderSize  = 8;
    constexpr const uint16_t m_responseHeaderSize = 4;

    constexpr const uint8_t bmReqType         = VENDOR_REQ_TRANSFER;
    constexpr const uint8_t bRequest          = REQ_BOARD_INFO;
    constexpr const uint16_t wValue           = REQ_BOARD_INFO_BOARD_INFO_WVALUE;
    constexpr const uint16_t wIndex           = 0;
    constexpr const uint16_t wLengthSend      = 0;
    constexpr const uint16_t maxLengthReceive = sizeof(IBridgeControl::BoardInfo_t);
}

EnumeratorEthernet::EnumeratorEthernet(bool useTcpConnection) :
    m_useTcpConnection(useTcpConnection)
{
}

bool EnumeratorEthernet::getUseTcpConnection()
{
    return m_useTcpConnection;
}

void EnumeratorEthernet::enumerate(IEnumerationListener &listener, BoardData::const_iterator begin, BoardData::const_iterator end)
{
    std::vector<remoteInfo_t> broadcastAddresses;
    m_socket.getBroadcastAddresses(broadcastAddresses);

    m_socket.open(0, 0, nullptr, timeoutStep);
    m_socket.setBroadcast(true);

    if (broadcastAddresses.empty())
    {
        LOG(DEBUG) << "Could not get broadcast addresses, using default";
        sendBroadcast(defaultBroadcastAddresses);
    }
    else
    {
        sendBroadcast(broadcastAddresses);
    }

    getResponses(listener, begin, end);

    m_socket.close();
}

inline void EnumeratorEthernet::sendBroadcast(const std::vector<remoteInfo_t> &broadcastAddresses)
{
    const uint8_t packet[m_commandHeaderSize] = {
        bmReqType,
        bRequest,
        static_cast<uint8_t>(wValue & 0xFF),
        static_cast<uint8_t>(wValue >> 8),
        static_cast<uint8_t>(wIndex & 0xFF),
        static_cast<uint8_t>(wIndex >> 8),
        static_cast<uint8_t>(wLengthSend & 0xFF),
        static_cast<uint8_t>(wLengthSend >> 8),
    };

    for (auto &b : broadcastAddresses)
    {
        LOG(DEBUG) << "Looking for boards under " << std::dec
                   << static_cast<int>(b.ip[0]) << "."
                   << static_cast<int>(b.ip[1]) << "."
                   << static_cast<int>(b.ip[2]) << "."
                   << static_cast<int>(b.ip[3]) << " over UDP ...";
        try
        {
            m_socket.sendTo(&packet[0], m_commandHeaderSize, &b);
        }
        catch (...)
        {}
    }
}

inline void EnumeratorEthernet::getResponses(IEnumerationListener &listener, BoardData::const_iterator begin, BoardData::const_iterator end)
{
    const auto expiry = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout);

    while (std::chrono::steady_clock::now() < expiry)
    {
        constexpr const uint16_t receiveSize = m_responseHeaderSize + maxLengthReceive;
        uint8_t packet[receiveSize];

        uint16_t ids[2];
        remoteInfo_t remote;

        try
        {
            uint16_t returnedLength  = m_socket.receiveFrom(&packet[0], receiveSize, &remote);
            const uint8_t &bmResType = packet[0];
            const uint8_t &bStatus   = packet[1];

            if (!returnedLength)
            {
                continue;
            }
            if (bmResType != bmReqType)
            {
                throw EProtocol("Response type error", (bmResType << 8) | bmReqType);
            }
            if (bStatus)
            {
                throw EProtocol("Response status error", bStatus);
            }
            if (returnedLength <= m_responseHeaderSize + sizeof(ids))
            {
                throw EProtocol("Response incomplete", returnedLength);
            }
            const uint16_t responseLength = static_cast<uint16_t>(packet[3] << 8) | packet[2];
            if (responseLength > maxLengthReceive)
            {
                throw EProtocol("Response length error", responseLength);
            }

            const uint8_t *it       = packet + m_responseHeaderSize;
            it                      = serialToHost(it, ids);
            const char *cName       = reinterpret_cast<const char *>(it);
            packet[receiveSize - 1] = 0;  // make sure string is terminated to avoid memory leak

            LOG(DEBUG) << "... found board at " << std::dec
                       << static_cast<int>(remote.ip[0]) << "."
                       << static_cast<int>(remote.ip[1]) << "."
                       << static_cast<int>(remote.ip[2]) << "."
                       << static_cast<int>(remote.ip[3]);

            const uint16_t &vid = ids[0], &pid = ids[1];
            LOG(DEBUG) << "... VID = " << std::hex << vid << " ; PID = " << pid << " ; name = \"" << cName << "\"";

            auto descriptor = identifyBoardFunction<BoardDescriptorEthernet>(begin, end, vid, pid, cName, remote.ip, m_useTcpConnection);
            if (listener.onEnumerate(std::move(descriptor)))
            {
                break;
            }
        }
        catch (const EException &e)
        {
            LOG(DEBUG) << "... handled " << e.what();
        }
    }
}
