/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "FramePool.hpp"

#include <common/Logger.hpp>
#include <common/cpp11/memory.hpp>
#include <common/exception/EGenericException.hpp>


FramePool::FramePool() :
    m_size {0}
{
}

FramePool::~FramePool()
{
    // All buffers will be automatically deleted when m_pool is destroyed after this
    // function returns. They should all be queued.
    //
    // If any of the buffers aren't queued then something else is still claiming ownership, and may
    // still access them.  This would indicate a bug, but we leave the still-accessible buffers
    // still allocated to hopefully avoid memory corruption.
    // If the size was not yet set (still 0), we don't need to worry about dequeued buffers
    const auto dequedCount = m_pool.size() - m_queue.size();
    if (dequedCount && (m_size != 0))
    {
        LOG(ERROR) << "Destroying FramePool with some buffers still dequeued: " << std::dec << dequedCount << " of " << m_pool.size();
        for (auto &buffer : m_pool)
        {
            buffer->unpool();
            buffer.release();  // NOLINT - we rather release the buffer, preferring a memory leak over memory corruption
        }
        for (auto buffer : m_queue)
        {
            delete buffer;
        }
    }
}

void FramePool::setFrameBufferSize(uint32_t size)
{
    if (size == 0)
    {
        throw EGenericException("Frame buffer size 0 is not allowed");
    }

    std::lock_guard<std::mutex> lock(m_lock);

    if (m_size != size)
    {
        for (auto &b : m_pool)
        {
            if (b)
            {
                //Only resize if there are already real buffers in the queue
                b->resizeBuffer(size);
            }
            else
            {
                //Create a real buffer
                b.reset(new Frame(this, size));
                m_queue.push_back(b.get());
            }
        }
        m_size = size;
    }
}

void FramePool::setFrameCount(uint16_t count)
{
    std::lock_guard<std::mutex> lock(m_lock);

    if ((m_size == 0) && (m_pool.size() != count))
    {
        //Fill the queue with empty pointers, will be initialized when setting size
        m_pool.resize(count);
        return;
    }

    if (m_pool.size() > count)
    {
        size_t delta = m_pool.size() - count;
        if (delta > m_queue.size())
        {
            LOG(ERROR) << "Too many buffers dequeued to reduce pool count";
            delta = m_queue.size();  // only dequeue what we can
        }

        for (auto i = delta; i > 0; i--)
        {
            IFrame *frame = m_queue.back();
            m_queue.pop_back();
            for (auto it = m_pool.begin(); it != m_pool.end(); it++)
            {
                if (frame == it->get())
                {
                    m_pool.erase(it);
                    break;
                }
            }
        }
    }

    if (m_pool.size() < count)
    {
        m_queue.reserve(count);
        m_pool.reserve(count);

        const size_t delta = count - m_pool.size();
        for (auto i = delta; i > 0; i--)
        {
            auto buffer = std::make_unique<Frame>(this, m_size);
            m_queue.push_back(buffer.get());
            m_pool.push_back(std::move(buffer));
        }
    }
}

void FramePool::queueFrame(IFrame *frame)
{
    std::lock_guard<std::mutex> lock(m_lock);
    auto buffer = dynamic_cast<Frame *>(frame);
    if (buffer == nullptr)
    {
        throw EGenericException("Queueing a buffer that wasn't allocated by this class");
    }

    for (auto &b : m_queue)
    {
        if (b == buffer)
        {
            throw EGenericException("Queueing already-queued buffer");
        }
    }

    m_queue.push_back(buffer);
}

bool FramePool::initialized() const
{
    return m_size && !m_pool.empty();
}

IFrame *FramePool::dequeueFrame()
{
    std::lock_guard<std::mutex> lock(m_lock);

    if (m_queue.empty())
    {
        return nullptr;
    }
    IFrame *frame = m_queue.back();
    m_queue.pop_back();
    return frame;
}
