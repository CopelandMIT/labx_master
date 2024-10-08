/**
 * @copyright 2020 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "Board.hpp"


IComponent *Board::getIComponent(uint16_t type, uint8_t id)
{
    return getInstance<IComponent>(type, id);
}

IModule *Board::getIModule(uint16_t type, uint8_t id)
{
    return getInstance<IModule>(type, id);
}

uint8_t Board::getIModuleCount(uint16_t type)
{
    return getCount<IModule>(type);
}

uint8_t Board::getIComponentCount(uint16_t type)
{
    return getCount<IComponent>(type);
}
