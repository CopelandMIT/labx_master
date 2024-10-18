/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include <components/exception/ENonvolatileMemory.hpp>
#include <components/nonvolatileMemory/NonvolatileMemory.hpp>

#include <common/Buffer.hpp>
#include <cstring>


NonvolatileMemory::NonvolatileMemory(const NonvolatileMemoryConfig_t &config, uint32_t maxTransfer) :
    Memory<uint32_t, uint8_t>(1),
    m_config(config),
    m_maxTransfer {maxTransfer}
{
    if (m_config.pageSize == 0)
    {
        throw ENonvolatileMemory("Page size set to zero");
    }

    if (m_config.sectorSize)
    {
        if (m_config.sectorSize % m_config.pageSize)
        {
            throw ENonvolatileMemory("Sector size is not a multiple of page size");
        }

        if (m_config.totalSize % m_config.sectorSize)
        {
            throw ENonvolatileMemory("Total size is not a multiple of sector size");
        }
    }

    if (m_config.accessOffset > m_config.totalSize)
    {
        throw ENonvolatileMemory("Access Offset is greater than total size");
    }
}

IMemory<uint32_t, uint8_t> *NonvolatileMemory::getIMemory()
{
    return this;
}

uint8_t NonvolatileMemory::read(uint32_t address)
{
    uint8_t value;
    readRandom(address, sizeof(value), &value);
    return value;
}

void NonvolatileMemory::read(uint32_t address, uint8_t &value)
{
    readRandom(address, sizeof(value), &value);
}

void NonvolatileMemory::read(uint32_t address, uint32_t length, uint8_t data[])
{
    readRandom(address, length, data);
}

void NonvolatileMemory::write(uint32_t address, uint8_t value)
{
    writeRandom(address, sizeof(value), &value);
}

void NonvolatileMemory::write(uint32_t address, uint32_t length, const uint8_t data[])
{
    writeRandom(address, length, data);
}

void NonvolatileMemory::applyAccessOffset(uint32_t &address)
{
    if (address + m_config.accessOffset < m_config.accessOffset)
    {
        throw ENonvolatileMemory("Overflow of sum of address and access offset", static_cast<int>(address));
    }

    address += m_config.accessOffset;
}

void NonvolatileMemory::checkMemoryBoundaries(uint32_t address, uint32_t length)
{
    if (address + length < address)
    {
        throw ENonvolatileMemory("Overflow of sum of address and length", static_cast<int>(address));
    }

    if (address + length > m_config.totalSize)
    {
        throw ENonvolatileMemory("Memory addressed beyond total size", static_cast<int>(address));
    }
}

void NonvolatileMemory::readMemoryHelper(uint32_t address, uint32_t length, uint8_t *buffer)
{
    if (!m_config.segmentSize)
    {
        readMemoryInterfaceHelper(address, length, buffer);
    }
    else
    {
        uint32_t firstChunkSize;
        const uint32_t firstChunkAlignment = address % m_config.segmentSize;
        if (firstChunkAlignment)
        {
            firstChunkSize = m_config.segmentSize - firstChunkAlignment;
            if (firstChunkSize > length)
            {
                firstChunkSize = length;
            }
        }
        else
        {
            firstChunkSize = 0;
        }

        if (firstChunkSize)
        {
            readMemoryInterfaceHelper(address, firstChunkSize, buffer);
        }
        const uint32_t lastChunkSize = length - firstChunkSize;  // clang-tidy gets confused if this line is not directly in front of the if-condition
        if (lastChunkSize)
        {
            readMemoryInterfaceHelper(address + firstChunkSize, lastChunkSize, buffer + firstChunkSize);
        }
    }
}

void NonvolatileMemory::readMemoryInterfaceHelper(uint32_t address, uint32_t length, uint8_t *buffer)
{
    const uint32_t lastChunk  = length % m_maxTransfer;
    const uint32_t firstChunk = length - lastChunk;

    for (uint_fast32_t i = 0; i < firstChunk; i += m_maxTransfer)
    {
        readMemoryInterface(address + i, m_maxTransfer, buffer + i);
    }
    if (lastChunk)
    {
        readMemoryInterface(address + firstChunk, lastChunk, buffer + firstChunk);
    }
}

void NonvolatileMemory::eraseMemoryHelper(uint32_t address, uint32_t length)
{
    for (uint_fast32_t i = 0; i < length; i += m_config.sectorSize)
    {
        eraseMemoryInterface(address + i);
    }
}

void NonvolatileMemory::readRandom(uint32_t address, uint32_t length, uint8_t buffer[])
{
    applyAccessOffset(address);
    checkMemoryBoundaries(address, length);

    readMemoryHelper(address, length, buffer);
}

void NonvolatileMemory::eraseAligned(uint32_t address, uint32_t length)
{
    applyAccessOffset(address);
    checkMemoryBoundaries(address, length);

    if ((address % m_config.sectorSize) || (length % m_config.sectorSize))
    {
        throw ENonvolatileMemory("Parameters for eraseMemory() are not sector-aligned");
    }

    eraseMemoryHelper(address, length);
}

void NonvolatileMemory::writeErased(uint32_t address, uint32_t length, const uint8_t buffer[])
{
    applyAccessOffset(address);
    checkMemoryBoundaries(address, length);

    uint32_t firstChunkSize        = 0;
    const uint32_t chunkBeforeSize = address % m_config.pageSize;
    if (chunkBeforeSize)
    {
        firstChunkSize = m_config.pageSize - chunkBeforeSize;
        if (firstChunkSize > length)
        {
            firstChunkSize = length;
        }
    }
    const uint32_t lastChunkSize   = (length - firstChunkSize) % m_config.pageSize;
    const uint32_t middleChunkSize = length - firstChunkSize - lastChunkSize;

    if (firstChunkSize)
    {
        writeMemoryInterface(address, firstChunkSize, buffer);
    }
    if (middleChunkSize)
    {
        for (uint_fast32_t i = 0; i < middleChunkSize; i += m_config.pageSize)
        {
            writeMemoryInterface(address + firstChunkSize + i, m_config.pageSize, buffer + firstChunkSize + i);
        }
    }
    if (lastChunkSize)
    {
        writeMemoryInterface(address + firstChunkSize + middleChunkSize, lastChunkSize, buffer + firstChunkSize + middleChunkSize);
    }
}

void NonvolatileMemory::writeRandom(uint32_t address, uint32_t length, const uint8_t buffer[])
{
    if (!m_config.sectorSize)
    {
        writeErased(address, length, buffer);
    }
    else
    {
        applyAccessOffset(address);
        checkMemoryBoundaries(address, length);

        const uint32_t chunkBeforeSize = address % m_config.sectorSize;
        const uint32_t chunkAfterSize  = (m_config.sectorSize - address - length) % m_config.sectorSize;
        strata::buffer<uint8_t> block;

        // if the data is not sector aligned, read back full sector before and after and
        // update the parameters to write this aligned new data
        if (chunkBeforeSize || chunkAfterSize)
        {
            const uint32_t sectorStart = address - chunkBeforeSize;
            const uint32_t blockLength = chunkBeforeSize + length + chunkAfterSize;

            block.resize(blockLength);

            if (chunkBeforeSize)
            {
                readMemoryHelper(sectorStart, chunkBeforeSize, &block[0]);
            }

            std::memcpy(&block[chunkBeforeSize], buffer, length);

            if (chunkAfterSize)
            {
                readMemoryHelper(address + length, chunkAfterSize, &block[chunkBeforeSize + length]);
            }

            address = sectorStart;
            length  = blockLength;
            buffer  = &block[0];
        }

        eraseMemoryHelper(address, length);

        for (uint_fast32_t i = 0; i < length; i += m_config.pageSize)
        {
            writeMemoryInterface(address + i, m_config.pageSize, buffer + i);
        }
    }
}
