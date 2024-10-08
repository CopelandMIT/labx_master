#include "Registers8bitPec.hpp"
#include "common/EndianConversion.hpp"
#include "common/cpp11/iterator.hpp"
#include "common/crc/Crc8.hpp"
#include "common/exception/EGenericException.hpp"
#include "universal/i2c_definitions.h"

namespace
{
    ///
    /// Helper function for calculating the PEC value
    /// @details The PEC calculation requires the slave address to include the read/write bit as its
    /// lest significant bit (LSB)
    ///
    /// @param devAddr Full I2c device address (busId, devAddress)
    /// @param readWriteBit the bit signaling if data is written (0) or read(1)
    /// @return the 7bit device address as most significant bits and the read/write bit as LSB
    ///
    uint8_t devAddressRWBit(uint16_t devAddr, uint8_t readWriteBit)
    {
        uint8_t devAddress7bit = GET_I2C_DEV_ADDR_7BIT(devAddr);
        return (devAddress7bit << 1) | readWriteBit;
    }

    const uint16_t payloadLength = 2;
}


Registers8bitPec::Registers8bitPec(II2c *accessI2c, uint16_t devAddr) :
    m_accessI2c(accessI2c),
    m_devAddr(devAddr)
{
}

uint8_t Registers8bitPec::read(uint8_t regAddr)
{
    uint8_t payload[payloadLength];
    m_accessI2c->readWith8BitPrefix(m_devAddr, regAddr, payloadLength, payload);
    const uint8_t value         = payload[0];
    const uint8_t pecFromDevice = payload[1];

    // Verify if the PEC (CRC-8 SMBus) value is correct
    const uint8_t crcCheckData[] = {devAddressRWBit(m_devAddr, 0U), regAddr, devAddressRWBit(m_devAddr, 1U), value};
    const uint8_t crcCheck       = Crc8Smbus(crcCheckData, static_cast<uint16_t>(std::size(crcCheckData)));

    if (crcCheck != pecFromDevice)
        throw EGenericException("Wrong CRC value received from PMIC");

    return value;
}

void Registers8bitPec::write(uint8_t regAddr, uint8_t value)
{
    // Calculate PEC (CRC-8 SMBus) value. Everything starting from the first start condition is part of it.
    const uint8_t dataForCrc[] = {devAddressRWBit(m_devAddr, 0U), regAddr, value};
    const uint8_t crc          = Crc8Smbus(dataForCrc, static_cast<uint16_t>(std::size(dataForCrc)));

    const uint8_t payload[payloadLength] = {value, crc};
    m_accessI2c->writeWith8BitPrefix(m_devAddr, regAddr, payloadLength, payload);
}

uint8_t Registers8bitPec::readWithoutPec(uint8_t regAddr)
{
    uint8_t result;
    m_accessI2c->readWith8BitPrefix(m_devAddr, regAddr, 1U, &result);
    return result;
}

void Registers8bitPec::writeWithoutPec(uint8_t regAddr, uint8_t value)
{
    m_accessI2c->writeWith8BitPrefix(m_devAddr, regAddr, 1U, &value);
}
