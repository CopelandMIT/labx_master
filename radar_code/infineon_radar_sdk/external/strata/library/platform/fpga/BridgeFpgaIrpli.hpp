/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <platform/interfaces/IBridge.hpp>
#include <platform/interfaces/IFrameListener.hpp>
#include <platform/interfaces/IVendorCommands.hpp>

#include <memory>
#include <platform/frames/FrameListenerCaller.hpp>


class BridgeFpgaIrpli :
    public IBridge,
    public IFrameListener<>,
    private IBridgeData
{
public:
    BridgeFpgaIrpli(std::shared_ptr<IBridge> &&bridge);

    void setChannelSwapping(uint8_t swapping);
    void setDataProperties(IDataProperties_t *props);

    //IBridge
    bool isConnected() override;
    void openConnection() override;
    void closeConnection() override;
    IBridgeControl *getIBridgeControl() override;
    IBridgeData *getIBridgeData() override;

    //IBridgeData
    void startStreaming() override;
    void stopStreaming() override;
    void setFrameBufferSize(uint32_t size) override;
    void setFrameQueueSize(uint16_t count) override;
    void clearFrameQueue() override;
    void registerListener(IFrameListener<> *listener) override;
    IFrame *getFrame(uint16_t timeoutMs) override;

    //IFrameListener
    void onNewFrame(IFrame *frame) override;

protected:
    std::shared_ptr<IBridge> m_bridge;

private:
    bool getFpgaDonePin();
    void sendFpgaBuffer(uint32_t count, const uint8_t buf[], uint16_t length);

    IBridgeData *m_bridgeData;
    IVendorCommands *m_commands;

    FrameListenerCaller<> m_caller;

    uint8_t m_swapping;
    IDataProperties_t m_properties;
};
