/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <platform/ethernet/SocketUdp.hpp>
#include <platform/interfaces/IEnumerator.hpp>


class EnumeratorEthernet :
    public IEnumerator
{
public:
    /**
     * Enumeration is always done in UDP mode as broadcasting is not possible using TCP
     * But when a board is found, it can then be controlled via either UDP or TCP
     * The useTcpConnection parameter defines this.
     */
    EnumeratorEthernet(bool useTcpConnection);

    /**
     * Request if TCP connection type is selected
     */
    bool getUseTcpConnection();

protected:
    void enumerate(IEnumerationListener &listener, BoardData::const_iterator begin, BoardData::const_iterator end) override;

private:
    inline void sendBroadcast(const std::vector<remoteInfo_t> &broadcastAddresses);
    inline void getResponses(IEnumerationListener &listener, BoardData::const_iterator begin, BoardData::const_iterator end);

    SocketUdp m_socket;
    bool m_useTcpConnection;
};
