/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "ErrorFrame.hpp"


ErrorFrame::ErrorFrame(uint32_t code) :
    m_code {code}
{
}

IFrame *ErrorFrame::create(uint32_t code, uint8_t virtualChannel, uint64_t timestamp)
{
    auto frame = new ErrorFrame(code);
    frame->setVirtualChannel(virtualChannel);
    frame->setTimestamp(timestamp);
    return frame;
}

uint8_t *ErrorFrame::getData() const
{
    return nullptr;
}

uint32_t ErrorFrame::getDataSize() const
{
    return 0;
}

void ErrorFrame::setDataOffset(uint32_t /*offset*/)
{
}

void ErrorFrame::setDataSize(uint32_t /*dataSize*/)
{
}

void ErrorFrame::setDataOffsetAndSize(uint32_t /*offset*/, uint32_t /*dataSize*/)
{
}

uint32_t ErrorFrame::getDataOffset() const
{
    return 0;
}

uint8_t *ErrorFrame::getBuffer() const
{
    return nullptr;
}

uint32_t ErrorFrame::getBufferSize() const
{
    return 0;
}

uint32_t ErrorFrame::getStatusCode() const
{
    return m_code;
}

void ErrorFrame::queue()
{
    delete this;
}
