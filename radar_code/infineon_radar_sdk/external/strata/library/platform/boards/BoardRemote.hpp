/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <common/cpp11/memory.hpp>
#include <platform/boards/Board.hpp>
#include <platform/interfaces/IBridge.hpp>

#include <memory>
#include <vector>


class BoardRemote :
    public Board
{
public:
    BoardRemote(IBridge *bridge);

    IComponent *getIComponent(uint16_t type, uint8_t id) override;
    IModule *getIModule(uint16_t type, uint8_t id) override;

    uint8_t getIModuleCount(uint16_t type) override;
    uint8_t getIComponentCount(uint16_t type) override;

protected:
    IComponent *createComponent(uint16_t type, uint8_t id);
    IModule *createModule(uint16_t type, uint8_t id);

    virtual IComponent *createComponentFunction(uint16_t type, uint8_t id);
    virtual IModule *createModuleFunction(uint16_t type, uint8_t id);

    template <typename T>
    inline IComponent *createComponentHelper(uint8_t id)
    {
        auto instance = std::make_unique<T>(m_commands, id);
        auto ptr      = instance.get();
        m_componentInstances.emplace_back(std::move(instance));
        registerInstance(*ptr, id);
        return ptr;
    }

    template <typename T>
    inline IModule *createModuleHelper(uint8_t id)
    {
        auto instance = std::make_unique<T>(m_bridge, id);
        auto ptr      = instance.get();
        m_moduleInstances.emplace_back(std::move(instance));
        registerInstance(*ptr, id);
        return ptr;
    }

private:
    IBridge *m_bridge;
    IVendorCommands *m_commands;

    std::vector<std::unique_ptr<IComponent>> m_componentInstances;
    std::vector<std::unique_ptr<IModule>> m_moduleInstances;

    inline decltype(m_componentInstances) &get_map(IComponent *)
    {
        return m_componentInstances;
    }
    inline decltype(m_moduleInstances) &get_map(IModule *)
    {
        return m_moduleInstances;
    }
};
