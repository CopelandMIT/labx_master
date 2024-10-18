/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "BridgeDirectShow.hpp"
#include "BridgeDirectShowDefinitions.hpp"

#include "EDirectShow.hpp"
#include <common/Logger.hpp>
#include <common/Time.hpp>
#include <handleapi.h>
#include <platform/exception/EBridgeData.hpp>
#include <platform/exception/EConnection.hpp>
#include <platform/exception/EUvcProperty.hpp>
#include <platform/uvc/VendorExtensionList.hpp>
#include <stdexcept>
#include <synchapi.h>
#include <windows.h>


namespace
{
    constexpr bool setLocalTimestamp = false;
}


/**
 * This sub-class provides an IFrame interface for a DirectShow MediaSample
 */
class FrameDirectShow :
    public IFrame
{
    friend class BridgeDirectShow;

protected:
    FrameDirectShow(IMediaSample *mediaSample, uint64_t timestamp) :
        m_mediaSample {mediaSample},
        m_offset {0},
        m_timestamp {timestamp}
    {
        m_mediaSample->AddRef();
    }

public:
    uint8_t *getData() const override
    {
        return getPointer() + m_offset;
    }

    uint32_t getDataSize() const override
    {
        return static_cast<uint32_t>(m_mediaSample->GetActualDataLength());
    }

    void setDataOffset(uint32_t offset) override
    {
        if (getDataSize() + offset > getBufferSize())
        {
            throw std::out_of_range("Buffer too small");
        }

        m_offset = offset;
    }

    void setDataSize(uint32_t dataSize) override
    {
        if (dataSize + m_offset > getBufferSize())
        {
            throw std::out_of_range("Buffer too small");
        }

        m_mediaSample->SetActualDataLength(static_cast<long>(dataSize));
    }

    void setDataOffsetAndSize(uint32_t offset, uint32_t dataSize) override
    {
        if (dataSize + offset > getBufferSize())
        {
            throw std::out_of_range("Buffer too small");
        }

        m_mediaSample->SetActualDataLength(static_cast<long>(dataSize));
        m_offset = offset;
    }

    uint32_t getDataOffset() const override
    {
        return m_offset;
    }

    uint8_t *getBuffer() const override
    {
        return getPointer();
    }

    uint32_t getBufferSize() const override
    {
        return static_cast<uint32_t>(m_mediaSample->GetSize());
    }

    uint8_t getVirtualChannel() const override
    {
        return m_virtualChannel;
    }

    void setVirtualChannel(uint8_t virtualChannel) override
    {
        m_virtualChannel = virtualChannel;
    }

    uint64_t getTimestamp() const override
    {
        return m_timestamp;
    }

    void setTimestamp(uint64_t timestamp) override
    {
        m_timestamp = timestamp;
    }

    uint32_t getStatusCode() const override
    {
        return 0;
    }

    void hold() override
    {
        m_mediaSample->AddRef();
    }

    void release() override
    {
        auto cRef = m_mediaSample->Release();
        if (cRef == 0)
        {
            delete this;
        }
    }

private:
    inline uint8_t *getPointer() const
    {
        uint8_t *ptr;
        m_mediaSample->GetPointer(&ptr);
        return ptr;
    }

    IMediaSample *m_mediaSample;
    uint32_t m_offset;
    uint8_t m_virtualChannel;
    uint64_t m_timestamp;
};


ULONG STDMETHODCALLTYPE BridgeDirectShow::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

ULONG STDMETHODCALLTYPE BridgeDirectShow::Release()
{
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0)
    {
        //delete this;
    }
    return cRef;
}

STDMETHODIMP BridgeDirectShow::QueryInterface(const IID &rrid, void **ppvObject)
{
    if (ppvObject == nullptr)
    {
        return E_POINTER;
    }
    if (rrid == IID_IUnknown)
    {
        *ppvObject = static_cast<IUnknown *>(this);
    }
    else if (rrid == IID_ISampleGrabberCB)
    {
        *ppvObject = static_cast<ISampleGrabberCB *>(this);
    }
    else
    {
        *ppvObject = nullptr;
        return E_NOTIMPL;
    }
    AddRef();
    return S_OK;
}

HRESULT STDMETHODCALLTYPE BridgeDirectShow::BufferCB(double /*SampleTime*/, BYTE * /*pBuffer*/, long /*BufferLen*/)
{
    //LOG(ERROR) << "Unexpected call to BufferCB";
    return S_FALSE;
}

HRESULT STDMETHODCALLTYPE BridgeDirectShow::SampleCB(double SampleTime, IMediaSample *MediaSample)
{
    uint64_t timestamp;
    if (setLocalTimestamp)
    {
        timestamp = getEpochTime();
    }
    else
    {
        timestamp = static_cast<uint64_t>(SampleTime * 1e6);
    }
    queueFrame(new FrameDirectShow(MediaSample, timestamp));
    return S_OK;
}

BridgeDirectShow::BridgeDirectShow(IMonikerPtr &&instanceId, std::string &&deviceId) :
    m_cRef {0},
    m_instanceId {std::move(instanceId)},
    m_deviceId {std::move(deviceId)},
    m_mutexHandle {NULL},
    m_deviceLost {false}
{
    auto hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
        LOG(ERROR) << "CoInitialize failed!";
    }

    m_mutexHandle = CreateMutexA(nullptr, false, m_deviceId.c_str());
    if (!m_mutexHandle)
    {
        throw EDirectShow("Named device mutex could not be created or opened");
    }

    BridgeDirectShow::openConnection();
}

BridgeDirectShow::~BridgeDirectShow()
{
    try
    {
        BridgeDirectShow::closeConnection();
    }
    catch (...)
    {
    }

    if (m_mutexHandle)
    {
        CloseHandle(m_mutexHandle);
        m_mutexHandle = NULL;
    }

    // make sure all pointers are released before CoUninitialize()
    // (since depending on the calling order from other places, it might de-allocate all memory immediately)
    if (m_instanceId)
    {
        m_instanceId.Release();
    }
    CoUninitialize();
}

bool BridgeDirectShow::isConnected()
{
    return m_graphHandle;
}

void BridgeDirectShow::openConnection()
{
    if (BridgeDirectShow::isConnected())
    {
        return;
    }
    LOG(DEBUG) << "Connecting BridgeUsbDirectShow ...";

    HRESULT hr;

    IGraphBuilderPtr graph;
    ICaptureGraphBuilder2Ptr builder;
    hr = graph.CreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC);
    if (SUCCEEDED(hr))
    {
        hr = builder.CreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC);
        if (SUCCEEDED(hr))
        {
            hr = builder->SetFiltergraph(graph);
        }
    }
    if (FAILED(hr))
    {
        throw EDirectShow("Unable to create media graph", hr);
    }

    IBaseFilter *interfacePointer;
    hr = m_instanceId->BindToObject(NULL, NULL, IID_IBaseFilter, reinterpret_cast<void **>(&interfacePointer));
    if (FAILED(hr))
    {
        throw EDirectShow("Unable to open the source device", hr);
    }

    IBaseFilterPtr mediaSourceFilter(interfacePointer, true);
    hr = graph->AddFilter(mediaSourceFilter, L"USB Video Source");
    if (FAILED(hr))
    {
        throw EDirectShow("Couldn't add USB Video Source to graph", hr);
    }

    // block for scoping the videoStreamConfig
    //    {
    //        IAMStreamConfigPtr videoStreamConfig;
    //        hr = builder->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, mediaSourceFilter, IID_IAMStreamConfig, reinterpret_cast<void **>(&videoStreamConfig));
    //        if (FAILED(hr))
    //        {
    //            throw EDirectShow("Couldn't get Video stream control", hr);
    //        }

    //        AM_MEDIA_TYPE *pMt;
    //        hr = videoStreamConfig->GetFormat(&pMt);
    //        if (FAILED(hr))
    //        {
    //            throw EDirectShow("Couldn't get Video stream format", hr);
    //        }

    //        VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER *) pMt->pbFormat;
    //        /*        pMt->formattype = FORMAT_VideoInfo;
    //                pMt->majortype  = MEDIATYPE_Video;
    //                pMt->subtype = mediaSubtype;
    //                pMt->bFixedSizeSamples = TRUE;
    //                pMt->bTemporalCompression = FALSE;
    //                pMt->lSampleSize = 32*2*224*2;
    //                pVih->bmiHeader.biCompression = BI_RGB;
    //                pVih->bmiHeader.biSizeImage = 0;
    //                pVih->bmiHeader.biBitCount = 24;
    //                pVih->bmiHeader.biWidth = uiWidth;
    //                pVih->bmiHeader.biHeight = uiHeight;
    //                */
    //        pMt->bFixedSizeSamples = FALSE;
    //        hr = videoStreamConfig->SetFormat(pMt);

    //        if (FAILED(hr))
    //        {
    //            throw EDirectShow("Couldn't set Video stream format", hr);
    //        }
    //    }

    IBaseFilterPtr sampleGrabberFilter;
    hr = sampleGrabberFilter.CreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC);
    if (SUCCEEDED(hr))
    {
        hr = graph->AddFilter(sampleGrabberFilter, L"Sample Grabber");
        if (SUCCEEDED(hr))
        {
            ISampleGrabberPtr sampleGrabber;
            hr = sampleGrabberFilter.QueryInterface(IID_ISampleGrabber, sampleGrabber.GetInterfacePtr());
            if (SUCCEEDED(hr))
            {
                sampleGrabber->SetBufferSamples(FALSE);
                sampleGrabber->SetOneShot(FALSE);
                sampleGrabber->SetCallback(this, 0);
            }
        }
    }
    if (FAILED(hr))
    {
        throw EDirectShow("Couldn't add sample grabber", hr);
    }

    // use Null Renderer as we do not want to display the data
    IBaseFilterPtr nullRendererFilter;
    hr = nullRendererFilter.CreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC);
    if (SUCCEEDED(hr))
    {
        hr = graph->AddFilter(nullRendererFilter, L"Null Renderer");
    }
    if (FAILED(hr))
    {
        throw EDirectShow("Couldn't add null renderer", hr);
    }

    // Set the Reference Clock of the Graph to NULL to make it run as fast as possible
    {
        IMediaFilterPtr mediaFilter;
        hr = graph.QueryInterface(IID_IMediaFilter, mediaFilter.GetInterfacePtr());
        if (SUCCEEDED(hr))
        {
            hr = mediaFilter->SetSyncSource(NULL);
        }
        if (FAILED(hr))
        {
            throw EDirectShow("Couldn't set the sync source");
        }
    }

    hr = builder->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, mediaSourceFilter, sampleGrabberFilter, nullRendererFilter);
    if (FAILED(hr))
    {
        throw EDirectShow("Couldn't render the stream", hr);
    }

    //Get the buffer allocator interface
    {
        IPinPtr pin;
        hr = nullRendererFilter->FindPin(L"In", &pin);
        if (FAILED(hr))
        {
            throw EDirectShow("Couldn't find Input Pin for Buffer settings", hr);
        }

        IMemInputPinPtr inputPin;
        hr = pin.QueryInterface(IID_IMemInputPin, inputPin.GetInterfacePtr());
        if (FAILED(hr))
        {
            throw EDirectShow("Couldn't query IMemInputPin", hr);
        }

        hr = inputPin->GetAllocator(&m_memAlloc);
        if (FAILED(hr))
        {
            throw EDirectShow("Couldn't get allocator", hr);
        }
    }

    getVendorExtension(mediaSourceFilter);

    hr = graph.QueryInterface(IID_IMediaControl, m_mediaControl.GetInterfacePtr());
    if (FAILED(hr))
    {
        throw EDirectShow("Couldn't get the media control");
    }

    hr = graph.QueryInterface(IID_IMediaEventEx, m_mediaEvent.GetInterfacePtr());
    if (FAILED(hr))
    {
        throw EDirectShow("Couldn't get the media events");
    }

    // Keep the graph for the full lifecycle of this Bridge, and make isConnected() return true
    m_graphHandle = std::move(graph);
}

void BridgeDirectShow::getVendorExtension(IBaseFilterPtr &mediaSourceFilter)
{
    IKsTopologyInfoPtr uvcTopology;

    HRESULT hr = mediaSourceFilter.QueryInterface(IID_IKsTopologyInfo, uvcTopology.GetInterfacePtr());
    if (FAILED(hr))
    {
        throw EDirectShow("Couldn't query interface topology");
    }

    // The UVC extensions are a device-specific type of node, so first find a
    // device specifc node, and then check if it's the right one.
    DWORD numNodes;
    hr = uvcTopology->get_NumNodes(&numNodes);
    if (FAILED(hr))
    {
        throw EDirectShow("Couldn't enumerate extension nodes");
    }

    m_extensionNode.Property.Id    = KSPROPERTY_EXTENSION_UNIT_INFO;
    m_extensionNode.Property.Flags = KSPROPERTY_TYPE_TOPOLOGY | KSPROPERTY_TYPE_GET;

    // Extension unit IDs start at one, not zero
    for (DWORD i = 1; i <= numNodes; i++)
    {
        GUID nodeType;
        hr = uvcTopology->get_NodeType(i, &nodeType);
        if (SUCCEEDED(hr) && IsEqualGUID(KSNODETYPE_DEV_SPECIFIC, nodeType))
        {
            hr = uvcTopology->CreateNodeInstance(i, IID_IKsControl, reinterpret_cast<void **>(&m_extensionControl));
            if (SUCCEEDED(hr))
            {
                m_extensionNode.NodeId = i;
                for (const auto &e : VendorExtensionList)
                {
                    // now find out if this node has an interface with the property that we want
                    // try to read 0 bytes from it
                    memcpy(&m_extensionNode.Property.Set, e.guid, 16);

                    ULONG BytesReturned;
                    hr = m_extensionControl->KsProperty(&m_extensionNode.Property, sizeof(m_extensionNode), NULL, 0, &BytesReturned);
                    if ((hr == HRESULT_FROM_WIN32(ERROR_MORE_DATA)) || (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)))
                    {
                        LOG(DEBUG) << "... UVC vendor extension found";
                        m_vendorExtension = e.factory(static_cast<IUvcExtension *>(this));
                        return;
                    }
                }
            }
        }
    }
    throw EConnection("Couldn't find any supported vendor extension");
}

void BridgeDirectShow::closeConnection()
{
    if (!BridgeDirectShow::isConnected())
    {
        return;
    }
    LOG(DEBUG) << "Closing BridgeUsbDirectShow ...";

    BridgeDirectShow::stopStreaming();

    m_vendorExtension.reset();
    m_extensionControl.Release();

    m_memAlloc.Release();
    m_mediaEvent.Release();
    m_mediaControl.Release();
    m_graphHandle.Release();
}

IBridgeControl *BridgeDirectShow::getIBridgeControl()
{
    return m_vendorExtension.get();
}

IBridgeData *BridgeDirectShow::getIBridgeData()
{
    return this;
}

void BridgeDirectShow::startStreaming()
{
    if (!isConnected())
    {
        throw EBridgeData("Calling startData() without being connected");
    }

    if (isBridgeDataStarted())
    {
        return;
    }

    HRESULT hr = m_mediaControl->Run();
    if (FAILED(hr))
    {
        throw EDirectShow("Couldn't run the device");
    }

    startBridgeData();
}

void BridgeDirectShow::stopStreaming()
{
    if (!isBridgeDataStarted())
    {
        return;
    }

    stopBridgeData();

    HRESULT hr = m_mediaControl->Stop();
    if (FAILED(hr))
    {
        throw EDirectShow("Couldn't stop the device");
    }
}

void BridgeDirectShow::getAllocatorProperties(ALLOCATOR_PROPERTIES &AllocProps)
{
    if (!isConnected())
    {
        throw EBridgeData("Cannot change allocation without being connected");
    }

    auto hr = m_memAlloc->GetProperties(&AllocProps);
    if (FAILED(hr))
    {
        throw EDirectShow("Couldn't get allocator properties", hr);
    }
}

void BridgeDirectShow::setAllocatorProperties(ALLOCATOR_PROPERTIES &AllocProps)
{
    ALLOCATOR_PROPERTIES AllocPropsAct;
    auto hr = m_memAlloc->SetProperties(&AllocProps, &AllocPropsAct);
    if (FAILED(hr))
    {
        throw EDirectShow("Couldn't set allocator properties", hr);
    }
}

void BridgeDirectShow::setFrameBufferSize(uint32_t size)
{
    ALLOCATOR_PROPERTIES AllocProps;
    getAllocatorProperties(AllocProps);
    AllocProps.cbAlign  = 8;  // align to 64bit
    AllocProps.cbPrefix = 0;  // do not add a prefix
    AllocProps.cbBuffer = static_cast<long>(size);
    setAllocatorProperties(AllocProps);
}

void BridgeDirectShow::setFramePoolCount(uint16_t count)
{
    ALLOCATOR_PROPERTIES AllocProps;
    getAllocatorProperties(AllocProps);
    AllocProps.cbAlign  = 8;  // align to 64bit
    AllocProps.cbPrefix = 0;  // do not add a prefix
    AllocProps.cBuffers = count;
    setAllocatorProperties(AllocProps);
}

void BridgeDirectShow::lock()
{
    if (m_mutexHandle)
    {
        WaitForSingleObject(m_mutexHandle, INFINITE);
    }
    else
    {
        LOG(ERROR) << "Handle to named mutex is invalid";
    }
}

void BridgeDirectShow::unlock()
{
    if (m_mutexHandle)
    {
        ReleaseMutex(m_mutexHandle);
    }
    else
    {
        LOG(ERROR) << "Handle to named mutex is invalid";
    }
}

bool BridgeDirectShow::variableSizeSupport() const
{
    return true;
}

bool BridgeDirectShow::hasDeviceLostEvent()
{
    long event;
    LONG_PTR param1, param2;

    // Get the corresponding filtergraph directshow media event to handle
    while (SUCCEEDED(m_mediaEvent->GetEvent(&event, &param1, &param2, 0L)))
    {
        switch (event)
        {

            case EC_DEVICE_LOST:
                // Check if we lost the device.
                // Right now we disregard that the device could become present again.
                m_deviceLost = true;
                break;
            default:
                break;
        }

        m_mediaEvent->FreeEventParams(event, param1, param2);
    }

    return m_deviceLost;
}

void BridgeDirectShow::setProperty(uint8_t id, uint16_t length, const uint8_t buffer[])
{
    if (hasDeviceLostEvent())
    {
        throw EConnection("setProperty() (disconnected?)");
    }

    ULONG BytesReturned;
    m_extensionNode.Property.Id    = id;
    m_extensionNode.Property.Flags = KSPROPERTY_TYPE_TOPOLOGY | KSPROPERTY_TYPE_SET;

    auto hr = m_extensionControl->KsProperty(&m_extensionNode.Property, sizeof(m_extensionNode), const_cast<uint8_t *>(buffer), length, &BytesReturned);
    if (hr == HRESULT_FROM_WIN32(ERROR_GEN_FAILURE) || hr == HRESULT_FROM_WIN32(ERROR_DEVICE_NOT_CONNECTED))
    {
        throw EConnection("setProperty() failed (disconnected?)");
    }
    else if (FAILED(hr))
    {
        throw EUvcProperty("setProperty() failed (status)", hr);
    }
}

void BridgeDirectShow::getProperty(uint8_t id, uint16_t length, uint8_t buffer[])
{
    if (hasDeviceLostEvent())
    {
        throw EConnection("getProperty() (disconnected?)");
    }

    ULONG BytesReturned;
    m_extensionNode.Property.Id    = id;
    m_extensionNode.Property.Flags = KSPROPERTY_TYPE_TOPOLOGY | KSPROPERTY_TYPE_GET;

    auto hr = m_extensionControl->KsProperty(&m_extensionNode.Property, sizeof(m_extensionNode), buffer, length, &BytesReturned);
    if (hr == HRESULT_FROM_WIN32(ERROR_INVALID_FUNCTION))
    {
        // This is reached when the firmware detects an error and sends a USB STALL packet, but
        // there may be other causes of this (including not being able to talk to the firmware).
        throw EUvcProperty("getProperty() invalid function");
    }
    else if (hr == HRESULT_FROM_WIN32(ERROR_DEVICE_NOT_CONNECTED))
    {
        throw EConnection("getProperty() failed (disconnected)");
    }
    else if (FAILED(hr))
    {
        throw EUvcProperty("getProperty() failed (status)", hr);
    }
    else if (BytesReturned != length)
    {
        throw EConnection("getProperty() did not return the expected size");
    }
}
