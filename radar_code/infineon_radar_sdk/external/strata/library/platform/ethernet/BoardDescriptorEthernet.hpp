/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <platform/BoardDescriptor.hpp>
#include <platform/interfaces/link/ISocket.hpp>  // ipAddress_t


class BoardDescriptorEthernet :
    public BoardDescriptor
{
public:
    BoardDescriptorEthernet(const BoardData &data, const char name[], ipAddress_t ipAddr, bool useTcp) :
        BoardDescriptor(data, name),
        m_identifier {ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]},
        m_useTcp(useTcp)
    {}

    std::shared_ptr<IBridge> createBridge() override;

private:
    ipAddress_t m_identifier;
    bool m_useTcp;
};
