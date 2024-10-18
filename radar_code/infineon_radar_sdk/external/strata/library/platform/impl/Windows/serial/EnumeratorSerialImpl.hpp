/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <platform/interfaces/IEnumerator.hpp>


class EnumeratorSerialImpl :
    public IEnumerator
{
public:
    void enumerate(IEnumerationListener &listener, BoardData::const_iterator begin, BoardData::const_iterator end) override;
};
