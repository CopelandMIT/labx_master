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

#define CRC16_LUT

#define CRC16_CCITT_FALSE_SEED 0xFFFF


uint16_t Crc16CcittFalse(const uint8_t buf[], unsigned int len, uint16_t crc = CRC16_CCITT_FALSE_SEED);
uint16_t Crc16CcittFalse(const uint16_t buf[], unsigned int len, unsigned int bits, uint16_t crc = CRC16_CCITT_FALSE_SEED);


#include <common/EndianConversion.hpp>
#include <type_traits>


template <typename T>
inline typename std::enable_if<std::is_integral<T>::value, uint16_t>::type
Crc16CcittFalse(T val, uint16_t crc = CRC16_CCITT_FALSE_SEED)
{
    const T buf = hostToBig(val);
    return Crc16CcittFalse(reinterpret_cast<const uint8_t *>(&buf), sizeof(val), crc);
}
