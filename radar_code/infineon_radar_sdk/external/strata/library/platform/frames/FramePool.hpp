/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <platform/frames/Frame.hpp>
#include <platform/interfaces/IFramePool.hpp>

#include <memory>
#include <mutex>
#include <vector>


class FramePool :
    public IFramePool
{
public:
    FramePool();
    ~FramePool();

    void setFrameBufferSize(uint32_t size) override;
    void setFrameCount(uint16_t count) override;
    IFrame *dequeueFrame() override;
    void queueFrame(IFrame *frame) override;

    bool initialized() const override;

private:
    std::mutex m_lock;

    uint32_t m_size;

    std::vector<std::unique_ptr<Frame>> m_pool;
    std::vector<IFrame *> m_queue;
};
