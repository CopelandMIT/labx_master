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
#include <components/interfaces/IPinsSmartar.hpp>
#include <platform/interfaces/IVendorCommands.hpp>


class RemotePinsSmartar :
    public IPinsSmartar
{
public:
    RemotePinsSmartar(IVendorCommands *commands, uint8_t id);

    void setResetPin(bool state) override;

    void reset() override;

private:
    RemoteVendorCommands m_commands;
};
