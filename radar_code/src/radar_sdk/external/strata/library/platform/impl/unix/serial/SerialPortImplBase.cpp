/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "SerialPortImplBase.hpp"

#include <common/Logger.hpp>
#include <platform/exception/EConnection.hpp>

#include <errno.h>   // ERROR number defintions
#include <fcntl.h>   // File control definitions
#include <unistd.h>  // UNIX Standard definitions


#define INVALID_PORT -1


SerialPortImplBase::SerialPortImplBase() :
    m_fd {INVALID_PORT, POLLIN, 0}
{
}

SerialPortImplBase::~SerialPortImplBase()
{
    close();
}

bool SerialPortImplBase::isOpened()
{
    return (m_fd.fd != INVALID_PORT);
}

void SerialPortImplBase::open(const char port[], uint32_t baudrate, uint16_t timeout)
{
    if (isOpened())
    {
        return;
    }
    LOG(DEBUG) << "Opening SerialPort ...";

    // Read+Write / No termal will control process
    m_fd.fd = ::open(port, O_RDWR | O_NOCTTY);
    if (m_fd.fd == INVALID_PORT)
    {
        throw EConnection("SerialPortImplBase::open - open() failed", errno);
    }

    int ret;
    struct termios serialPortSettings;

    ret = tcgetattr(m_fd.fd, &serialPortSettings);
    if (ret < 0)
    {
        close();
        throw EConnection("SerialPortImplBase::open - tcgetattr() failed", errno);
    }

    //--------------------------------------------------------------------------------
    // SETTINGS
    //--------------------------------------------------------------------------------
    setBaudrate(&serialPortSettings, baudrate);  // read speed

    serialPortSettings.c_cflag &= static_cast<unsigned int>(~(PARENB | CSTOPB | CRTSCTS | CSIZE));  // no parity, 1 stopbit, clear CS mask
    serialPortSettings.c_cflag |= static_cast<unsigned int>(CREAD | CLOCAL | CS8);                  // enable receiving, 8 databits
    serialPortSettings.c_lflag     = 0u;                                                            // raw mode (non-canonical mode)
    serialPortSettings.c_iflag     = 0u;                                                            // no input flags
    serialPortSettings.c_oflag     = 0u;                                                            // no output flags
    serialPortSettings.c_cc[VMIN]  = 0;
    serialPortSettings.c_cc[VTIME] = 1;  // return when for 100ms there hasn't been a new byte received while still expecting bytes

    ret = tcsetattr(m_fd.fd, TCSANOW, &serialPortSettings);  // apply settings immediately
    if (ret < 0)
    {
        close();
        throw EConnection("SerialPortImplBase::open - tcsetattr() failed", errno);
    }

    // Flush the input buffer
    clearInputBuffer();

    m_timeout = timeout;
}

void SerialPortImplBase::close()
{
    if (!isOpened())
    {
        return;
    }

    LOG(DEBUG) << "Closing SerialPort";
    ::close(m_fd.fd);
    m_fd.fd = INVALID_PORT;
}

void SerialPortImplBase::setTimeout(uint16_t timeout)
{
    m_timeout = timeout;
}

void SerialPortImplBase::clearInputBuffer()
{
    const int ret = tcflush(m_fd.fd, TCIFLUSH);
    if (ret < 0)
    {
        throw EConnection("SerialPortImplBase::clearInputBuffer - tcflush() failed", errno);
    }
}

void SerialPortImplBase::send(const uint8_t buffer[], uint16_t length)
{
    const ssize_t ret = ::write(m_fd.fd, buffer, length);
    if (ret != length)
    {
        throw EConnection("SerialPortImplBase::send - write() failed", static_cast<int>(ret));
    }
}

uint16_t SerialPortImplBase::readInputBuffer(uint8_t buffer[], uint16_t length)
{
    if (length == 0)
    {
        return 0;
    }

    const int ret = ::poll(&m_fd, 1, m_timeout);
    if (ret < 0)
    {
        throw EConnection("SerialPortImplBase::receiveInputBuffer - poll() failed", errno);
    }
    if (ret == 0)
    {
        return 0;
    }

    const ssize_t count = ::read(m_fd.fd, buffer, length);
    if ((count == 0) && (ret > 0))
    {
        throw EConnection("SerialPortImplBase::readInputBuffer - poll() returned event but read() did not return data");
    }
    if (count < 0)
    {
        throw EConnection("SerialPortImplBase::readInputBuffer - read() failed", errno);
    }

    return static_cast<uint16_t>(count);
}
