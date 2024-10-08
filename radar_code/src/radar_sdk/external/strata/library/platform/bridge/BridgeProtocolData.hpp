/**
 * @copyright 2020 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <platform/interfaces/access/IData.hpp>

class IVendorCommands;

/**
 * This class implements the Data interface
 */
class BridgeProtocolData : public IData
{
public:
    using IData::configure;

    BridgeProtocolData(IVendorCommands *commands);
    virtual ~BridgeProtocolData() = default;

    void configure(uint8_t index, const IDataProperties_t *dataProperties, const uint8_t *settings, uint16_t settingsSize) override;
    void start(uint8_t index = 0) override;
    void stop(uint8_t index = 0) override;
    uint32_t getStatusFlags(uint8_t index = 0) override;

private:
    IVendorCommands *m_commands;
};
