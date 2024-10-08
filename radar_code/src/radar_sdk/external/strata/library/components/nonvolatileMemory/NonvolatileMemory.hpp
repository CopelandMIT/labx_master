/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <Definitions.hpp>
#include <components/interfaces/INonvolatileMemory.hpp>
#include <platform/Memory.hpp>


typedef struct
{
    uint32_t totalSize;
    uint32_t accessOffset;
    uint32_t pageSize;
    uint32_t segmentSize;
    uint32_t sectorSize;
} NonvolatileMemoryConfig_t;


class NonvolatileMemory :
    public INonvolatileMemory,
    private Memory<uint32_t, uint8_t>
{
public:
    STRATA_API NonvolatileMemory(const NonvolatileMemoryConfig_t &config, uint32_t maxTransfer);

    STRATA_API IMemory<uint32_t, uint8_t> *getIMemory() override;

    STRATA_API void readRandom(uint32_t address, uint32_t length, uint8_t buffer[]) override;
    STRATA_API void eraseAligned(uint32_t address, uint32_t length) override;
    STRATA_API void writeErased(uint32_t address, uint32_t length, const uint8_t buffer[]) override;
    STRATA_API void writeRandom(uint32_t address, uint32_t length, const uint8_t buffer[]) override;

private:
    // IMemory
    uint8_t read(uint32_t address) override;
    void read(uint32_t address, uint8_t &value) override;
    void read(uint32_t address, uint32_t length, uint8_t data[]) override;
    void write(uint32_t address, uint8_t value) override;
    void write(uint32_t address, uint32_t length, const uint8_t data[]) override;

private:
    void applyAccessOffset(uint32_t &address);
    void checkMemoryBoundaries(uint32_t address, uint32_t length);
    void readMemoryHelper(uint32_t address, uint32_t length, uint8_t *buffer);
    void readMemoryInterfaceHelper(uint32_t address, uint32_t length, uint8_t *buffer);
    void eraseMemoryHelper(uint32_t address, uint32_t length);

    virtual void readMemoryInterface(uint32_t address, uint32_t length, uint8_t *buffer)        = 0;
    virtual void writeMemoryInterface(uint32_t address, uint32_t length, const uint8_t *buffer) = 0;
    virtual void eraseMemoryInterface(uint32_t address)                                         = 0;

protected:
    const NonvolatileMemoryConfig_t m_config;
    uint32_t m_maxTransfer;
};
