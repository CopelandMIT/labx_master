/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include "ComDefines.hpp"
#include "SampleGrabberInterface.hpp"
#include <platform/bridge/BridgeControl.hpp>
#include <platform/bridge/BridgeData.hpp>
#include <platform/interfaces/IBridge.hpp>
#include <platform/interfaces/link/IUvcExtension.hpp>


#include <atomic>


class BridgeDirectShow :
    public IBridge,
    private BridgeData,
    private IUvcExtension,
    private ISampleGrabberCB
{
public:
    BridgeDirectShow(IMonikerPtr &&instanceId, std::string &&deviceId);
    ~BridgeDirectShow();

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
    void setFramePoolCount(uint16_t count) override;

    //IUvcExtension
    void lock() override;
    void unlock() override;

protected:
    // IUvcExtension
    bool variableSizeSupport() const override;
    void setProperty(uint8_t id, uint16_t length, const uint8_t buffer[]) override;
    void getProperty(uint8_t id, uint16_t length, uint8_t buffer[]) override;

private:
    //ISamplegrabberCB
    ULONG STDMETHODCALLTYPE AddRef() override;
    ULONG STDMETHODCALLTYPE Release() override;
    STDMETHODIMP QueryInterface(const IID &rrid, void **ppvObject) override;
    HRESULT STDMETHODCALLTYPE BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen) override;
    HRESULT STDMETHODCALLTYPE SampleCB(double SampleTime, IMediaSample *MediaSample) override;

    bool hasDeviceLostEvent();

    volatile ULONG m_cRef;

private:
    IMonikerPtr m_instanceId;
    std::string m_deviceId;

    // UVC Video
    IGraphBuilderPtr m_graphHandle;
    IMediaControlPtr m_mediaControl;
    IMediaEventExPtr m_mediaEvent;

    // UVC buffer allocator
    void getAllocatorProperties(ALLOCATOR_PROPERTIES &AllocProps);
    void setAllocatorProperties(ALLOCATOR_PROPERTIES &AllocProps);
    IMemAllocatorPtr m_memAlloc;

    // UVC Extension
    KSP_NODE m_extensionNode;
    IKsControlPtr m_extensionControl;

    // Vendor Extension
    std::unique_ptr<IBridgeControl> m_vendorExtension;
    void getVendorExtension(IBaseFilterPtr &mediaSourceFilter);

    // Handle to named mutex, the named mutex is used to synchronize processes on windows
    HANDLE m_mutexHandle;

    bool m_deviceLost;
};
