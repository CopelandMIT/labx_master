/**
 * @copyright 2020 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <common/exception/EGenericException.hpp>


class ESpiMaxPayload :
    public EGenericException
{
private:
    static std::string getDescription(bool write, uint8_t maxWords, uint8_t actualWords)
    {
        std::ostringstream desc;
        if (write)
        {
            desc << "Too many payload words in frame to write. ";
        }
        else
        {
            desc << "Too many payload words in frame to read. ";
        }
        desc << "Actual: " << static_cast<unsigned int>(actualWords) << ", ";
        desc << "Allowed: " << static_cast<unsigned int>(maxWords);
        return desc.str();
    }

public:
    ESpiMaxPayload(bool write, uint8_t maxWords, uint8_t actualWords) :
        EGenericException(getDescription(write, maxWords, actualWords).c_str(), 0, "SPI Maximum Payload Exception")
    {
    }
};
