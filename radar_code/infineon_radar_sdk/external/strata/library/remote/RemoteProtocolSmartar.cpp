#include "RemoteProtocolSmartar.hpp"

#include <algorithm>
#include <common/Buffer.hpp>
#include <common/Serialization.hpp>
#include <universal/components/radar.h>
#include <universal/components/subinterfaces.h>
#include <universal/components/subinterfaces/iprotocol.h>
#include <universal/protocol/protocol_definitions.h>


RemoteProtocolSmartar::RemoteProtocolSmartar(IVendorCommands *commands, uint8_t id) :
    m_commands {commands, CMD_COMPONENT, COMPONENT_TYPE_RADAR_SMARTAR, id, COMPONENT_SUBIF_PROTOCOL}
{
}

void RemoteProtocolSmartar::executeRead(const Read &command, uint16_t count, uint32_t values[])
{
    Read cmd = command;  // create a local copy of the command to be able to increment it for multi-part transfers

    auto updateParams = [&cmd](decltype(count) wCount) {
        cmd.increment(wCount);
    };

    m_commands.vendorTransferMultiRead(FN_PROTOCOL_EXECUTE, count, values, updateParams, cmd.value());
}

void RemoteProtocolSmartar::executeWrite(const Write &command, uint16_t count, const WriteValue values[])
{
    Write cmd = command;  // create a local copy of the command to be able to increment it for multi-part transfers

    auto updateParams = [&cmd](decltype(count) wCount) {
        cmd.increment(wCount);
    };

    m_commands.vendorWriteMulti(FN_PROTOCOL_EXECUTE, count, values, updateParams, cmd.value());
}

void RemoteProtocolSmartar::executeWrite16(const Write &command, uint16_t count, const uint16_t values[])
{
    Write cmd = command;  // create a local copy of the command to be able to increment it for multi-part transfers

    auto updateParams = [&cmd](decltype(count) wCount) {
        cmd.increment(wCount / 2);
    };

    m_commands.vendorWriteMulti(FN_PROTOCOL_SETTING, count, values, updateParams, cmd.value());
}

void RemoteProtocolSmartar::executeWriteBatch(const BatchWrite commands[], uint16_t count)
{
    m_commands.vendorWriteMulti(FN_PROTOCOL_EXECUTE_HELPER, count, commands);
}

void RemoteProtocolSmartar::setBits(uint16_t address, uint32_t bitMask)
{
    m_commands.vendorWriteParameters(FN_PROTOCOL_SET_BITS, address, bitMask);
}
