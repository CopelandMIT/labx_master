/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "BoardWiggler.hpp"
#include "BridgeWiggler.hpp"

#include <common/Logger.hpp>
#include <platform/BoardListProtocol.hpp>
#include <platform/templates/searchBoardFunction.hpp>


std::unique_ptr<BoardDescriptor> BoardWiggler::searchBoard(uint8_t systemIndex, BoardData::const_iterator begin, BoardData::const_iterator end)
{
    LOG(DEBUG) << "Looking for device with index " << systemIndex << " ...";

    return searchBoardFunctionBridge<BridgeWiggler>(begin, end, systemIndex);
}

std::unique_ptr<BoardInstance> BoardWiggler::createBoardInstance(uint8_t systemIndex)
{
    return searchBoard(systemIndex, BoardListProtocol::begin, BoardListProtocol::end)->createBoardInstance();
}
