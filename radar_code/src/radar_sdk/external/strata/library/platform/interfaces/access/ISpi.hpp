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
#include <universal/spi_definitions.h>


/**
 * This is a generic interface for supporting arbitrary Serial Peripheral Interface
 * access to connected devices. Each instance of this class represents one SPI master.
 */
class ISpi
{
public:
    virtual ~ISpi() = default;
    /**
    * Returns the maximum accepted length of a buffer for a transmission,
    * which is specific to the implementation of the interface.
    */
    virtual uint32_t getMaxTransfer() const = 0;

    /**
    * Configure the clock speed for SPI transactions.
    * Optionally, specify a slave for which this setting is intended.
    *
    * @param devId (optional, only if supported) the device ID for which this setting is intended
    * @param flags specifies the configuration flags
    * @param wordSize specifies the word length in bits
    * @param speed specifies the clock speed in Hz
    */
    virtual void configure(uint8_t devId, uint8_t flags, uint8_t wordSize, uint32_t speed) = 0;

    /**
    * Reading data from an SPI device in half-duplex mode.
    *
    * @param devId the device ID that identifies the slave select signal to be used
    * @param count number of words to be read
    * @param buffer a buffer of the specified length
    * @param keepSel leave the slave-select line activated for another immediately following transaction
    */
    virtual void read(uint8_t devId, uint32_t count, uint8_t buffer[], bool keepSel = false)  = 0;
    virtual void read(uint8_t devId, uint32_t count, uint16_t buffer[], bool keepSel = false) = 0;
    virtual void read(uint8_t devId, uint32_t count, uint32_t buffer[], bool keepSel = false) = 0;

    /**
    * Write data to an SPI device in half-duplex mode.
    * The slave select line can be kept active to follow the write with a read to form a single transmission.
    *
    * @param devId the device ID that identifies the slave select signal to be used
    * @param count number of words to be written
    * @param buffer a buffer of the specified length
    * @param keepSel leave the slave-select line activated for another immediately following transaction
    */
    virtual void write(uint8_t devId, uint32_t count, const uint8_t buffer[], bool keepSel = false)  = 0;
    virtual void write(uint8_t devId, uint32_t count, const uint16_t buffer[], bool keepSel = false) = 0;
    virtual void write(uint8_t devId, uint32_t count, const uint32_t buffer[], bool keepSel = false) = 0;

    /**
    * Read and write data from / to an SPI device in full-duplex mode.
    * The slave select line can be kept active to follow the write with a read to form a single transmission.
    *
    * @param devId the device ID that identifies the slave select signal to be used
    * @param count number of words to be read / written
    * @param bufWrite a buffer of the specified length with the data to write
    * @param bufRead a buffer of the specified length where the read data will be written to
    * @param keepSel leave the slave-select line activated for another immediately following transaction
    */
    virtual void transfer(uint8_t devId, uint32_t count, const uint8_t bufWrite[], uint8_t bufRead[], bool keepSel = false)   = 0;
    virtual void transfer(uint8_t devId, uint32_t count, const uint16_t bufWrite[], uint16_t bufRead[], bool keepSel = false) = 0;
    virtual void transfer(uint8_t devId, uint32_t count, const uint32_t bufWrite[], uint32_t bufRead[], bool keepSel = false) = 0;
};
