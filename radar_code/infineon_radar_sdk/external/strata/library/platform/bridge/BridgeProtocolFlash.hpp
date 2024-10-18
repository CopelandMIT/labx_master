/**
 * @copyright 2020 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <platform/interfaces/access/IFlash.hpp>

class IVendorCommands;

/**
 * This class implements the Flash interface
 */
class BridgeProtocolFlash : public IFlash
{

public:
    BridgeProtocolFlash(IVendorCommands *commands);
    virtual ~BridgeProtocolFlash() = default;

    uint32_t getMaxTransfer() const override;

    void read(uint8_t devId, uint32_t address, uint32_t length, uint8_t buffer[]) override;
    void write(uint8_t devId, uint32_t address, uint32_t length, const uint8_t buffer[]) override;
    void erase(uint8_t devId, uint32_t address) override;
    uint8_t getStatus(uint8_t devId) override;

private:
    IVendorCommands *m_commands;
};
