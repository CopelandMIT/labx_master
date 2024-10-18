/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include "IFrameListener.hpp"


class IBridgeData
{
public:
    virtual ~IBridgeData() = default;

    /**
     * Set the size of the buffer for a single frame
     * @param size The size in bytes
     */
    virtual void setFrameBufferSize(uint32_t size) = 0;

    /**
     * Set the maximum number of frames queued in the frame queue
     * The queue works as a circular buffer and deletes the oldest frame upon
     * new frames if it is full
     * @param count Maximum number of frames in the queue
     */
    virtual void setFrameQueueSize(uint16_t count) = 0;

    /**
      * Removes all frames from the internal frame queue
      */
    virtual void clearFrameQueue() = 0;

    /**
     * Starts the streaming pipeline while also handling all necessary implementation specific internals
     */
    virtual void startStreaming() = 0;

    /**
     * Stops the streaming pipeline while also handling all necessary implementation specific internals
     */
    virtual void stopStreaming() = 0;


    /**
     * Register a listener to be called upon availability of new frames
     *
     * The listener is responsible for release()'ing the frames once their data is no longer needed.
     *
     * @param listener The listener to be called when a frame is available
     */
    virtual void registerListener(IFrameListener<> *listener) = 0;

    /**
     * Get the next frame from the queue. Returns when a frame is available, upon timeout or when exiting.
     * Returns nullptr immediately if a listener is registered
     *
     * The consumer is responsible for release()'ing the frame once its data is no longer needed.
     *
     * @param timeoutMs The maximum time to wait for a frame in milliseconds
     * @return pointer to frame or nullptr if no frame was received within the timeout
     */
    virtual IFrame *getFrame(uint16_t timeoutMs = 5000) = 0;
};
