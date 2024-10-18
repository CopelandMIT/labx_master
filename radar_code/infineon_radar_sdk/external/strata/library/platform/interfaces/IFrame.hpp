/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <cstdint>
#include <memory>


/**
 * This contains one received frame.
 *
 * Although this has a virtual destructor, release() should always be called
 */
class IFrame
{
public:
    virtual ~IFrame() = default;

    /**
     * The data received as a plain buffer.
     * The pointer is valid until release() is called.
     */
    virtual uint8_t *getData() const = 0;

    /**
     * The number of used bytes in the array returned by getData().
     */
    virtual uint32_t getDataSize() const = 0;

    /**
     * Set the offset of the buffer to be returned by getData().
     */
    virtual void setDataOffset(uint32_t offset) = 0;

    /**
     * Set the number of used bytes in the array returned by getData().
     */
    virtual void setDataSize(uint32_t size) = 0;

    /**
     * Set the offset of the buffer and number of used bytes in the array returned by getData().
     */
    virtual void setDataOffsetAndSize(uint32_t offset, uint32_t dataSize) = 0;

    /**
     * The number of unused bytes at the beginning of the buffer.
     */
    virtual uint32_t getDataOffset() const = 0;

    /**
     * The underlying buffer without an offset.
     * The pointer is valid until FramePool::queueFrame() is called.
     */
    virtual uint8_t *getBuffer() const = 0;

    /**
     * The number of total bytes in the buffer returned by getBuffer().
     */
    virtual uint32_t getBufferSize() const = 0;

    /**
     * Returns the virtual channel ID.
     * (This can be used for multiple logical streams and/or as configuration index)
     */
    virtual uint8_t getVirtualChannel() const = 0;

    /**
     * Set the virtual channel ID / configuration index.
     */
    virtual void setVirtualChannel(uint8_t virtualChannel) = 0;

    /**
     * If a timestamp was available when the frame was captured, the time
     * given as microseconds after the 1970 epoch.  If a timestamp was not available, zero.
     */
    virtual uint64_t getTimestamp() const = 0;

    /**
     * Set the timestamp for this frame buffer.
     * If no timestamp is available, it should be set to zero.
     */
    virtual void setTimestamp(uint64_t timestamp) = 0;

    /**
     * Returns a status code for this frame buffer.
     * Zero means no error/status to report
     */
    virtual uint32_t getStatusCode() const = 0;

    /**
     * Hold on to the buffer even after the callback returns.
     * Can be called multiple times in different functions.
     * Has to be followed by an equal number of calls to release()
     * to requeue the buffer
     */
    virtual void hold() = 0;

    /**
     * After hold() has been called, this has to be called to signal
     * that the buffer is not needed any more and can be requeued.
     */
    virtual void release() = 0;
};


struct IFrameDeleter
{
    void operator()(IFrame *frame)
    {
        frame->release();
    }
};


using SmartIFrame = std::unique_ptr<IFrame, IFrameDeleter>;
