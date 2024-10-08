/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "EnumeratorLibUsb.hpp"
#include "BridgeLibUsb.hpp"
#include <common/Logger.hpp>
#include <platform/templates/identifyBoardFunction.hpp>

#include <libusb-1.0/libusb.h>


namespace
{
    int getDeviceName(libusb_device *dev, struct libusb_device_descriptor *descriptor, char *buf, int size)
    {
        libusb_device_handle *handle;
        const auto ret_open = libusb_open(dev, &handle);
        if (ret_open != LIBUSB_SUCCESS)
        {
            return ret_open;
        }

        const auto ret = libusb_get_string_descriptor_ascii(handle, descriptor->iProduct, reinterpret_cast<unsigned char *>(buf), size);
        libusb_close(handle);
        return ret;
    }
}


std::shared_ptr<IBridge> BoardDescriptorLibUsb::createBridge()
{
    return std::make_shared<BridgeLibUsb>(m_device, m_fd);
}

EnumeratorLibUsbImpl::EnumeratorLibUsbImpl(uint8_t classCode) :
    m_classCode {classCode}
{
    initialiseLibUsb();
}

EnumeratorLibUsbImpl::~EnumeratorLibUsbImpl()
{
    libusb_exit(defaultContext);
}

void EnumeratorLibUsbImpl::enumerate(IEnumerationListener &listener, BoardData::const_iterator begin, BoardData::const_iterator end)
{
    LOG(DEBUG) << "Looking for USB boards with class code 0x" << std::hex << std::setw(2) << std::setfill('0') << +m_classCode << " (or 0x00) ...";

    libusb_device **devices;
    const ssize_t count = libusb_get_device_list(defaultContext, &devices);
    char cName[64];

    for (ssize_t i = 0; i < count; i++)
    {
        libusb_device *dev = devices[i];
        struct libusb_device_descriptor descriptor;
        const auto ret = libusb_get_device_descriptor(dev, &descriptor);
        if (ret < 0)
        {
            throw EConnection("EnumeratorLibUsbImpl::enumerate - libusb_get_device_descriptor() failed", ret);
        }

        if (descriptor.bDeviceClass && (descriptor.bDeviceClass != m_classCode))
        {
            continue;
        }

        const uint16_t &vid = descriptor.idVendor;
        const uint16_t &pid = descriptor.idProduct;

        auto it = findBoardData(begin, end, vid, pid);
        if (it == end)
        {
            continue;
        }

        // used for debug print only
        const auto bus  = libusb_get_bus_number(dev);
        const auto port = libusb_get_port_number(dev);

        if (getDeviceName(dev, &descriptor, cName, sizeof(cName)) < LIBUSB_SUCCESS)
        {
            LOG(DEBUG) << "... error getting device name: VID = " << std::hex << vid << " ; PID = " << pid << " ; bus = " << +bus << " ; port = " << +port;
            continue;
        }

        LOG(DEBUG) << "... device found: VID = " << std::hex << vid << " ; PID = " << pid << " ; bus = " << +bus << " ; port = " << +port << " \" ; name = \"" << cName << "\"";

        const bool stop = listener.onEnumerate(identifyBoardFunction<BoardDescriptorLibUsb>(begin, end, vid, pid, cName, dev));
        if (stop)
        {
            break;
        }
    }

    libusb_free_device_list(devices, 1);
}
