/**
 * @copyright 2022 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <components/interfaces/IComponent.hpp>
#include <components/interfaces/IRegisters.hpp>
#include <universal/components/power_supply.h>

///
/// Interface for Max20430Pec power supply
///
class IPowerSupplyMax2043x :
    public IComponent
{
public:
    static constexpr uint16_t getType()
    {
        return COMPONENT_TYPE_POWER_SUPPLY_MAX2043X;
    }

    virtual ~IPowerSupplyMax2043x() = default;

    ///
    /// \brief Get the registers interface of the power supply component
    /// \retval The registers interface
    ///
    virtual IRegisters<uint8_t> *getIRegisters() = 0;

    // Voltage definitions, to be used as function parameters
    static constexpr uint8_t VOLTAGE_OUT1    = 0x01;
    static constexpr uint8_t VOLTAGE_OUT2    = 0x02;
    static constexpr uint8_t VOLTAGE_OUT3    = 0x04;
    static constexpr uint8_t VOLTAGE_OUT4    = 0x08;
    static constexpr uint8_t VOLTAGE_IN5     = 0x10;
    static constexpr uint8_t VOLTAGE_IN6     = 0x20;
    static constexpr uint8_t VOLTAGE_OUT_ALL = VOLTAGE_OUT1 | VOLTAGE_OUT2 | VOLTAGE_OUT3 | VOLTAGE_OUT4;
    static constexpr uint8_t VOLTAGE_IN_ALL  = VOLTAGE_IN5 | VOLTAGE_IN6;
    static constexpr uint8_t VOLTAGE_ALL     = VOLTAGE_OUT_ALL | VOLTAGE_IN_ALL;

    ///
    /// \brief Switch voltage outputs on or off
    /// \param output Defines which output to switch, the others will be left unchanged. Using the VOLTAGE_x defines, multiple outputs can be combined (|) for one call.
    /// \param enable True to switch the voltage on, false to switch it off
    /// \retval true on success
    ///
    virtual bool switchVoltageOutput(uint8_t output, bool enable) = 0;

    ///
    /// \brief Set the voltage for voltage outputs
    /// \note The output is not switched on automatically, if it is off. Just the voltage value is changed.
    /// \param output Defines the output to change the voltage for. Use the VOLTAGE_x defines. Only one output can be changed at a time.
    /// \param mV The voltage value to set in millivolts
    /// \retval true on success
    ///
    virtual bool setVoltageOutput(uint8_t output, uint16_t mV) = 0;

    ///
    /// \brief Map a voltage input or output to the reset output, meaning that the reset output is set if the voltage is not as specified.
    /// \param voltage Defines which voltage to map, the others will be left unchanged. Using the VOLTAGE_x defines, multiple voltages can be combined (|) for one call.
    /// \param active True to map the voltage to reset, false to unmap it
    /// \retval true on success
    ///
    virtual bool mapVoltageToReset(uint8_t voltage, bool active) = 0;
};
