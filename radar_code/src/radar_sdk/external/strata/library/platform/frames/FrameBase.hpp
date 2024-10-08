/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <atomic>
#include <platform/interfaces/IFrame.hpp>


class FrameBase :
    public IFrame
{
public:
    FrameBase();
    FrameBase(const FrameBase &) = delete;
    FrameBase &operator=(const FrameBase &) = delete;

    //IFrame
    uint8_t getVirtualChannel() const override;
    void setVirtualChannel(uint8_t virtualChannel) override;
    uint64_t getTimestamp() const override;
    void setTimestamp(uint64_t timestamp) override;

    void hold() override;
    void release() override;

protected:
    virtual void queue() = 0;

private:
    std::atomic<unsigned int> m_refs;

    uint8_t m_virtualChannel;
    uint64_t m_timestamp;
};
