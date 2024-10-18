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
#include <components/interfaces/IPinsAvian.hpp>
#include <components/interfaces/IProtocolAvian.hpp>
#include <components/interfaces/IRegisters.hpp>
#include <universal/components/radar.h>


/**
 * \brief Access interface to a radar front end device of the Avian family,
 * e.g. BGT60TR13C, BGT60ATR24C, BGT60TR13D and BGT60TR12E.
 */
class IRadarAvian :
    public IComponent
{
public:
    static constexpr uint16_t getType()
    {
        return COMPONENT_TYPE_RADAR_AVIAN;
    }

    virtual ~IRadarAvian() = default;

    virtual void initialize()                 = 0;
    virtual void reset(bool softReset = true) = 0;
    virtual void startData()                  = 0;
    virtual void stopData()                   = 0;
    virtual uint8_t getDataIndex()            = 0;

    virtual IRegisters<uint8_t, uint32_t> *getIRegisters() = 0;
    virtual IPinsAvian *getIPinsAvian()                    = 0;
    virtual IProtocolAvian *getIProtocolAvian()            = 0;
};
