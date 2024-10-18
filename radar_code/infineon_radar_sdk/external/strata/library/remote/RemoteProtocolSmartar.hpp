/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include "RemoteVendorCommands.hpp"
#include <components/interfaces/IProtocolSmartar.hpp>
#include <platform/interfaces/IVendorCommands.hpp>


class RemoteProtocolSmartar :
    public IProtocolSmartar
{
public:
    RemoteProtocolSmartar(IVendorCommands *commands, uint8_t id);

    void executeRead(const Read &command, uint16_t count, uint32_t values[]) override;
    void executeWrite(const Write &command, uint16_t count, const WriteValue values[]) override;
    void executeWrite16(const Write &command, uint16_t count, const uint16_t values[]) override;
    void executeWriteBatch(const BatchWrite commands[], uint16_t count) override;
    void setBits(uint16_t address, uint32_t bitMask) override;

private:
    RemoteVendorCommands m_commands;
};
