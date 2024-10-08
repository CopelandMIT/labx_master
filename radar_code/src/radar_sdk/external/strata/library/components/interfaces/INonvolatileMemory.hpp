/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <platform/interfaces/access/IMemory.hpp>


/**
 * Interface to an arbitrary memory.
 * If the actual hardware only supports paged access, the implementation takes care of this.
 */
class INonvolatileMemory
{
public:
    virtual ~INonvolatileMemory() = default;

    ///
    /// Get an IMemory interface for a non-volatile memory
    /// \details This interface hides the complexity of random access for a non-volatile memory.
    ///          If it is used e.g. for byte-wise writing, the life-time of the memory will be
    ///          drastically reduced.
    ///          Therefore only use with appropriate care!
    ///
    virtual IMemory<uint32_t, uint8_t> *getIMemory() = 0;

    virtual void readRandom(uint32_t address, uint32_t length, uint8_t buffer[])        = 0;
    virtual void eraseAligned(uint32_t address, uint32_t length)                        = 0;
    virtual void writeErased(uint32_t address, uint32_t length, const uint8_t buffer[]) = 0;
    virtual void writeRandom(uint32_t address, uint32_t length, const uint8_t buffer[]) = 0;
};
