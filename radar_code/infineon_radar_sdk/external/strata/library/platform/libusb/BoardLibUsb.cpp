/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "BoardLibUsb.hpp"
#include "EnumeratorLibUsb.hpp"

#include <common/Logger.hpp>
#include <platform/BoardListProtocol.hpp>
#include <platform/templates/searchBoardFunction.hpp>


std::unique_ptr<BoardDescriptor> BoardLibUsb::searchBoard(uint16_t vid, uint16_t pid, BoardData::const_iterator begin, BoardData::const_iterator end)
{
    LOG(DEBUG) << "Looking for USB board with VID/PID: 0x" << std::hex << vid << " / 0x" << pid << " ...";

    return searchBoardFunctionEnumerator<EnumeratorLibUsbImpl>(begin, end, vid, pid);
}

std::unique_ptr<BoardInstance> BoardLibUsb::createBoardInstance(uint16_t vid, uint16_t pid)
{
    return searchBoard(vid, pid, BoardListProtocol::begin, BoardListProtocol::end)->createBoardInstance();
}

std::unique_ptr<BoardInstance> BoardLibUsb::createBoardInstanceFd(int fd, uint16_t vid, uint16_t pid)
{
    LOG(DEBUG) << "Creating USB board with file descriptor ...";

    if (vid == 0 && pid == 0)
    {
        const BoardData d = {vid, pid, BoardFactoryFunction<BoardRemote>};
        return std::make_unique<BoardDescriptorLibUsb>(d, "", nullptr, fd)->createBoardInstance();
    }
    else
    {
        return identifyBoardFunction<BoardDescriptorLibUsb>(BoardListProtocol::begin, BoardListProtocol::end, vid, pid, "", nullptr, fd)->createBoardInstance();
    }
}
