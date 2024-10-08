/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "Crc6.hpp"


unsigned int Crc6Itu(unsigned int data, unsigned int fromBit, unsigned int toBit)
{
    unsigned int crc = 0;  // with non-zero start value reflection needs to be taken into account!

    const unsigned int poly  = 0x03;
    const unsigned int order = 6;

    const unsigned int mask  = 1 << (fromBit);
    const unsigned int limit = (1 << toBit) - 1;

    for (unsigned int b = mask; b > limit; b >>= 1)
    {
        const bool xor_flag = static_cast<bool>(crc & (1 << (order - 1)));
        crc <<= 1;

        if (static_cast<bool>(data & b) != xor_flag)  // bool != is the same as bool xor
        {
            crc ^= poly;
        }
    }

    return crc & ((1 << order) - 1);
}
