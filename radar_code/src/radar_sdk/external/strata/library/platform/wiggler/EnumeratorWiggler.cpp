/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "EnumeratorWiggler.hpp"

#include <platform/templates/enumerateFunction.hpp>
#include <platform/wiggler/BoardWiggler.hpp>

#include <mcd_loader_class/mcd_loader_class.h>


namespace
{
    const uint8_t maxSystems = 16;
}


EnumeratorWiggler::EnumeratorWiggler()
{
}

EnumeratorWiggler::~EnumeratorWiggler()
{
}

void EnumeratorWiggler::enumerate(IEnumerationListener &listener, BoardData::const_iterator begin, BoardData::const_iterator end)
{
    for (uint8_t i = 0; i < maxSystems; i++)
    {
        if (enumerateFunction<BoardWiggler>(listener, begin, end, i))
        {
            break;
        }
    }
}
