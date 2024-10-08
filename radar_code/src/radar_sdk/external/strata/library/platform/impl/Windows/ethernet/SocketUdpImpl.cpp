/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "SocketUdpImpl.hpp"

#include <algorithm>
#include <common/Buffer.hpp>
#include <common/Logger.hpp>
#include <platform/exception/EConnection.hpp>

#include <iphlpapi.h>
#include <netioapi.h>


ISocket::Mode SocketUdpImpl::getMode() const
{
    return ISocket::Mode::Datagram;
}

SocketUdpImpl::SocketType SocketUdpImpl::socket()
{
    LOG(DEBUG) << "Opening SocketUdpImpl ...";

    return ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
}

void SocketUdpImpl::setBroadcast(bool enable)
{
    BOOL bBroadcast = enable;
    const int ret   = ::setsockopt(m_socket, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<const char *>(&bBroadcast), sizeof(bBroadcast));
    if (ret == SOCKET_ERROR)
    {
        LOG(ERROR) << "SocketUdpImpl::setBroadcast - error setting SO_BROADCAST: " << WSAGetLastError();
    }
}

void SocketUdpImpl::getBroadcastAddresses(std::vector<remoteInfo_t> &broadcastAddresses)
{
    broadcastAddresses.clear();

    strata::buffer<uint8_t> buffer;
    DWORD bufferSize = 0;

    for (int attempts = 0; attempts != 3; ++attempts)
    {
        const DWORD result = ::GetAdaptersAddresses(
            AF_UNSPEC,
            GAA_FLAG_SKIP_ANYCAST |
                GAA_FLAG_SKIP_MULTICAST |
                GAA_FLAG_SKIP_DNS_SERVER |
                GAA_FLAG_SKIP_FRIENDLY_NAME,
            NULL,
            reinterpret_cast<IP_ADAPTER_ADDRESSES *>(buffer.data()),
            &bufferSize);

        if (ERROR_SUCCESS == result)
        {
            break;
        }
        else if (ERROR_BUFFER_OVERFLOW == result)
        {
            // Increase the size of the buffer allocated
            buffer.resize(bufferSize);
            continue;
        }
        else
        {
            // Unexpected error code
            return;
        }
    }

    // Iterate through all of the adapters
    for (auto adapter = reinterpret_cast<IP_ADAPTER_ADDRESSES *>(buffer.data());
         NULL != adapter;
         adapter = adapter->Next)
    {
        // Skip loopback adapters
        if (IF_TYPE_SOFTWARE_LOOPBACK == adapter->IfType)
        {
            continue;
        }

        // Parse all IPv4 addresses
        for (auto address = adapter->FirstUnicastAddress;
             NULL != address;
             address = address->Next)
        {
            const auto family = address->Address.lpSockaddr->sa_family;
            if (AF_INET != family)
            {
                continue;
            }

            const SOCKADDR_IN *adapterAddress = reinterpret_cast<SOCKADDR_IN *>(address->Address.lpSockaddr);
            DWORD prefixMask;
            ConvertLengthToIpv4Mask(address->OnLinkPrefixLength, &prefixMask);

            const DWORD adapterIpV4          = adapterAddress->sin_addr.S_un.S_addr;
            const DWORD broadcastIp          = adapterIpV4 | ~prefixMask;
            const remoteInfo_t broadcastInfo = {
                {
                    static_cast<uint8_t>(broadcastIp & 0x000000FF),
                    static_cast<uint8_t>((broadcastIp & 0x0000FF00) >> 8),
                    static_cast<uint8_t>((broadcastIp & 0x00FF0000) >> 16),
                    static_cast<uint8_t>((broadcastIp & 0xFF000000) >> 24),
                },
                55055,
            };

            broadcastAddresses.emplace_back(broadcastInfo);
        }
    }
}

void SocketUdpImpl::sendTo(const uint8_t buffer[], uint16_t length, const remoteInfo_t *remote)
{
    m_addr.sin_family = AF_INET;
    std::copy(remote->ip, remote->ip + 4, &m_addr.sin_addr.S_un.S_un_b.s_b1);
    m_addr.sin_port = htons(remote->port);

    const int ret = ::sendto(m_socket, reinterpret_cast<const char *>(buffer), length, 0, reinterpret_cast<sockaddr *>(&m_addr), sizeof(m_addr));
    if (ret != length)
    {
        if (ret == SOCKET_ERROR)
        {
            throw EConnection("SocketUdpImpl::sendTo - sendto() failed", WSAGetLastError());
        }
        throw EConnection("SocketUdpImpl::sendTo - sendto() incomplete", (ret << 16) | static_cast<int>(length));
    }
}

uint16_t SocketUdpImpl::receiveFrom(uint8_t buffer[], uint16_t length, remoteInfo_t *remote)
{
    m_addrSize    = sizeof(m_addr);
    const int ret = ::recvfrom(m_socket, reinterpret_cast<char *>(buffer), length, 0, reinterpret_cast<sockaddr *>(&m_addr), &m_addrSize);
    if (ret == SOCKET_ERROR)
    {
        const int code = WSAGetLastError();
        if (code == WSAETIMEDOUT)
        {
            //LOG(DEBUG) << "SocketUdpImpl::receiveFrom - recvfrom() timeout";
            return 0;
        }
        else if (code == WSAEMSGSIZE)
        {
            //LOG(DEBUG) << "SocketUdpImpl::receiveFrom - recvfrom() message size greater than provided buffer";
            return length;
        }
        else
        {
            throw EConnection("SocketUdpImpl::receiveFrom - recvfrom() failed", code);
        }
    }

    if (remote != nullptr)
    {
        std::copy(&m_addr.sin_addr.S_un.S_un_b.s_b1, &m_addr.sin_addr.S_un.S_un_b.s_b4 + 1, remote->ip);
        remote->port = ntohs(m_addr.sin_port);
    }

    return static_cast<uint16_t>(ret);
}
