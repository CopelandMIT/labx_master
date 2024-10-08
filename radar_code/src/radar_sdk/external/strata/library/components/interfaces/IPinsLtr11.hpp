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


class IPinsLtr11
{
public:
    virtual ~IPinsLtr11() = default;

    /**
    * Sets the state of the radar reset pin. 
    * The reset signal of the connected Ltr11 device must be driven low
    * and kept low for at least 1ms, before going HIGH again.
    */
    virtual void setResetPin(bool state) = 0;

    /**
    * Performs a hard reset to the Ltr11 chip. 
    */
    virtual void reset() = 0;

    /**
    * Gets states of target detect and phase detect pins of the Ltr11
    */
    virtual uint8_t getDetectionPins() = 0;
};
