/**
 * @copyright 2020 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "RemoteRadarAvian.hpp"

#include <common/Buffer.hpp>
#include <common/Serialization.hpp>
#include <universal/components/radar/iradaravian.h>
#include <universal/components/subinterfaces.h>
#include <universal/protocol/protocol_definitions.h>


RemoteRadarAvian::RemoteRadarAvian(IVendorCommands *commands, uint8_t id) :
    m_commands {commands, CMD_COMPONENT, getType(), id, COMPONENT_SUBIF_DEFAULT},
    m_registers(sizeof(uint8_t), commands, getType(), id),
    m_pins(commands, id),
    m_protocol(commands, id)
{
}

IRegisters<uint8_t, uint32_t> *RemoteRadarAvian::getIRegisters()
{
    return &m_registers;
}

void RemoteRadarAvian::initialize()
{
    m_commands.vendorWrite(FN_RADAR_AVIAN_INITIALIZE, 0, nullptr);
}

void RemoteRadarAvian::reset(bool softReset)
{
    const uint8_t mode = softReset ? 1 : 0;
    m_commands.vendorWriteParameters(FN_RADAR_AVIAN_RESET, mode);
}

uint8_t RemoteRadarAvian::getDataIndex()
{
    uint8_t idx;
    m_commands.vendorReadParameters(FN_RADAR_AVIAN_GET_DATA_INDEX, idx);
    return idx;
}

void RemoteRadarAvian::startData()
{
    m_commands.vendorWrite(FN_RADAR_AVIAN_START_DATA);
}

void RemoteRadarAvian::stopData()
{
    m_commands.vendorWrite(FN_RADAR_AVIAN_STOP_DATA);
}

IPinsAvian *RemoteRadarAvian::getIPinsAvian()
{
    return &m_pins;
}

IProtocolAvian *RemoteRadarAvian::getIProtocolAvian()
{
    return &m_protocol;
}
