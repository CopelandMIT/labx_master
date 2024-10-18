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


class ESpiProtocol :
    public EGenericException
{
public:
    ESpiProtocol(const char desc[] = "SPI Protocol error", int errorCode = 0, uint8_t cri = 0) :
        EGenericException(desc, errorCode, "SPI Protocol Exception")
    {
        m_what.append(" CRI: ");
        m_what.append(std::to_string(cri));
    }
};
