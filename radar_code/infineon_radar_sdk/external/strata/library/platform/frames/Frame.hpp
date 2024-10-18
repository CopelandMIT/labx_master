/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include "FrameBase.hpp"
#include <platform/interfaces/IFramePool.hpp>


class Frame :
    public FrameBase
{
public:
    using AlignmentType = uint64_t;

    Frame(IFramePool *owner, uint32_t bufferSize);
    virtual ~Frame() override;

    void resizeBuffer(uint32_t bufferSize);

    /* Release the frame from the pool */
    void unpool();

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

private:
    AlignmentType *m_buffer;
    IFramePool *m_owner;

    uint32_t m_offset;
    uint32_t m_dataSize;
    uint32_t m_bufferSize;
};
