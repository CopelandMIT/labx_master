/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <platform/interfaces/IBridgeControl.hpp>
#include <platform/interfaces/link/IUvcExtension.hpp>

#include <platform/uvc/VendorExtensionCypress.hpp>
#include <platform/uvc/VendorExtensionRealtek.hpp>

#include <common/cpp11/memory.hpp>


typedef uint8_t byteGuid[16];  // byte-wise definition in 3 little endian values of byte lengths 4-2-2 and an 8 byte array


using VendorExtensionFactoryFunction = std::unique_ptr<IBridgeControl>(IUvcExtension *uvcExtension);

template <typename T>
std::unique_ptr<IBridgeControl> vendorExtensionFactory(IUvcExtension *uvcExtension)
{
    return std::make_unique<T>(uvcExtension);
}


struct VendorExtensionData
{
    const byteGuid guid;                      ///< UVC Extension GUID
    VendorExtensionFactoryFunction &factory;  ///< Reference to corresponding factory function for vendor extension
};

const VendorExtensionData VendorExtensionList[] {
    {{UVC_XU_GUID}, vendorExtensionFactory<VendorExtensionCypress>},
    {{REALTEK_XU_GUID}, vendorExtensionFactory<VendorExtensionRealtek>},
};
