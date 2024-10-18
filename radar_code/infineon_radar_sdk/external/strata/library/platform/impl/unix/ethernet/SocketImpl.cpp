/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "SocketImpl.hpp"

#include <common/Logger.hpp>
#include <platform/exception/EConnection.hpp>

#include <errno.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define INVALID_SOCKET -1


SocketImpl::SocketImpl() :
    m_socket {INVALID_SOCKET}
{
}

SocketImpl::~SocketImpl()
{
    close();
}

void SocketImpl::setInputBufferSize(uint32_t size)
{
    int param     = static_cast<int>(size);
    const int ret = ::setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<char *>(&param), sizeof(param));
    if (ret < 0)
    {
        LOG(ERROR) << "SocketImpl::setInputBufferSize - error setting SO_RCVBUF: " << errno;
    }
}

bool SocketImpl::isOpened()
{
    return m_socket != INVALID_SOCKET;
}

bool SocketImpl::checkInputBuffer()
{
    unsigned int bytes;
    const int ret = ::ioctl(m_socket, FIONREAD, &bytes);
    if (ret == -1)
    {
        LOG(ERROR) << "SocketImpl::checkInputBuffer - error calling FIONREAD: " << errno;
    }

    return bytes > 0;
}

void SocketImpl::open(uint16_t localPort, uint16_t remotePort, ipAddress_t remoteIpAddr, uint16_t timeout)
{
    if (isOpened())
    {
        return;
    }

    LOG(DEBUG) << "Opening SocketImpl ...";

    m_socket = socket();
    if (m_socket < 0)
    {
        m_socket = INVALID_SOCKET;
        throw EConnection("SocketImpl::open - socket() failed", errno);
    }

    int ret;

    // set up local address
    m_addr.sin_family      = AF_INET;
    m_addr.sin_addr.s_addr = INADDR_ANY;
    m_addr.sin_port        = htons(localPort);

    ret = ::bind(m_socket, reinterpret_cast<struct sockaddr *>(&m_addr), sizeof(m_addr));
    if (ret < 0)
    {
        const int code = errno;
        ::close(m_socket);
        m_socket = INVALID_SOCKET;

        std::string str("SocketImpl::open - bind() to port ");
        str.append(std::to_string(localPort));
        str.append(" failed");
        throw EConnection(str.c_str(), code);
    }

    // set up remote address if specified
    if (remoteIpAddr != nullptr)
    {
        std::copy(remoteIpAddr, remoteIpAddr + 4, reinterpret_cast<uint8_t *>(&m_addr.sin_addr));
        m_addr.sin_port = htons(remotePort);

        ret = ::connect(m_socket, reinterpret_cast<sockaddr *>(&m_addr), sizeof(m_addr));
        if (ret < 0)
        {
            const int code = errno;
            ::close(m_socket);
            m_socket = INVALID_SOCKET;
            throw EConnection("SocketImpl::open - connect() failed", code);
        }
    }

    setTimeout(timeout);
}

void SocketImpl::setTimeout(uint16_t timeout)
{
    struct timeval tv;
    tv.tv_sec     = timeout / 1000;
    tv.tv_usec    = (timeout % 1000) * 1000;
    const int ret = ::setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<struct timeval *>(&tv), sizeof(struct timeval));
    if (ret < 0)
    {
        LOG(ERROR) << "SocketImpl::setTimeout - error setting SO_RCVTIMEO: " << errno;
    }
}

void SocketImpl::close()
{
    if (!isOpened())
    {
        return;
    }

    LOG(DEBUG) << "Closing SocketImpl with protocol ...";

    ::close(m_socket);
    m_socket = INVALID_SOCKET;
}

void SocketImpl::send(const uint8_t buffer[], uint16_t length)
{
    const ssize_t ret = ::send(m_socket, reinterpret_cast<const char *>(buffer), length, 0);
    if (ret != length)
    {
        if (ret < 0)
        {
            throw EConnection("SocketImpl::send - send() failed", errno);
        }
        throw EConnection("SocketImpl::send - send() incomplete", ret);
    }
}

uint16_t SocketImpl::receive(uint8_t buffer[], uint16_t length)
{
    const ssize_t ret = ::recv(m_socket, reinterpret_cast<char *>(buffer), length, 0);
    if (ret < 0)
    {
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
        {
            //LOG(DEBUG) << "SocketImpl::receive - recv() timeout";
            return 0;
        }
        else if (errno == EMSGSIZE)
        {
            //LOG(DEBUG) << "SocketImpl::receive - recv() message size greater than provided buffer";
            return length;
        }
        else
        {
            throw EConnection("SocketImpl::receive - recv() failed", errno);
        }
    }

    return static_cast<uint16_t>(ret);
}
