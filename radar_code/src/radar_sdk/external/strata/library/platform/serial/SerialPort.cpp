/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "SerialPort.hpp"

#include <cstring>


using namespace strata;


uint16_t SerialPort::receive(uint8_t buffer[], uint16_t length)
{
    uint16_t count = 0;

    do
    {
        const uint16_t size = readInputBuffer(buffer + count, length - count);
        if (size == 0)
        {
            break;
        }
        count += size;
    } while (count < length);

    return count;
}

void SerialPort::sendString(const char data[])
{
    send(reinterpret_cast<const uint8_t *>(data), static_cast<uint16_t>(strlen(data)));
}
