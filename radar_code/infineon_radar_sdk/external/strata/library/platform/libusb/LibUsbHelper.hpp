/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once


#include <libusb-1.0/libusb.h>


namespace LibUsbHelper
{
    constexpr libusb_context *defaultContext = NULL;

    void init(libusb_context **context, bool disableDeviceDiscovery = false);

    void open(libusb_device *device, libusb_device_handle **deviceHandle);
    void open(libusb_context *context, int fd, libusb_device_handle **deviceHandle);

    int readBulk(libusb_device_handle *deviceHandle, uint8_t endpoint, uint8_t buffer[], int length, unsigned int timeout);
    int writeBulk(libusb_device_handle *deviceHandle, uint8_t endpoint, const uint8_t buffer[], int length, unsigned int timeout);
}
