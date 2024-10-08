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
#include <components/interfaces/IPinsLtr11.hpp>
#include <components/interfaces/IProtocolLtr11.hpp>
#include <components/interfaces/IRegisters.hpp>
#include <universal/components/radar.h>


/**
 * \brief Access interface to a radar front end device of type Ltr11,
 * e.g. 
 */
class IRadarLtr11 :
    public IComponent
{
public:
    static constexpr uint16_t getType()
    {
        return COMPONENT_TYPE_RADAR_LTR11;
    }

    virtual ~IRadarLtr11() = default;

    virtual void initialize()                 = 0;
    virtual void reset(bool softReset = true) = 0;
    virtual uint8_t getDataIndex()            = 0;

    virtual IRegisters<uint8_t, uint16_t> *getIRegisters() = 0;
    virtual IPinsLtr11 *getIPinsLtr11()                    = 0;
    virtual IProtocolLtr11 *getIProtocolLtr11()            = 0;
};
