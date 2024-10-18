/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <platform/interfaces/IBridgeControl.hpp>


class BridgeControl :
    public IBridgeControl
{
public:
    BridgeControl();

    virtual IVendorCommands *getIVendorCommands() override;
    void checkVersion() override;
    virtual void setDefaultTimeout() override;
    uint16_t getMaxTransfer() const override;
    virtual IData *getIData() override;
    virtual IGpio *getIGpio() override;
    virtual II2c *getII2c() override;
    virtual ISpi *getISpi() override;
    virtual IFlash *getIFlash() override;
    virtual IMemory<uint32_t> *getIMemory() override;

    const VersionInfo_t &getVersionInfo() override;
    const std::string &getVersionString() override;
    const std::string &getExtendedVersionString() override;
    const Uuid_t &getUuid() override;
    const std::string &getUuidString() override;
    void activateBootloader() override;

protected:
    virtual void readUuid();

    VersionInfo_t m_versionInfo;
    Uuid_t m_uuid;
    std::string m_uuidString;
    std::string m_versionString;
    std::string m_extendedVersionString;

private:
    bool m_uuidRead;
};
