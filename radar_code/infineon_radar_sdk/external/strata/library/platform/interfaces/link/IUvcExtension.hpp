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


class IUvcExtension
{
public:
    virtual ~IUvcExtension() = default;

    virtual bool variableSizeSupport() const = 0;

    virtual void setProperty(uint8_t id, uint16_t length, const uint8_t buffer[]) = 0;
    virtual void getProperty(uint8_t id, uint16_t length, uint8_t buffer[])       = 0;

    // Functions to fulfill the "BasicLockable" requirement (implement lock() and unlock() methods)
    // which enables usage with std::lock_guard
    ///
    /// \brief Blocks until a lock can be obtained
    ///
    virtual void lock() = 0;
    ///
    /// \brief Releases the lock
    ///
    virtual void unlock() = 0;
};
