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
#include <platform/interfaces/link/ISocket.hpp>  // ipAddress_t


class BoardEthernetTcp
{
public:
    static std::unique_ptr<BoardDescriptor> searchBoard(ipAddress_t ipAddr, BoardData::const_iterator begin, BoardData::const_iterator end);
    STRATA_API static std::unique_ptr<BoardInstance> createBoardInstance(ipAddress_t ipAddr);
};
