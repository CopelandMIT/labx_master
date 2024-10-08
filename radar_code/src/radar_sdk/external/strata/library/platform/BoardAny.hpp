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

class BoardManager;

class BoardAny
{
public:
    static std::unique_ptr<BoardDescriptor> searchBoard(uint16_t vid, uint16_t pid, const char *interfaces);
    static std::unique_ptr<BoardInstance> createBoardInstance(uint16_t vid, uint16_t pid, const char *interfaces);

    //Deprecated
    static std::unique_ptr<BoardDescriptor> searchBoard(uint16_t vid, uint16_t pid, bool serial = true, bool ethernetUdp = true, bool uvc = true, bool wiggler = false);
    static std::unique_ptr<BoardInstance> createBoardInstance(uint16_t vid, uint16_t pid, bool serial = true, bool ethernetUdp = true, bool uvc = true, bool wiggler = false);

private:
    static std::unique_ptr<BoardDescriptor> searchBoard(uint16_t vid, uint16_t pid, BoardManager &boardMgr);
};
