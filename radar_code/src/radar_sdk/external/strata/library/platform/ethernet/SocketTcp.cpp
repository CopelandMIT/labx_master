/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "SocketTcp.hpp"
#include <common/Buffer.hpp>
#include <universal/link_definitions.h>


uint16_t SocketTcp::maxPayload() const
{
    return ETH_TCP_MAX_PAYLOAD;
}

bool SocketTcp::dumpPacket()
{
    const bool result = checkInputBuffer();
    if (result)
    {
        strata::buffer<uint8_t> dump(maxPayload());
        receive(dump.data(), static_cast<uint16_t>(dump.size()));
    }
    return result;
}
