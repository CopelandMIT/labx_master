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
#include <universal/i2c_definitions.h>


/**
 * This is a generic interface for an arbitrary I2C Master implementation
 * to access connected devices.
 */
class II2c
{
public:
    virtual ~II2c() = default;

    /**
    * Returns the maximum accepted length of a buffer for a transmission,
    * which is specific to the implementation of the interface.
    */
    virtual uint16_t getMaxTransfer() const = 0;

    /**
     * Read register(s) from an I2C device.
     *
     * @param devAddr the device address (incl. busId)
     * @param length number of bytes to be read
     * @param buffer a buffer of the specified length
     */
    virtual void readWithoutPrefix(uint16_t devAddr, uint16_t length, uint8_t buffer[])                    = 0;
    virtual void readWith8BitPrefix(uint16_t devAddr, uint8_t prefix, uint16_t length, uint8_t buffer[])   = 0;
    virtual void readWith16BitPrefix(uint16_t devAddr, uint16_t prefix, uint16_t length, uint8_t buffer[]) = 0;

    /**
     * Write register(s) to an I2C device.
     *
     * @param devAddr the device address (incl. busId)
     * @param length number of bytes to be written
     * @param buffer a buffer of the specified length
     */
    virtual void writeWithoutPrefix(uint16_t devAddr, uint16_t length, const uint8_t buffer[])                    = 0;
    virtual void writeWith8BitPrefix(uint16_t devAddr, uint8_t prefix, uint16_t length, const uint8_t buffer[])   = 0;
    virtual void writeWith16BitPrefix(uint16_t devAddr, uint16_t prefix, uint16_t length, const uint8_t buffer[]) = 0;

    /**
     * Change the speed of an I2C bus.
     * This is generic for the whole bus, so all devices connected to it must support it.
     *
     * @param devAddr specifies the bus to configure by the busId contained in devAddr
     * @param speed specifies the bus speed in Hz
     */
    virtual void configureBusSpeed(uint16_t devAddr, uint32_t speed) = 0;

    /**
    * Clears an I2C bus.
    *
    * @param devAddr specifies the bus to reset by the busId contained in devAddr
    */
    virtual void clearBus(uint16_t devAddr) = 0;

    /**
    * Waits for an ACK from an I2C device.
    *
    * @param devAddr the device address  (incl. busId)
    */
    virtual void pollForAck(uint16_t devAddr) = 0;
};
