/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <platform/BoardDescriptor.hpp>
#include <platform/boards/BoardRemote.hpp>
#include <platform/templates/boardFactoryFunction.hpp>

#include <common/cpp11/memory.hpp>


inline BoardData::const_iterator findBoardData(BoardData::const_iterator begin, BoardData::const_iterator end, uint16_t vid, uint16_t pid)
{
    BoardData::const_iterator it;
    for (it = begin; it < end; it++)
    {
        if ((it->vid == vid) && (it->pid == pid))
        {
            break;
        }
    }

    return it;
}

template <typename DescriptorType, typename... Args>
inline std::unique_ptr<DescriptorType> identifyBoardFunction(BoardData::const_iterator begin, BoardData::const_iterator end, uint16_t vid, uint16_t pid, const char *cName, Args... args)
{
    auto it = findBoardData(begin, end, vid, pid);
    if (it != end)
    {
        return std::make_unique<DescriptorType>(*it, cName, std::forward<Args>(args)...);
    }
    else
    {
        const BoardData d = {vid, pid, BoardFactoryFunction<BoardRemote>};
        return std::make_unique<DescriptorType>(d, cName, std::forward<Args>(args)...);
    }
}
