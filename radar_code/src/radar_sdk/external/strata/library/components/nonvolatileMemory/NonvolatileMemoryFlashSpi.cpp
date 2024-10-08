/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include <components/nonvolatileMemory/NonvolatileMemoryFlashSpi.hpp>


namespace
{
    const uint8_t cmdWritePage   = 0x02;
    const uint8_t cmdReadBytes   = 0x03;
    const uint8_t cmdGetStatus   = 0x05;
    const uint8_t cmdWriteEnable = 0x06;
    const uint8_t cmdSectorErase = 0xD8;
}

FlashSpi::FlashSpi(ISpi *access) :
    m_access {access}
{
}

FlashSpi::~FlashSpi()
{
}

uint32_t FlashSpi::getMaxTransfer() const
{
    return m_access->getMaxTransfer();
}

void FlashSpi::readCommand(uint8_t devId, uint8_t command, uint32_t *address, uint32_t length, uint8_t buffer[])
{
    if (address)
    {
        const uint8_t cmdBuf[4] = {
            command,
            static_cast<uint8_t>(*address >> 16),
            static_cast<uint8_t>(*address >> 8),
            static_cast<uint8_t>(*address),
        };
        m_access->write(devId, sizeof(cmdBuf), cmdBuf, true);
    }
    else
    {
        m_access->write(devId, sizeof(command), &command, true);
    }
    m_access->read(devId, length, buffer);
}

void FlashSpi::writeCommand(uint8_t devId, uint8_t command, uint32_t *address, uint32_t length, const uint8_t buffer[])
{
    const bool writeBuffer = (buffer != nullptr);
    if (address)
    {
        const uint8_t cmdBuf[4] = {
            command,
            static_cast<uint8_t>(*address >> 16),
            static_cast<uint8_t>(*address >> 8),
            static_cast<uint8_t>(*address),
        };
        m_access->write(devId, sizeof(cmdBuf), cmdBuf, writeBuffer);
    }
    else
    {
        m_access->write(devId, sizeof(command), &command, writeBuffer);
    }
    if (writeBuffer)
    {
        m_access->write(devId, length, buffer);
    }
}

void FlashSpi::read(uint8_t devId, uint32_t address, uint32_t length, uint8_t buffer[])
{
    readCommand(devId, cmdReadBytes, &address, length, buffer);
}

void FlashSpi::write(uint8_t devId, uint32_t address, uint32_t length, const uint8_t buffer[])
{
    writeCommand(devId, cmdWriteEnable);
    writeCommand(devId, cmdWritePage, &address, length, buffer);
}

void FlashSpi::erase(uint8_t devId, uint32_t address)
{
    writeCommand(devId, cmdWriteEnable);
    writeCommand(devId, cmdSectorErase, &address);
}

uint8_t FlashSpi::getStatus(uint8_t devId)
{
    uint8_t result[2];
    readCommand(devId, cmdGetStatus, nullptr, 2, result);
    return result[0] & 0x01;
}

NonvolatileMemoryFlashSpi::NonvolatileMemoryFlashSpi(ISpi *access, uint8_t devId, uint32_t speed, const NonvolatileMemoryConfig_t &config) :
    FlashSpi(access),
    NonvolatileMemoryFlash(this, devId, config)
{
    access->configure(devId, 0, 8, speed);
}

NonvolatileMemoryFlashSpi::~NonvolatileMemoryFlashSpi()
{
}
