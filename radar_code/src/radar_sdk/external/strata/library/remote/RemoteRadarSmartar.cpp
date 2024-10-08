/**
 * @copyright 2020 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "RemoteRadarSmartar.hpp"

#include <common/Buffer.hpp>
#include <common/Serialization.hpp>
#include <universal/components/radar/iradarsmartar.h>
#include <universal/components/subinterfaces.h>
#include <universal/protocol/protocol_definitions.h>


RemoteRadarSmartar::RemoteRadarSmartar(IVendorCommands *commands, uint8_t id) :
    m_commands {commands, CMD_COMPONENT, getType(), id, COMPONENT_SUBIF_DEFAULT},
    m_registers(sizeof(uint32_t), commands, getType(), id),
    m_pins(commands, id),
    m_protocol(commands, id)
{
}

IRegisters<uint16_t, uint32_t> *RemoteRadarSmartar::getIRegisters()
{
    return &m_registers;
}

IProtocolSmartar *RemoteRadarSmartar::getIProtocolSmartar()
{
    return &m_protocol;
}

IPinsSmartar *RemoteRadarSmartar::getIPinsSmartar()
{
    return &m_pins;
}

uint8_t RemoteRadarSmartar::getDataIndex()
{
    uint8_t idx;
    m_commands.vendorReadParameters(FN_RADAR_SMARTAR_GET_DATA_INDEX, idx);
    return idx;
}

void RemoteRadarSmartar::reset(bool softReset)
{
    const uint8_t mode = softReset ? 1 : 0;
    m_commands.vendorWriteParameters(FN_RADAR_SMARTAR_RESET, mode);
}
