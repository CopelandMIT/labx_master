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


class EUvcProperty :
    public EGenericException
{
public:
    EUvcProperty(const char desc[] = "UVC Property Error", int code = 0, const char type[] = "UVC Property Exception") :
        EGenericException(desc, code, type)
    {}
};
