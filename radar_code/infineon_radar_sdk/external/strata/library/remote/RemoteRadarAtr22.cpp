/**
 * @copyright 2020 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "RemoteRadarAtr22.hpp"

#include <common/Buffer.hpp>
#include <common/Serialization.hpp>
#include <universal/components/radar/iradaratr22.h>
#include <universal/components/subinterfaces.h>
#include <universal/protocol/protocol_definitions.h>


RemoteRadarAtr22::RemoteRadarAtr22(IVendorCommands *commands, uint8_t id) :
    m_commands {commands, CMD_COMPONENT, getType(), id, COMPONENT_SUBIF_DEFAULT},
    m_registers(1, commands, getType(), id),
    m_protocol(commands, id)
{
}

void RemoteRadarAtr22::initialize()
{
    m_commands.vendorWrite(FN_RADAR_ATR22_INITIALIZE);
}

void RemoteRadarAtr22::reset(bool softReset)
{
    const uint8_t mode = softReset ? 1 : 0;
    m_commands.vendorWriteParameters(FN_RADAR_ATR22_RESET, mode);
}

uint8_t RemoteRadarAtr22::getDataIndex()
{
    uint8_t idx;
    m_commands.vendorReadParameters(FN_RADAR_ATR22_GET_DATA_INDEX, idx);
    return idx;
}

IRegisters<uint16_t, uint16_t> *RemoteRadarAtr22::getIRegisters()
{
    return &m_registers;
}

IProtocolAtr22 *RemoteRadarAtr22::getIProtocolAtr22()
{
    return &m_protocol;
}
