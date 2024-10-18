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
#include <components/interfaces/IPinsAvian.hpp>
#include <platform/interfaces/IVendorCommands.hpp>


class RemotePinsAvian :
    public IPinsAvian
{
public:
    RemotePinsAvian(IVendorCommands *commands, uint8_t id);

    void setResetPin(bool state) override;
    bool getIrqPin() override;

    void reset() override;

private:
    RemoteVendorCommands m_commands;
};
