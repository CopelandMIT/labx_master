/**
 * @copyright 2020 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <platform/interfaces/access/IGpio.hpp>

class IVendorCommands;

/**
 * This class implements the Gpio interface.
 */
class BridgeProtocolGpio : public IGpio
{
public:
    BridgeProtocolGpio(IVendorCommands *commands);
    virtual ~BridgeProtocolGpio() = default;

    void configurePin(uint16_t id, uint8_t flags) override;
    void setPin(uint16_t id, bool state) override;
    bool getPin(uint16_t id) override;
    void configurePort(uint16_t port, uint8_t flags, uint32_t mask = 0xFFFFFFFF) override;
    void setPort(uint16_t port, uint32_t state, uint32_t mask = 0xFFFFFFFF) override;
    uint32_t getPort(uint16_t port) override;

private:
    IVendorCommands *m_commands;
};
