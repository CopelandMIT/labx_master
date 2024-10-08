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
#include <components/interfaces/IProtocolLtr11.hpp>
#include <platform/interfaces/IVendorCommands.hpp>


class RemoteProtocolLtr11 :
    public IProtocolLtr11
{
public:
    RemoteProtocolLtr11(IVendorCommands *commands, uint8_t id);

    void executeWrite(const Write &command) override;
    void executeRead(const Read &command, uint16_t &value) override;
    void setBits(uint8_t address, uint16_t bitMask) override;
    void executeWriteBatch(const Write commands[], uint16_t count) override;
    void executeWriteBurst(const WriteBurst &command, uint16_t count, const WriteValue values[]) override;
    void executeReadBurst(const ReadBurst &command, uint16_t count, uint16_t values[]) override;
    void setMisoArbitration(uint16_t prt) override;

private:
    RemoteVendorCommands m_commands;
};
