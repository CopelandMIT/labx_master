/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "BridgeData.hpp"
#include <platform/exception/EBridgeData.hpp>

BridgeData::BridgeData() :
    m_frameForwarder(&m_frameQueue),
    m_dataStarted(false)
{
}

BridgeData::~BridgeData()
{
    m_frameQueue.stop();
}

void BridgeData::registerListener(IFrameListener<> *listener)
{
    m_frameForwarder.registerListener(listener);
}

void BridgeData::setFrameQueueSize(uint16_t count)
{
    if (count == 0)
    {
        throw EBridgeData("The frame queue size 0 is not allowed");
    }
    m_frameQueue.setMaxCount(count);
    //The frame pool must contain one entry more than the queue.
    //When a new frame is received, it needs a frame buffer to be queued
    //before the oldest buffer is released.
    setFramePoolCount(count + 1);
}

void BridgeData::clearFrameQueue()
{
    m_frameQueue.clear();
}

void BridgeData::queueFrame(IFrame *frame)
{
    if (isBridgeDataStarted())
    {
        m_frameQueue.enqueue(frame);
    }
    else
    {
        frame->release();
    }
}

IFrame *BridgeData::getFrame(uint16_t timeoutMs)
{
    if (m_dataStarted && !m_frameForwarder.hasListener())
    {
        return m_frameQueue.blockingDequeue(timeoutMs);
    }
    else
    {
        return nullptr;
    }
}

void BridgeData::startBridgeData()
{
    m_frameQueue.start();
    m_frameForwarder.start();
    m_dataStarted = true;
}

void BridgeData::stopBridgeData()
{
    m_dataStarted = false;
    m_frameQueue.stop();
    m_frameForwarder.stop();
    m_frameQueue.clear();
}

bool BridgeData::isBridgeDataStarted() const
{
    return m_dataStarted;
}
