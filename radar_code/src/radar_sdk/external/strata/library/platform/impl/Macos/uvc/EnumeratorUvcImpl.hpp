/**
 * @copyright 2018 - 2021 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <platform/interfaces/IEnumerator.hpp>
#include <common/exception/ENotImplemented.hpp>

/*
 * UVC is not implemented for MacOS yet.
 * This is just an empty dummy implementation of the UVC enumerator to make the project link.
 */
class EnumeratorUvcImpl :
    public IEnumerator
{
public:
    ~EnumeratorUvcImpl() = default;

    void enumerate(IEnumerationListener & /*listener*/, BoardData::const_iterator /*begin*/, BoardData::const_iterator /*end*/) override
    {
        throw ENotImplemented();
    }
};
