/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <components/nonvolatileMemory/NonvolatileMemoryFlash.hpp>
#include <platform/interfaces/access/IFlash.hpp>
#include <platform/interfaces/access/ISpi.hpp>


class FlashSpi :
    public IFlash
{
public:
    STRATA_API FlashSpi(ISpi *access);
    STRATA_API ~FlashSpi();

    STRATA_API uint32_t getMaxTransfer() const override;
    STRATA_API void read(uint8_t devId, uint32_t address, uint32_t length, uint8_t buffer[]) override;
    STRATA_API void write(uint8_t devId, uint32_t address, uint32_t length, const uint8_t buffer[]) override;
    STRATA_API void erase(uint8_t devId, uint32_t address) override;
    STRATA_API uint8_t getStatus(uint8_t devId) override;

private:
    inline void readCommand(uint8_t devId, uint8_t command, uint32_t *address, uint32_t length, uint8_t buffer[]);
    inline void writeCommand(uint8_t devId, uint8_t command, uint32_t *address = nullptr, uint32_t length = 0, const uint8_t buffer[] = nullptr);

    ISpi *m_access;
};


class NonvolatileMemoryFlashSpi :
    private FlashSpi,
    public NonvolatileMemoryFlash
{
public:
    STRATA_API NonvolatileMemoryFlashSpi(ISpi *access, uint8_t devId, uint32_t speed, const NonvolatileMemoryConfig_t &config);
    STRATA_API virtual ~NonvolatileMemoryFlashSpi();
};
