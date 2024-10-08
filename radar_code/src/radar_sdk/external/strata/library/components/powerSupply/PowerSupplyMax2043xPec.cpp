#include "PowerSupplyMax2043xPec.hpp"

PowerSupplyMax2043xPec::PowerSupplyMax2043xPec(II2c *accessI2c, uint16_t devAddrI2c) :
    m_registers(accessI2c, devAddrI2c)
{
}

IRegisters<uint8_t> *PowerSupplyMax2043xPec::getIRegisters()
{
    return &m_registers;
}

bool PowerSupplyMax2043xPec::switchVoltageOutput(uint8_t output, bool enable)
{
    if ((output & ~VOLTAGE_OUT_ALL) != 0)
    {
        // Invalid value, unsupported bit is set (only the output voltages are allowed)
        return false;
    }
    uint8_t regValue = m_registers.read(CONFIGE);
    if (enable)
    {
        regValue |= output;
    }
    else
    {
        regValue &= ~output;
    }
    m_registers.write(CONFIGE, regValue);
    return true;
}

bool PowerSupplyMax2043xPec::setVoltageOutput(uint8_t output, uint16_t mV)
{
    uint16_t offset = getVoutVoltageOffset();
    if (mV < offset)
    {
        // would produce a negative value wrapping around and setting a too high value
        return false;
    }
    uint32_t vout32 = static_cast<uint32_t>((mV - offset) * 8 / 100);  // This is the same as dividing by 12.5mv and then dividing by 1000 (mV => V), but avoids floating point calculation
    if (vout32 > 0xFF)
    {
        // Too high for the 8bit register
        return false;
    }
    uint8_t vout = static_cast<uint8_t>(vout32);
    switch (output)
    {
        case VOLTAGE_OUT2:
            m_registers.write(VOUT2, vout);
            return true;
        case VOLTAGE_OUT4:
            m_registers.write(VOUT4, vout);
            return true;
        default:
            // Can't set this voltage
            return false;
    }
}

bool PowerSupplyMax2043xPec::mapVoltageToReset(uint8_t voltage, bool active)
{
    if ((voltage & ~VOLTAGE_ALL) != 0)
    {
        // Invalid value, unsupported bit is set (only the input and output voltages are allowed)
        return false;
    }
    uint8_t regValue = m_registers.read(PINMAP1);
    if (active)
    {
        regValue |= voltage;
    }
    else
    {
        regValue &= ~voltage;
    }
    m_registers.write(PINMAP1, regValue);
    return true;
}
