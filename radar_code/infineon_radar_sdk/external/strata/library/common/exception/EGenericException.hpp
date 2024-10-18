/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include "EException.hpp"
#include <sstream>


class EGenericException :
    public EException
{
public:
    /// Constructor
    /// \param desc Description of what went wrong (for debugging)
    /// \param code Error code corresponding to the exception
    /// \param type Descripton of the exception type. Leave empty for default description.
    EGenericException(const char desc[] = "Unspecified failure", int code = 0, const char type[] = "Generic Exception") :
        EException(code)
    {
        std::ostringstream s;

        s << type;
        s << ": ";
        s << desc;

        s << " [Code: 0x";
        s << std::hex << code;
        s << " / ";
        s << std::dec << code;
        s << "]";

        m_what = s.str();
    }
};
