/**
 * @copyright 2020 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "RemoteVendorCommands.hpp"


RemoteVendorCommands::RemoteVendorCommands(IVendorCommands *commands, uint8_t bRequest, uint16_t wType, uint8_t bId, uint8_t bSubInterface) :
    m_commands {commands},
    m_bRequest {bRequest},
    m_wType {wType},
    m_bId {bId},
    m_bSubInterface {bSubInterface}
{
    if (getProtocolVersion() < 0x00040000)  // STRATA_LEGACY_PROTOCOL_3
    {
        m_wType = (m_wType << 8) | (m_wType >> 8);
    }
}
