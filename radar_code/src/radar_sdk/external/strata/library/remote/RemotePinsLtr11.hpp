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
#include <components/interfaces/IPinsLtr11.hpp>
#include <platform/interfaces/IVendorCommands.hpp>


class RemotePinsLtr11 :
    public IPinsLtr11
{
public:
    RemotePinsLtr11(IVendorCommands *commands, uint8_t id);

    void setResetPin(bool state) override;

    void reset() override;

    uint8_t getDetectionPins() override;

private:
    RemoteVendorCommands m_commands;
};
