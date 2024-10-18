/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include "IFrame.hpp"


class IFramePool
{
public:
    virtual ~IFramePool() = default;

    /**
     * Returns whether this frame pool has been properly initialized and is ready to use.
     */
    virtual bool initialized() const = 0;

    /**
     * Put a frame back into the pool when it is no longer needed
     * @param frame Pointer to the frame to queue
     */
    virtual void queueFrame(IFrame *frame) = 0;

protected:
    /**
     * Set the size in bytes of a single frame buffer
     * @param size The size in bytes
     */
    virtual void setFrameBufferSize(uint32_t size) = 0;

    /**
     * Set the number of frames to be available in the pool
     * @param count The number of frames
     */
    virtual void setFrameCount(uint16_t count) = 0;

    /**
     * Get a frame from the pool to work with.
     * The dequeued frame is removed from the available frames until it is queued again.
     * Throws an EGenericException if there is no more frame available
     * @return Pointer to the dequeued frame
     */
    virtual IFrame *dequeueFrame() = 0;
};
