/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <components/interfaces/ITemperatureSensor.hpp>
#include <platform/interfaces/access/II2c.hpp>


/**
 * MCP9843 and MCP98243 sensors.
 */
class TemperatureSensorMCP98x43 : public ITemperatureSensor
{
public:
    TemperatureSensorMCP98x43(II2c *access, uint16_t devAddr);
    ~TemperatureSensorMCP98x43();

    float getTemperature() override;

private:
    II2c *m_access;
    const uint16_t m_devAddr;
};
