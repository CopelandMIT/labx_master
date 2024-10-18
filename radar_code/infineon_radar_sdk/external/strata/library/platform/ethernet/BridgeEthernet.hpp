/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <platform/ethernet/BridgeEthernetControl.hpp>
#include <platform/ethernet/BridgeEthernetData.hpp>
#include <platform/interfaces/IBridge.hpp>


template <typename SocketType>
class BaseSockets
{
protected:
    SocketType m_controlSocket;
    SocketType m_dataSocket;
};


class BridgeEthernet :
    public IBridge
{
protected:
    BridgeEthernet(ISocket &controlSocket, ISocket &dataSocket, ipAddress_t ipAddr);

public:
    ~BridgeEthernet();

    // IBridge implementation
    bool isConnected() override;
    void openConnection() override;
    void closeConnection() override;
    IBridgeControl *getIBridgeControl() override;
    IBridgeData *getIBridgeData() override;

private:
    BridgeEthernetControl m_control;
    BridgeEthernetData m_data;
};
