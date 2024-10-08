/**
 * @copyright 2020 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <platform/interfaces/IVendorCommands.hpp>


class VendorCommandsImpl :
    public IVendorCommands
{
protected:
    constexpr static const uint16_t m_commandHeaderSize  = 8;
    constexpr static const uint16_t m_responseHeaderSize = 4;

public:
    void readVersionInfo(uint16_t *buffer, std::size_t count);

public:
    VendorCommandsImpl();
    uint32_t getProtocolVersion() const override;

private:
    uint32_t m_protocolVersion;
};
