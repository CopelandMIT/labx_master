/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include <components/exception/ENonvolatileMemory.hpp>
#include <components/nonvolatileMemory/NonvolatileMemoryEepromI2c.hpp>

#include <chrono>
#include <thread>

namespace
{
    constexpr uint8_t bitsPerByte = 8;
}

NonvolatileMemoryEepromI2c::NonvolatileMemoryEepromI2c(II2c *access, uint16_t devAddr, uint8_t addressByteCount, const NonvolatileMemoryConfig_t &config, uint16_t maximumPageWriteTimeMs) :
    NonvolatileMemory(config, access->getMaxTransfer()),
    m_access {access},
    m_devAddr {devAddr},
    m_addressByteCount {addressByteCount},
    m_maximumPageWriteTimeMs {maximumPageWriteTimeMs}
{
    if ((m_addressByteCount < 1) || (m_addressByteCount > 2))
    {
        throw ENonvolatileMemory("Unsupported number of address bytes", m_addressByteCount);
    }

    const uint8_t bits = m_addressByteCount * bitsPerByte;
    if (((config.totalSize - 1) >> bits) & m_devAddr)
    {
        throw ENonvolatileMemory("Device address for I2C EEPROM and total size are conflicting", config.totalSize);
    }

    if (m_maxTransfer < config.pageSize)
    {
        throw ENonvolatileMemory("Access supports writes only smaller than page size", m_maxTransfer);
    }
}

NonvolatileMemoryEepromI2c::~NonvolatileMemoryEepromI2c()
{
}

void NonvolatileMemoryEepromI2c::readMemoryInterface(uint32_t address, uint32_t length, uint8_t buffer[])
{
    const uint8_t bits        = m_addressByteCount * bitsPerByte;
    const uint8_t addressHigh = static_cast<uint8_t>(address >> bits);

    if (addressHigh & m_devAddr)
    {
        throw ENonvolatileMemory("Device address for I2C EEPROM and read address are conflicting", address);
    }

    if (m_addressByteCount == 1)
    {
        const auto addressLow = static_cast<uint8_t>(address);
        m_access->readWith8BitPrefix(m_devAddr | addressHigh, addressLow, length, buffer);
    }
    else
    {
        const auto addressLow = static_cast<uint16_t>(address);
        m_access->readWith16BitPrefix(m_devAddr | addressHigh, addressLow, length, buffer);
    }
}

void NonvolatileMemoryEepromI2c::writeMemoryInterface(uint32_t address, uint32_t length, const uint8_t buffer[])
{
    const uint8_t bits        = m_addressByteCount * bitsPerByte;
    const uint8_t addressHigh = static_cast<uint8_t>(address >> bits);

    if (addressHigh & m_devAddr)
    {
        throw ENonvolatileMemory("Device address for I2C EEPROM and write address are conflicting", address);
    }

    if (m_addressByteCount == 1)
    {
        const auto addressLow = static_cast<uint8_t>(address);
        m_access->writeWith8BitPrefix(m_devAddr | addressHigh, addressLow, length, buffer);
    }
    else
    {
        const auto addressLow = static_cast<uint16_t>(address);
        m_access->writeWith16BitPrefix(m_devAddr | addressHigh, addressLow, length, buffer);
    }
    // Before the write cycle finishes, the EEPROM should not be accessed.
    // Either poll for ACK from the EEPROM, or simply wait the maximum write cycle time.
    if (m_maximumPageWriteTimeMs == 0)
    {
        m_access->pollForAck(m_devAddr | addressHigh);  // ACK polling to ensure write cycle has finished
    }
    else
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(m_maximumPageWriteTimeMs));
    }
}

void NonvolatileMemoryEepromI2c::eraseMemoryInterface(uint32_t /*address*/)
{
}
