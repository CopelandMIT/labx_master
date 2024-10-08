/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <components/Registers.hpp>
#include <platform/interfaces/access/II2c.hpp>


class RegistersIrs :
    public Registers<uint16_t>
{
public:
    RegistersIrs(II2c *accessI2c, uint16_t devAddr);

    using Registers<uint16_t>::BatchType;

    using IRegisters<uint16_t>::read;
    using IRegisters<uint16_t>::write;
    uint16_t read();
    uint16_t read(uint16_t regAddr) override;
    void write(uint16_t regAddr, uint16_t value) override;

    void read(uint16_t regAddr, uint16_t count, uint16_t values[]) override;
    void read(uint16_t count, uint16_t values[]);
    void write(uint16_t regAddr, uint16_t count, const uint16_t values[]) override;

protected:
    void writeBatchImpl(const BatchType regVals[], uint16_t count) override;

private:
    II2c *m_accessI2c;
    const uint16_t m_devAddr;
    uint16_t m_maxCount;
};
