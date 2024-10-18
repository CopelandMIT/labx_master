/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <common/exception/EGenericException.hpp>


class ENonvolatileMemory :
    public EGenericException
{
public:
    ENonvolatileMemory(const char desc[] = "NonvolatileMemory Error", unsigned int code = 0, const char type[] = "NonvolatileMemory Exception") :
        EGenericException(desc, static_cast<int>(code), type)
    {}
};
