/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include "identifyBoardFunction.hpp"
#include <platform/interfaces/IEnumerator.hpp>

#include <common/Logger.hpp>
#include <common/Serialization.hpp>
#include <platform/exception/EConnection.hpp>


template <typename BridgeType, typename... Identifier>
inline std::unique_ptr<BoardDescriptor> searchBoardFunctionBridge(BoardData::const_iterator begin, BoardData::const_iterator end, Identifier &&...identifier)
{
    IBridgeControl::BoardInfo_t boardInfo;
    const char *cName = reinterpret_cast<char *>(&boardInfo[4]);
    uint16_t ids[2];

    try
    {
        std::unique_ptr<IBridge> bridge = std::make_unique<BridgeType>(std::forward<Identifier>(identifier)...);

        bridge->getIBridgeControl()->getBoardInfo(boardInfo);
        serialToHost(boardInfo.data(), ids);

        const uint16_t &vid = ids[0], &pid = ids[1];
        return identifyBoardFunction<BoardDescriptor>(begin, end, vid, pid, cName, std::move(bridge));
    }
    catch (const EException &e)
    {
        LOG(DEBUG) << "... handled " << e.what();
        throw EConnection("Board not found! (no response received)");
    }
}

template <typename EnumeratorType>
inline std::unique_ptr<BoardDescriptor> searchBoardFunctionEnumerator(BoardData::const_iterator begin, BoardData::const_iterator end, uint16_t vid, uint16_t pid)
{
    class ListenerClass : public IEnumerationListener
    {
    public:
        bool onEnumerate(std::unique_ptr<BoardDescriptor> &&descriptor) override
        {
            m_descriptor = std::move(descriptor);
            return true;
        }

        std::unique_ptr<BoardDescriptor> m_descriptor;
    } listener;

    auto it = findBoardData(begin, end, vid, pid);
    if (it != end)
    {
        EnumeratorType().enumerate(listener, it, it + 1);
    }
    else
    {
        const BoardData d = {vid, pid, BoardFactoryFunction<BoardRemote>};
        EnumeratorType().enumerate(listener, &d, &d + 1);
    }

    return std::move(listener.m_descriptor);
}
