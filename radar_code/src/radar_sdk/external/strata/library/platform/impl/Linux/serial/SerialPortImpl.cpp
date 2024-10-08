/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "SerialPortImpl.hpp"

#include <platform/exception/EConnection.hpp>


namespace
{
    speed_t getBaudrateFlags(uint32_t baudrate)
    {
        switch (baudrate)
        {
            case 9600:
                return B9600;
            case 19200:
                return B19200;
            case 38400:
                return B38400;
            case 57600:
                return B57600;
            case 115200:
                return B115200;
            case 230400:
                return B230400;
            case 460800:
                return B460800;
            case 500000:
                return B500000;
            case 576000:
                return B576000;
            case 921600:
                return B921600;
            case 1000000:
                return B1000000;
            case 1152000:
                return B1152000;
            case 1500000:
                return B1500000;
            case 2000000:
                return B2000000;
            case 2500000:
                return B2500000;
            case 3000000:
                return B3000000;
            case 3500000:
                return B3500000;
            case 4000000:
                return B4000000;
            default:
                throw EConnection("SerialPortImpl::getBaudrateFlags - baud rate conversion failed", baudrate);
        }
    }

}

void SerialPortImpl::setBaudrate(struct termios *serialPortSettings, uint32_t baudrate)
{
    const auto br = getBaudrateFlags(baudrate);

    cfsetispeed(serialPortSettings, br);
    cfsetospeed(serialPortSettings, br);
}
