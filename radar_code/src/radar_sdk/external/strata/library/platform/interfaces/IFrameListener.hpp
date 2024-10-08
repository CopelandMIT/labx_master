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


template <typename FrameType = IFrame>
class IFrameListener
{
public:
    virtual ~IFrameListener() = default;

    /**
     * Called each time a new frame is available
     * @param frame Pointer to a frame containing the received data
     */
    virtual void onNewFrame(FrameType *frame) = 0;
};
