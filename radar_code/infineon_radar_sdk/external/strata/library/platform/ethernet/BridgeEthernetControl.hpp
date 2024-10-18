/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <platform/bridge/BridgeProtocol.hpp>
#include <platform/bridge/VendorCommandsImpl.hpp>
#include <platform/interfaces/link/ISocket.hpp>

#include <mutex>


class BridgeEthernetControl :
    private VendorCommandsImpl
{
public:
    BridgeEthernetControl(ISocket &socket, ipAddress_t ipAddr);
    ~BridgeEthernetControl() override;

    IBridgeControl *getIBridgeControl();

    void openConnection();
    void closeConnection();

    // IVendorCommands implementation
    void setDefaultTimeout() override;
    uint16_t getMaxTransfer() const override;

    void vendorWrite(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength, const uint8_t buffer[]) override;
    void vendorRead(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength, uint8_t buffer[]) override;
    void vendorTransfer(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLengthSend, const uint8_t bufferSend[], uint16_t &wLengthReceive, uint8_t bufferReceive[]) override;

private:
    void sendRequest(uint8_t *packet, uint16_t sendSize, uint8_t bRequestType, uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength, const uint8_t buffer[]);
    void receiveResponse(uint8_t *packet, uint16_t receiveSize, uint8_t bRequestType, uint8_t bRequest, uint16_t &wLength, uint8_t buffer[]);
    uint16_t receiveStream(uint8_t *buffer, uint16_t receiveSize);
    uint16_t receiveDatagram(uint8_t *buffer, uint16_t receiveSize);

    BridgeProtocol m_protocol;
    ISocket &m_socket;
    uint8_t m_ipAddr[4];

    std::mutex m_lock;
    bool m_commandError;
};
