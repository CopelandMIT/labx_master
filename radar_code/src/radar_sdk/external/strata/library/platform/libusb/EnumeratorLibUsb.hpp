
/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <platform/BoardDescriptor.hpp>
#include <platform/exception/EConnection.hpp>
#include <platform/interfaces/IEnumerator.hpp>

#include <libusb-1.0/libusb.h>


namespace
{
    constexpr libusb_context *defaultContext = NULL;

    void initialiseLibUsb(void)
    {
        const auto ret = libusb_init(NULL);
        if (ret != LIBUSB_SUCCESS)
        {
            throw EConnection("initialiseLibUsb - libusb_init() failed", ret);
        }

        // libusb_set_option(defaultContext, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_DEBUG);
    }
}


class BoardDescriptorLibUsb :
    public BoardDescriptor
{
public:
    BoardDescriptorLibUsb(const BoardData &data, const char name[], libusb_device *device, int fd = 0) :
        BoardDescriptor(data, name),
        m_device {device},
        m_fd {fd}
    {
        initialiseLibUsb();
        libusb_ref_device(m_device);
    }

    ~BoardDescriptorLibUsb()
    {
        libusb_unref_device(m_device);
        libusb_exit(defaultContext);
    }

    std::shared_ptr<IBridge> createBridge() override;

private:
    libusb_device *m_device;
    int m_fd;
};


class EnumeratorLibUsbImpl :
    public IEnumerator
{
public:
    EnumeratorLibUsbImpl(uint8_t classCode = LIBUSB_CLASS_VENDOR_SPEC);
    ~EnumeratorLibUsbImpl();

    void enumerate(IEnumerationListener &listener, BoardData::const_iterator begin, BoardData::const_iterator end) override;

private:
    const uint8_t m_classCode;
};
