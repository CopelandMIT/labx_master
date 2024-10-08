/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "BridgeV4l2.hpp"
#include <platform/uvc/VendorExtensionList.hpp>

#include <common/Logger.hpp>
#include <platform/exception/EBridgeData.hpp>
#include <platform/exception/EConnection.hpp>


#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <linux/usb/video.h>
#include <linux/uvcvideo.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/mman.h>


static int xioctl(int fd, int request, void *arg)
{
    int r;

    do
    {
        r = ioctl(fd, request, arg);
    } while (r == -1 && ((errno == EINTR) || (errno == EAGAIN)));

    return r;
}

BridgeV4l2::BridgeV4l2(std::string &&devicePath) :
    m_fd {-1},
    m_framePool(m_fd),
    m_frameForwarder {&m_framePool},
    m_dataStarted {false},
    m_devicePath {std::move(devicePath)}
{
    openConnection();
}

BridgeV4l2::~BridgeV4l2()
{
    closeConnection();
}

bool BridgeV4l2::isConnected()
{
    return (m_fd >= 0);
}

void BridgeV4l2::openConnection()
{
    /*
        if (isConnected ())
        {
            LOG (ERROR) << "bridge already connected";
            throw LogicError ("bridge already connected");
        }
    */
    LOG(DEBUG) << "Connecting BridgeV4l2 ...";


    m_fd = open(m_devicePath.c_str(), O_RDWR | O_NONBLOCK);

    if (m_fd < 0)
    {
        throw EConnection("Could not open file handle", m_fd);
    }

#ifdef BRIDGE_V4L2_EXCLUSIVE_USE
    // Ask for exclusive use of the device.  Not doing this allows two BridgeV4l instances to
    // successfully openConnection() on the same device, the problem is not spotted until the call
    // to createAndQueueV4lBuffers(), which is not an expected time for a CouldNotOpen to be thrown.
    v4l2_priority priority = V4L2_PRIORITY_RECORD;
    const auto err         = xioctl(m_fd, VIDIOC_S_PRIORITY, &priority);
    if (err)
    {
        m_fd = -1;

        // store the current errno, as close() may change it
        const auto errnum = errno;
        LOG(ERROR) << "Failed BridgeV4l::openConnection, error " << errno;

        switch (errnum)
        {
            case EBUSY:
                throw EConnection("Another application is using this camera");
            default:
                throw EConnection("Couldn't set access priority", errnum);
        }
    }
#endif

    getVendorExtension();
}

void BridgeV4l2::getVendorExtension()
{
    m_extensionUnit = 3;

    m_vendorExtension = VendorExtensionList[0].factory(static_cast<IUvcExtension *>(this));
}

void BridgeV4l2::closeConnection()
{
    close(m_fd);
    m_fd = -1;
    m_vendorExtension.reset();
}

IBridgeControl *BridgeV4l2::getIBridgeControl()
{
    return m_vendorExtension.get();
}

IBridgeData *BridgeV4l2::getIBridgeData()
{
    return this;
}

void BridgeV4l2::setProperty(uint8_t id, uint16_t length, const uint8_t buffer[])
{
    struct uvc_xu_control_query query = {};
    query.unit                        = m_extensionUnit;
    query.selector                    = id;
    query.query                       = UVC_SET_CUR;
    query.size                        = length;
    query.data                        = const_cast<uint8_t *>(buffer);

    auto ret = xioctl(m_fd, UVCIOC_CTRL_QUERY, &query);
    if (ret == -1)
    {
        if (errno == EIO)
        {
            throw EConnection("PossiblyUsbStallError");
        }
        else
        {
            throw EConnection("RuntimeError Data transfer failed (status)", errno);
        }
    }
}

void BridgeV4l2::getProperty(uint8_t id, uint16_t length, uint8_t buffer[])
{
    struct uvc_xu_control_query query = {};
    query.unit                        = m_extensionUnit;
    query.selector                    = id;
    query.query                       = UVC_GET_CUR;
    query.size                        = length;
    query.data                        = buffer;

    auto ret = xioctl(m_fd, UVCIOC_CTRL_QUERY, &query);
    if (ret == -1)
    {
        if (errno == EIO)
        {
            // Although sending USB stalls are Arctic's standard error handling, with the kernel
            // UVC driver every stall triggers a line in the dmesg / syslog.
            // "Failed to query (GET_CUR) UVC control 2 on unit 3: -32 (exp. 64)."
            throw EConnection("PossiblyUsbStallError()");
        }
        else
        {
            throw EConnection("RuntimeError (Data transfer failed (status)", errno);
        }
    }
}

void BridgeV4l2::setVideoFormat(unsigned int width, unsigned int height)
{
    struct v4l2_format fmt;
    int ret;

    memset(&fmt, 0, sizeof fmt);
    fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width       = width;
    fmt.fmt.pix.height      = height;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_SBGGR12;
    fmt.fmt.pix.field       = V4L2_FIELD_ANY;

    ret = ioctl(m_fd, VIDIOC_S_FMT, &fmt);
    LOG(DEBUG) << "ioctl VIDIOC_S_FMT";
    if (ret < 0)
    {
        throw EConnection("Could not set v4l2 format err: ", errno);
    }

    LOG(DEBUG) << "V4L Format set ok. Width: " << fmt.fmt.pix.width
               << " Height: " << fmt.fmt.pix.height
               << " Size: " << fmt.fmt.pix.sizeimage;

    //    m_width = imageWidth;
    //    m_height = imageHeight;
}

void BridgeV4l2::startStreaming()
{
    if (m_dataStarted)
    {
        return;
    }

    if (m_fd < 0)
    {
        throw EBridgeData("Calling startData() without being connected");
    }
    if (!m_framePool.initialized())
    {
        throw EBridgeData("Calling startData() without frame pool being initialized");
    }

    cleanupStreaming();

    const int streamType = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int err              = ioctl(m_fd, VIDIOC_STREAMON, &streamType);
    LOG(DEBUG) << "ioctl VIDIOC_STREAMON";
    if (err)
    {
        LOG(ERROR) << "Failed to start the streaming, error " << errno;
        throw EConnection("NotImplemented (TODO: Add error handling");
    }

    m_dataStarted = true;
    m_framePool.start();
    m_frameForwarder.start();
}

void BridgeV4l2::stopStreaming()
{
    if (!m_dataStarted)
    {
        return;
    }
    if (m_fd < 0)
    {
        throw EConnection("not opened");
    }

    // Calling VIDIOC_STREAMOFF will unblock the acquisition thread
    const int streamType = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int err              = ioctl(m_fd, VIDIOC_STREAMOFF, &streamType);
    LOG(DEBUG) << "ioctl VIDIOC_STREAMOFF";
    switch (err)
    {
        case 0:
            // no error
            break;
        case ENODEV:
        case ENXIO:
            // device disconnected
            LOG(WARN) << "Error when stopping streaming, device already disconnected " << errno;
            break;
        default:
            LOG(ERROR) << "Error when stopping streaming, error " << errno;
    }

    m_dataStarted = false;
    m_framePool.stop();
    m_frameForwarder.stop();
}

void BridgeV4l2::cleanupStreaming()
{
    m_framePool.clear();
}

void BridgeV4l2::setFrameBufferSize(uint32_t size)
{
    if (m_fd < 0)
    {
        throw EConnection("not opened");
    }

    m_framePool.setFrameBufferSize(size);
}

void BridgeV4l2::setFrameQueueSize(uint16_t count)
{
    if (m_fd < 0)
    {
        throw EConnection("not opened");
    }

    m_framePool.setFrameCount(count);
}

void BridgeV4l2::clearFrameQueue()
{
    m_framePool.clear();
}

void BridgeV4l2::registerListener(IFrameListener<> *listener)
{
    m_frameForwarder.registerListener(listener);
}

IFrame *BridgeV4l2::getFrame(uint16_t timeoutMs)
{
    if (m_dataStarted && !m_frameForwarder.hasListener())
    {
        return m_framePool.blockingDequeue(timeoutMs);
    }
    else
    {
        return nullptr;
    }
}

void BridgeV4l2::lock()
{
}

void BridgeV4l2::unlock()
{
}

bool BridgeV4l2::variableSizeSupport() const
{
    return false;
}
