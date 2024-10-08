/**
 * @copyright 2022 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

class ISupplyMonitor
{
public:
    virtual ~ISupplyMonitor() = default;

    //! This structure groups measurement values of a supply line.
    struct SupplyState
    {
        float voltage_V;  //!< The supply voltage in volts.
        float current_A;  //!< The supply current in amperes.
        float power_W;    //!< The supplied power in watts.
    };

    //! This method returns the supply voltage, current and power.
    virtual void getSupplyState(SupplyState &state) = 0;
};
