/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "BoardRemote.hpp"

#include <common/exception/ENotImplemented.hpp>
#include <platform/exception/EConnection.hpp>
#include <platform/exception/EProtocolFunction.hpp>
#include <remote/RemoteGasBoyle.hpp>
#include <remote/RemoteProcessingRadar.hpp>
#include <remote/RemoteRadarAtr22.hpp>
#include <remote/RemoteRadarAvian.hpp>
#include <remote/RemoteRadarLtr11.hpp>
#include <remote/RemoteRadarSmartar.hpp>
#include <universal/components/subinterfaces.h>
#include <universal/modules/subinterfaces.h>
#include <universal/protocol/protocol_definitions.h>


BoardRemote::BoardRemote(IBridge *bridge) :
    m_bridge {bridge}
{
    m_commands = bridge->getIBridgeControl()->getIVendorCommands();
    if (m_commands == nullptr)
    {
        throw EConnection("IVendorCommands not implemented in this Bridge");
    }
}

IComponent *BoardRemote::createComponent(uint16_t type, uint8_t id)
{
    return createComponentFunction(type, id);
}

IModule *BoardRemote::createModule(uint16_t type, uint8_t id)
{
    return createModuleFunction(type, id);
}

IComponent *BoardRemote::createComponentFunction(uint16_t type, uint8_t id)
{
    switch (type)
    {
        case COMPONENT_TYPE_RADAR_AVIAN:
            return createComponentHelper<RemoteRadarAvian>(id);
            break;
        case COMPONENT_TYPE_RADAR_ATR22:
            return createComponentHelper<RemoteRadarAtr22>(id);
            break;
        case COMPONENT_TYPE_RADAR_LTR11:
            return createComponentHelper<RemoteRadarLtr11>(id);
            break;
        case COMPONENT_TYPE_PROCESSING_RADAR:
            return createComponentHelper<RemoteProcessingRadar>(id);
            break;
        case COMPONENT_TYPE_RADAR_SMARTAR:
            return createComponentHelper<RemoteRadarSmartar>(id);
            break;
        case COMPONENT_TYPE_GAS_BOYLE:
            return createComponentHelper<RemoteGasBoyle>(id);
            break;
        default:
            break;
    }

    return nullptr;
}

IModule *BoardRemote::createModuleFunction(uint16_t /*type*/, uint8_t /*id*/)
{
    // no general modules available yet
    return nullptr;
}

IComponent *BoardRemote::getIComponent(uint16_t type, uint8_t id)
{
    auto entry = getInstance<IComponent>(type, id);
    if (entry)
    {
        return entry;
    }
    else
    {
        // we haven't found the component, so try to create a remote instance
        return createComponent(type, id);
    }
}

IModule *BoardRemote::getIModule(uint16_t type, uint8_t id)
{
    auto entry = getInstance<IModule>(type, id);
    if (entry)
    {
        return entry;
    }
    else
    {
        // we haven't found the module, so try to create a remote instance
        return createModule(type, id);
    }
}

uint8_t BoardRemote::getIModuleCount(uint16_t type)
{
    uint8_t count;
    m_commands->vendorReadParameters(CMD_MODULE, 0, type, count);
    return count;
}

uint8_t BoardRemote::getIComponentCount(uint16_t type)
{
    uint8_t count;
    m_commands->vendorReadParameters(CMD_COMPONENT, 0, type, count);
    return count;
}
