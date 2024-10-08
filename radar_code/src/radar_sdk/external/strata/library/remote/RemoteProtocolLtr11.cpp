/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "RemoteProtocolLtr11.hpp"

#include <algorithm>
#include <common/Buffer.hpp>
#include <common/Serialization.hpp>
#include <universal/components/radar.h>
#include <universal/components/subinterfaces.h>
#include <universal/components/subinterfaces/iprotocol.h>
#include <universal/protocol/protocol_definitions.h>


RemoteProtocolLtr11::RemoteProtocolLtr11(IVendorCommands *commands, uint8_t id) :
    m_commands {commands, CMD_COMPONENT, COMPONENT_TYPE_RADAR_LTR11, id, COMPONENT_SUBIF_PROTOCOL}
{
}

void RemoteProtocolLtr11::executeWrite(const Write &command)
{
    m_commands.vendorWriteParameters(FN_PROTOCOL_EXECUTE, command.value());
}

void RemoteProtocolLtr11::executeRead(const Read &command, uint16_t &value)
{
    const auto &cmd = command.value();
    m_commands.vendorTransferChecked(FN_PROTOCOL_EXECUTE, sizeof(cmd), &cmd, sizeof(value), &value);
}

void RemoteProtocolLtr11::setBits(uint8_t address, uint16_t bitMask)
{
    m_commands.vendorWriteParameters(FN_PROTOCOL_SET_BITS, address, bitMask);
}

void RemoteProtocolLtr11::executeWriteBatch(const Write commands[], uint16_t count)
{
    m_commands.vendorWriteMulti(FN_PROTOCOL_EXECUTE_HELPER, count, commands);
}

void RemoteProtocolLtr11::executeWriteBurst(const WriteBurst &command, uint16_t count, const WriteValue values[])
{
    WriteBurst cmd = command;  // create a local copy of the command to be able to increment it for multi-part transfers

    auto updateParams = [&cmd](decltype(count) wCount) {
        cmd.increment(wCount);
    };

    m_commands.vendorWriteMulti(FN_PROTOCOL_EXECUTE, count, values, updateParams, cmd.value());
}

void RemoteProtocolLtr11::executeReadBurst(const ReadBurst &command, uint16_t count, uint16_t values[])
{
    ReadBurst cmd = command;  // create a local copy of the command to be able to increment it for multi-part transfers

    if (m_commands.getProtocolVersion() < 0x00040000)  // STRATA_LEGACY_PROTOCOL_3
    {
        constexpr uint16_t elemSize    = sizeof(values[0]);
        const decltype(count) maxCount = m_commands.getMaxTransfer() / elemSize;
        const decltype(count) wCount   = std::min(count, maxCount);

        while (count > 0)
        {
            const uint16_t wLengthReceive = wCount * elemSize;
            const uint16_t wLengthSend    = sizeof(cmd) + sizeof(wCount);
            uint8_t payload[wLengthSend];
            uint8_t *it = payload;
            it          = hostToSerial(it, cmd.value());
            hostToSerial(it, wCount);

            m_commands.vendorTransferChecked(FN_PROTOCOL_EXECUTE, wLengthSend, payload, wLengthReceive, reinterpret_cast<uint8_t *>(values));

            cmd.increment(wCount);
            values += wCount;
            count -= wCount;
        }
        return;
    }

    auto updateParams = [&cmd](decltype(count) wCount) {
        cmd.increment(wCount);
    };

    m_commands.vendorTransferMultiRead(FN_PROTOCOL_EXECUTE, count, values, updateParams, cmd.value());
}

void RemoteProtocolLtr11::setMisoArbitration(uint16_t prt)
{
    m_commands.vendorWriteParameters(FN_PROTOCOL_SETTING, prt);
}
