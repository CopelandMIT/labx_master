/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <platform/bridge/BridgeData.hpp>
#include <platform/frames/FramePool.hpp>
#include <platform/interfaces/link/ISocket.hpp>
#include <universal/data_definitions.h>

#include <atomic>
#include <thread>


class BridgeEthernetData :
    public BridgeData
{
public:
    BridgeEthernetData(ISocket &socket, ipAddress_t ipAddr);
    ~BridgeEthernetData() override;

    void openConnection();
    void closeConnection();

    // IBridgeData implementation
    void setFrameBufferSize(uint32_t size) override;
    void setFramePoolCount(uint16_t count) override;
    void startStreaming() override;
    void stopStreaming() override;

private:
    void cleanupStreaming();

    FramePool m_framePool;
    ISocket &m_socket;
    uint8_t m_ipAddr[4];
    std::thread m_dataThread;
    uint16_t m_packetCounter;

    // Variables used by frame streaming

    enum State
    {
        WaitForFrameStart,
        WaitForMiddleOrEnd,
        DropFrame,
    };

    void dataThreadFunctionDatagrams();
    void dataThreadFunctionStreaming();
    bool checkCounter(bool &firstFrame, uint16_t actualCounter, uint16_t expectedCounter, uint8_t channel);
    State receivePayload(IFrame *&frame, uint16_t length, uint8_t bmPktType);
    bool receive(uint8_t *buffer, uint16_t length);
    void dropPayload(uint16_t length);
    State handleFirstPacket(IFrame *&frame, uint8_t bmPktType, uint8_t bChannel, uint16_t wLength);
};
