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
#include <platform/bridge/BridgeProtocol.hpp>
#include <platform/bridge/VendorCommandsImpl.hpp>
#include <platform/frames/FramePool.hpp>
#include <platform/interfaces/IBridge.hpp>
#include <serial/SerialPortImplBridge.hpp>
#include <universal/link_definitions.h>

#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>


class BridgeSerial :
    public IBridge,
    private BridgeData,
    private VendorCommandsImpl
{
private:
    constexpr static const uint16_t m_maxPayload = SERIAL_MAX_PACKET_SIZE;

public:
    BridgeSerial(const char port[]);
    ~BridgeSerial();

    //IBridge
    bool isConnected() override;
    void openConnection() override;
    void closeConnection() override;
    IBridgeControl *getIBridgeControl() override;
    IBridgeData *getIBridgeData() override;

    // IBridgeData implementation
    void setFrameBufferSize(uint32_t size) override;
    void setFramePoolCount(uint16_t count) override;
    void startStreaming() override;
    void stopStreaming() override;

    // IVendorCommands implementation
    void setDefaultTimeout() override;
    uint16_t getMaxTransfer() const override;

    void vendorWrite(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength, const uint8_t buffer[]) override;
    void vendorRead(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength, uint8_t buffer[]) override;
    void vendorTransfer(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLengthSend, const uint8_t bufferSend[], uint16_t &wLengthReceive, uint8_t bufferReceive[]) override;

private:
    void sendRequest(uint8_t bmReqType, uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wHeaderLength, const uint8_t buffer[]);
    void receiveResponse(uint8_t bmReqType, uint8_t bRequest, uint16_t &wLength, uint8_t buffer[]);

    FramePool m_framePool;
    BridgeProtocol m_protocol;
    SerialPortImplBridge m_port;
    std::string m_portName;
    std::chrono::duration<unsigned int, std::milli> m_timeout;

    std::condition_variable m_cv;
    std::mutex m_lock;
    std::mutex m_commandLock;

    // flags used in CV do not need to be atomic: setting is not critical, and clearing needs to happen with the CV mutex anyways
    bool m_commandActive;
    bool m_resynchronize;
    uint16_t m_packetCounter;

    enum PacketType
    {
        None,
        Control,
        Data
    } m_cachedPacket;
    uint8_t m_packetStartCache[4];

    bool readPacketStart(uint8_t buffer[], PacketType type, bool discardOther);
    void dumpRemainder(uint16_t wLength);

    std::thread m_dataThread;
    void dataThreadFunction();
};
