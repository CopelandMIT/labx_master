/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "Win32Utils.hpp"
#include <cwchar>


bool getIds(const wchar_t *start, uint16_t &vid, uint16_t &pid)
{
    start = wcsstr(start, L"usb#");
    if (start == nullptr)
    {
        // not a USB device path
        return false;
    }

    vid = static_cast<uint16_t>(wcstoul(start + 8, nullptr, 16));
    pid = static_cast<uint16_t>(wcstoul(start + 17, nullptr, 16));

    return true;
}

std::string copyDeviceString(const wchar_t *start)
{
    std::string result;
    start = wcsstr(start, L"usb#");
    if (start == nullptr)
    {
        // not a USB device path
        return result;
    }
    start = wcsstr(start + 4, L"#");
    if (start == nullptr)
    {
        return result;
    }
    decltype(start) end = wcsstr(start + 1, L"#");
    if (end == nullptr)
    {
        return result;
    }

    // manually assign characters to avoid narrowing conversion warning
    result.reserve(end - start);
    while (start < end)
    {
        result.push_back(static_cast<decltype(result)::basic_string::value_type>(*start++));
    }
    return result;
}
