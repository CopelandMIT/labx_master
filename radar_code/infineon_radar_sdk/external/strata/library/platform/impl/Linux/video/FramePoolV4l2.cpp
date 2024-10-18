/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "FramePoolV4l2.hpp"

#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <common/Logger.hpp>
#include <common/Timing.hpp>
#include <common/cpp11/memory.hpp>
#include <common/exception/EGenericException.hpp>
#include <platform/exception/EConnection.hpp>


FramePoolV4l2::FramePoolV4l2(int &fd) :
    m_fd {fd},
    m_size {0},
    m_dequeuedCount {0},
    m_queueing {false},
    m_discardFirst {false}  //true}
{
}

FramePoolV4l2::~FramePoolV4l2()
{
}

void FramePoolV4l2::setFrameBufferSize(uint32_t size)
{
    if (size == 0)
    {
        throw EGenericException("Frame buffer size 0 is not allowed");
    }

    m_size = size;
}

void FramePoolV4l2::setFrameCount(uint16_t count)
{
    if (m_size == 0)
    {
        throw EGenericException("Size has to be set first");
    }

    if (m_pool.size() == count)
    {
        return;
    }

    allocate(count, m_size);
}

void FramePoolV4l2::queueFrame(IFrame *frame)
{
    std::lock_guard<std::mutex> lock(m_lock);
    auto buffer = dynamic_cast<FrameV4l2 *>(frame);
    if (buffer == nullptr)
    {
        throw EGenericException("Queueing a buffer that wasn't allocated by this class");
    }

    const int err = queue(buffer->m_index);
    if (!err)
    {
        m_dequeuedCount--;
    }
    else
    {
        LOG(ERROR) << "Queueing error";
    }
}

bool FramePoolV4l2::initialized() const
{
    return m_size && !m_pool.empty();
}

void FramePoolV4l2::clear()
{
    std::unique_lock<std::mutex> lock(m_lock);

    struct v4l2_buffer buf;
    while (true)
    {
        const int err = dequeue(buf);
        if (err)
        {
            break;
        }
        else
        {
            queue(buf.index);
        }
    };
}

IFrame *FramePoolV4l2::blockingDequeue(uint16_t timeoutMs)
{
    IFrame *frame;

    auto dequeueFunction = [&] {
        if (!m_queueing)
        {
            return false;
        }
        frame = dequeueFrame();
        return frame != nullptr;
    };

    //Wait for new frames or timeout.
    if (timeoutMs != 0)
    {
        waitFor(dequeueFunction, std::chrono::milliseconds(timeoutMs));
    }
    else
    {
        frame = dequeueFrame();
    }

    return frame;
}

void FramePoolV4l2::start()
{
    // queue allocated buffers (has to be done again after VIDIOC_STREAMOFF, so we do it here)
    for (size_t i = 0; i < m_pool.size(); i++)
    {
        queue(static_cast<__u32>(i));
    }

    m_discarding = m_discardFirst;
    m_queueing   = true;
}

bool FramePoolV4l2::stop()
{
    const bool wasQueueing = m_queueing.exchange(false);
    return wasQueueing;
}

void FramePoolV4l2::allocate(uint16_t count, uint32_t size)
{
    std::lock_guard<std::mutex> lock(m_lock);

    struct v4l2_requestbuffers req;
    req.reserved[0] = 0;
    req.count       = count;
    req.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory      = V4L2_MEMORY_MMAP;

    clearPool();
    m_pool.reserve(count);

    const int err = ioctl(m_fd, VIDIOC_REQBUFS, &req);
    if (err)
    {
        LOG(ERROR) << "Failed to allocate video buffers, error " << errno;
        if (errno == EBUSY)
        {
            throw EConnection("Device busy");
        }
        throw EConnection("TODO: Add error handling");
    }

    struct v4l2_buffer buf = {};
    buf.type               = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory             = V4L2_MEMORY_MMAP;

    // create buffers
    for (__u32 i = 0; i < count; i++)
    {
        buf.index = i;

        const int err = ioctl(m_fd, VIDIOC_QUERYBUF, &buf);
        if (err)
        {
            LOG(ERROR) << "Failed to allocate video buffer number " << i << ", error " << errno;
            throw EConnection("TODO: Add error handling");
        }

        // The buffer length is chosen by the driver, probably from a configuration provided with
        // the camera.  For V4L's UVC driver, it's based on the UVC dwMaxVideoFrameSize, which in
        // Arctic fFrameV4l2irmware is larger than any expected superframe size.

        if (buf.length < size)
        {
            LOG(ERROR) << "Buffer is too small to handle the expected image size";
            throw EConnection("NotImplemented TODO: configure larger buffers in the driver");
        }

        // mmap only the size that's needed, assuming the driver does not use 24 bits per pixel.
        buf.length = size;

        // Here PROT_WRITE is requested because of the in-place data normalization in the
        // acquisitionFunction. If the driver is providing data that's already in the format for
        // ICapturedBuffer then PROT_READ should be sufficient.
        auto data = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, buf.m.offset);
        if (data == MAP_FAILED)
        {
            LOG(ERROR) << "Failed to mmap video buffer number " << i << ", error " << errno;
            throw EConnection("NotImplemented TODO: Add error handling");
        }

        // add buffer to pool
        m_pool.emplace_back(std::make_unique<FrameV4l2>(this, i, static_cast<uint8_t *>(data), static_cast<uint32_t>(buf.length)));
    }
}

void FramePoolV4l2::deallocate()
{
    std::lock_guard<std::mutex> lock(m_lock);

    clearPool();

    struct v4l2_requestbuffers req;
    req.reserved[0] = 0;
    req.count       = 0;
    req.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory      = V4L2_MEMORY_MMAP;

    const int err = ioctl(m_fd, VIDIOC_REQBUFS, &req);
    if (err)
    {
        LOG(ERROR) << "Failed to destroy video buffers, error " << errno;
        if (errno == EBUSY)
        {
            throw EConnection("Device busy");
        }
        throw EConnection("TODO: Add error handling");
    }
}

int FramePoolV4l2::queue(__u32 index)
{
    if (index >= m_pool.size())
    {
        LOG(WARN) << "Queing frame index does not fit";
        return -1;
    }

    struct v4l2_buffer buf = {};

    buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    //buf.flags = V4L2_BUF_FLAG_TSTAMP_SRC_SOE;
    buf.index = index;

    const int err = ioctl(m_fd, VIDIOC_QBUF, &buf);
    switch (err)
    {
        case 0:
            // no error
            break;
        case ENODEV:
        case ENXIO:
            // device disconnected
            LOG(WARN) << "Failed to queue video buffer, device aleady disconnected. errno = " << std::dec << errno;
            break;
        default:
            LOG(ERROR) << "Failed to queue video buffer. errno = " << std::dec << errno;
            break;
    }
    return err;
}

int FramePoolV4l2::dequeue(struct v4l2_buffer &buf)
{
    buf        = {};
    buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    const int err = ioctl(m_fd, VIDIOC_DQBUF, &buf);
    if (err)
    {
        const int errnum = errno;
        switch (errnum)
        {
            case ECHILD:  // acquitions process not started
            case EAGAIN:  // no buffer available
            case EBUSY:   // not streaming
            case EINVAL:  // not streaming
                break;
            case ENODEV:
            case ENXIO:
                LOG(ERROR) << "Error dequeuing buffer, likely a device disconnection";
                break;
            default:
                LOG(ERROR) << "Error dequeuing buffer, errno: " << std::dec << errnum;
                break;
        }
    }
    else if (m_discarding)
    {
        m_discarding = false;
        queue(buf.index);
        return -1;
    }

    return err;
}

void FramePoolV4l2::clearPool()
{
    if (m_dequeuedCount)
    {
        LOG(ERROR) << "Clearing FramePoolV4l2 with " << m_dequeuedCount << " buffers still dequeued";
        m_dequeuedCount = 0;
    }

    m_pool.clear();
}

IFrame *FramePoolV4l2::dequeueFrame()
{
    std::lock_guard<std::mutex> lock(m_lock);

    struct v4l2_buffer buf;
    const int err = dequeue(buf);
    if (err)
    {
        return nullptr;
    }

    if (buf.flags & V4L2_BUF_FLAG_ERROR)
    {
        LOG(WARN) << "Buffer error flag set";
    }

    if (buf.index < m_pool.size())
    {
        FrameV4l2 *frame = m_pool[buf.index].get();
        frame->setDataOffset(0);
        frame->setDataSize(buf.bytesused);
        const uint64_t timestamp = (buf.timestamp.tv_sec * 1000000) + buf.timestamp.tv_usec;
        frame->setTimestamp(timestamp);

        m_dequeuedCount++;
        return frame;
    }
    else
    {
        // should be unreachable, or indicates an error
        return nullptr;
    }
}
