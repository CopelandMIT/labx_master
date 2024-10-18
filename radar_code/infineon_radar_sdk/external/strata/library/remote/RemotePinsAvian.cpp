/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "RemotePinsAvian.hpp"

#include <platform/exception/EProtocol.hpp>
#include <universal/components/radar.h>
#include <universal/components/subinterfaces.h>
#include <universal/components/subinterfaces/ipins.h>
#include <universal/protocol/protocol_definitions.h>


RemotePinsAvian::RemotePinsAvian(IVendorCommands *commands, uint8_t id) :
    m_commands {commands, CMD_COMPONENT, COMPONENT_TYPE_RADAR_AVIAN, id, COMPONENT_SUBIF_PINS}
{
}

void RemotePinsAvian::setResetPin(bool state)
{
    m_commands.vendorWriteParameters(FN_PINS_SET_RESET_PIN, state);
}

bool RemotePinsAvian::getIrqPin()
{
    uint8_t state;
    m_commands.vendorReadParameters(FN_PINS_GET_IRQ, state);
    return (state != 0);
}

void RemotePinsAvian::reset()
{
    m_commands.vendorWrite(FN_PINS_RESET);
}
