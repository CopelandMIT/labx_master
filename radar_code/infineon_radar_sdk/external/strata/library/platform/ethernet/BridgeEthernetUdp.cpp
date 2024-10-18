/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "BridgeEthernetUdp.hpp"


BridgeEthernetUdp::BridgeEthernetUdp(ipAddress_t ipAddr) :
    BridgeEthernet(m_controlSocket, m_dataSocket, ipAddr)
{
}
