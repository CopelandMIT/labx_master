/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "BoardUvc.hpp"
#include <uvc/EnumeratorUvcImpl.hpp>

#include <common/Logger.hpp>
#include <platform/BoardListProtocol.hpp>
#include <platform/templates/searchBoardFunction.hpp>


std::unique_ptr<BoardDescriptor> BoardUsb::searchBoard(uint16_t vid, uint16_t pid, BoardData::const_iterator begin, BoardData::const_iterator end)
{
    LOG(DEBUG) << "Looking for USB board with VID/PID: 0x" << std::hex << vid << " / 0x" << pid << " ...";

    return searchBoardFunctionEnumerator<EnumeratorUvcImpl>(begin, end, vid, pid);
}

std::unique_ptr<BoardInstance> BoardUsb::createBoardInstance(uint16_t vid, uint16_t pid)
{
    return searchBoard(vid, pid, BoardListProtocol::begin, BoardListProtocol::end)->createBoardInstance();
}
