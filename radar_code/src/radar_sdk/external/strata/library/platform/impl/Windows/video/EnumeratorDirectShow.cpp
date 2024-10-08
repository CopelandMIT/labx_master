/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "EnumeratorDirectShow.hpp"
#include "BridgeDirectShow.hpp"
#include "EDirectShow.hpp"
#include "EnumeratorDirectShowDefinitions.hpp"
#include "Win32Utils.hpp"

#include <common/Logger.hpp>
#include <common/cpp11/memory.hpp>
#include <platform/BoardInstance.hpp>
#include <platform/exception/EAlreadyOpened.hpp>


bool BoardDescriptorDirectShow::isUsed()
{
    if (!m_instanceId)
    {
        throw EAlreadyOpened();
    }

    m_instanceId->AddRef();
    return (m_instanceId->Release() > 1);
}

std::shared_ptr<IBridge> BoardDescriptorDirectShow::createBridge()
{
    if (isUsed())
    {
        throw EAlreadyOpened();
    }
    return std::make_shared<BridgeDirectShow>(std::move(m_instanceId), std::move(m_deviceId));
}

EnumeratorDirectShow::EnumeratorDirectShow()
{
    auto hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
        LOG(ERROR) << "CoInitialize failed!";
    }
}

EnumeratorDirectShow::~EnumeratorDirectShow()
{
    CoUninitialize();
}

void EnumeratorDirectShow::enumerate(IEnumerationListener &listener, BoardData::const_iterator begin, BoardData::const_iterator end)
{
    ICreateDevEnumPtr devEnum;
    auto hr = devEnum.CreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC);
    if (FAILED(hr))
    {
        throw EDirectShow("Could not enumerate devices");
    }

    IEnumMonikerPtr enumCat;
    hr = devEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &enumCat, 0);
    if (hr != S_OK)  // hr==S_FALSE means there were no devices, and enumCat is a nullptr
    {
        return;
    }

    IMonikerPtr moniker;
    _variant_t varStr;

    uint16_t VID, PID;
    const size_t length = 64;
    char cName[length];

    LOG(DEBUG) << "Enumerating video devices ...";
    while (enumCat->Next(1, &moniker, NULL) == S_OK)
    {
        IPropertyBagPtr propBag;
        hr = moniker->BindToStorage(NULL, NULL, IID_IPropertyBag, reinterpret_cast<void **>(&propBag));
        if (FAILED(hr))
        {
            continue;
        }

        // Extract the VID/PID information from device path
        hr = propBag->Read(L"DevicePath", &varStr, NULL);
        if (FAILED(hr))
        {
            continue;
        }

        if (!getIds(varStr.bstrVal, VID, PID))
        {
            continue;
        }

        for (auto it = begin; it < end; it++)
        {
            if ((it->vid == VID) && (it->pid == PID))
            {
                std::string deviceString = copyDeviceString(varStr.bstrVal);

                hr = propBag->Read(L"FriendlyName", &varStr, NULL);
                //const size_t length = SysStringLen (varString.bstrVal);
                if (SUCCEEDED(hr))
                {
                    size_t s;
                    wcstombs_s(&s, cName, length, varStr.bstrVal, length);
                }
                else
                {
                    cName[0] = 0;
                }
                LOG(DEBUG) << "... device found: VID = " << std::hex << VID << " ; PID = " << PID << " ; name = \"" << cName << "\"";
                const bool stop = listener.onEnumerate(std::make_unique<BoardDescriptorDirectShow>(*it, cName, std::move(moniker), std::move(deviceString)));
                if (stop)
                {
                    return;
                }

                // reset to defined state after using std::move
                moniker = decltype(moniker)(0);

                // stop this loop since board was already found, continue with outer loop
                break;
            }
        }

        // Release if not recognized and therefor not moved to enumeratedList
        if (moniker)
        {
            //LOG (DEBUG) << "... unrecognized device: VID = " << std::hex << VID << " ; PID = " << PID;
            moniker.Release();
        }
    }
}
