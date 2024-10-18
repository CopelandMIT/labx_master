/**
 * @copyright 2022 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "BoardInstanceMacro.hpp"


BoardInstanceMacro::BoardInstanceMacro(BoardInstance &&instance) :
    BoardInstance(std::move(instance))
{
    auto bridge   = std::make_shared<BridgeMacro>(m_bridge.get());
    m_macroBridge = bridge.get();
    m_realBridge  = std::move(m_bridge);
    m_bridge      = std::move(bridge);

    m_realBoard  = std::move(m_board);
    auto board   = std::make_unique<BoardRemote>(m_macroBridge);
    m_macroBoard = board.get();
    m_board      = std::move(board);
}

BoardInstanceMacro::BoardInstanceMacro(std::unique_ptr<BoardInstance> &&instance) :
    BoardInstanceMacro(std::move(*instance.release()))
{
}

BoardInstanceMacro::~BoardInstanceMacro()
{
}

uint8_t BoardInstanceMacro::getIModuleCount(uint16_t type)
{
    return m_realBoard->getIModuleCount(type);
}

uint8_t BoardInstanceMacro::getIComponentCount(uint16_t type)
{
    return m_realBoard->getIComponentCount(type);
}

void BoardInstanceMacro::enableMacroRecording()
{
    m_macroBridge->enableRecording();
}

void BoardInstanceMacro::enableMacroPlayback()
{
    m_macroBridge->enablePlayback();
}

void BoardInstanceMacro::enableNormalMode()
{
    m_macroBridge->enableNormalMode();
}

void BoardInstanceMacro::wait(uint32_t microseconds)
{
    m_macroBridge->wait(microseconds);
}
