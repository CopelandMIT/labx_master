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


/**
 * Unpack Raw12 data from a uint8_t buffer to a buffer of type T.
 * The destination buffer has to be allocated for the unpacked data size of ((last - first) / 3 * 2) elements!
 *
 * @param first beginning of the packed data
 * @param last end of the packed data
 * @param dest beginning of unpacked data
 */
template <typename InputIt = const uint8_t *, typename OutputIt = uint16_t *>
inline void unpackRaw12(InputIt first, InputIt last, OutputIt dest)
{
    dest += (last - first) / 3 * 2;
    while (last > first)
    {
        const uint8_t b2 = *(--last);
        const uint8_t b1 = *(--last);
        const uint8_t b0 = *(--last);
        *(--dest)        = (b1 << 4) | (b2 >> 4);
        *(--dest)        = (b0 << 4) | (b2 & 0x0F);
    }
}


/**
 * Unpack Raw12 data within a buffer.
 * The buffer has to be allocated for (last - first) elements!
 *
 * @param first beginning of both the packed and the unpacked data!
 * @param last end of the unpacked data!
 */
inline void unpackRaw12(uint16_t *first, uint16_t *last)
{
    auto first8 = reinterpret_cast<const uint8_t *>(first);
    auto last8  = first8 + (last - first) * 3 / 2;
    unpackRaw12(first8, last8, first);
}
