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


class EFirmwareCommand :
    public EGenericException
{
private:
    static std::string getDescription(uint8_t command)
    {
        std::ostringstream desc;
        desc << "Error executing firmware command ";
        desc << static_cast<unsigned int>(command);
        return desc.str();
    }

public:
    EFirmwareCommand(uint8_t command, uint8_t cmdErrorCode) :
        EGenericException(getDescription(command).c_str(), cmdErrorCode, "Firmware Command Exception")
    {
    }
};
