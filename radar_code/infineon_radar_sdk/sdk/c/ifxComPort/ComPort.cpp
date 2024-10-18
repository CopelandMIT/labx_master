/* ===========================================================================
** Copyright (C) 2021 Infineon Technologies AG
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**
** 1. Redistributions of source code must retain the above copyright notice,
**    this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. Neither the name of the copyright holder nor the names of its
**    contributors may be used to endorse or promote products derived from
**    this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
** LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
** CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
** INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
** CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
** ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
** POSSIBILITY OF SUCH DAMAGE.
** ===========================================================================
*/

#include "COMPort.h"
#include "internal/ComPort.hpp"

#include "ifxBase/Exception.hpp"

#include <cstring>
#include <functional>
#include <list>
#include <regex>
#include <vector>

#ifndef __APPLE__
static bool hex2uint16(const std::string& s, uint16_t& value)
{
    if (s.length() == 0 || s.length() > 4)
        return false;

    uint16_t v = 0;
    for (const char c : s)
    {
        v <<= 4;

        if (c >= '0' && c <= '9')
            v |= c - '0';
        else if (c >= 'A' && c <= 'F')
            v |= c - 'A' + 10;
        else if (c >= 'a' && c <= 'f')
            v |= c - 'a' + 10;
        else
            return false;
    }

    value = v;
    return true;
}
#endif

#if defined(_WIN32)
#include <tchar.h>
#include <windows.h>

// setupapi.h has to be included after windows.h
#include <setupapi.h>


std::list<ComPortInfo> ComPortInfo::enumerate(std::function<bool(const ComPortInfo&)>& predicate)
{
    // SetupDiGetClassDevs returns a handle to a device information set that
    // contains requested device information elements for a local computer
    HDEVINFO DeviceInfoSet = SetupDiGetClassDevs(nullptr, TEXT("USB"), nullptr,
                                                 DIGCF_ALLCLASSES | DIGCF_PRESENT);

    if (DeviceInfoSet == INVALID_HANDLE_VALUE)
        return std::list<ComPortInfo>();

    std::list<ComPortInfo> matching;
    SP_DEVINFO_DATA DeviceInfoData = {0, {0, 0, 0, {0, 0, 0, 0}}, 0, 0};
    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    const std::regex vid_pid_regex("USB\\\\VID_([0-9a-fA-F]{4})&PID_([0-9a-fA-F]{4})");

    // SetupDiEnumDeviceInfo returns a SP_DEVINFO_DATA structure that specifies
    // a device information element in a device information set
    for (DWORD DeviceIndex = 0;
         SetupDiEnumDeviceInfo(DeviceInfoSet, DeviceIndex, &DeviceInfoData) != FALSE;
         DeviceIndex++)
    {
        DEVPROPTYPE ulPropertyType;
        DWORD requiredSize = 0;

        SetupDiGetDeviceRegistryPropertyA(DeviceInfoSet, &DeviceInfoData,
                                          SPDRP_HARDWAREID, &ulPropertyType,
                                          nullptr, 0, &requiredSize);

        std::vector<BYTE> propertyBuffer;
        propertyBuffer.resize(std::vector<BYTE>::size_type(requiredSize) + 1);
        propertyBuffer[requiredSize] = '\0';

        // get specified plug and play device property
        if (SetupDiGetDeviceRegistryPropertyA(DeviceInfoSet, &DeviceInfoData,
                                              SPDRP_HARDWAREID, &ulPropertyType,
                                              propertyBuffer.data(), requiredSize, &requiredSize))
        {
            std::string properties((char*)propertyBuffer.data());
            std::smatch vid_pid_match;
            std::string port_name;

            if (!std::regex_search(properties, vid_pid_match, vid_pid_regex))
                continue;

            uint16_t vid = 0;
            uint16_t pid = 0;
            if (hex2uint16(vid_pid_match[1].str(), vid) && hex2uint16(vid_pid_match[2].str(), pid))
            {
                // open registry
                HKEY hDeviceRegistryKey = SetupDiOpenDevRegKey(DeviceInfoSet,
                                                               &DeviceInfoData,
                                                               DICS_FLAG_GLOBAL, 0,
                                                               DIREG_DEV, KEY_READ);
                if (hDeviceRegistryKey == INVALID_HANDLE_VALUE)
                    continue;

                // query
                std::vector<BYTE> port_name_buffer;
                port_name_buffer.resize(8192);

                DWORD dwSize2 = DWORD(port_name_buffer.size());
                DWORD dwType = REG_SZ;
                LSTATUS ret = RegQueryValueExA(hDeviceRegistryKey, "PortName",
                                               nullptr, &dwType, (LPBYTE)port_name_buffer.data(), &dwSize2);

                // close registry key
                RegCloseKey(hDeviceRegistryKey);

                if (ret != ERROR_SUCCESS)
                    continue;
                if (dwType != REG_SZ)
                    continue;

                port_name = std::string((char*)(port_name_buffer.data()), size_t(dwSize2));
            }

            if (port_name.find("COM") != std::string::npos)
            {
                ComPortInfo port_info(port_name, port_name, vid, pid);
                if (predicate(port_info))
                {
                    matching.push_back(port_info);
                }
            }
        }
    }

    SetupDiDestroyDeviceInfoList(DeviceInfoSet);

    return matching;
}
#elif defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOBSD.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/serial/IOSerialKeys.h>
#include <IOKit/serial/ioss.h>
#include <sys/param.h>

std::list<ComPortInfo> ComPortInfo::enumerate(std::function<bool(const ComPortInfo&)>& predicate)
{
    std::list<ComPortInfo> matching;

    CFMutableDictionaryRef classes = IOServiceMatching(kIOSerialBSDServiceValue);
    if (classes == NULL)
        return matching;

    // Look for devices that claim to be modems.
    CFDictionarySetValue(classes, CFSTR(kIOSerialBSDTypeKey), CFSTR(kIOSerialBSDAllTypes));

    // Get an iterator across all matching devices.
    io_iterator_t iterator;
    if (IOServiceGetMatchingServices(kIOMainPortDefault, classes, &iterator) != KERN_SUCCESS)
        return matching;

    // Iterate across all modems found. In this example, we bail after finding the first modem.
    io_object_t it;
    while ((it = IOIteratorNext(iterator)))
    {
        ComPortInfo port_info("", "", 0, 0);

        auto bsdPathAsCFString = static_cast<CFStringRef>(IORegistryEntryCreateCFProperty(it, CFSTR(kIOCalloutDeviceKey), kCFAllocatorDefault, 0));
        if (!bsdPathAsCFString)
            continue;

        char port_path_buffer[MAXPATHLEN] = {0};
        if (!CFStringGetCString(bsdPathAsCFString, port_path_buffer, sizeof(port_path_buffer), kCFStringEncodingASCII))
            continue;

        port_info.m_name = std::string(port_path_buffer);
        port_info.m_path = std::string(port_path_buffer);
        CFRelease(bsdPathAsCFString);

        if (port_info.m_name.size() == 0)
            continue;

        auto cf_vid = static_cast<CFNumberRef>(IORegistryEntrySearchCFProperty(it, kIOServicePlane, CFSTR("idVendor"), kCFAllocatorDefault, kIORegistryIterateRecursively | kIORegistryIterateParents));
        auto cf_pid = static_cast<CFNumberRef>(IORegistryEntrySearchCFProperty(it, kIOServicePlane, CFSTR("idProduct"), kCFAllocatorDefault, kIORegistryIterateRecursively | kIORegistryIterateParents));

        if (cf_vid && cf_pid)
        {
            uint16_t vid = 0;
            CFNumberGetValue(cf_vid, kCFNumberIntType, &vid);
            port_info.m_vid = vid;
            CFRelease(cf_vid);

            uint16_t pid = 0;
            CFNumberGetValue(cf_pid, kCFNumberIntType, &pid);
            port_info.m_pid = pid;
            CFRelease(cf_pid);

            if (predicate(port_info))
                matching.push_back(port_info);
        }

        // Release CFTypeRef
        if (cf_vid)
            CFRelease(cf_vid);
        if (cf_pid)
            CFRelease(cf_pid);

        // Release the io_service_t now that we are done with it.
        IOObjectRelease(it);
    }

    IOObjectRelease(iterator);

    return matching;
}
#else /* Linux */
#include <fstream>

#include <dirent.h>
#include <sys/types.h>

static bool file_exists(const std::string& name)
{
    std::ifstream f(name);
    return f.good();
}

// get PID and VID from uevent file
static bool get_vid_pid(const std::string& path_uevent, uint16_t& vid, uint16_t& pid)
{
    std::ifstream file(path_uevent);

    // search for something like PRODUCT=58b/58/1
    const std::regex vid_pid_regex("PRODUCT=([0-9a-fA-F]+)/([0-9a-fA-F]+)/");

    for (std::string line; getline(file, line);)
    {
        std::smatch match;
        if (!std::regex_search(line, match, vid_pid_regex))
            continue;

        return hex2uint16(match[1], vid) && hex2uint16(match[2], pid);
    }

    return false;
}

std::list<ComPortInfo> ComPortInfo::enumerate(std::function<bool(const ComPortInfo&)>& predicate)
{
    std::list<ComPortInfo> matching;

    const std::string sysdir = "/sys/class/tty/";
    DIR* dir_handle = opendir(sysdir.c_str());
    if (dir_handle == nullptr)
        return matching;

    struct dirent* dir_entry;
    while ((dir_entry = readdir(dir_handle)) != nullptr)
    {
        // search for filenames starting with ttyACM
        const std::string port_name(dir_entry->d_name);
        if (port_name.compare(0, 6, "ttyACM") != 0)
            continue;

        // check that device file exists
        const std::string port_path = "/dev/" + port_name;
        if (!file_exists(port_path))
            continue;

        // get vendor and product id from /sys/class/tty/ttyACMXXX/device/uevent
        uint16_t vid = 0;
        uint16_t pid = 0;
        if (get_vid_pid(sysdir + port_name + "/device/uevent", vid, pid))
        {
            ComPortInfo port_info(port_name, port_path, vid, pid);
            if (predicate(port_info))
                matching.push_back(port_info);
        }
    }

    closedir(dir_handle);

    return matching;
}
#endif

uint32_t ComPortInfo::to_compat_buffer(const std::list<ComPortInfo>& ports, char* buffer, size_t buffer_size)
{
    if (!buffer_size)
        return 0;

    std::memset(buffer, 0, buffer_size);

    uint32_t count = 0;
    for (const auto& info : ports)
    {
        size_t pos = strlen(buffer);

        // left: buffer_size - trailing '\0' byte
        const size_t left = (buffer_size - 1) - pos;

        const std::string path = info.get_path();
        const bool semicol = (count != 0);  // extra space for semicolon needed?
        const size_t append_len = path.size() + (semicol ? 1 : 0);
        if (left < append_len)
            continue;

        if (semicol)
            buffer[pos++] = ';';

        std::memcpy(&buffer[pos], path.c_str(), path.length());
        count++;
    }

    return count;
}

uint32_t ifx_comport_get_list(uint16_t vid, uint16_t pid, char* port_list, size_t buffer_size)
{
    std::function<bool(const ComPortInfo&)> predicate = [vid, pid](const ComPortInfo& info) {
        return (info.get_vid() == vid && info.get_pid() == pid);
    };

    auto ports = ComPortInfo::enumerate(predicate);
    return ComPortInfo::to_compat_buffer(ports, port_list, buffer_size);
}

ComPort::ComPort(const char* port_name, uint32_t baud_rate)
{
    m_com = ifx_comport_open(port_name, baud_rate);
}

ComPort::~ComPort()
{
    close();
}

void ComPort::set_timeout(uint32_t timeout_period_ms)
{
    ifx_comport_set_timeout(m_com, timeout_period_ms);
}

size_t ComPort::send_data(const void* data, size_t num_bytes)
{
    return ifx_comport_send_data(m_com, data, num_bytes);
}

size_t ComPort::get_data(void* data, size_t num_requested_bytes)
{
    return ifx_comport_get_data(m_com, data, num_requested_bytes);
}

char ComPort::get_char()
{
    char c;
    if (get_data(&c, 1) != 1)
    {
        throw rdk::exception::communication_error();
    }
    return c;
}

void ComPort::close()
{
    if (m_com == nullptr)
        return;

    ifx_comport_close(m_com);

    m_com = nullptr;
}

com_t* ComPort::disown()
{
    auto* r = m_com;
    m_com = nullptr;
    return r;
}
