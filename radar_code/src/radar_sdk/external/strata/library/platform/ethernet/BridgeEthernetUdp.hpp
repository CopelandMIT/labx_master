/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <platform/ethernet/BridgeEthernet.hpp>
#include <platform/ethernet/SocketUdp.hpp>


class BridgeEthernetUdp :
    private BaseSockets<SocketUdp>,
    public BridgeEthernet
{
public:
    BridgeEthernetUdp(ipAddress_t ipAddr);
};
