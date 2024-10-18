/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "BoardDescriptorEthernet.hpp"
#include "BridgeEthernetTcp.hpp"
#include "BridgeEthernetUdp.hpp"

std::shared_ptr<IBridge> BoardDescriptorEthernet::createBridge()
{
    if (m_useTcp)
    {
        return std::make_shared<BridgeEthernetTcp>(m_identifier);
    }
    else
    {
        return std::make_shared<BridgeEthernetUdp>(m_identifier);
    }
}
