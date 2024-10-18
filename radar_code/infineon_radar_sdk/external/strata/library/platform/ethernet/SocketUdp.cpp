/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "SocketUdp.hpp"
#include <universal/link_definitions.h>


uint16_t SocketUdp::maxPayload() const
{
    return ETH_UDP_MAX_PAYLOAD;
}

bool SocketUdp::dumpPacket()
{
    const bool result = checkInputBuffer();
    if (result)
    {
        uint8_t dump[1];
        receive(dump, sizeof(dump));  // if we read from a message based protocol with a too small buffer, the excess packet is discarded
    }
    return result;
}
