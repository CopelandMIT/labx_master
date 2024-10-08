/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once
#include <platform/serial/SerialPort.hpp>

#include <poll.h>
#include <termios.h>


using namespace strata;


class SerialPortImplBase :
    public SerialPort
{
public:
    SerialPortImplBase();
    ~SerialPortImplBase();

    //ISerialPort
    bool isOpened() override;
    void open(const char port[], uint32_t baudrate, uint16_t timeout) override;
    void close() override;

    void setTimeout(uint16_t timeout) override;
    void clearInputBuffer() override;

    void send(const uint8_t buffer[], uint16_t length) override;

protected:
    uint16_t readInputBuffer(uint8_t buffer[], uint16_t length) override;

private:
    virtual void setBaudrate(struct termios *serialPortSettings, uint32_t baudrate) = 0;

    struct pollfd m_fd;
    int m_timeout;
};
