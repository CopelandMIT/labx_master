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


class IFrameQueue
{
public:
    virtual ~IFrameQueue() = default;

    ///
    /// Clear the queue and free all frames
    ///
    virtual void clear() = 0;

    ///
    /// Blocks until there is a new frame available or the queue is destroyed
    /// \param timeoutMs Time to wait in milliseconds for a new frame, 0 means wait forever (or until destruction)
    /// \return the next frame in the queue
    /// \retval nullptr on exit
    ///
    virtual IFrame *blockingDequeue(uint16_t timeoutMs = 0) = 0;

    ///
    /// Start functionality in case it was stopped before
    ///
    virtual void start() = 0;

    ///
    /// Stop functionality and release all blocking calls in the blockingDequeue function
    /// \return wheter the queue was active before stopping
    /// \retval bool
    ///
    virtual bool stop() = 0;
};
