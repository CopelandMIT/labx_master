/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include "SocketImpl.hpp"

#include <vector>


class SocketUdpImpl :
    public SocketImpl
{
public:
    Mode getMode() const override;

    void setBroadcast(bool enable);
    void getBroadcastAddresses(std::vector<remoteInfo_t> &broadcastList);

    void sendTo(const uint8_t buffer[], uint16_t length, const remoteInfo_t *remote);
    uint16_t receiveFrom(uint8_t buffer[], uint16_t length, remoteInfo_t *remote = nullptr);

protected:
    SocketType socket() override;
};
