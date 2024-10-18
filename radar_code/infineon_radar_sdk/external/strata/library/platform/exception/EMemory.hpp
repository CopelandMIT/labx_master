/**
 * @copyright 2021 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <common/exception/EGenericException.hpp>


class EMemory :
    public EGenericException
{
public:
    EMemory(const char desc[] = "Memory Error", int code = 0, const char type[] = "Memory Exception") :
        EGenericException(desc, code, type)
    {}
};
