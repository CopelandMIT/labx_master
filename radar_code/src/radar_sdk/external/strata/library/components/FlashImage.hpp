/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <cstdint>
#include <vector>


struct FlashSegment
{
public:
    uint32_t address;
    std::vector<uint8_t> data;
};

class FlashImage :
    public std::vector<FlashSegment>
{
public:
    void readFromHexFile(const char filename[]);

    uint32_t m_startAddress = 0;
};
