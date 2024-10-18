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


class EnumeratorSerialImplBase :
    public IEnumerator
{
public:
    EnumeratorSerialImplBase() = delete;

    void enumerate(IEnumerationListener &listener, BoardData::const_iterator begin, BoardData::const_iterator end) override;

protected:
    EnumeratorSerialImplBase(const char *devBegin[], const char *devEnd[]);

private:
    const char **m_devBegin;
    const char **m_devEnd;
};
