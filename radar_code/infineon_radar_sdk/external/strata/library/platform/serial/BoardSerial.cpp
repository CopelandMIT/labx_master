/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "BoardSerial.hpp"
#include "BridgeSerial.hpp"

#include <common/Logger.hpp>
#include <platform/BoardListProtocol.hpp>
#include <platform/templates/searchBoardFunction.hpp>


std::unique_ptr<BoardDescriptor> BoardSerial::searchBoard(const char port[], BoardData::const_iterator begin, BoardData::const_iterator end)
{
    LOG(DEBUG) << "Looking for board on " << port << " ...";

    return searchBoardFunctionBridge<BridgeSerial>(begin, end, port);
}

std::unique_ptr<BoardInstance> BoardSerial::createBoardInstance(const char port[])
{
    return searchBoard(port, BoardListProtocol::begin, BoardListProtocol::end)->createBoardInstance();
}
