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
#include <platform/ethernet/SocketTcp.hpp>


class BridgeEthernetTcp :
    private BaseSockets<SocketTcp>,
    public BridgeEthernet
{
public:
    BridgeEthernetTcp(ipAddress_t ipAddr);
};
