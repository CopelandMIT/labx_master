/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "Crc32.hpp"

namespace
{
#ifdef CRC32_LUT
    static uint32_t crcMpeg2Table[16] = {
        0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005,
        0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61, 0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd  // clang-format
    };
#endif

    static uint32_t crcAutosarTable[256] = {
        0x00000000, 0xF4ACFB13, 0x1DF50D35, 0xE959F626, 0x3BEA1A6A, 0xCF46E179, 0x261F175F, 0xD2B3EC4C,
        0x77D434D4, 0x8378CFC7, 0x6A2139E1, 0x9E8DC2F2, 0x4C3E2EBE, 0xB892D5AD, 0x51CB238B, 0xA567D898,
        0xEFA869A8, 0x1B0492BB, 0xF25D649D, 0x06F19F8E, 0xD44273C2, 0x20EE88D1, 0xC9B77EF7, 0x3D1B85E4,
        0x987C5D7C, 0x6CD0A66F, 0x85895049, 0x7125AB5A, 0xA3964716, 0x573ABC05, 0xBE634A23, 0x4ACFB130,
        0x2BFC2843, 0xDF50D350, 0x36092576, 0xC2A5DE65, 0x10163229, 0xE4BAC93A, 0x0DE33F1C, 0xF94FC40F,
        0x5C281C97, 0xA884E784, 0x41DD11A2, 0xB571EAB1, 0x67C206FD, 0x936EFDEE, 0x7A370BC8, 0x8E9BF0DB,
        0xC45441EB, 0x30F8BAF8, 0xD9A14CDE, 0x2D0DB7CD, 0xFFBE5B81, 0x0B12A092, 0xE24B56B4, 0x16E7ADA7,
        0xB380753F, 0x472C8E2C, 0xAE75780A, 0x5AD98319, 0x886A6F55, 0x7CC69446, 0x959F6260, 0x61339973,
        0x57F85086, 0xA354AB95, 0x4A0D5DB3, 0xBEA1A6A0, 0x6C124AEC, 0x98BEB1FF, 0x71E747D9, 0x854BBCCA,
        0x202C6452, 0xD4809F41, 0x3DD96967, 0xC9759274, 0x1BC67E38, 0xEF6A852B, 0x0633730D, 0xF29F881E,
        0xB850392E, 0x4CFCC23D, 0xA5A5341B, 0x5109CF08, 0x83BA2344, 0x7716D857, 0x9E4F2E71, 0x6AE3D562,
        0xCF840DFA, 0x3B28F6E9, 0xD27100CF, 0x26DDFBDC, 0xF46E1790, 0x00C2EC83, 0xE99B1AA5, 0x1D37E1B6,
        0x7C0478C5, 0x88A883D6, 0x61F175F0, 0x955D8EE3, 0x47EE62AF, 0xB34299BC, 0x5A1B6F9A, 0xAEB79489,
        0x0BD04C11, 0xFF7CB702, 0x16254124, 0xE289BA37, 0x303A567B, 0xC496AD68, 0x2DCF5B4E, 0xD963A05D,
        0x93AC116D, 0x6700EA7E, 0x8E591C58, 0x7AF5E74B, 0xA8460B07, 0x5CEAF014, 0xB5B30632, 0x411FFD21,
        0xE47825B9, 0x10D4DEAA, 0xF98D288C, 0x0D21D39F, 0xDF923FD3, 0x2B3EC4C0, 0xC26732E6, 0x36CBC9F5,
        0xAFF0A10C, 0x5B5C5A1F, 0xB205AC39, 0x46A9572A, 0x941ABB66, 0x60B64075, 0x89EFB653, 0x7D434D40,
        0xD82495D8, 0x2C886ECB, 0xC5D198ED, 0x317D63FE, 0xE3CE8FB2, 0x176274A1, 0xFE3B8287, 0x0A977994,
        0x4058C8A4, 0xB4F433B7, 0x5DADC591, 0xA9013E82, 0x7BB2D2CE, 0x8F1E29DD, 0x6647DFFB, 0x92EB24E8,
        0x378CFC70, 0xC3200763, 0x2A79F145, 0xDED50A56, 0x0C66E61A, 0xF8CA1D09, 0x1193EB2F, 0xE53F103C,
        0x840C894F, 0x70A0725C, 0x99F9847A, 0x6D557F69, 0xBFE69325, 0x4B4A6836, 0xA2139E10, 0x56BF6503,
        0xF3D8BD9B, 0x07744688, 0xEE2DB0AE, 0x1A814BBD, 0xC832A7F1, 0x3C9E5CE2, 0xD5C7AAC4, 0x216B51D7,
        0x6BA4E0E7, 0x9F081BF4, 0x7651EDD2, 0x82FD16C1, 0x504EFA8D, 0xA4E2019E, 0x4DBBF7B8, 0xB9170CAB,
        0x1C70D433, 0xE8DC2F20, 0x0185D906, 0xF5292215, 0x279ACE59, 0xD336354A, 0x3A6FC36C, 0xCEC3387F,
        0xF808F18A, 0x0CA40A99, 0xE5FDFCBF, 0x115107AC, 0xC3E2EBE0, 0x374E10F3, 0xDE17E6D5, 0x2ABB1DC6,
        0x8FDCC55E, 0x7B703E4D, 0x9229C86B, 0x66853378, 0xB436DF34, 0x409A2427, 0xA9C3D201, 0x5D6F2912,
        0x17A09822, 0xE30C6331, 0x0A559517, 0xFEF96E04, 0x2C4A8248, 0xD8E6795B, 0x31BF8F7D, 0xC513746E,
        0x6074ACF6, 0x94D857E5, 0x7D81A1C3, 0x892D5AD0, 0x5B9EB69C, 0xAF324D8F, 0x466BBBA9, 0xB2C740BA,
        0xD3F4D9C9, 0x275822DA, 0xCE01D4FC, 0x3AAD2FEF, 0xE81EC3A3, 0x1CB238B0, 0xF5EBCE96, 0x01473585,
        0xA420ED1D, 0x508C160E, 0xB9D5E028, 0x4D791B3B, 0x9FCAF777, 0x6B660C64, 0x823FFA42, 0x76930151,
        0x3C5CB061, 0xC8F04B72, 0x21A9BD54, 0xD5054647, 0x07B6AA0B, 0xF31A5118, 0x1A43A73E, 0xEEEF5C2D,
        0x4B8884B5, 0xBF247FA6, 0x567D8980, 0xA2D17293, 0x70629EDF, 0x84CE65CC, 0x6D9793EA, 0x993B68F9  // clang-format
    };

    template <typename ValueType>
    ValueType reflect(ValueType val)
    {
        ValueType result   = 0;
        const uint8_t bits = static_cast<uint8_t>(sizeof(ValueType) * 8);
        for (uint8_t idx = 0; idx < bits; idx++)
        {
            result <<= 1;
            result |= (val & 1);
            val >>= 1;
        }
        return result;
    }
}

uint32_t Crc32Ethernet(const uint16_t buf[], uint16_t len, unsigned int bits, uint32_t crc)
{
    const uint32_t poly = 0xEDB88320;  // reversed 0x04C11DB7, since both input and output should be reflected
    //const unsigned int order = 32;
    const unsigned int mask = 1u << (bits - 1);

    // check if we have a whole byte to process at the end (to use optimized calculation for it)
    const unsigned int limit = 0;  // (static_cast<unsigned int>(bits > 7) << 8) - 1;

    while (len--)
    {
        uint16_t data = *buf++;

        for (unsigned int b = limit + 1; b <= mask; b <<= 1)
        {
            // reversed shifting
            const bool xor_flag = static_cast<bool>(crc & 1);  //(1u << (order - 1)));
            crc >>= 1;

            if (static_cast<bool>(data & b) != xor_flag)  // bool != is the same as bool xor
            {
                crc ^= poly;
            }
        }
    }

    return ~crc;  // XOR with all ones
}

uint32_t Crc32Mpeg2(const uint16_t buf[], uint16_t len, unsigned int bits, uint32_t crc)
{
    const uint32_t poly      = 0x04C11DB7;
    const unsigned int order = 32;
    const unsigned int mask  = 1u << (bits - 1);

#ifdef CRC32_LUT
    const unsigned int lutBitSize = 4;

    // check if we have whole nibbles to process at the end (to use LUT for it)
    const unsigned int dataLutBits = bits / lutBitSize * lutBitSize;
    const unsigned int limit       = (1u << dataLutBits) - 1;
    const unsigned int lutBitMask  = (1u << lutBitSize) - 1;
#else
    const unsigned int limit = 0;
#endif

    while (len--)
    {
        uint16_t data = *buf++;

        for (unsigned int b = mask; b > limit; b >>= 1)
        {
            const bool xor_flag = static_cast<bool>(crc & (1u << (order - 1)));
            crc <<= 1;

            if (static_cast<bool>(data & b) != xor_flag)  // bool != is the same as bool xor
            {
                crc ^= poly;
            }
        }

#ifdef CRC32_LUT
        unsigned int i = dataLutBits;
        while (i)
        {
            i -= lutBitSize;

            const uint8_t x = (crc >> 28) ^ static_cast<uint8_t>(data >> i);
            crc             = (crc << lutBitSize) ^ crcMpeg2Table[x & lutBitMask];
        }
#endif
    }

    return crc;
}

uint32_t Crc32Autosar(const uint8_t buf[], uint16_t len, uint32_t crc)
{
    while (len--)
    {
        uint8_t data = *buf++;
        crc          = ((crc << 8) ^ crcAutosarTable[(crc >> 24) ^ data]);
    }
    return crc;
}

uint32_t Crc32(const uint8_t buf[], uint16_t len, uint32_t polynomial, bool reflectIn, bool reflectOut, bool invertOut, uint32_t crc)
{
    while (len--)
    {
        uint8_t val = *buf++;
        if (reflectIn)
        {
            val = reflect(val);
        }

        for (unsigned int b = 0x80; b >= 0x01; b >>= 1)
        {
            const bool xor_flag = static_cast<bool>(crc & 0x80000000);
            crc <<= 1;

            if (static_cast<bool>(val & b) != xor_flag)  // bool != is the same as bool xor
            {
                crc ^= polynomial;
            }
        }
    }

    if (reflectOut)
    {
        crc = reflect(crc);
    }

    if (invertOut)
    {
        return ~crc;  // XOR with all ones
    }
    else
    {
        return crc;
    }
}
