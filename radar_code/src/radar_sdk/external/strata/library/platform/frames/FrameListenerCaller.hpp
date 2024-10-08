/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <mutex>
#include <platform/interfaces/IFrameListener.hpp>


template <typename FrameType = IFrame>
class FrameListenerCaller
{
public:
    FrameListenerCaller();
    virtual ~FrameListenerCaller() = default;

    /**
     * Determine if a listener is registered
     * @return True if there is a registered listener, otherwise false
     */
    inline bool hasListener() const noexcept
    {
        return m_listener;
    }

    /**
     * Register a listener to receive incoming frames
     * Make sure the listener is alive as long as it is registered here.
     * When destroying the listener it must be unregistered first (register nullptr)
     * When this function returns, the old listener is free, there is no callback active
     * @param listener Pointer to the listener that shall receive the frames, nullptr to unregister
     */
    virtual void registerListener(IFrameListener<FrameType> *listener);

    /**
     * Call the registered listener with the received frame
     * @param frame Pointer to a frame containing the received data
     */
    void callListener(FrameType *frame);

private:
    IFrameListener<FrameType> *m_listener;
    std::mutex m_listenerMutex;
};


template <typename FrameType>
FrameListenerCaller<FrameType>::FrameListenerCaller() :
    m_listener {nullptr}
{
}

template <typename FrameType>
void FrameListenerCaller<FrameType>::registerListener(IFrameListener<FrameType> *listener)
{
    //Locking the mutex makes sure the function only returns if any call on a previous listener is completed
    std::lock_guard<std::mutex> lock(m_listenerMutex);
    m_listener = listener;
}

template <typename FrameType>
void FrameListenerCaller<FrameType>::callListener(FrameType *frame)
{
    //Locking the mutex makes sure the pointer is not changed while it is inside a call
    std::lock_guard<std::mutex> lock(m_listenerMutex);
    if (m_listener)
    {
        m_listener->onNewFrame(frame);
    }
    else
    {
        frame->release();
    }
}
