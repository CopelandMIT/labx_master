/**
 * @copyright 2020 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <platform/interfaces/access/ISpi.hpp>
#include <set>

class IVendorCommands;

/**
 * This class implements the SPI interface
 */
class BridgeProtocolSpi : public ISpi
{
public:
    BridgeProtocolSpi(IVendorCommands *commands);
    virtual ~BridgeProtocolSpi() = default;

    void configure(uint8_t devId, uint8_t flags, uint8_t wordSize, uint32_t speed) override;
    uint32_t getMaxTransfer() const override;

    void read(uint8_t devId, uint32_t count, uint8_t buffer[], bool keepSel = false) override;
    void read(uint8_t devId, uint32_t count, uint16_t buffer[], bool keepSel = false) override;
    void read(uint8_t devId, uint32_t count, uint32_t buffer[], bool keepSel = false) override;

    void write(uint8_t devId, uint32_t count, const uint8_t buffer[], bool keepSel = false) override;
    void write(uint8_t devId, uint32_t count, const uint16_t buffer[], bool keepSel = false) override;
    void write(uint8_t devId, uint32_t count, const uint32_t buffer[], bool keepSel = false) override;

    void transfer(uint8_t devId, uint32_t count, const uint8_t bufWrite[], uint8_t bufRead[], bool keepSel = false) override;
    void transfer(uint8_t devId, uint32_t count, const uint16_t bufWrite[], uint16_t bufRead[], bool keepSel = false) override;
    void transfer(uint8_t devId, uint32_t count, const uint32_t bufWrite[], uint32_t bufRead[], bool keepSel = false) override;

private:
    IVendorCommands *m_commands;
};
