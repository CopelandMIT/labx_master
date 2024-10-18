/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "RemotePinsSmartar.hpp"

#include <platform/exception/EProtocol.hpp>
#include <universal/components/radar.h>
#include <universal/components/subinterfaces.h>
#include <universal/components/subinterfaces/ipins.h>
#include <universal/protocol/protocol_definitions.h>


RemotePinsSmartar::RemotePinsSmartar(IVendorCommands *commands, uint8_t id) :
    m_commands {commands, CMD_COMPONENT, COMPONENT_TYPE_RADAR_SMARTAR, id, COMPONENT_SUBIF_PINS}
{
}

void RemotePinsSmartar::setResetPin(bool state)
{
    m_commands.vendorWriteParameters(FN_PINS_SET_RESET_PIN, state);
}

void RemotePinsSmartar::reset()
{
    m_commands.vendorWrite(FN_PINS_RESET);
}
