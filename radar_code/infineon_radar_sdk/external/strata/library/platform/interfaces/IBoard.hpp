/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <components/interfaces/IComponent.hpp>
#include <modules/interfaces/IModule.hpp>

#include <cstdint>


class IBoard
{
public:
    virtual ~IBoard() = default;

    virtual IModule *getIModule(uint16_t type, uint8_t id)       = 0;
    virtual IComponent *getIComponent(uint16_t type, uint8_t id) = 0;

    virtual uint8_t getIModuleCount(uint16_t type)    = 0;
    virtual uint8_t getIComponentCount(uint16_t type) = 0;

    template <class ComponentType>
    ComponentType *getComponent(uint8_t id)
    {
        return dynamic_cast<ComponentType *>(getIComponent(ComponentType::getType(), id));
    }

    template <class ModuleType>
    ModuleType *getModule(uint8_t id = 0)
    {
        return dynamic_cast<ModuleType *>(getIModule(ModuleType::getType(), id));
    }

    template <class ComponentType>
    uint8_t getComponentCount()
    {
        return getIComponentCount(ComponentType::getType());
    }

    template <class ModuleType>
    uint8_t getModuleCount()
    {
        return getIModuleCount(ModuleType::getType());
    }
};
