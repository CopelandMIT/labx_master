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
#include <universal/link_definitions.h>

#include <libusb-1.0/libusb.h>
#include <mutex>
#include <string>
#include <thread>


class BridgeLibUsb :
    public IBridge,
    private BridgeData,
    private VendorCommandsImpl
{
private:
    constexpr static const uint16_t m_maxPayload = LIBUSB_MAX_REQUEST_LENGTH;
    constexpr static const int m_maxPacketSize   = LIBUSB_MAX_DATA_LENGTH;

public:
    BridgeLibUsb(libusb_device *device, int fd = 0);
    ~BridgeLibUsb();

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
    void checkStatus();
    void controlEndpointWrite(uint8_t bmReqType, uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength, const uint8_t buffer[]);
    uint16_t controlEndpointRead(uint8_t bmReqType, uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength, uint8_t buffer[]);
    uint16_t controlEndpointReadChecked(uint8_t bmReqType, uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength, uint8_t buffer[]);

    uint16_t bulkEndpointRead(uint8_t buffer[], uint16_t length, const uint16_t timeout);
    bool dumpPacket();

    BridgeProtocol m_protocol;
    FramePool m_framePool;
    std::mutex m_lock;
    uint16_t m_packetCounter;

    libusb_context *m_context;
    libusb_device *m_device;
    int m_fd;
    libusb_device_handle *m_deviceHandle;

    void dataThreadFunction();
    std::thread m_dataThread;
};
