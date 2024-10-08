/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <components/nonvolatileMemory/NonvolatileMemory.hpp>
#include <platform/interfaces/access/II2c.hpp>

///
/// Specific implementation of non-volatile memory for I2C EEPROMs
///
class NonvolatileMemoryEepromI2c :
    public NonvolatileMemory
{
public:
    ///
    /// \brief Construct an object
    /// \param access The I2C interface to use for communication with the EEPROM
    /// \param devAddr The device address of the EEPPROM to talk to as it is wired on the device
    /// \param addressByteCount The number of bytes used in the I2C transfers for addressing memory during read and write operations
    ///     The number of address bytes is described in the datasheet of the EEPROM type. These are the bytes between the
    ///     device address byte and the actual data.
    /// \param config A configuration structure describing the memory organisation of the EEPROM
    /// \param maximumPageWriteTimeMs The period of time which will be waited after write operations to ensure that the write finished.
    /// For many EEPROMs the time is specified as 5 ms in the datasheet, like 24AA32A/24LC32A, AT24CS04/AT24CS08, and AT24CS32
    /// (http://ww1.microchip.com/downloads/en/devicedoc/21713m.pdf). In case maximumPageWriteTimeMs is se to 0,
    /// poll for ACK from the EEPROM will take place instead of sleeping.
    ///
    STRATA_API NonvolatileMemoryEepromI2c(II2c *access, uint16_t devAddr, uint8_t addressByteCount, const NonvolatileMemoryConfig_t &config, uint16_t maximumPageWriteTimeMs = 5);
    STRATA_API virtual ~NonvolatileMemoryEepromI2c();

protected:
    void readMemoryInterface(uint32_t address, uint32_t length, uint8_t buffer[]) override;
    void writeMemoryInterface(uint32_t address, uint32_t length, const uint8_t buffer[]) override;
    void eraseMemoryInterface(uint32_t address) override;

private:
    II2c *m_access;
    const uint16_t m_devAddr;
    const uint8_t m_addressByteCount;
    const uint16_t m_maximumPageWriteTimeMs;
};
