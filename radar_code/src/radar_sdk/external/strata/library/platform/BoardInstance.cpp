/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "BoardInstance.hpp"


BoardInstance::BoardInstance(std::shared_ptr<IBridge> bridge, std::unique_ptr<IBoard> board, const std::string &name, uint16_t vid, uint16_t pid) :
    m_bridge {std::move(bridge)},
    m_board {std::move(board)},
    m_vid {vid},
    m_pid {pid},
    m_name {name}
{
}

IModule *BoardInstance::getIModule(uint16_t type, uint8_t id)
{
    return m_board->getIModule(type, id);
}

IComponent *BoardInstance::getIComponent(uint16_t type, uint8_t id)
{
    return m_board->getIComponent(type, id);
}

uint8_t BoardInstance::getIModuleCount(uint16_t type)
{
    return m_board->getIModuleCount(type);
}

uint8_t BoardInstance::getIComponentCount(uint16_t type)
{
    return m_board->getIComponentCount(type);
}
