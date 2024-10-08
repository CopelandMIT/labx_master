/**
 * @copyright 2020 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include "RemoteProtocolAtr22.hpp"
#include "RemoteRegisters.hpp"
#include <components/interfaces/IRadarAtr22.hpp>


class RemoteRadarAtr22 :
    public IRadarAtr22
{
public:
    RemoteRadarAtr22(IVendorCommands *commands, uint8_t id);

    void initialize() override;
    void reset(bool softReset) override;
    uint8_t getDataIndex() override;

    IRegisters<uint16_t, uint16_t> *getIRegisters() override;
    IProtocolAtr22 *getIProtocolAtr22() override;

private:
    RemoteVendorCommands m_commands;

    RemoteRegisters<uint16_t, uint16_t> m_registers;
    RemoteProtocolAtr22 m_protocol;
};
