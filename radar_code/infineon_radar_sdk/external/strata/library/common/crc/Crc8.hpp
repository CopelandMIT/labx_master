#pragma once

#include <cstdint>

///
/// Calculates a CRC-8 value from a provided byte stream
///
/// @param buf          The input data for the CRC calculation
/// @param len          Length of the input data in bytes
/// @param polynomial   The generator polynomial to be used
/// @param crcInitial   The initial CRC value to start with
/// @return The calculated CRC value
///
uint8_t Crc8(const uint8_t buf[], uint16_t len, uint16_t polynomial, uint8_t crcInitial = 0x00);

///
/// Calculates the CRC-8 value according to the SMBus implementation
///
/// @details A zero initialized CRC value using the generator polynomial x^8 + x^2 + x + 1 is
/// calculated with the use of a lookup table.
///
/// @param buf          The input data for the CRC calculation
/// @param len          Length of the input data in bytes
/// @return The calculated CRC value
///
uint8_t Crc8Smbus(const uint8_t buf[], uint16_t len);