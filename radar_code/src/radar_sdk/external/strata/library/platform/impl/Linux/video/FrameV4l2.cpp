/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "FrameV4l2.hpp"
#include <common/Buffer.hpp>
#include <common/Logger.hpp>

#include <sys/mman.h>
#include <stdexcept>


FrameV4l2::FrameV4l2(IFramePool *owner, __u32 index, uint8_t *buffer, uint32_t bufferSize) :
    m_index {index},
    m_buffer {buffer},
    m_owner {owner},
    m_offset {0},
    m_dataSize {0},
    m_bufferSize {bufferSize}
{
}

FrameV4l2::~FrameV4l2()
{
    const int ret = munmap(m_buffer, m_bufferSize);
    if (ret == -1)
    {
        LOG(ERROR) << "Error while munmapping buffer " << errno;
    }
}

void FrameV4l2::resizeBuffer(uint32_t bufferSize)
{
    // not possible
}

uint8_t *FrameV4l2::getData() const
{
    return reinterpret_cast<uint8_t *>(m_buffer) + m_offset;
}

uint32_t FrameV4l2::getDataSize() const
{
    return m_dataSize;
}

void FrameV4l2::setDataOffset(uint32_t offset)
{
    if (m_dataSize + offset > m_bufferSize)
    {
        throw std::out_of_range("Buffer too small");
    }

    m_offset = offset;
}

void FrameV4l2::setDataSize(uint32_t dataSize)
{
    if (dataSize + m_offset > m_bufferSize)
    {
        throw std::out_of_range("Buffer too small");
    }

    m_dataSize = dataSize;
}

void FrameV4l2::setDataOffsetAndSize(uint32_t offset, uint32_t dataSize)
{
    if (dataSize + offset > m_bufferSize)
    {
        throw std::out_of_range("Buffer too small");
    }

    m_offset   = offset;
    m_dataSize = dataSize;
}

uint32_t FrameV4l2::getDataOffset() const
{
    return m_offset;
}

uint8_t *FrameV4l2::getBuffer() const
{
    return reinterpret_cast<uint8_t *>(m_buffer);
}

uint32_t FrameV4l2::getBufferSize() const
{
    return m_bufferSize;
}

uint32_t FrameV4l2::getStatusCode() const
{
    return 0;
}

void FrameV4l2::queue()
{
    m_owner->queueFrame(this);
}
