/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "RemotePinsLtr11.hpp"

#include <platform/exception/EProtocol.hpp>
#include <universal/components/radar.h>
#include <universal/components/subinterfaces.h>
#include <universal/components/subinterfaces/ipins.h>
#include <universal/protocol/protocol_definitions.h>


RemotePinsLtr11::RemotePinsLtr11(IVendorCommands *commands, uint8_t id) :
    m_commands {commands, CMD_COMPONENT, COMPONENT_TYPE_RADAR_LTR11, id, COMPONENT_SUBIF_PINS}
{
}

void RemotePinsLtr11::setResetPin(bool state)
{
    m_commands.vendorWriteParameters(FN_PINS_SET_RESET_PIN, state);
}

void RemotePinsLtr11::reset()
{
    m_commands.vendorWrite(FN_PINS_RESET);
}

uint8_t RemotePinsLtr11::getDetectionPins()
{
    uint8_t states;
    m_commands.vendorRead(FN_PINS_GET_DETECTIONS, states);
    return states;
}
