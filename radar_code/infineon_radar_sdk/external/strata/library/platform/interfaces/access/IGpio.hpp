/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <cstdint>
#include <universal/gpio_definitions.h>


/**
 * This is a generic interface for supporting access to GPIOs.
 * They can be identified either by a number, which is hardware-specific,
 * or by a predefined name that is independent of the hardware used,
 * as long as it is defined in the given bridge implementation.
 */
class IGpio
{
public:
    virtual ~IGpio() = default;

    /**
     * Configure a GPIO pin for pull-up/pull-down, drive-high, drive-low and/or input/output
     * @param id either a generic number or a predefined name
     * @param flags bit-wise setting of features
     */
    virtual void configurePin(uint16_t id, uint8_t flags) = 0;

    /**
     * Set a GPIO pin to logic high or low.
     * @param id either a generic number or a predefined name
     * @param state true for high and false for low
     */
    virtual void setPin(uint16_t id, bool state) = 0;

    /**
     * Get the current logic level of a GPIO pin (which should have been configured for input)
     * @param id either a generic number or a predefined name
     * @return true for high and false for low
     */
    virtual bool getPin(uint16_t id) = 0;

    /**
     * Configure specific pins of a GPIO port for pull-up/pull-down, drive-high, drive-low and/or input/output
     * @param port number of the port to configure
     * @param flags bit-wise setting of features for all pins to configure
     * @mask defines the pins to configure, 1 means configure, 0 means don't change
     */
    virtual void configurePort(uint16_t port, uint8_t flags, uint32_t mask = 0xFFFFFFFF) = 0;

    /**
     * Set specific pins of a GPIO port to logic high or low.
     * @param port number of the port to write to
     * @param state defines the states to set for the single pins, 1 bit corresponds to 1 GPIO pin
     * @mask defines the pins to change, 1 means set as defined in state, 0 means don't change
     */
    virtual void setPort(uint16_t port, uint32_t state, uint32_t mask = 0xFFFFFFFF) = 0;

    /**
     * Get the current logic levels of the single pins of a GPIO port (which should have been configured as inputs)
     * @param port number of the port to read
     * @return value containing the single GPIO pins as bits, 1 for high, 0 for low
     */
    virtual uint32_t getPort(uint16_t port) = 0;
};
