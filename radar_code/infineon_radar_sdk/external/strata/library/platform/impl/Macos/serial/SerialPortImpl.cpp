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


void SerialPortImpl::setBaudrate(struct termios *serialPortSettings, uint32_t baudrate)
{
    cfsetispeed(serialPortSettings, baudrate);
    cfsetospeed(serialPortSettings, baudrate);
}
