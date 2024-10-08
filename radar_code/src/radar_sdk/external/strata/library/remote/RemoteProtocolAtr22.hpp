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
#include <components/interfaces/IProtocolAtr22.hpp>
#include <platform/interfaces/IVendorCommands.hpp>


class RemoteProtocolAtr22 :
    public IProtocolAtr22
{
public:
    RemoteProtocolAtr22(IVendorCommands *commands, uint8_t id);

    void executeWrite(const Write commands[], uint16_t count) override;
    void executeRead(const Read &command, uint16_t count, uint16_t values[]) override;
    void setBits(uint16_t address, uint16_t bitMask) override;

private:
    RemoteVendorCommands m_commands;
};
