/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include <common/Buffer.hpp>
#include <common/Logger.hpp>
#include <platform/exception/EProtocol.hpp>

#include "VendorExtensionRealtek.hpp"
#include "VendorExtensionRealtekI2c.hpp"


//todo: proper namespace
namespace
{
    /* Values for command */
#define CMD_I2C_WRITE 0xAA00
#define CMD_I2C_READ  0xEA00

#define CMD_IMAGER_WRITE 0x8A00
#define CMD_IMAGER_READ  0xCA00

}


VendorExtensionRealtekI2c::VendorExtensionRealtekI2c(VendorExtensionRealtek *vendorExtension) :
    m_vendorExtension {vendorExtension}
{
}

uint16_t VendorExtensionRealtekI2c::getMaxTransfer() const
{
    return m_vendorExtension->getMaxTransfer();
}

void VendorExtensionRealtekI2c::writeWithoutPrefix(uint16_t devAddr, uint16_t length, const uint8_t buffer[])
{
    if (devAddr & I2C_DEFAULT_DEVICE)
    {
        m_vendorExtension->vendorWrite(CMD_IMAGER_WRITE, 0, length, 0, buffer);
    }
    else
    {
        devAddr <<= 1;
        m_vendorExtension->vendorWrite(CMD_I2C_WRITE, devAddr, length, 0, buffer);
    }
}

void VendorExtensionRealtekI2c::writeWith8BitPrefix(uint16_t devAddr, uint8_t prefix, uint16_t length, const uint8_t buffer[])
{
    if (devAddr & I2C_DEFAULT_DEVICE)
    {
        m_vendorExtension->vendorWrite(CMD_IMAGER_WRITE, prefix, length, 0, buffer);
    }
    else
    {
        devAddr <<= 1;
        const uint16_t bufSize = sizeof(prefix) + length;
        strata::buffer<uint8_t> buf(bufSize);
        buf[0] = static_cast<uint8_t>(prefix);
        std::copy(buffer, buffer + length, &buf[sizeof(prefix)]);
        m_vendorExtension->vendorWrite(CMD_I2C_WRITE, devAddr, bufSize, 0, buf.data());
    }
}

void VendorExtensionRealtekI2c::writeWith16BitPrefix(uint16_t devAddr, uint16_t prefix, uint16_t length, const uint8_t buffer[])
{
    if (devAddr & I2C_DEFAULT_DEVICE)
    {
        m_vendorExtension->vendorWrite(CMD_IMAGER_WRITE, prefix, length, 0, buffer);
    }
    else
    {
        devAddr <<= 1;
        const uint16_t bufSize = sizeof(prefix) + length;
        strata::buffer<uint8_t> buf(bufSize);
        buf[0] = static_cast<uint8_t>(prefix >> 8);
        buf[1] = static_cast<uint8_t>(prefix);
        std::copy(buffer, buffer + length, &buf[sizeof(prefix)]);
        m_vendorExtension->vendorWrite(CMD_I2C_WRITE, devAddr, bufSize, 0, buf.data());
    }
}

void VendorExtensionRealtekI2c::readWithoutPrefix(uint16_t devAddr, uint16_t length, uint8_t buffer[])
{
    if (devAddr & I2C_DEFAULT_DEVICE)
    {
        m_vendorExtension->vendorRead(CMD_IMAGER_READ, 0, length, 0, buffer);
    }
    else
    {
        devAddr <<= 1;
        m_vendorExtension->vendorRead(CMD_I2C_READ, devAddr, length, 0, buffer);
    }
}

void VendorExtensionRealtekI2c::readWith8BitPrefix(uint16_t devAddr, uint8_t prefix, uint16_t length, uint8_t buffer[])
{
    if (devAddr & I2C_DEFAULT_DEVICE)
    {
        m_vendorExtension->vendorRead(CMD_IMAGER_READ, prefix, length, 0, buffer);
    }
    else
    {
        devAddr <<= 1;
        m_vendorExtension->vendorTransfer(CMD_I2C_WRITE, devAddr, sizeof(prefix), 0, &prefix,
                                          CMD_I2C_READ, devAddr, length, 0, buffer);
    }
}

void VendorExtensionRealtekI2c::readWith16BitPrefix(uint16_t devAddr, uint16_t prefix, uint16_t length, uint8_t buffer[])
{
    if (devAddr & I2C_DEFAULT_DEVICE)
    {
        m_vendorExtension->vendorRead(CMD_IMAGER_READ, prefix, length, 0, buffer);
    }
    else
    {
        devAddr <<= 1;
        const uint8_t buf[sizeof(prefix)] = {
            static_cast<uint8_t>(prefix >> 8),
            static_cast<uint8_t>(prefix),
        };
        m_vendorExtension->vendorTransfer(CMD_I2C_WRITE, devAddr, sizeof(buf), 0, buf,
                                          CMD_I2C_READ, devAddr, length, 0, buffer);
    }
}

void VendorExtensionRealtekI2c::configureBusSpeed(uint16_t /*devAddr*/, uint32_t /*speed*/)
{
}

void VendorExtensionRealtekI2c::clearBus(uint16_t /*devAddr*/)
{
}

void VendorExtensionRealtekI2c::pollForAck(uint16_t /*devAddr*/)
{
}
