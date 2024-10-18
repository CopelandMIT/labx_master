/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include <common/Timing.hpp>
#include <components/exception/ENonvolatileMemory.hpp>
#include <components/nonvolatileMemory/NonvolatileMemoryFlash.hpp>


NonvolatileMemoryFlash::NonvolatileMemoryFlash(IFlash *access, uint8_t devId, const NonvolatileMemoryConfig_t &config) :
    NonvolatileMemory(config, access->getMaxTransfer()),
    m_access {access},
    m_devId {devId},
    m_isReady {false}
{
    if (m_maxTransfer < config.pageSize)
    {
        throw ENonvolatileMemory("Access supports writes only smaller than page size", m_maxTransfer);
    }
}

NonvolatileMemoryFlash::~NonvolatileMemoryFlash()
{
}

void NonvolatileMemoryFlash::waitUntilIdle()
{
    const auto timeout = std::chrono::milliseconds(1000);
    const auto step    = std::chrono::milliseconds(10);

    auto idle = [&] {
        return (m_access->getStatus(m_devId) & 0x01) == 0;
    };

    if (!waitFor(idle, timeout, step))
    {
        throw ENonvolatileMemory("Timeout waiting for Flash status");
    }
}


void NonvolatileMemoryFlash::checkReady()
{
    if (m_isReady)
    {
        return;
    }

    waitUntilIdle();
    m_isReady = true;
}

void NonvolatileMemoryFlash::readMemoryInterface(uint32_t address, uint32_t length, uint8_t buffer[])
{
    checkReady();
    m_access->read(m_devId, address, length, buffer);
}

void NonvolatileMemoryFlash::writeMemoryInterface(uint32_t address, uint32_t length, const uint8_t buffer[])
{
    checkReady();
    m_access->write(m_devId, address, length, buffer);
    waitUntilIdle();
}

void NonvolatileMemoryFlash::eraseMemoryInterface(uint32_t address)
{
    checkReady();
    m_access->erase(m_devId, address);
    waitUntilIdle();
}
