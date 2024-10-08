/**
 * @copyright 2022 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <platform/BoardInstance.hpp>
#include <platform/boards/BoardRemote.hpp>
#include <platform/macro/BridgeMacro.hpp>


class BoardInstanceMacro :
    public BoardInstance
{
public:
    BoardInstanceMacro(BoardInstance &&instance);
    BoardInstanceMacro(std::unique_ptr<BoardInstance> &&instance);
    ~BoardInstanceMacro();

    uint8_t getIModuleCount(uint16_t type) override;
    uint8_t getIComponentCount(uint16_t type) override;

    void enableMacroRecording();
    void enableMacroPlayback();
    void enableNormalMode();

    void wait(uint32_t microseconds);

private:
    std::shared_ptr<IBridge> m_realBridge;
    BridgeMacro *m_macroBridge;

    std::unique_ptr<IBoard> m_realBoard;
    BoardRemote *m_macroBoard;
};
