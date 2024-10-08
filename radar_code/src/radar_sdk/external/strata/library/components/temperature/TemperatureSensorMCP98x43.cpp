/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "TemperatureSensorMCP98x43.hpp"

namespace
{
    const uint8_t REGISTER_TEMPERATURE = 0x05;
}

TemperatureSensorMCP98x43::TemperatureSensorMCP98x43(II2c *access, uint16_t devAddr) :
    m_access {access},
    m_devAddr {devAddr}
{
}

TemperatureSensorMCP98x43::~TemperatureSensorMCP98x43()
{
}

float TemperatureSensorMCP98x43::getTemperature()
{
    uint8_t buf[2];
    m_access->readWith8BitPrefix(m_devAddr, REGISTER_TEMPERATURE, 2, buf);

    // The value is a 13-bit 2's-complement number, in the 13 least significant bits.
    auto fixedTemp = static_cast<int16_t>(((buf[0] & 0x1Fu) << 8) | buf[1]);
    // Now check if it was negative
    if (fixedTemp >= 0x1000)
    {
        fixedTemp -= 0x2000;
    }
    return 0.0625f * static_cast<float>(fixedTemp);
}
