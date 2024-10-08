/**
 * @copyright 2018 Infineon Technologies
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
    uint16_t gpioTrigger;
} IPinsIrsConfig_t;


class IPinsIrs
{
public:
    virtual ~IPinsIrs() = default;

    virtual void setResetPin(bool state)   = 0;
    virtual void setTriggerPin(bool state) = 0;
};
