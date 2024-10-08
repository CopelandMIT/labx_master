/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include "FrameV4l2.hpp"
#include <platform/interfaces/IFramePool.hpp>
#include <platform/interfaces/IFrameQueue.hpp>

#include <memory>
#include <mutex>
#include <vector>


class FramePoolV4l2 :
    public IFramePool,
    public IFrameQueue
{
public:
    FramePoolV4l2(int &fd);
    ~FramePoolV4l2();

    void setFrameBufferSize(uint32_t size) override;
    void setFrameCount(uint16_t count) override;
    IFrame *dequeueFrame() override;
    void queueFrame(IFrame *frame) override;

    bool initialized() const override;

    void clear() override;
    IFrame *blockingDequeue(uint16_t timeoutMs = 0) override;
    void start() override;
    bool stop() override;

private:
    void allocate(uint16_t count, uint32_t size);
    void deallocate();
    void clearPool();
    int queue(__u32 index);
    int dequeue(struct v4l2_buffer &buf);

    int &m_fd;
    std::mutex m_lock;

    uint32_t m_size;
    std::vector<std::unique_ptr<FrameV4l2>> m_pool;
    int m_dequeuedCount;

    std::atomic<bool> m_queueing;

    // discard UVC_FIRST_FRAME_FIX frame (this is needed on Windows)
    bool m_discardFirst;
    bool m_discarding;
};
