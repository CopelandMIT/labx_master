/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "TemperatureSensorTMP102.hpp"

TemperatureSensorTMP102::TemperatureSensorTMP102(II2c *access, uint16_t devAddr) :
    m_access {access},
    m_devAddr {devAddr}
{
}

TemperatureSensorTMP102::~TemperatureSensorTMP102()
{
}

float TemperatureSensorTMP102::getTemperature()
{
    uint8_t buf[2];
    m_access->readWithoutPrefix(m_devAddr, 2, buf);

    // The value is a 12-bit 2's-complement number, in the 12 most significant bits.
    auto fixedTemp = static_cast<int16_t>((buf[0] << 4) | (buf[1] >> 4));
    // Now check if it was negative
    if (fixedTemp >= 0x800)
    {
        fixedTemp -= 0x1000;
    }
    return 0.0625f * static_cast<float>(fixedTemp);
}
