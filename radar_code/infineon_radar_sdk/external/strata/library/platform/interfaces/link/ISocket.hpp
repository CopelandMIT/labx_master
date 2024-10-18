/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <cstdint>

using ipAddress_t = uint8_t[4];


//using remoteInfo_t = struct
typedef struct
{
    ipAddress_t ip;
    uint16_t port;
} remoteInfo_t;

enum class TransportProtocol
{
    Tcp,
    Udp
};

inline const char *transportProtocolName(TransportProtocol protocol)
{
    switch (protocol)
    {
        case TransportProtocol::Tcp:
            return "TCP";

        case TransportProtocol::Udp:
            return "UDP";
    }

    return "";
}


class ISocket
{
public:
    virtual ~ISocket() = default;

    ///
    /// \brief Describes how the data is transmitted
    ///
    enum class Mode
    {
        Stream,
        Datagram
    };

    virtual Mode getMode() const = 0;

    /**
    * Return the maximum size of the payload supported by one packet.
    */
    virtual uint16_t maxPayload() const = 0;

    virtual bool isOpened() = 0;
    virtual void close()    = 0;

    virtual void setInputBufferSize(uint32_t size) = 0;
    virtual bool checkInputBuffer()                = 0;
    virtual void setTimeout(uint16_t timeout)      = 0;

    /**
    * @brief open the socket and create a connection when a remoteIpAddr is provided.
    * @note You need to provide remoteIpAddr for the Tcp protocol
    * @param localPort
    * @param remotePort
    * @param remoteIpAddr
    * @param timeout
    */
    virtual void open(uint16_t localPort, uint16_t remotePort, ipAddress_t remoteIpAddr, uint16_t timeout) = 0;

    /**
    * Send an arbitrary buffer to the remote device.
    *
    * @param buffer a buffer of the specified length
    * @param length number of bytes to be written
    */
    virtual void send(const uint8_t buffer[], uint16_t length) = 0;

    /**
    * Receive data from the remote device.
    * If there is no data to read, the function returns immediately and length is 0.
    * Otherwise, the function returns up to maxLength bytes.
    * If the buffer is smaller than the packet, the remainder of the packet will be lost.
    *
    * @param buffer a buffer of the specified maximum length
    * @param length maximum number of bytes to be read (size of buffer)
    * @return the actual number of bytes received
    */
    virtual uint16_t receive(uint8_t buffer[], uint16_t length) = 0;

    virtual bool dumpPacket() = 0;
};
