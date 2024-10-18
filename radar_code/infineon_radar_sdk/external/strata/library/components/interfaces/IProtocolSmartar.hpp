/**
 * @copyright 2020 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <cstddef>
#include <cstdint>


class IProtocolSmartar
{
public:
    using CommandType    = uint16_t[2];
    using WriteValueType = uint16_t[2];
    using BatchWriteType = uint16_t[2][2];

    constexpr static uint16_t writeBit = (1u << 0);

    struct Command
    {
        constexpr const CommandType &value() const
        {
            return m_command;
        }

        constexpr operator const CommandType &() const
        {
            return value();
        }

        Command() = delete;

        inline void increment(uint16_t count)
        {
            uint32_t prefixAndAddress = m_command[0] | (m_command[1] << 16);
            prefixAndAddress += ((count * sizeof(CommandType)) << 8);
            m_command[0] = prefixAndAddress & 0xFFFF;
            m_command[1] = prefixAndAddress >> 16;
        }

    protected:
        constexpr Command(uint16_t commandPrefix, uint16_t address) :
            m_command {static_cast<uint16_t>(commandPrefix | ((address & 0xFF) << 8)), static_cast<uint16_t>(address >> 8)}
        {}

    private:
        CommandType m_command;
    };


    struct Read :
        public Command
    {
        constexpr Read(uint16_t address) :
            Command(0, address)
        {}
    };

    struct Write :
        public Command
    {
        constexpr Write(uint16_t address) :
            Command(writeBit, address)
        {}
    };

    struct WriteValue
    {
        constexpr const WriteValueType &value() const
        {
            return m_value;
        }

        constexpr operator const WriteValueType &() const
        {
            return value();
        }

        WriteValue() = delete;

        constexpr WriteValue(uint32_t value) :
            m_value {static_cast<uint16_t>(value & 0xFFFF),
                     static_cast<uint16_t>(value >> 16)}
        {}

    private:
        const WriteValueType m_value;
    };

    struct BatchWrite
    {
        constexpr const BatchWriteType &value() const
        {
            return m_batch;
        }

        constexpr operator const BatchWriteType &() const
        {
            return value();
        }

        constexpr BatchWrite(uint16_t address, uint32_t value) :
            m_batch {{Write(address)[0], Write(address)[1]},
                     {WriteValue(value)[0], WriteValue(value)[1]}}
        {
        }
        BatchWrite() = default;

    private:
        BatchWriteType m_batch;
    };


    /**
    * Executes a read command.
    */
    virtual void executeRead(const Read &command, uint16_t count, uint32_t values[]) = 0;

    /**
    * Executes a write command.
    */
    virtual void executeWrite(const Write &command, uint16_t count, const WriteValue values[]) = 0;

    /**
    * Executes a 16 bit write command.
    */
    virtual void executeWrite16(const Write &command, uint16_t count, const uint16_t values[]) = 0;

    /**
    * Sets a mask of bits at a given address.
    */
    virtual void setBits(uint16_t address, uint32_t bitMask) = 0;

    /**
    * Executes a write batch command.
    */
    virtual void executeWriteBatch(const BatchWrite commands[], uint16_t count) = 0;


    inline void executeRead(const Read &command, uint32_t &value)
    {
        executeRead(command, 1, &value);
    }

    inline void executeWrite(const Write &command, const WriteValue &value)
    {
        executeWrite(command, 1, &value);
    }

    inline uint32_t executeRead(const Read &command)
    {
        uint32_t value;
        executeRead(command, value);
        return value;
    }
};
