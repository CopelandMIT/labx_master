/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "FrameQueue.hpp"
#include "ErrorFrame.hpp"
#include <universal/data_definitions.h>


FrameQueue::FrameQueue() :
    m_queueing {false},
    m_maxCount {0}
{
}

FrameQueue::~FrameQueue()
{
    FrameQueue::stop();
    FrameQueue::clear();
}

void FrameQueue::trimQueue()
{
    if (m_maxCount == 0)
    {
        // no limitation on queue count
        return;
    }

    if (m_queue.size() > m_maxCount)
    {
        // try to remove one more frame, since in the end we also want to prepend an error frame
        auto count = m_queue.size() - m_maxCount + 1;
        while (count--)
        {
            auto frame = m_queue.front();
            m_queue.pop_front();
            frame->release();
        }
        m_queue.push_front(ErrorFrame::create(DataError_FrameQueueTrimmed, VIRTUAL_CHANNEL_UNDEFINED));
    }
}

void FrameQueue::setMaxCount(uint32_t count)
{
    std::unique_lock<std::mutex> lock(m_lock);
    m_maxCount = count;
    trimQueue();
}

void FrameQueue::enqueue(IFrame *frame)
{
    if (m_queueing)
    {
        std::unique_lock<std::mutex> lock(m_lock);
        m_queue.push_back(frame);
        trimQueue();
        m_cv.notify_one();
    }
    else
    {
        frame->release();
    }
}

IFrame *FrameQueue::dequeue()
{
    std::unique_lock<std::mutex> lock(m_lock);
    if (!m_queueing || m_queue.empty())
    {
        return nullptr;
    }

    auto frame = m_queue.front();
    m_queue.pop_front();
    return frame;
}

IFrame *FrameQueue::blockingDequeue(uint16_t timeoutMs)
{
    //Predicate for the condition_variable to exit
    auto predicate = [&] {
        return (!m_queueing || !m_queue.empty());
    };

    std::unique_lock<std::mutex> lock(m_lock);

    //Wait for new frames or timeout. The condition variable checks the predicate before blocking.
    if (timeoutMs != 0)
    {
        m_cv.wait_for(lock, std::chrono::milliseconds(timeoutMs), predicate);
    }
    else
    {
        m_cv.wait(lock, predicate);
    }

    if (m_queue.empty())
    {
        return nullptr;
    }

    auto frame = m_queue.front();
    m_queue.pop_front();
    return frame;
}

void FrameQueue::clear()
{
    // release buffers before clearing the queue, since this is expected by the consumer
    std::unique_lock<std::mutex> lock(m_lock);
    if (!m_queue.empty())
    {
        for (auto &frame : m_queue)
        {
            frame->release();
        }
        m_queue.clear();
    }
}

void FrameQueue::start()
{
    m_queueing = true;
}

bool FrameQueue::stop()
{
    const bool wasQueueing = m_queueing.exchange(false);
    m_cv.notify_all();  //using notify_all in case multiple threads are waiting for frames
    return wasQueueing;
}
