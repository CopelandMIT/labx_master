/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "SocketUdpImpl.hpp"

#include <common/Logger.hpp>
#include <platform/exception/EConnection.hpp>

#include <ifaddrs.h>
#include <net/if.h>


ISocket::Mode SocketUdpImpl::getMode() const
{
    return ISocket::Mode::Datagram;
}

SocketUdpImpl::SocketType SocketUdpImpl::socket()
{
    LOG(DEBUG) << "Opening SocketUdpImpl ...";

    return ::socket(AF_INET, SOCK_DGRAM, 0);
}

void SocketUdpImpl::setBroadcast(bool enable)
{
    int param     = static_cast<int>(enable);
    const int ret = ::setsockopt(m_socket, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<char *>(&param), sizeof(param));
    if (ret < 0)
    {
        LOG(ERROR) << "SocketUdpImpl::setBroadcast - error setting SO_BROADCAST: " << errno;
    }
}

void SocketUdpImpl::getBroadcastAddresses(std::vector<remoteInfo_t> &broadcastList)
{
    broadcastList.clear();

    struct ifaddrs *ifaddrs;
    if (getifaddrs(&ifaddrs) == -1)
    {
        return;
    }

    for (struct ifaddrs *adapter = ifaddrs;
         adapter != NULL;
         adapter = adapter->ifa_next)
    {
        if (adapter->ifa_addr == NULL)
        {
            continue;
        }

        const auto family = adapter->ifa_addr->sa_family;
        if (family != AF_INET)
        {
            continue;
        }

        if (adapter->ifa_flags & IFF_LOOPBACK)
        {
            continue;
        }

        const auto adapterIpV4 = reinterpret_cast<struct sockaddr_in *>(adapter->ifa_addr)->sin_addr.s_addr;
        const auto netMask     = reinterpret_cast<struct sockaddr_in *>(adapter->ifa_netmask)->sin_addr.s_addr;

        const auto broadcastIp           = adapterIpV4 | ~netMask;
        const remoteInfo_t broadcastInfo = {
            {
                static_cast<uint8_t>(broadcastIp & 0x000000FF),
                static_cast<uint8_t>((broadcastIp & 0x0000FF00) >> 8),
                static_cast<uint8_t>((broadcastIp & 0x00FF0000) >> 16),
                static_cast<uint8_t>((broadcastIp & 0xFF000000) >> 24),
            },
            55055,
        };

        broadcastList.emplace_back(broadcastInfo);
    }

    freeifaddrs(ifaddrs);
}

void SocketUdpImpl::sendTo(const uint8_t buffer[], uint16_t length, const remoteInfo_t *remote)
{
    m_addr.sin_family = AF_INET;
    std::copy(remote->ip, remote->ip + 4, reinterpret_cast<uint8_t *>(&m_addr.sin_addr));
    m_addr.sin_port = htons(remote->port);

    const ssize_t ret = ::sendto(m_socket, reinterpret_cast<const char *>(buffer), length, 0, reinterpret_cast<struct sockaddr *>(&m_addr), sizeof(m_addr));
    if (ret != length)
    {
        if (ret < 0)
        {
            throw EConnection("SocketUdpImpl::sendTo - sendto() failed", errno);
        }
        throw EConnection("SocketUdpImpl::sendTo - sendto() incomplete", ret);
    }
}

uint16_t SocketUdpImpl::receiveFrom(uint8_t buffer[], uint16_t length, remoteInfo_t *remote)
{
    m_addrSize        = sizeof(m_addr);
    const ssize_t ret = ::recvfrom(m_socket, reinterpret_cast<char *>(buffer), length, 0, reinterpret_cast<sockaddr *>(&m_addr), &m_addrSize);
    if (ret < 0)
    {
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
        {
            //LOG(DEBUG) << "SocketUdpImpl::receiveFrom - recvfrom() timeout";
            return 0;
        }
        else if (errno == EMSGSIZE)
        {
            //LOG(DEBUG) << "SocketUdpImpl::receiveFrom - recvfrom() message size greater than provided buffer";
            return length;
        }
        else
        {
            throw EConnection("SocketUdpImpl::receiveFrom - recvfrom() failed", errno);
        }
    }

    if (remote != nullptr)
    {
        std::copy(reinterpret_cast<uint8_t *>(&m_addr.sin_addr), reinterpret_cast<uint8_t *>(&m_addr.sin_addr) + 4, remote->ip);
        remote->port = ntohs(m_addr.sin_port);
    }

    return static_cast<uint16_t>(ret);
}
