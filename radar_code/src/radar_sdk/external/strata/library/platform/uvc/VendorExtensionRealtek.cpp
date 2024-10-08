/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "VendorExtensionRealtek.hpp"

#include <platform/exception/EProtocol.hpp>

#include <mutex>


//todo: proper namespace
namespace
{
    const uint16_t defaultChunkSize = 8;

#define UVC_XU_PROPERTY_COMMAND  10
#define UVC_XU_PROPERTY_DATA     11
#define UVC_XU_PROPERTY_DATA_256 12


    /* Values for command */

#define CMD_SET_VARIABLE_DATA_LENGTH 0x1600

#define CMD_FLASH_READ  0xC202
#define CMD_MEMORY_READ 0xC200

#define CMD_ADDRESS_USB_MODE  0xFE80
#define CMD_ADDRESS_RESET_PIN 0xFC48

}


VendorExtensionRealtek::VendorExtensionRealtek(IUvcExtension *uvcExtension) :
    m_uvcExtension {uvcExtension},
    m_flash(this),
    m_i2c(this)
{
}

void VendorExtensionRealtek::getBoardInfo(BoardInfo_t & /*buffer*/)
{
}

void VendorExtensionRealtek::checkVersion()
{
}

uint16_t VendorExtensionRealtek::getMaxTransfer() const
{
    // we can write multiple times to the variable data property, so we are in theory not limited
    return std::numeric_limits<uint16_t>::max();
}

IGpio *VendorExtensionRealtek::getIGpio()
{
    return this;
}

II2c *VendorExtensionRealtek::getII2c()
{
    return &m_i2c;
}

IFlash *VendorExtensionRealtek::getIFlash()
{
    return &m_flash;
}

void VendorExtensionRealtek::sendCommand(uint16_t command, uint16_t addressLow, uint16_t length, uint16_t addressHigh)
{
    const uint8_t commandPacket[8] = {
        static_cast<uint8_t>(command),
        static_cast<uint8_t>(command >> 8),
        static_cast<uint8_t>(addressLow),
        static_cast<uint8_t>(addressLow >> 8),
        static_cast<uint8_t>(length),
        static_cast<uint8_t>(length >> 8),
        static_cast<uint8_t>(addressHigh),
        static_cast<uint8_t>(addressHigh >> 8),
    };

    m_uvcExtension->setProperty(UVC_XU_PROPERTY_COMMAND, sizeof(commandPacket), commandPacket);
}

void VendorExtensionRealtek::sendData(uint16_t length, const uint8_t buffer[])
{
    while (length > defaultChunkSize)
    {
        m_uvcExtension->setProperty(UVC_XU_PROPERTY_DATA, defaultChunkSize, buffer);
        length -= defaultChunkSize;
        buffer += defaultChunkSize;
    }
    if (length)
    {
        m_uvcExtension->setProperty(UVC_XU_PROPERTY_DATA, length, buffer);
    }
}

void VendorExtensionRealtek::sendDataVariable(uint32_t chunkSize, uint32_t length, const uint8_t buffer[])
{
    sendCommand(CMD_SET_VARIABLE_DATA_LENGTH, 0, chunkSize, 0);
    while (length > chunkSize)
    {
        m_uvcExtension->setProperty(UVC_XU_PROPERTY_DATA, chunkSize, buffer);
        length -= chunkSize;
        buffer += chunkSize;
    }
    if (length)
    {
        m_uvcExtension->setProperty(UVC_XU_PROPERTY_DATA, length, buffer);
    }
    sendCommand(CMD_SET_VARIABLE_DATA_LENGTH, 0, defaultChunkSize, 0);
}

void VendorExtensionRealtek::receiveData(uint32_t length, uint8_t buffer[])
{
    while (length > defaultChunkSize)
    {
        m_uvcExtension->getProperty(UVC_XU_PROPERTY_DATA, defaultChunkSize, buffer);
        length -= defaultChunkSize;
        buffer += defaultChunkSize;
    }
    if (length)
    {
        m_uvcExtension->getProperty(UVC_XU_PROPERTY_DATA, length, m_buffer);
        std::copy(m_buffer, m_buffer + length, buffer);
    }
}

void VendorExtensionRealtek::receiveDataVariable(uint32_t chunkSize, uint32_t length, uint8_t buffer[])
{
    sendCommand(CMD_SET_VARIABLE_DATA_LENGTH, 0, chunkSize, 0);
    while (length > chunkSize)
    {
        m_uvcExtension->getProperty(UVC_XU_PROPERTY_DATA, chunkSize, buffer);
        length -= chunkSize;
        buffer += chunkSize;
    }
    if (length)
    {
        m_uvcExtension->getProperty(UVC_XU_PROPERTY_DATA, length, m_buffer);
        std::copy(m_buffer, m_buffer + length, buffer);
    }
    sendCommand(CMD_SET_VARIABLE_DATA_LENGTH, 0, defaultChunkSize, 0);
}

void VendorExtensionRealtek::vendorWrite(uint16_t command, uint16_t addressLow, uint16_t length, uint16_t addressHigh)
{
    sendCommand(command, addressLow, length, addressHigh);
}

void VendorExtensionRealtek::vendorWrite(uint16_t command, uint16_t addressLow, uint16_t length, uint16_t addressHigh, const uint8_t buffer[])
{
    std::lock_guard<IUvcExtension> lock(*m_uvcExtension);
    sendCommand(command, addressLow, length, addressHigh);
    sendData(length, buffer);
}

void VendorExtensionRealtek::vendorRead(uint16_t command, uint16_t addressLow, uint16_t length, uint16_t addressHigh, uint8_t buffer[])
{
    std::lock_guard<IUvcExtension> lock(*m_uvcExtension);
    sendCommand(command, addressLow, length, addressHigh);
    if (command == CMD_FLASH_READ)
    {
        receiveDataVariable(m_maxPayload, length, buffer);
    }
    else
    {
        receiveData(length, buffer);
    }
}

void VendorExtensionRealtek::vendorTransfer(uint16_t commandSend, uint16_t addressLowSend, uint16_t lengthSend, uint16_t addressHighSend, const uint8_t bufferSend[], uint16_t commandReceive, uint16_t addressLowReceive, uint16_t lengthReceive, uint16_t addressHighReceive, uint8_t bufferReceive[])
{
    std::lock_guard<IUvcExtension> lock(*m_uvcExtension);
    sendCommand(commandSend, addressLowSend, lengthSend, addressHighSend);
    if (bufferSend)
    {
        sendData(lengthSend, bufferSend);
    }
    sendCommand(commandReceive, addressLowReceive, lengthReceive, addressHighReceive);
    receiveData(lengthReceive, bufferReceive);
}

void VendorExtensionRealtek::checkError()
{
    //todo: how can write errors be checked?
}


void VendorExtensionRealtek::configurePin(uint16_t id, uint8_t /*flags*/)
{
    if (id != GPIO_NAME_RESET)
    {
        throw EProtocol("VendorExtensionRealtek::configurePin() - other pins than reset are not implemented");
    }

    // config is done when setting
}

void VendorExtensionRealtek::setPin(uint16_t id, bool state)
{
    if (id != GPIO_NAME_RESET)
    {
        throw EProtocol("VendorExtensionRealtek::setPin() - other pins than reset are not implemented");
    }

    uint8_t buf;
    vendorRead(CMD_MEMORY_READ, CMD_ADDRESS_RESET_PIN, sizeof(buf), 0, &buf);
    buf |= 0x02;  // this enables GPIO output! should be done in firmware configuration
    if (state)
    {
        buf &= ~0x20;  // logic value: false
    }
    else
    {
        buf |= 0x20;  // logic value: true
    }
    vendorWrite(CMD_MEMORY_READ, CMD_ADDRESS_RESET_PIN, sizeof(buf), 0, &buf);
}

bool VendorExtensionRealtek::getPin(uint16_t /*id*/)
{
    throw EProtocol("VendorExtensionRealtek::getPin() - not implemented");
}

void VendorExtensionRealtek::configurePort(uint16_t /*port*/, uint8_t /*flags*/, uint32_t /*mask*/)
{
    throw EProtocol("not implemented");
}

void VendorExtensionRealtek::setPort(uint16_t /*port*/, uint32_t /*state*/, uint32_t /*mask*/)
{
    throw EProtocol("not implemented");
}

uint32_t VendorExtensionRealtek::getPort(uint16_t /*port*/)
{
    throw EProtocol("not implemented");
}
