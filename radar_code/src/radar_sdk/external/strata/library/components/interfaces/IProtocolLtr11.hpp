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


class IProtocolLtr11
{
public:
    using WriteType = uint8_t[3];
    using ReadType  = uint8_t;

    using BurstType      = uint8_t[2];
    using WriteValueType = uint8_t[2];

    constexpr static unsigned int valueWidth    = 16;
    constexpr static unsigned int adressWidth   = 7;
    constexpr static unsigned int addressOffset = 1;

    constexpr static uint8_t burstPrefix = 0xFF;
    constexpr static uint8_t writeBit    = (1u << 0);


    struct Write
    {
        constexpr const WriteType &value() const
        {
            return m_command;
        }

        constexpr operator const WriteType &() const
        {
            return value();
        }

        Write() = default;

        constexpr Write(uint8_t address, uint16_t value) :
            m_command {static_cast<uint8_t>((address << 1) | writeBit), static_cast<uint8_t>(value >> 8), static_cast<uint8_t>(value & 0xFF)}
        {}

    private:
        WriteType m_command;
    };


    struct Read
    {
        constexpr const ReadType &value() const
        {
            return m_command;
        }

        constexpr operator const ReadType &() const
        {
            return value();
        }

        Read() = default;

        constexpr Read(uint8_t address) :
            m_command {static_cast<uint8_t>(address << 1)}
        {}

    private:
        ReadType m_command;
    };


    struct BurstCommand
    {
        constexpr const BurstType &value() const
        {
            return m_command;
        }

        constexpr operator const BurstType &() const
        {
            return value();
        }

        BurstCommand() = delete;

        inline void increment(uint16_t count)
        {
            m_command[1] += (static_cast<uint8_t>(count << addressOffset));
        }

    protected:
        constexpr BurstCommand(uint8_t command) :
            m_command {burstPrefix, command}
        {}

    private:
        BurstType m_command;
    };


    struct WriteBurst :
        public BurstCommand
    {
        constexpr WriteBurst(uint8_t address) :
            BurstCommand(static_cast<uint8_t>((address << addressOffset) | writeBit))
        {}
    };


    struct ReadBurst :
        public BurstCommand
    {
        constexpr ReadBurst(uint8_t address) :
            BurstCommand(static_cast<uint8_t>(address << addressOffset))
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

        constexpr WriteValue(uint16_t value) :
            m_value {static_cast<uint8_t>(value >> 8), static_cast<uint8_t>(value & 0xFF)}
        {}

    private:
        const WriteValueType m_value;
    };
    static_assert(sizeof(WriteValue) == sizeof(WriteValueType), "Memory layout of WriteValue contains additional stuff");

    /**
    * Executes a single write command.
    */
    virtual void executeWrite(const Write &command) = 0;

    /**
    * Executes a single read command.
    */
    virtual void executeRead(const Read &command, uint16_t &value) = 0;

    /**
    * Sets a mask of bits at a given address.
    */
    virtual void setBits(uint8_t address, uint16_t bitMask) = 0;

    /**
    * Executes a batch of write commands.
    */
    virtual void executeWriteBatch(const Write commands[], uint16_t count) = 0;

    /**
    * Executes a burst write command.
    */
    virtual void executeWriteBurst(const WriteBurst &command, uint16_t count, const WriteValue values[]) = 0;

    /**
    * Executes a burst read command.
    */
    virtual void executeReadBurst(const ReadBurst &command, uint16_t count, uint16_t values[]) = 0;

    /**
    * Enables the Miso Arbitration if the given Pulse Repetition Time (PRT) is different than 0, and disables it if prt is equal to 0.
    */
    virtual void setMisoArbitration(uint16_t prt) = 0;


    template <std::size_t N>
    inline void executeWriteBatch(const Write (&commands)[N])
    {
        executeWriteBatch(commands, N);
    }

    inline uint16_t executeRead(const Read &command)
    {
        uint16_t value;
        executeRead(command, value);
        return value;
    }

    template <std::size_t N>
    inline void executeWriteBurst(const WriteBurst &command, const WriteValue (&values)[N])
    {
        executeWriteBurst(command, N, values);
    }

    template <std::size_t N>
    inline void executeReadBurst(const ReadBurst &command, uint16_t (&values)[N])
    {
        executeReadBurst(command, N, values);
    }
};
