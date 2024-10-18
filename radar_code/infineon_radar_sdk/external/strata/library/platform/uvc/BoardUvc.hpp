/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <Definitions.hpp>
#include <platform/BoardDescriptor.hpp>


class BoardUsb
{
public:
    static std::unique_ptr<BoardDescriptor> searchBoard(uint16_t vid, uint16_t pid, BoardData::const_iterator begin, BoardData::const_iterator end);
    STRATA_API static std::unique_ptr<BoardInstance> createBoardInstance(uint16_t vid, uint16_t pid);
};
