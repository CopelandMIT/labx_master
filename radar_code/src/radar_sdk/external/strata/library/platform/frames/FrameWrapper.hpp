/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <memory>
#include <platform/interfaces/IFrame.hpp>


template <typename FrameType>
struct FrameReleaser
{
    void operator()(FrameType *frame)
    {
        if (frame)
        {
            frame->release();
        }
    }
};


template <typename FrameType = IFrame>
struct FrameWrapper :
    public std::unique_ptr<FrameType, FrameReleaser<FrameType>>
{
    FrameWrapper() = default;
    FrameWrapper(FrameType *frame)
    {
        this->reset(frame);
    }

    inline void allocate()
    {
        this->reset(new FrameType);
    }
};
