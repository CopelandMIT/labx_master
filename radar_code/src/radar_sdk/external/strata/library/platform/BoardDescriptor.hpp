/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <Definitions.hpp>
#include <platform/BoardInstance.hpp>
#include <platform/interfaces/IBridge.hpp>

#include <memory>
#include <string>
#include <vector>


class BoardDescriptor;
using BoardDescriptorList          = std::vector<std::unique_ptr<BoardDescriptor>>;
using BoardInstanceFactoryFunction = std::unique_ptr<BoardInstance>(std::shared_ptr<IBridge> &&bridge, BoardDescriptor *d);


struct BoardData
{
    using const_iterator = const BoardData *;

    const uint16_t vid;
    const uint16_t pid;
    BoardInstanceFactoryFunction &factory;
};


class BoardDescriptor
{
protected:
    BoardDescriptor(const BoardData &data, const char name[]);

public:
    BoardDescriptor(const BoardData &data, const char name[], std::shared_ptr<IBridge> &&bridge);

    virtual ~BoardDescriptor() = default;

    inline const char *getName()
    {
        return m_name.c_str();
    }

    inline const uint16_t &getVid()
    {
        return m_data.vid;
    }

    inline const uint16_t &getPid()
    {
        return m_data.pid;
    }

    inline const std::string &getVersionString()
    {
        return getIBridge()->getIBridgeControl()->getVersionString();
    }

    inline const std::string &getExtendedVersionString()
    {
        return getIBridge()->getIBridgeControl()->getExtendedVersionString();
    }

    inline const IBridgeControl::VersionInfo_t &getVersionInfo()
    {
        return getIBridge()->getIBridgeControl()->getVersionInfo();
    }

    inline const IBridgeControl::Uuid_t &getUuid()
    {
        return getIBridge()->getIBridgeControl()->getUuid();
    }

    inline const std::string &getUuidString()
    {
        return getIBridge()->getIBridgeControl()->getUuidString();
    }

    STRATA_API std::unique_ptr<BoardInstance> createBoardInstance();
    STRATA_API IBridge *getIBridge();

protected:
    virtual std::shared_ptr<IBridge> createBridge();

    const BoardData m_data;
    const std::string m_name;

    std::shared_ptr<IBridge> m_bridge;

private:
    void checkBridge();

    bool m_checked = false;
};
