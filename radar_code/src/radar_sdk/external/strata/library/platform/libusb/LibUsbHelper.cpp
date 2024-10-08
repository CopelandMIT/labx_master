/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "LibUsbHelper.hpp"
#include <platform/exception/EConnection.hpp>

#if LIBUSB_API_VERSION < 0x01000108
    #include <common/Logger.hpp>
#endif


namespace LibUsbHelper
{

    inline void disableDeviceDiscovery()
    {
        // disabling device discovery is not supported on Windows
#ifndef _WIN32
    #if LIBUSB_API_VERSION < 0x01000108
        LOG(WARN) << "LibUsbHelper::disableDeviceDiscovery - not supported by LIBUSB_API_VERSION < 0x01000108";
    #else
        #if LIBUSB_API_VERSION == 0x01000108
            #define LIBUSB_OPTION_NO_DEVICE_DISCOVERY LIBUSB_OPTION_WEAK_AUTHORITY
        #endif
        const auto ret = libusb_set_option(NULL, LIBUSB_OPTION_NO_DEVICE_DISCOVERY, NULL);
        if (ret != LIBUSB_SUCCESS)
        {
            throw EConnection("LibUsbHelper::disableDeviceDiscovery - libusb_set_option() failed", ret);
        }
    #endif
#endif
    }

    void init(libusb_context **context, bool disableDeviceDiscovery)
    {
        if (disableDeviceDiscovery)
        {
            LibUsbHelper::disableDeviceDiscovery();
        }

        const auto ret = libusb_init(context);
        if (ret != LIBUSB_SUCCESS)
        {
            throw EConnection("LibUsbHelper::init - libusb_init() failed", ret);
        }
    }

    void open(libusb_device *device, libusb_device_handle **deviceHandle)
    {
        const auto ret = libusb_open(device, deviceHandle);
        if (ret != LIBUSB_SUCCESS)
        {
            throw EConnection("LibUsbHelper::open - libusb_open() failed", ret);
        }
    }

    void open(libusb_context *context, int fd, libusb_device_handle **deviceHandle)
    {
#if LIBUSB_API_VERSION < 0x01000107
        throw EConnection("LibUsbHelper::open - libusb_wrap_sys_device() not supported by LIBUSB_API_VERSION < 0x01000107");
#else
        const auto ret = libusb_wrap_sys_device(context, static_cast<intptr_t>(fd), deviceHandle);
        if (ret != LIBUSB_SUCCESS)
        {
            throw EConnection("LibUsbHelper::open - libusb_wrap_sys_device() failed", ret);
        }
#endif
    }

    int readBulk(libusb_device_handle *deviceHandle, uint8_t endpoint, uint8_t buffer[], int length, unsigned int timeout)
    {
        int transferred = 0;
        const int ret   = libusb_bulk_transfer(deviceHandle, endpoint, buffer, length, &transferred, timeout);

        switch (ret)
        {
            case LIBUSB_TRANSFER_COMPLETED:
            case LIBUSB_ERROR_TIMEOUT:
                break;
            default:
                throw EConnection("LibUsbHelper::readBulk - libusb_bulk_transfer() failed", ret);
                break;
        }

        return transferred;
    }

    int writeBulk(libusb_device_handle *deviceHandle, uint8_t endpoint, const uint8_t buffer[], int length, unsigned int timeout)
    {
        int transferred = 0;
        const int ret   = libusb_bulk_transfer(deviceHandle, endpoint, const_cast<unsigned char *>(buffer), length, &transferred, timeout);

        switch (ret)
        {
            case LIBUSB_TRANSFER_COMPLETED:
            case LIBUSB_ERROR_TIMEOUT:
                break;
            default:
                throw EConnection("LibUsbHelper::writeBulk - libusb_bulk_transfer() failed", ret);
                break;
        }

        return transferred;
    }


}
