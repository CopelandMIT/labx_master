/**
 * @copyright 2021 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "SupplyMonitorIna231.hpp"
#include <cstdint>

SupplyMonitorIna231::SupplyMonitorIna231(II2c *access, uint16_t devAddr, float shunt) :
    m_access(access),
    m_devAddr(devAddr),
    m_shunt_inv(1.0f / shunt)
{
    // This is the configuration word to setup an INA231 sensor.
    uint16_t configWord = 0x0400 |  // AVG = 010b -> average over 16 values
                          0x01C0 |  // VBUSCT=111, 8.344ms Vbus conversion
                          0x0031 |  // VSHCT=111, 8.344ms Vshunt conversion
                          0x0007;   // MODE = 111b -> continuous mode

    /*
     * The configuration word defined above is written to register 1 of the
     * sensor to let it continuously measure the supply state.
     */
    uint8_t i2cSequence[3];
    i2cSequence[0] = 1;
    i2cSequence[1] = configWord >> 8;
    i2cSequence[2] = configWord & 0xFF;
    m_access->writeWithoutPrefix(m_devAddr, 3, i2cSequence);
}

void SupplyMonitorIna231::getSupplyState(SupplyState &state)
{
    // Values are read from current sensor.
    auto readRegister = [&](uint8_t register_number) -> uint16_t {
        uint8_t value[2];
        m_access->writeWithoutPrefix(m_devAddr, 1, &register_number);
        m_access->readWithoutPrefix(m_devAddr, 2, value);
        return (uint16_t(value[0]) << 8) | value[1];
    };

    /*
     * The value in the sensor's register is actually a signed voltage in
     * 2.5 micro volts. To turn it into a current, the voltage must be
     * divided by the shunt resistor (Ohm's law).
     */
    state.current_A = static_cast<int16_t>(readRegister(1)) * 2.5e-6f;
    state.current_A *= m_shunt_inv;

    // Bus voltage LSB is 1.25mV.
    state.voltage_V = readRegister(2) * 1.25e-3f;

    /*
     * Even though the current sensor can also measure the power, it is easier
     * to calculate it from measured voltage and current.
     */
    state.power_W = state.voltage_V * state.current_A;
}
