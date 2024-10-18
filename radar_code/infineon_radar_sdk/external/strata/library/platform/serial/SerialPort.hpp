/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <platform/interfaces/link/ISerialPort.hpp>

namespace strata
{

    class SerialPort :
        public ISerialPort
    {
    public:
        //ISerialPort
        uint16_t receive(uint8_t buffer[], uint16_t length) override;

        void sendString(const char data[]);

    protected:
        virtual uint16_t readInputBuffer(uint8_t buffer[], uint16_t length) = 0;
    };

};
