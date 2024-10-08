/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <platform/boards/Board.hpp>
#include <platform/interfaces/IBridge.hpp>


class BoardGeneric :
    public Board

{
public:
    BoardGeneric(IBridge *bridge);

protected:
    IBridge *m_bridge;
};
