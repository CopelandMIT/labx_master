/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include "ComDefines.hpp"
#include <platform/interfaces/IEnumerator.hpp>


class BoardDescriptorDirectShow :
    public BoardDescriptor
{
public:
    BoardDescriptorDirectShow(const BoardData &data, const char name[], IMonikerPtr &&instanceId, std::string &&deviceId) :
        BoardDescriptor(data, name),
        m_instanceId {std::move(instanceId)},
        m_deviceId {std::move(deviceId)}
    {
        CoInitialize(NULL);
    }

    virtual ~BoardDescriptorDirectShow()
    {
        // make sure all pointers are released before CoUninitialize()
        // (since depending on the calling order from other places, it might de-allocate all memory immediately)
        if (m_instanceId)
        {
            m_instanceId.Release();
        }
        CoUninitialize();
    }

    std::shared_ptr<IBridge> createBridge() override;

private:
    bool isUsed();

    IMonikerPtr m_instanceId;
    std::string m_deviceId;
};


class EnumeratorDirectShow :
    public IEnumerator
{
public:
    EnumeratorDirectShow();
    ~EnumeratorDirectShow();

    void enumerate(IEnumerationListener &listener, BoardData::const_iterator begin, BoardData::const_iterator end) override;
};
