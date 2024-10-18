/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "SocketImpl.hpp"

#include <common/Buffer.hpp>
#include <common/Logger.hpp>
#include <platform/exception/EConnection.hpp>


SocketImpl::SocketImpl() :
    m_socket {INVALID_SOCKET}
{
    const int ret = WSAStartup(MAKEWORD(2, 2), &m_wsaData);
    if (ret)
    {
        throw EConnection("WSAStartup failed", ret);
    }
}

SocketImpl::~SocketImpl()
{
    SocketImpl::close();

    WSACleanup();
}

bool SocketImpl::isOpened()
{
    return (m_socket != INVALID_SOCKET);
}

void SocketImpl::close()
{
    if (!SocketImpl::isOpened())
    {
        return;
    }

    LOG(DEBUG) << "Closing SocketImpl ...";

    ::closesocket(m_socket);

    m_socket = INVALID_SOCKET;
}

void SocketImpl::setInputBufferSize(uint32_t size)
{
    int param     = static_cast<int>(size);
    const int ret = ::setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<char *>(&param), sizeof(param));
    if (ret == SOCKET_ERROR)
    {
        LOG(ERROR) << "SocketImpl::setInputBufferSize - error setting SO_RCVBUF: " << WSAGetLastError();
    }
}

bool SocketImpl::checkInputBuffer()
{
    unsigned long bytes;
    const int ret = ::ioctlsocket(m_socket, FIONREAD, &bytes);
    if (ret == SOCKET_ERROR)
    {
        LOG(ERROR) << "SocketImpl::checkInputBuffer - error calling FIONREAD: " << WSAGetLastError();
    }

    return bytes > 0;
}

void SocketImpl::open(uint16_t localPort, uint16_t remotePort, ipAddress_t remoteIpAddr, uint16_t timeout)
{
    if (isOpened())
    {
        return;
    }

    m_socket = socket();
    if (m_socket == INVALID_SOCKET)
    {
        throw EConnection("SocketImpl::open - socket() failed", WSAGetLastError());
    }

    int ret;

    // set up local address
    m_addr.sin_family      = AF_INET;
    m_addr.sin_addr.s_addr = INADDR_ANY;
    m_addr.sin_port        = htons(localPort);

    ret = ::bind(m_socket, reinterpret_cast<sockaddr *>(&m_addr), sizeof(m_addr));
    if (ret == SOCKET_ERROR)
    {
        const int code = WSAGetLastError();
        ::closesocket(m_socket);
        m_socket = INVALID_SOCKET;

        std::string str("SocketImpl::open - bind() to port ");
        str.append(std::to_string(localPort));
        str.append(" failed");
        throw EConnection(str.c_str(), code);
    }

    // set up remote address if specified
    if (remoteIpAddr != nullptr)
    {
        std::copy(remoteIpAddr, remoteIpAddr + 4, &m_addr.sin_addr.S_un.S_un_b.s_b1);
        m_addr.sin_port = htons(remotePort);

        ret = ::connect(m_socket, reinterpret_cast<sockaddr *>(&m_addr), sizeof(m_addr));
        if (ret == SOCKET_ERROR)
        {
            const int code = WSAGetLastError();
            ::closesocket(m_socket);
            m_socket = INVALID_SOCKET;
            throw EConnection("SocketImpl::open - connect() failed", code);
        }
    }

    setTimeout(timeout);
}

void SocketImpl::setTimeout(uint16_t timeout)
{
    DWORD dwTimeout = timeout;
    const int ret   = ::setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char *>(&dwTimeout), sizeof(dwTimeout));
    if (ret == SOCKET_ERROR)
    {
        LOG(ERROR) << "SocketImpl::setTimeout - error setting SO_RCVTIMEO: " << WSAGetLastError();
    }
}

void SocketImpl::send(const uint8_t buffer[], uint16_t length)
{
    const int ret = ::send(m_socket, reinterpret_cast<const char *>(buffer), length, 0);
    if (ret != length)
    {
        if (ret == SOCKET_ERROR)
        {
            throw EConnection("SocketImpl::send - send() failed", WSAGetLastError());
        }
        throw EConnection("SocketImpl::send - send() incomplete", (ret << 16) | static_cast<int>(length));
    }
}

uint16_t SocketImpl::receive(uint8_t buffer[], uint16_t length)
{
    const int ret = ::recv(m_socket, reinterpret_cast<char *>(buffer), length, 0);
    if (ret == SOCKET_ERROR)
    {
        const int code = WSAGetLastError();
        if (code == WSAETIMEDOUT)
        {
            //LOG(DEBUG) << "SocketImpl::receive - recv() timeout";
            return 0;
        }
        else if (code == WSAEMSGSIZE)
        {
            //LOG(DEBUG) << "SocketImpl::receive - recv() message size greater than provided buffer";
            return length;
        }
        else
        {
            throw EConnection("SocketImpl::receive - recv() failed", code);
        }
    }

    return static_cast<uint16_t>(ret);
}
