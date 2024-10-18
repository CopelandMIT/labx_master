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


class EUninitialized :
    public EGenericException
{
public:
    EUninitialized(const char desc[] = "Object or Memory has not been initialized before use") :
        EGenericException(desc, 0, "Uninitialized Data Exception")
    {}
};
