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
    uint16_t gpioChipSelect;
    uint16_t gpioIrq;
} IPinsAvianConfig_t;


class IPinsAvian
{
public:
    virtual ~IPinsAvian() = default;

    /**
     * Because the reset input of the Avian device is multiplexed with the quad
     * SPI data line DIO3, the SPI CS signal must be HIGH all time during a reset
     * condition.
     *
     * The reset signal of the connected Avian device must be driven low
     * and kept low for at least 1000ns, before going HIGH again.
     */
    virtual void setResetPin(bool state) = 0;
    virtual bool getIrqPin()             = 0;

    virtual void reset() = 0;
};
