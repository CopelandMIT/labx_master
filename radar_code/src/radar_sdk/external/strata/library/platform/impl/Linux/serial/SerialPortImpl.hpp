/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once
#include "../unix/serial/SerialPortImplBase.hpp"

class SerialPortImpl :
    public SerialPortImplBase
{
public:
    SerialPortImpl()  = default;
    ~SerialPortImpl() = default;

protected:
    void setBaudrate(struct termios *serialPortSettings, uint32_t baudrate) override;
};
