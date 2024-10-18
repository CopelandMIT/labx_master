/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "FlashImage.hpp"
#include "exception/EFlashImage.hpp"

#include <fstream>
#include <string>


typedef enum : uint8_t
{
    Data                  = 0x00,
    EndOfFile             = 0x01,
    ExtendedLinearAddress = 0x04,
    StartLinearAddress    = 0x05
} recordTypeEnum;


void FlashImage::readFromHexFile(const char filename[])
{
    m_startAddress = 0;  // not needed for writing to the program flash
    auto m_image   = this;

    std::ifstream inFile;
    inFile.open(filename);

    m_image->clear();
    m_image->emplace_back();
    FlashSegment *segment = &m_image->back();

    std::string line;
    uint32_t extendedLinearAddress = 0;

    while (std::getline(inFile, line))
    {
        const auto firstChar = line[0];
        if (firstChar != ':')
        {
            throw EFlashImage("readFromHexFile() - unexpected beginning of line", firstChar);
        }

        const uint8_t byteCount  = static_cast<uint8_t>(std::stoi(line.substr(1, 2), nullptr, 16));
        const uint32_t address   = static_cast<uint32_t>(std::stoi(line.substr(3, 4), nullptr, 16));
        const uint8_t recordType = static_cast<uint8_t>(std::stoi(line.substr(7, 2), nullptr, 16));
        std::basic_string<char>::size_type subCount;

        switch (recordType)
        {
            case Data:
            {
                const uint32_t baseAddress = extendedLinearAddress + address;
                if (segment->data.empty())
                {
                    segment->address = baseAddress;
                }
                else if (segment->address + segment->data.size() != baseAddress)
                {
                    m_image->emplace_back();
                    segment          = &m_image->back();
                    segment->address = baseAddress;
                }
                for (uint_fast16_t i = 0; i < byteCount; i++)
                {
                    segment->data.push_back(static_cast<uint8_t>(std::stoi(line.substr(9 + 2 * i, 2), nullptr, 16)));
                }
            }
            break;
            case EndOfFile:
                if (byteCount != 0)
                {
                    throw EFlashImage("readFromHexFile() - unexptected byteCount for recordType", (byteCount << 16) | recordType);
                }
                return;  //break;
            case ExtendedLinearAddress:
                if (byteCount != 2)
                {
                    throw EFlashImage("readFromHexFile() - unexptected byteCount for recordType", (byteCount << 16) | recordType);
                }
                subCount              = static_cast<std::basic_string<char>::size_type>(2 * 2);
                extendedLinearAddress = static_cast<uint32_t>(std::stoi(line.substr(9, subCount), nullptr, 16) << 16);
                break;
            case StartLinearAddress:
                if (byteCount != 4)
                {
                    throw EFlashImage("readFromHexFile() - unexptected byteCount for recordType", (byteCount << 16) | recordType);
                }
                subCount       = static_cast<std::basic_string<char>::size_type>(2 * 4);
                m_startAddress = static_cast<uint32_t>(std::stoi(line.substr(9, subCount), nullptr, 16));
                break;
            default:
                throw EFlashImage("readFromHexFile() - unexpected recordType", recordType);
        }
    }

    throw EFlashImage("readFromHexFile() - unexpected end of file");
}
