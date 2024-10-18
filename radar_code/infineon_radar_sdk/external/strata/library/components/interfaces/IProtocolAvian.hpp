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


class IProtocolAvian
{
public:
    using CommandType                        = uint8_t[4];
    constexpr static uint8_t writeBit        = (1u << 0);
    constexpr static uint8_t addressOffset   = 1u;
    constexpr static unsigned int valueWidth = 24u;
    constexpr static uint32_t valueMask      = ((1u << valueWidth) - 1);

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

        Command() = default;

    protected:
        constexpr Command(uint8_t address) :
            m_command {static_cast<uint8_t>(address << addressOffset), 0u, 0u, 0u}
        {}

        constexpr Command(uint8_t address, uint32_t value) :
            m_command {static_cast<uint8_t>((address << addressOffset) | writeBit), static_cast<uint8_t>(value >> 16), static_cast<uint8_t>(value >> 8), static_cast<uint8_t>(value & 0xFF)}
        {}

    private:
        CommandType m_command;
    };
    static_assert(sizeof(Command) == sizeof(CommandType), "Memory layout of Command contains additional stuff");


    struct Read :
        public Command
    {
        Read() = delete;

        constexpr Read(uint8_t address) :
            Command(address)
        {}
    };

    struct Write :
        public Command
    {
        constexpr Write(uint8_t address, uint32_t value) :
            Command(address, value)
        {}
    };


    virtual void execute(const Command commands[], uint32_t count, uint32_t results[] = nullptr) = 0;
    virtual void setBits(uint8_t address, uint32_t bitMask)                                      = 0;

    template <std::size_t N>
    inline void execute(const Command (&commands)[N], uint32_t (&results)[N])
    {
        execute(commands, N, results);
    }
    template <std::size_t N>
    inline void execute(const Command (&commands)[N])
    {
        execute(commands, N);
    }
    inline void execute(const Write &command)
    {
        execute(&command, 1);
    }
    inline void execute(const Read &command, uint32_t &result)
    {
        execute(&command, 1, &result);
    }
    inline uint32_t execute(const Read &command)
    {
        uint32_t result;
        execute(command, result);
        return result;
    }
};
