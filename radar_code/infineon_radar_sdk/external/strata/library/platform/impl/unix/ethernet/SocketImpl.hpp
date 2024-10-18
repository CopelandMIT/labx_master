/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <netinet/in.h>
#include <platform/interfaces/link/ISocket.hpp>


///
/// \brief The SocketImpl class is a thin wrapper over the Linux Socket.
/// \details The functions are an implementation of ISocket.
/// \sa ISocket
///
class SocketImpl :
    public ISocket
{
public:
    SocketImpl();
    ~SocketImpl();

    //ISocket
    bool isOpened() override;
    void close() override;

    void setInputBufferSize(uint32_t size) override;
    bool checkInputBuffer() override;

    ///
    /// \brief open the socket and create a connection when a remoteIpAddr is provided.
    /// \note You need to provide remoteIpAddr for the Tcp protocol
    ///
    void open(uint16_t localPort, uint16_t remotePort, ipAddress_t remoteIpAddr, uint16_t timeout) override;

    void setTimeout(uint16_t timeout) override;

    void send(const uint8_t buffer[], uint16_t length) override;
    uint16_t receive(uint8_t buffer[], uint16_t length) override;

protected:
    using SocketType = int;

    virtual SocketType socket() = 0;
    int m_socket;

    struct sockaddr_in m_addr;
    socklen_t m_addrSize;
};
