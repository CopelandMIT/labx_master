/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <platform/frames/FrameForwarder.hpp>
#include <platform/frames/FrameListenerCaller.hpp>
#include <platform/frames/FrameQueue.hpp>
#include <platform/interfaces/IBridgeData.hpp>

class BridgeData :
    public IBridgeData
{
public:
    BridgeData();

    virtual ~BridgeData();

    void setFrameQueueSize(uint16_t count) override;
    void clearFrameQueue() override;

    void registerListener(IFrameListener<> *listener) override;

    IFrame *getFrame(uint16_t timeoutMs = 5000) override;

protected:
    void startBridgeData();
    void stopBridgeData();

    bool isBridgeDataStarted() const;

    void queueFrame(IFrame *frame);

    // decouple consumer callback from receiving thread
    // FrameForwarder must outlast FrameListenerCaller since it is used there!
    // Using the FrameForwarder between us and the listener decouples all calls to the listener from the receiving thread.
    // In case a direct connection is needed in future, either a parameter can be added to this function
    // or the FrameForwarder is removed here and used outside of this call where necessary.
    FrameQueue m_frameQueue;
    FrameForwarder m_frameForwarder;

private:
    std::atomic_bool m_dataStarted;

    /**
     * Set the size of the frame pool
     * @param count The number of frame buffers to be available in the frame pool
     */
    virtual void setFramePoolCount(uint16_t count) = 0;
};
