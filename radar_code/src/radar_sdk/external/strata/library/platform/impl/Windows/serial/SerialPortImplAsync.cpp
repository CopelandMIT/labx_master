/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "SerialPortImplAsync.hpp"

#include <common/Logger.hpp>
#include <platform/exception/EConnection.hpp>
#include <platform/exception/EInUse.hpp>

#include <algorithm>


SerialPortImplAsync::SerialPortImplAsync() :
    m_comDev {NULL},
    m_dcb {},
    m_overlappedWrite {}
{
    m_dcb.DCBlength = sizeof(DCB);

    m_dcb.fBinary  = 1;
    m_dcb.ByteSize = 8;
    m_dcb.Parity   = NOPARITY;
    m_dcb.StopBits = ONESTOPBIT;
}

SerialPortImplAsync::~SerialPortImplAsync()
{
    SerialPortImplAsync::close();
}

bool SerialPortImplAsync::isOpened()
{
    return (m_comDev != NULL);
}

void SerialPortImplAsync::open(const char port[], uint32_t baudrate, uint16_t timeout)
{
    if (isOpened())
    {
        return;
    }
    LOG(DEBUG) << "Opening SerialPort ...";

    std::string devPath("\\\\.\\");
    devPath.append(port);
    m_comDev = CreateFile(devPath.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

    if (m_comDev == NULL)
    {
        throw EConnection("SerialPortImplAsync::open - CreateFile() failed");
    }

    m_dcb.BaudRate = baudrate;
    if (!SetCommState(m_comDev, &m_dcb))
    {
        CloseHandle(m_comDev);
        m_comDev = NULL;
        throw EInUse();
        //throw EConnection("SerialPortImplAsync::open - SetCommState() failed", static_cast<int>(GetLastError()));
    }

    if (!SetCommMask(m_comDev, 0))
    {
        CloseHandle(m_comDev);
        m_comDev = NULL;
        throw EConnection("SetCommMask failed", static_cast<int>(GetLastError()));
    }

    // never time out for asynchronous implementation, this is handled by event timeouts instead
    COMMTIMEOUTS CommTimeOuts;
    CommTimeOuts.ReadIntervalTimeout         = 1;  // return when for 1ms there hasn't been a new byte received
    CommTimeOuts.ReadTotalTimeoutMultiplier  = 0;
    CommTimeOuts.ReadTotalTimeoutConstant    = 0;
    CommTimeOuts.WriteTotalTimeoutMultiplier = 0;
    CommTimeOuts.WriteTotalTimeoutConstant   = 0;
    if (!SetCommTimeouts(m_comDev, &CommTimeOuts))
    {
        CloseHandle(m_comDev);
        m_comDev = NULL;
        throw EConnection("SerialPortImplAsync::open - SetCommTimeouts() failed", static_cast<int>(GetLastError()));
    }

    m_overlappedWrite.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    setTimeout(timeout);
    setBuffers(16, 32 * 1024);
}

void SerialPortImplAsync::close()
{
    if (!SerialPortImplAsync::isOpened())
    {
        return;
    }

    LOG(DEBUG) << "Closing SerialPort";

    CancelIo(m_comDev);

    if (m_overlappedWrite.hEvent != NULL)
    {
        CloseHandle(m_overlappedWrite.hEvent);
    }

    CloseHandle(m_comDev);
    m_comDev = NULL;
}

void SerialPortImplAsync::setTimeout(uint16_t timeout)
{
    m_timeout = timeout;
}

void SerialPortImplAsync::clearInputBuffer()
{
    PurgeComm(m_comDev, PURGE_RXABORT | PURGE_RXCLEAR);

    // cancel all queued reads on handle
    CancelIo(m_comDev);

    // re-queue all overlapped buffers
    auto count = m_buffers.size();
    while (count--)
    {
        queueRead();
    }
}

void SerialPortImplAsync::send(const uint8_t buffer[], uint16_t length)
{
    const BOOL bFileStatus = WriteFile(m_comDev, buffer, length, NULL, &m_overlappedWrite);
    if (!bFileStatus)
    {
        const DWORD lastError = GetLastError();
        if (lastError != ERROR_IO_PENDING)
        {
            throw EConnection("SerialPortImplAsync::send - WriteFile() failed", static_cast<int>(lastError));
        }
        else
        {
            const DWORD wait = WaitForSingleObjectEx(m_overlappedWrite.hEvent, m_timeout, TRUE);
            switch (wait)
            {
                case WAIT_OBJECT_0:
                    // wait succeeded, just continue
                    break;
                case WAIT_TIMEOUT:
                    throw EConnection("SerialPortImplAsync::send - timed out");
                    break;
                case WAIT_FAILED:
                    throw EConnection("SerialPortImplAsync::send - WaitForSingleObjectEx() returned an error", static_cast<int>(GetLastError()));
                    break;
                default:
                    throw EConnection("SerialPortImplAsync::send - WaitForSingleObjectEx() failed", static_cast<int>(wait));
                    break;
            }
        }
    }

    DWORD dwBytesTransferred;
    const BOOL bOverlappedStatus = GetOverlappedResult(m_comDev, &m_overlappedWrite, &dwBytesTransferred, FALSE);
    if (!bOverlappedStatus)
    {
        throw EConnection("SerialPortImplAsync::send - GetOverlappedResult() failed", static_cast<int>(GetLastError()));
    }
    if (dwBytesTransferred != length)
    {
        throw EConnection("SerialPortImplAsync::send - WriteFile() incomplete", static_cast<int>((dwBytesTransferred << 16) | length));
    }
}

uint16_t SerialPortImplAsync::readInputBuffer(uint8_t buffer[], uint16_t length)
{
    auto tries               = m_buffers.size();
    DWORD dwLength           = length;
    DWORD dwBytesTransferred = 0;

    while (tries--)
    {
        auto &b = m_buffers[m_bufferIndex];

        // check if overlapped buffer result has been checked
        if (!m_resultChecked)
        {
            DWORD wait = WaitForSingleObjectEx(b.overlapped.hEvent, m_timeout, TRUE);
            if (wait != WAIT_OBJECT_0)
            {
                // if overlapped result is not signalled, no data has been received, so return immediately
                break;
            }
            const BOOL bOverlappedStatus = GetOverlappedResult(m_comDev, &b.overlapped, &b.dwBytesTransferred, FALSE);
            if (!bOverlappedStatus)
            {
                // overlapped buffer has been signalled, get result and re-queue
                const DWORD lastError = GetLastError();
                queueRead();
                if (lastError == ERROR_OPERATION_ABORTED)
                {
                    // this can occur when clearing the input buffer, so this is normally not really a problem
                    // if it happens unexpectedly and data is actually lost, the mismatch will anyways be detected by the protocol handling
                    continue;
                }
                LOG(DEBUG) << "SerialPortImplAsync::readInputBuffer - GetOverlappedResult() failed: " << lastError;
                break;
            }
            m_resultChecked = true;
        }

        // read bytes from this buffer up to dwLength
        const DWORD remainingBuffer = b.dwBytesTransferred - b.dwOffset;
        const DWORD bytes           = std::min(remainingBuffer, dwLength);
        if (bytes)
        {
            const auto prevOffset = b.dwOffset;
            b.dwOffset += bytes;
            // copy to consumer buffer
            std::copy(&b.buffer[prevOffset], &b.buffer[b.dwOffset], buffer);

            buffer += bytes;
            dwBytesTransferred += bytes;
            dwLength -= bytes;
        }

        if (b.dwBytesTransferred == b.dwOffset)
        {
            // buffer content has been completely consumed, so queue it again
            queueRead();
        }
        if (!dwLength)
        {
            // read request has been completely filled at this point, so we can return
            break;
        }
        // otherwise, continue with next overlapped buffer
    }

    return static_cast<uint16_t>(dwBytesTransferred);
}

void SerialPortImplAsync::setBuffers(uint32_t count, uint32_t size)
{
    // todo: if this should be changeable during open connection,
    // cancel already queued transactions
    //CancelIo(m_comDev);

    m_bufferIndex = 0;
    m_buffers.resize(count);
    for (auto &b : m_buffers)
    {
        b.buffer.resize(size);
        queueRead();
    }
}

void SerialPortImplAsync::queueRead()
{
    // queue current buffer and advance index to the oldest unchecked buffer
    auto &b         = m_buffers[m_bufferIndex];
    m_bufferIndex   = (m_bufferIndex + 1) % m_buffers.size();
    m_resultChecked = false;

    b.dwOffset           = 0;
    b.dwBytesTransferred = 0;

    BOOL bFileStatus = ReadFile(m_comDev, b.buffer.data(), static_cast<DWORD>(b.buffer.size()), &b.dwBytesTransferred, &b.overlapped);
    if (!bFileStatus)
    {
        const DWORD lastError = GetLastError();
        if (lastError != ERROR_IO_PENDING)
        {
            if (lastError == ERROR_BAD_COMMAND)
            {
                // This error occurs if the board was unplugged.
                throw EConnection("SerialPortImplAsync::queueRead - ReadFile() failed: ERROR_BAD_COMMAND (device probably unplugged)");
            }

            LOG(DEBUG) << "SerialPortImplAsync::queueRead - ReadFile() failed: " << lastError;
        }
    }
}
