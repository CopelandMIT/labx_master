/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <platform/frames/FrameListenerCaller.hpp>
#include <platform/interfaces/IFrameQueue.hpp>

#include <atomic>
#include <thread>


/**
 * This class is used to decouple processing frames from the receiving thread.
 * Sometimes processing can take some time but the receiving thread shall not be blocked for this time
 * to avoid data loss. In this case the FrameForwarder can be used.
 * It calls the registered listener from a separate thread.
 */
class FrameForwarder :
    public FrameListenerCaller<>
{
public:
    FrameForwarder(IFrameQueue *queue);
    ~FrameForwarder() override;

    void registerListener(IFrameListener<> *listener) override;

    void start();
    void stop();

private:
    void startForwardingThread();
    void waitForThreadReturn();

    IFrameQueue *m_queue;

    std::atomic<bool> m_isRunning;
    std::atomic<bool> m_stopThread;
    std::atomic<bool> m_threadReturned;

    std::thread m_forwardingThread;
    void forwardingThreadFunction();
};
