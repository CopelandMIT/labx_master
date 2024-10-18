/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <platform/interfaces/access/II2c.hpp>

class VendorExtensionRealtek;

class VendorExtensionRealtekI2c :
    public II2c
{
public:
    VendorExtensionRealtekI2c(VendorExtensionRealtek *vendorExtension);
    virtual ~VendorExtensionRealtekI2c() = default;

    uint16_t getMaxTransfer() const override;

    void configureBusSpeed(uint16_t devAddr, uint32_t speed) override;
    void clearBus(uint16_t devAddr) override;
    void pollForAck(uint16_t devAddr) override;

    void writeWithoutPrefix(uint16_t devAddr, uint16_t length, const uint8_t buffer[]) override;
    void writeWith8BitPrefix(uint16_t devAddr, uint8_t prefix, uint16_t length, const uint8_t buffer[]) override;
    void writeWith16BitPrefix(uint16_t devAddr, uint16_t prefix, uint16_t length, const uint8_t buffer[]) override;
    void readWithoutPrefix(uint16_t devAddr, uint16_t length, uint8_t buffer[]) override;
    void readWith8BitPrefix(uint16_t devAddr, uint8_t prefix, uint16_t length, uint8_t buffer[]) override;
    void readWith16BitPrefix(uint16_t devAddr, uint16_t prefix, uint16_t length, uint8_t buffer[]) override;

private:
    VendorExtensionRealtek *m_vendorExtension;
};
