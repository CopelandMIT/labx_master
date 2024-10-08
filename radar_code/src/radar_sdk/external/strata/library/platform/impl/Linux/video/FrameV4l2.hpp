/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <platform/frames/FrameBase.hpp>
#include <platform/interfaces/IFramePool.hpp>

#include <linux/types.h>


class FrameV4l2 :
    public FrameBase
{
    friend class FramePoolV4l2;

public:
    using AlignmentType = uint8_t;

    FrameV4l2(IFramePool *owner, __u32 index, uint8_t *buffer, uint32_t bufferSize);
    virtual ~FrameV4l2() override;

    void resizeBuffer(uint32_t bufferSize);

    //IFrame
    uint8_t *getData() const override;
    uint32_t getDataSize() const override;
    void setDataOffset(uint32_t offset) override;
    void setDataSize(uint32_t dataSize) override;
    void setDataOffsetAndSize(uint32_t offset, uint32_t dataSize) override;
    uint32_t getDataOffset() const override;
    uint8_t *getBuffer() const override;
    uint32_t getBufferSize() const override;
    uint32_t getStatusCode() const override;

protected:
    void queue() override;

    __u32 m_index;

private:
    AlignmentType *m_buffer;
    IFramePool *m_owner;

    uint32_t m_offset;
    uint32_t m_dataSize;
    uint32_t m_bufferSize;
};
