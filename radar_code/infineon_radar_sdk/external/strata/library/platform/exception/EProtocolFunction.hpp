/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <common/exception/EException.hpp>
#include <iomanip>
#include <sstream>
#include <universal/error_definitions.h>
#include <universal/error_strings.h>
#include <universal/protocol/protocol_definitions.h>


class EProtocolFunction :
    public EException
{
public:
    EProtocolFunction(uint8_t error) :
        EException(error)
    {
        std::ostringstream s;

        s << "ProtocolFunction Exception: Calling a function through the protocol returned the following error: ";
        s << getErrorString(error);
        s << " (0x" << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(error) << ")";

        m_what = s.str();
    }
};
