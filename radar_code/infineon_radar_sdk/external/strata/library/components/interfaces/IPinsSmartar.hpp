/**
 * @copyright 2020 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <cstdint>

typedef struct
{
    uint16_t gpioReset;
} IPinsSmartarConfig_t;


class IPinsSmartar
{
public:
    virtual ~IPinsSmartar() = default;

    /**
     * The reset signal of the connected Smartar device must be driven low
     * and kept low for at least 10ns, before going HIGH again.
     */
    virtual void setResetPin(bool state) = 0;

    virtual void reset() = 0;
};
