/**
 * @copyright 2020 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <components/interfaces/IComponent.hpp>
#include <components/interfaces/IProtocolAtr22.hpp>
#include <components/interfaces/IRegisters.hpp>
#include <universal/components/radar.h>


/**
 * \brief Access interface to a radar front end device of the Atr22 family,
 * e.g. 
 */
class IRadarAtr22 :
    public IComponent
{
public:
    static constexpr uint16_t getType()
    {
        return COMPONENT_TYPE_RADAR_ATR22;
    }

    virtual ~IRadarAtr22() = default;

    virtual void initialize()                 = 0;
    virtual void reset(bool softReset = true) = 0;
    virtual uint8_t getDataIndex()            = 0;

    virtual IRegisters<uint16_t, uint16_t> *getIRegisters() = 0;
    virtual IProtocolAtr22 *getIProtocolAtr22()             = 0;
};
