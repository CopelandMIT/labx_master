/**
 * @copyright 2020 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "RemoteRadarLtr11.hpp"

#include <common/Buffer.hpp>
#include <common/Serialization.hpp>
#include <universal/components/radar/iradarltr11.h>
#include <universal/components/subinterfaces.h>
#include <universal/protocol/protocol_definitions.h>


RemoteRadarLtr11::RemoteRadarLtr11(IVendorCommands *commands, uint8_t id) :
    m_commands {commands, CMD_COMPONENT, getType(), id, COMPONENT_SUBIF_DEFAULT},
    m_registers(1, commands, getType(), id),
    m_pins(commands, id),
    m_protocol(commands, id)
{
}

void RemoteRadarLtr11::initialize()
{
    m_commands.vendorWrite(FN_RADAR_LTR11_INITIALIZE);
}

void RemoteRadarLtr11::reset(bool softReset)
{
    const uint8_t mode = softReset ? 1 : 0;
    m_commands.vendorWriteParameters(FN_RADAR_LTR11_RESET, mode);
}

uint8_t RemoteRadarLtr11::getDataIndex()
{
    uint8_t idx;
    m_commands.vendorReadParameters(FN_RADAR_LTR11_GET_DATA_INDEX, idx);
    return idx;
}

IRegisters<uint8_t, uint16_t> *RemoteRadarLtr11::getIRegisters()
{
    return &m_registers;
}

IPinsLtr11 *RemoteRadarLtr11::getIPinsLtr11()
{
    return &m_pins;
}

IProtocolLtr11 *RemoteRadarLtr11::getIProtocolLtr11()
{
    return &m_protocol;
}
