/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "RegistersIrs.hpp"

#include <common/Buffer.hpp>
#include <common/EndianConversion.hpp>
#include <components/exception/ERegisters.hpp>


namespace
{
    const uint16_t increment = 1;
}

RegistersIrs::RegistersIrs(II2c *accessI2c, uint16_t devAddr) :
    Registers<uint16_t>(increment),
    m_accessI2c {accessI2c},
    m_devAddr {devAddr}
{
    m_maxCount = m_accessI2c->getMaxTransfer() / sizeof(uint16_t);
    if (m_maxCount == 0)
    {
        throw ERegisters("MaxTransfer too small, won't be able to transfer any registers", m_maxCount);
    }
}

uint16_t RegistersIrs::read(uint16_t regAddr)
{
    uint16_t value;
    m_accessI2c->readWith16BitPrefix(m_devAddr, regAddr, sizeof(value), reinterpret_cast<uint8_t *>(&value));
    return bigToHost(value);
}

uint16_t RegistersIrs::read()
{
    uint16_t value;
    m_accessI2c->readWithoutPrefix(m_devAddr, sizeof(value), reinterpret_cast<uint8_t *>(&value));
    return bigToHost(value);
}

void RegistersIrs::write(uint16_t regAddr, uint16_t value)
{
    value = hostToBig(value);
    m_accessI2c->writeWith16BitPrefix(m_devAddr, regAddr, sizeof(value), reinterpret_cast<uint8_t *>(&value));
}

void RegistersIrs::read(uint16_t count, uint16_t values[])
{
    uint16_t *valuesBegin     = values;
    const uint16_t lastChunk  = count % m_maxCount;
    const uint16_t firstChunk = count - lastChunk;

    const auto firstChunkEnd = values + firstChunk;
    while (values < firstChunkEnd)
    {
        m_accessI2c->readWithoutPrefix(m_devAddr, m_maxCount * sizeof(*values), reinterpret_cast<uint8_t *>(values));
        values += m_maxCount;
    }

    if (lastChunk)
    {
        m_accessI2c->readWithoutPrefix(m_devAddr, lastChunk * sizeof(*values), reinterpret_cast<uint8_t *>(values));
    }

    bigToHost(valuesBegin, valuesBegin + count);
}

void RegistersIrs::read(uint16_t regAddr, uint16_t count, uint16_t values[])
{
    uint16_t *valuesBegin     = values;
    const uint16_t lastChunk  = count % m_maxCount;
    const uint16_t firstChunk = count - lastChunk;

    const auto firstChunkEnd = values + firstChunk;
    while (values < firstChunkEnd)
    {
        m_accessI2c->readWith16BitPrefix(m_devAddr, regAddr, m_maxCount * sizeof(*values), reinterpret_cast<uint8_t *>(values));
        regAddr += m_maxCount;
        values += m_maxCount;
    }

    if (lastChunk)
    {
        m_accessI2c->readWith16BitPrefix(m_devAddr, regAddr, lastChunk * sizeof(*values), reinterpret_cast<uint8_t *>(values));
    }

    bigToHost(valuesBegin, valuesBegin + count);
}

void RegistersIrs::write(uint16_t regAddr, uint16_t count, const uint16_t values[])
{
    const uint16_t lastChunk  = count % m_maxCount;
    const uint16_t firstChunk = count - lastChunk;

    // to optimize for big-endian systems where a conversion is not necessary, remove the intermediate buffer
    strata::buffer<uint16_t> buf(firstChunk ? m_maxCount : lastChunk);

    const auto firstChunkEnd = values + firstChunk;
    while (values < firstChunkEnd)
    {
        hostToBig(values, values + m_maxCount, buf.data());
        m_accessI2c->writeWith16BitPrefix(m_devAddr, regAddr, m_maxCount * sizeof(*values), reinterpret_cast<uint8_t *>(buf.data()));
        regAddr += m_maxCount;
        values += m_maxCount;
    }

    if (lastChunk)
    {
        hostToBig(values, values + lastChunk, buf.data());
        m_accessI2c->writeWith16BitPrefix(m_devAddr, regAddr, lastChunk * sizeof(*values), reinterpret_cast<uint8_t *>(buf.data()));
    }
}

void RegistersIrs::writeBatchImpl(const BatchType regVals[], uint16_t count)
{
    strata::buffer<uint16_t> buffer(std::min<size_t>(count, m_maxCount));

    const auto last = regVals + count;
    while (regVals < last)
    {
        auto buf           = buffer.data();
        const auto regAddr = regVals->address;
        uint16_t regCount  = 0;
        uint16_t nextAddr  = regAddr;
        do
        {
            *buf++ = hostToBig(regVals++->value);
            regCount++;
            nextAddr += m_increment;
        } while ((regVals != last) &&
                 (regCount != m_maxCount) &&
                 (regVals->address == nextAddr));
        m_accessI2c->writeWith16BitPrefix(m_devAddr, regAddr, regCount * sizeof(uint16_t), reinterpret_cast<const uint8_t *>(buffer.data()));
    }
}
