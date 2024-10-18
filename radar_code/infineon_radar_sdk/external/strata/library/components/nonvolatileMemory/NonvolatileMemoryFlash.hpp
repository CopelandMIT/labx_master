/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <components/nonvolatileMemory/NonvolatileMemory.hpp>
#include <platform/interfaces/access/IFlash.hpp>

class NonvolatileMemoryFlash :
    public NonvolatileMemory
{
public:
    STRATA_API NonvolatileMemoryFlash(IFlash *access, uint8_t devId, const NonvolatileMemoryConfig_t &config);
    STRATA_API virtual ~NonvolatileMemoryFlash();

protected:
    void readMemoryInterface(uint32_t address, uint32_t length, uint8_t buffer[]) override;
    void writeMemoryInterface(uint32_t address, uint32_t length, const uint8_t buffer[]) override;
    void eraseMemoryInterface(uint32_t address) override;

private:
    void checkReady();
    void waitUntilIdle();

private:
    IFlash *m_access;
    const uint8_t m_devId;
    bool m_isReady;
};
