/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <common/Logger.hpp>

#include <platform/bridge/BridgeData.hpp>
#include <platform/bridge/BridgeProtocol.hpp>
#include <platform/bridge/VendorCommandsImpl.hpp>
#include <platform/interfaces/IBridge.hpp>
#include <platform/wiggler/MulticoreDebugger.hpp>


class BridgeWiggler :
    public IBridge,
    private BridgeData,
    private VendorCommandsImpl
{
public:
    BridgeWiggler(uint8_t systemIndex);
    ~BridgeWiggler() override;

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

    //    void queueFrame(uint8_t *buffer, uint32_t length, uint64_t timestamp);

    //    void calibrate(uint8_t index);
    //    void setFrameDimensions(uint16_t samples, uint16_t ramps, uint8_t inputChannels, uint8_t acquisitions);
    //    void setInputDataFormat(DataFormat_t format);

    // IVendorCommands implementation
    uint16_t getMaxTransfer() const override;
    void setDefaultTimeout() override
    {}

    void vendorWrite(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength, const uint8_t buffer[]) override;
    void vendorRead(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength, uint8_t buffer[]) override;
    void vendorTransfer(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLengthSend, const uint8_t bufferSend[], uint16_t &wLengthReceive, uint8_t bufferReceive[]) override;

private:
    BridgeProtocol m_protocol;
    uint8_t m_systemIndex;
    MulticoreDebugger m_debugger;
};
