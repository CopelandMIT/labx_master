#pragma once

#include "RemotePinsSmartar.hpp"
#include "RemoteProtocolSmartar.hpp"
#include "RemoteRegisters.hpp"
#include <components/interfaces/IRadarSmartar.hpp>


class RemoteRadarSmartar :
    public IRadarSmartar
{
public:
    RemoteRadarSmartar(IVendorCommands *commands, uint8_t id);

    IRegisters<uint16_t, uint32_t> *getIRegisters() override;
    IPinsSmartar *getIPinsSmartar() override;
    IProtocolSmartar *getIProtocolSmartar() override;
    uint8_t getDataIndex() override;
    void reset(bool softReset) override;

private:
    RemoteVendorCommands m_commands;

    RemoteRegisters<uint16_t, uint32_t> m_registers;
    RemotePinsSmartar m_pins;
    RemoteProtocolSmartar m_protocol;
};
