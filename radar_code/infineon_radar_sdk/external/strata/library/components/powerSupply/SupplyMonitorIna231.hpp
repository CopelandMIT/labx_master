/**
 * @copyright 2021 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <components/interfaces/ISupplyMonitor.hpp>
#include <platform/interfaces/access/II2c.hpp>


/**
 * This class represents an INA231 or INA226 voltage and current sensor.
 */
class SupplyMonitorIna231 : public ISupplyMonitor
{
public:
    /**
     * The constructor takes the initializes the supply sensor and starts
     * continuous measurements in the background.
     *
     * \param[in] access   The I2C interface object, the sensor is available
     *                     through.
     * \param[in] devAddr  The I2C device address of the sensor.
     * \param[in] shunt    The value of the shunt resistor used for current
     *                     measurement in Ohms.
     */
    SupplyMonitorIna231(II2c *access, uint16_t devAddr, float shunt);

    /**
     * This method just reads the most recently measured supply state from
     * the sensor.
     */
    void getSupplyState(SupplyState &state) override;

private:
    II2c *m_access;
    const uint16_t m_devAddr;

    // This is the reciprocal shunt resistor value.
    float m_shunt_inv;
};
