/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <platform/BoardDescriptor.hpp>


class IEnumerationListener
{
public:
    virtual ~IEnumerationListener() = default;

    virtual bool onEnumerate(std::unique_ptr<BoardDescriptor> &&descriptor) = 0;
};


class IEnumerator
{
public:
    virtual ~IEnumerator() = default;

    virtual void enumerate(IEnumerationListener &listener, BoardData::const_iterator begin, BoardData::const_iterator end) = 0;
};
