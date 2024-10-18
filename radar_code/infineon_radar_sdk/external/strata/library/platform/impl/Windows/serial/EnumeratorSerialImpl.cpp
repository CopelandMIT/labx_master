/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "EnumeratorSerialImpl.hpp"

#include <platform/exception/EConnection.hpp>
#include <platform/serial/BoardSerial.hpp>
#include <platform/templates/enumerateFunction.hpp>

#include <windows.h>


static const char *devList[] = {
    "\\Device\\Serial",
    "\\Device\\USBSER",
    "\\Device\\VCP",
};

inline bool checkDevice(const char *devString)
{
    for (auto &c : devList)
    {
        if (!strncmp(devString, c, strlen(c)))
        {
            return true;
        }
    }
    return false;
}

void EnumeratorSerialImpl::enumerate(IEnumerationListener &listener, BoardData::const_iterator begin, BoardData::const_iterator end)
{
    LONG lResult;
    HKEY hKey;
    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Hardware\\DeviceMap\\SerialComm", 0, KEY_READ, &hKey);
    if ((lResult == ERROR_FILE_NOT_FOUND) || (lResult == ERROR_NO_MORE_ITEMS))
    {
        return;
    }
    if (lResult != ERROR_SUCCESS)
    {
        throw EConnection("Could not enumerate serial ports through registry", lResult);
    }

    char cValName[32];
    char cValue[8];
    DWORD dwValNameSize;
    DWORD dwValueSize;
    DWORD dwIndex = 0;

    do
    {
        dwValNameSize = sizeof(cValName);
        dwValueSize   = sizeof(cValue);
        lResult       = RegEnumValue(hKey, dwIndex++, cValName, &dwValNameSize, NULL, NULL, reinterpret_cast<LPBYTE>(cValue), &dwValueSize);
        if (lResult != ERROR_SUCCESS)
        {
            continue;
        }

        if (!checkDevice(cValName))
        {
            continue;
        }
        if (enumerateFunction<BoardSerial>(listener, begin, end, cValue))
        {
            break;
        }
    } while (lResult != ERROR_NO_MORE_ITEMS);

    RegCloseKey(hKey);
}
