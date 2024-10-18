/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "FrameBase.hpp"


FrameBase::FrameBase() :
    m_refs {0},
    m_virtualChannel {0},
    m_timestamp {0}
{
}

uint8_t FrameBase::getVirtualChannel() const
{
    return m_virtualChannel;
}

void FrameBase::setVirtualChannel(uint8_t virtualChannel)
{
    m_virtualChannel = virtualChannel;
}

uint64_t FrameBase::getTimestamp() const
{
    return m_timestamp;
}

void FrameBase::setTimestamp(uint64_t timestamp)
{
    m_timestamp = timestamp;
}

void FrameBase::hold()
{
    m_refs++;
}

void FrameBase::release()
{
    if (!m_refs)
    {
        queue();
    }
    else
    {
        m_refs--;
    }
}
