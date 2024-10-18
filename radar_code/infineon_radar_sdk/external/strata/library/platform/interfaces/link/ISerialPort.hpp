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


/**
 * This is a generic interface for a serial port connection.
 * Each instance of this class represents one connection.
 */
class ISerialPort
{
public:
    virtual ~ISerialPort() = default;

    virtual bool isOpened()                                                   = 0;
    virtual void open(const char port[], uint32_t baudrate, uint16_t timeout) = 0;
    virtual void close()                                                      = 0;

    /**
    * Set timeout for read function call
    *
    * @param timeout in milliseconds time before returning from call to read if no data is received in the mean time
    */
    virtual void setTimeout(uint16_t timeout) = 0;

    /**
    * Clear data available to read from remote device.
    */
    virtual void clearInputBuffer() = 0;

    /**
    * Send a buffer to the remote device.
    *
    * @param buffer a buffer of the specified length
    * @param length number of bytes to be written
    */
    virtual void send(const uint8_t buffer[], uint16_t length) = 0;

    /**
    * Receive data from the remote device.
    * The function tries to read up to length bytes within the set timeout.
    *
    * @param buffer a buffer of the specified maximum length
    * @param length maximum number of bytes to be read (size of buffer)
    *
    * @return the actual number of bytes received
    */
    virtual uint16_t receive(uint8_t buffer[], uint16_t length) = 0;
};
