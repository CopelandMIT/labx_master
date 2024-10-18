/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "EnumeratorSerialImplBase.hpp"

#include <platform/serial/BoardSerial.hpp>
#include <platform/templates/enumerateFunction.hpp>

#include <glob.h>


EnumeratorSerialImplBase::EnumeratorSerialImplBase(const char *devBegin[], const char *devEnd[]) :
    m_devBegin {devBegin},
    m_devEnd {devEnd}
{
}

void EnumeratorSerialImplBase::enumerate(IEnumerationListener &listener, BoardData::const_iterator begin, BoardData::const_iterator end)
{
    glob_t glob_results = {};

    for (auto d = m_devBegin; d < m_devEnd; d++)
    {
        glob(*d, GLOB_APPEND, nullptr, &glob_results);
    }

    for (uint_fast16_t i = 0; i < glob_results.gl_pathc; i++)
    {
        if (enumerateFunction<BoardSerial>(listener, begin, end, glob_results.gl_pathv[i]))
        {
            break;
        }
    }

    globfree(&glob_results);
}
