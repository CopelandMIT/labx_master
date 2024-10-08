#pragma once

#include <components/Registers.hpp>
#include <platform/interfaces/access/II2c.hpp>


///
/// Registers implementation for I2C devices with Packet Error Checking (PEC) and 8bit registers
///
class Registers8bitPec :
    public Registers<uint8_t>
{
public:
    Registers8bitPec(II2c *accessI2c, uint16_t devAddr);

    using IRegisters<uint8_t>::read;
    using IRegisters<uint8_t>::write;

    ///
    /// @copydoc IMemory::read
    /// @details This implementation expects a PEC byte appended to the read I2C data. This CRC-8 value
    /// is used to validate the received data and throws an exception on failure.
    ///
    uint8_t read(uint8_t regAddr) override;

    ///
    /// @copydoc IMemroy::write
    /// @details This implementation appends a PEC byte (CRC-8 value) to the sent data.
    ///
    void write(uint8_t regAddr, uint8_t value) override;

    ///
    /// Read register without PEC
    /// @note Used to setup the I2C device in PEC mode
    ///
    /// @param regAddr address of the register to read
    /// @return uint8_t the read value
    ///
    uint8_t readWithoutPec(uint8_t regAddr);

    ///
    /// Write register without PEC
    /// @note Used to setup the I2C device in PEC mode
    ///
    /// @param regAddr address of the register to read
    /// @param value register value to write
    ///
    void writeWithoutPec(uint8_t regAddr, uint8_t value);

protected:
    II2c *m_accessI2c;
    const uint16_t m_devAddr;
};
