/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <common/Logger.hpp>
#include <platform/BoardDescriptor.hpp>
#include <platform/interfaces/IEnumerator.hpp>


template <typename BoardType, typename IdentifierType>
inline bool enumerateFunction(IEnumerationListener &listener, BoardData::const_iterator begin, BoardData::const_iterator end, IdentifierType &identifier)
{
    try
    {
        auto descriptor = BoardType::searchBoard(identifier, begin, end);
        LOG(DEBUG) << "... board recognized: VID = 0x" << std::hex << std::setw(4) << std::setfill('0') << descriptor->getVid() << ", PID = 0x" << std::setfill('0') << descriptor->getPid();
        return listener.onEnumerate(std::move(descriptor));
    }
    catch (...)
    {
        LOG(DEBUG) << "... no board recognized";
    }
    return false;
}
