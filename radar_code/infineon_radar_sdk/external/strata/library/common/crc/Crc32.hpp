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

#define CRC32_LUT

#define CRC32_ETHERNET_SEED 0xFFFFFFFF
#define CRC32_MPEG2_SEED    0xFFFFFFFF
#define CRC32_AUTOSAR_SEED  0xFFFFFFFF
#define CRC32_DEFAULT_SEED  0xFFFFFFFF

// The following functions are optimized for one specific set of parameters.
// Therefore they don't call the generic Crc32 function below.
uint32_t Crc32Ethernet(const uint16_t buf[], uint16_t len, unsigned int bits, uint32_t crc = CRC32_ETHERNET_SEED);
uint32_t Crc32Mpeg2(const uint16_t buf[], uint16_t len, unsigned int bits, uint32_t crc = CRC32_MPEG2_SEED);
uint32_t Crc32Autosar(const uint8_t buf[], uint16_t len, uint32_t crc = CRC32_AUTOSAR_SEED);

/**
* @brief Calculates a CRC32 value from a provided byte stream.
* @param buf The input data to be used for the calculation
* @param len The length of the input data in bytes
* @param polynomial The generator polynomial to be used
* @param reflectIn If true, each single input byte will be reflected before the calculation, that means, the bit order will be reversed
* @param reflectOut If true, the result CRC will be reflected, that means the bit order of the whole value will be reversed
* @param invertOut If true, the result CRC will be inverted, that means XOR-ed with 0xFFFFFFFF
* @param crc The initial CRC value to start with
* @return The calculated CRC32 value
*/
uint32_t Crc32(const uint8_t buf[], uint16_t len, uint32_t polynomial, bool reflectIn, bool reflectOut, bool invertOut, uint32_t crc = CRC32_DEFAULT_SEED);

/**
* @brief Calculates a CRC32 value from a provided stream of integer values.
* @note This function uses the 8bit overload, just formats the stream accordingly.
* @param buf The input data to be used for the calculation
* @param len The length of the input data (number of values)
* @param polynomial The generator polynomial to be used
* @param reflectIn If true, each single input byte will be reflected before the calculation, that means, the bit order will be reversed
* @param reflectOut If true, the result CRC will be reflected, that means the bit order of the whole value will be reversed
* @param invertOut If true, the result CRC will be inverted, that means XOR-ed with 0xFFFFFFFF
* @param bigEndian If true, the integers will be handed to the CRC algorithm in Big Endian order (MSB is used first). Otherwise Little Endian order is used (LSB first).
* @param crc The initial CRC value to start with
* @return The calculated CRC32 value
*/
template <typename ValueType>
uint32_t Crc32(const ValueType buf[], uint16_t len, uint32_t polynomial, bool reflectIn, bool reflectOut, bool invertOut, bool bigEndian = false, uint32_t crc = CRC32_DEFAULT_SEED)
{
    const auto valueSize = sizeof(buf[0]);
    const auto byteLen   = static_cast<uint16_t>(len * valueSize);
    uint32_t resultCrc;
    if (bigEndian)
    {
        uint8_t byteIdx = 0;
        uint8_t *buf8   = new uint8_t[byteLen];
        for (uint16_t idx = 0; idx < len; idx++)
        {
            auto value = buf[idx];
            for (uint8_t byteInWord = 0; byteInWord < valueSize; byteInWord++)
            {
                *(buf8 + byteIdx) = value >> ((valueSize - 1) * 8);  //upper byte
                value <<= 8;
                byteIdx++;
            }
        }
        resultCrc = Crc32(buf8, byteLen, polynomial, reflectIn, reflectOut, invertOut, crc);
        delete[] buf8;
    }
    else
    {
        auto buf8 = reinterpret_cast<const uint8_t *>(buf);
        resultCrc = Crc32(buf8, byteLen, polynomial, reflectIn, reflectOut, invertOut, crc);
    }
    return resultCrc;
}

/**
* @brief Calculates the BZIP2 CRC32 value from a provided stream of integer values.
* @note This function uses the general Crc32 function with the appropriate parameters
* @param buf The input data to be used for the calculation
* @param len The length of the input data (number of values)
* @param bigEndian If true, the integers will be handed to the CRC algorithm in Big Endian order (MSB is used first). Otherwise Little Endian order is used (LSB first).
* @return The calculated CRC32 value
*/
template <typename ValueType>
uint32_t Crc32Bzip2(const ValueType buf[], uint16_t len, bool bigEndian = false)
{
    return Crc32(buf, len, 0x04C11DB7, false, false, true, bigEndian, 0xFFFFFFFF);
}

/**
* @brief Calculates the MPEG-2 CRC32 value from a provided stream of integer values.
* @note This function uses the general Crc32 function with the appropriate parameters
* @param buf The input data to be used for the calculation
* @param len The length of the input data (number of values)
* @param bigEndian If true, the integers will be handed to the CRC algorithm in Big Endian order (MSB is used first). Otherwise Little Endian order is used (LSB first).
* @return The calculated CRC32 value
*/
template <typename ValueType>
uint32_t Crc32Mpeg2(const ValueType buf[], uint16_t len, bool bigEndian = false)
{
    return Crc32(buf, len, 0x04C11DB7, false, false, false, bigEndian, 0xFFFFFFFF);
}
