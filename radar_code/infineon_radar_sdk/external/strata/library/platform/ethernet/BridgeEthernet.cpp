/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "BridgeEthernet.hpp"


BridgeEthernet::BridgeEthernet(ISocket &controlSocket, ISocket &dataSocket, ipAddress_t ipAddr) :
    m_control(controlSocket, ipAddr),
    m_data(dataSocket, ipAddr)
{
}

BridgeEthernet::~BridgeEthernet()
{
}

bool BridgeEthernet::isConnected()
{
    return true;
}

void BridgeEthernet::openConnection()
{
    m_control.openConnection();
    m_data.openConnection();
}

void BridgeEthernet::closeConnection()
{
    m_data.closeConnection();
    m_control.closeConnection();
}

IBridgeControl *BridgeEthernet::getIBridgeControl()
{
    return m_control.getIBridgeControl();
}

IBridgeData *BridgeEthernet::getIBridgeData()
{
    return &m_data;
}
