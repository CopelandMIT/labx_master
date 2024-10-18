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

#include <windows.h>

#include <common/Buffer.hpp>
#include <vector>


using namespace strata;


class SerialPortImplAsync :
    public SerialPort
{
public:
    SerialPortImplAsync();
    ~SerialPortImplAsync();

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
    HANDLE m_comDev;
    DCB m_dcb;


    void setBuffers(uint32_t count, uint32_t size);
    void queueRead();

    OVERLAPPED m_overlappedWrite;
    DWORD m_timeout;


    class OverlappedBuffer
    {
    public:
        OverlappedBuffer() :
            overlapped {},
            dwOffset {0},
            dwBytesTransferred {0}
        {
            overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        }
        ~OverlappedBuffer()
        {
            CloseHandle(overlapped.hEvent);
        }

        OverlappedBuffer(const OverlappedBuffer &) = delete;
        OverlappedBuffer &operator=(const OverlappedBuffer &) = delete;
        OverlappedBuffer &operator=(OverlappedBuffer &&other) = delete;

        OverlappedBuffer(OverlappedBuffer &&other) noexcept
        {
            buffer             = std::move(other.buffer);
            overlapped         = other.overlapped;
            dwOffset           = other.dwOffset;
            dwBytesTransferred = other.dwBytesTransferred;
        }

        strata::buffer<uint8_t> buffer;
        OVERLAPPED overlapped;
        DWORD dwOffset;
        DWORD dwBytesTransferred;
    };

    std::vector<OverlappedBuffer> m_buffers;
    unsigned int m_bufferIndex;
    bool m_resultChecked;
};
