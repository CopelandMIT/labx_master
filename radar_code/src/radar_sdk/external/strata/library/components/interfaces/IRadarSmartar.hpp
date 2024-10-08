#pragma once

#include <components/interfaces/IComponent.hpp>
#include <components/interfaces/IPinsSmartar.hpp>
#include <components/interfaces/IProtocolSmartar.hpp>
#include <components/interfaces/IRegisters.hpp>
#include <universal/components/radar.h>


/**
 * \brief Access interface to a radar front end device of type Smartar (CMOS).
 */
class IRadarSmartar :
    public IComponent
{
public:
    static constexpr uint16_t getType()
    {
        return COMPONENT_TYPE_RADAR_SMARTAR;
    }

    virtual ~IRadarSmartar() = default;

    virtual IRegisters<uint16_t, uint32_t> *getIRegisters() = 0;
    virtual IPinsSmartar *getIPinsSmartar()                 = 0;
    virtual IProtocolSmartar *getIProtocolSmartar()         = 0;
    virtual uint8_t getDataIndex()                          = 0;
    virtual void reset(bool softReset = false)              = 0;
};
