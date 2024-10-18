/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "SerialPortImpl.hpp"

#include <common/Logger.hpp>
#include <platform/exception/EConnection.hpp>
#include <platform/exception/EInUse.hpp>


SerialPortImpl::SerialPortImpl() :
    m_comDev {NULL},
    m_dcb {}
{
    m_dcb.DCBlength = sizeof(DCB);

    m_dcb.fBinary  = 1;
    m_dcb.ByteSize = 8;
    m_dcb.Parity   = NOPARITY;
    m_dcb.StopBits = ONESTOPBIT;
}

SerialPortImpl::~SerialPortImpl()
{
    SerialPortImpl::close();
}

bool SerialPortImpl::isOpened()
{
    return (m_comDev != NULL);
}

void SerialPortImpl::open(const char port[], uint32_t baudrate, uint16_t timeout)
{
    if (isOpened())
    {
        return;
    }
    LOG(DEBUG) << "Opening SerialPort ...";

    std::string devPath("\\\\.\\");
    devPath.append(port);
    m_comDev = CreateFile(devPath.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

    if (m_comDev == NULL)
    {
        throw EConnection("SerialPortImpl::open - CreateFile() failed");
    }

    m_dcb.BaudRate = baudrate;
    if (!SetCommState(m_comDev, &m_dcb))
    {
        CloseHandle(m_comDev);
        m_comDev = NULL;
        throw EInUse();
        //throw EConnection("SerialPortImpl::open - SetCommState() failed", static_cast<int>(GetLastError()));
    }

    if (!SetCommMask(m_comDev, 0))
    {
        CloseHandle(m_comDev);
        m_comDev = NULL;
        throw EConnection("SetCommMask failed", static_cast<int>(GetLastError()));
    }

    setTimeout(timeout);
}

void SerialPortImpl::close()
{
    if (!SerialPortImpl::isOpened())
    {
        return;
    }

    LOG(DEBUG) << "Closing SerialPort";
}

void SerialPortImpl::setTimeout(uint16_t timeout)
{
    COMMTIMEOUTS CommTimeOuts;
    CommTimeOuts.ReadIntervalTimeout         = 100;  // return when for 100ms there hasn't been a new byte received
    CommTimeOuts.ReadTotalTimeoutMultiplier  = 0;
    CommTimeOuts.ReadTotalTimeoutConstant    = timeout;
    CommTimeOuts.WriteTotalTimeoutMultiplier = 0;
    CommTimeOuts.WriteTotalTimeoutConstant   = timeout;
    if (!SetCommTimeouts(m_comDev, &CommTimeOuts))
    {
        CloseHandle(m_comDev);
        m_comDev = NULL;
        throw EConnection("SerialPortImpl::setTimeout - SetCommTimeouts() failed", static_cast<int>(GetLastError()));
    }
}

void SerialPortImpl::clearInputBuffer()
{
    PurgeComm(m_comDev, PURGE_RXABORT | PURGE_RXCLEAR);
}

void SerialPortImpl::send(const uint8_t buffer[], uint16_t length)
{
    DWORD dwBytesTransferred;

    const BOOL bFileStatus = WriteFile(m_comDev, buffer, length, &dwBytesTransferred, NULL);
    if (!bFileStatus)
    {
        throw EConnection("SerialPortImpl::send - WriteFile() failed", static_cast<int>(GetLastError()));
    }
    if (dwBytesTransferred != length)
    {
        throw EConnection("SerialPortImpl::send - WriteFile() incomplete", static_cast<int>((dwBytesTransferred << 16) | length));
    }
}

uint16_t SerialPortImpl::readInputBuffer(uint8_t buffer[], uint16_t length)
{
    DWORD dwBytesTransferred = 0;

    const BOOL bFileStatus = ReadFile(m_comDev, buffer, length, &dwBytesTransferred, NULL);
    if (!bFileStatus)
    {
        throw EConnection("SerialPortImpl::readInputBuffer - ReadFile() failed", static_cast<int>(GetLastError()));
    }

    return static_cast<uint16_t>(dwBytesTransferred);
}
