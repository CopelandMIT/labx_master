/**
 * @copyright 2020 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <platform/Memory.hpp>

class IVendorCommands;

class BridgeProtocolMemory : public Memory<uint32_t>
{
public:
    BridgeProtocolMemory(IVendorCommands *commands);
    virtual ~BridgeProtocolMemory() = default;

    using IMemory<uint32_t>::read;
    using IMemory<uint32_t>::write;

    uint32_t read(uint32_t address) override;
    void write(uint32_t address, uint32_t value) override;
    void read(uint32_t address, uint32_t count, uint32_t data[]) override;
    void write(uint32_t address, uint32_t count, const uint32_t data[]) override;

private:
    IVendorCommands *m_commands;
};
