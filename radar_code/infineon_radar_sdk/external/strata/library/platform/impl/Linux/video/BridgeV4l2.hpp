/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include "FramePoolV4l2.hpp"
#include <platform/bridge/BridgeControl.hpp>
#include <platform/frames/FrameForwarder.hpp>
#include <platform/interfaces/IBridge.hpp>
#include <platform/interfaces/link/IUvcExtension.hpp>
#include <string>


class BridgeV4l2 :
    private IUvcExtension,
    public IBridgeData,
    public IBridge
{
public:
    BridgeV4l2(std::string &&devicePath);
    ~BridgeV4l2();

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
    IFrame *getFrame(uint16_t timeoutMs = 5000) override;

    //IUvcExtension
    void lock() override;
    void unlock() override;

protected:
    // IUvcExtension
    bool variableSizeSupport() const override;
    void setProperty(uint8_t id, uint16_t length, const uint8_t buffer[]) override;
    void getProperty(uint8_t id, uint16_t length, uint8_t buffer[]) override;

private:
    void setVideoFormat(unsigned int width, unsigned int height);
    void cleanupStreaming();

    int m_fd;

    FramePoolV4l2 m_framePool;
    FrameForwarder m_frameForwarder;

    std::atomic_bool m_dataStarted;

    std::string m_devicePath;

    // UVC Extension
    uint8_t m_extensionUnit;

    // Vendor Extension
    std::unique_ptr<IBridgeControl> m_vendorExtension;
    void getVendorExtension();
};
