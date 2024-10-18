/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include <common/Logger.hpp>
#include <platform/exception/EProtocol.hpp>

#include "VendorExtensionRealtek.hpp"
#include "VendorExtensionRealtekFlash.hpp"

#include <thread>


//todo: proper namespace
namespace
{

    /* Values for command */

#define CMD_FLASH_READ  0xC202
#define CMD_FLASH_WRITE 0x8202
#define CMD_FLASH_ERASE 0x0502

}


VendorExtensionRealtekFlash::VendorExtensionRealtekFlash(VendorExtensionRealtek *vendorExtension) :
    m_vendorExtension {vendorExtension}
{
}

uint32_t VendorExtensionRealtekFlash::getMaxTransfer() const
{
    return 32 * 1024;  // this was the magic value that was tested
}

void VendorExtensionRealtekFlash::read(uint8_t devId, uint32_t address, uint32_t length, uint8_t buffer[])
{
    if (devId > 0)
    {
        throw EProtocol("VendorExtensionRealtekFlash - devId > 0 not supported");
    }

    const auto addressLow  = static_cast<uint16_t>(address);
    const auto addressHigh = static_cast<uint16_t>(address >> 16);
    m_vendorExtension->vendorRead(CMD_FLASH_READ, addressLow, length, addressHigh, buffer);
}

void VendorExtensionRealtekFlash::write(uint8_t devId, uint32_t address, uint32_t length, const uint8_t buffer[])
{
    if (devId > 0)
    {
        throw EProtocol("VendorExtensionRealtekFlash - devId > 0 not supported");
    }

    const auto addressLow  = static_cast<uint16_t>(address);
    const auto addressHigh = static_cast<uint16_t>(address >> 16);
    m_vendorExtension->vendorWrite(CMD_FLASH_WRITE, addressLow, length, addressHigh, buffer);
}

void VendorExtensionRealtekFlash::erase(uint8_t devId, uint32_t address)
{
    if (devId > 0)
    {
        throw EProtocol("VendorExtensionRealtekFlash - devId > 0 not supported");
    }

    const auto addressLow  = static_cast<uint16_t>(address);
    const auto addressHigh = static_cast<uint16_t>(address >> 16);
    m_vendorExtension->vendorWrite(CMD_FLASH_ERASE, addressLow, 0, addressHigh);
}

uint8_t VendorExtensionRealtekFlash::getStatus(uint8_t devId)
{
    if (devId > 0)
    {
        throw EProtocol("VendorExtensionRealtekFlash - devId > 0 not supported");
    }

    // todo: actually read out WIP bit

    // simply wait for default worst case for page write duration: 1ms
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    return 0;
}
