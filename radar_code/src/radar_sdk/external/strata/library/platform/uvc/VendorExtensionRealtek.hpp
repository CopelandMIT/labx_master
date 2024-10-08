/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include "VendorExtensionRealtekFlash.hpp"
#include "VendorExtensionRealtekI2c.hpp"
#include <platform/bridge/BridgeControl.hpp>
#include <platform/interfaces/link/IUvcExtension.hpp>


// {1229A78C-47B4-4094-B0CE-DB07386FB938}
#define REALTEK_XU_GUID 0x8C, 0xA7, 0x29, 0x12, 0xB4, 0x47, 0x94, 0x40, 0xCE, 0xB0, 0xDB, 0x07, 0x38, 0x6F, 0xB9, 0x38


class VendorExtensionRealtek :
    public BridgeControl,
    private IGpio
{
private:
    constexpr static const uint16_t m_maxPayload = 4096;

public:
    VendorExtensionRealtek(IUvcExtension *uvcExtension);
    virtual ~VendorExtensionRealtek() = default;

    void checkVersion() override;
    void getBoardInfo(BoardInfo_t &buffer) override;
    uint16_t getMaxTransfer() const override;

    IGpio *getIGpio() override;
    II2c *getII2c() override;
    IFlash *getIFlash() override;

    //IGpio
    void configurePin(uint16_t id, uint8_t flags) override;
    void setPin(uint16_t id, bool state) override;
    bool getPin(uint16_t id) override;
    void configurePort(uint16_t port, uint8_t flags, uint32_t mask = 0xFFFFFFFF) override;
    void setPort(uint16_t port, uint32_t state, uint32_t mask = 0xFFFFFFFF) override;
    uint32_t getPort(uint16_t port) override;

    void vendorWrite(uint16_t command, uint16_t addressLow, uint16_t length, uint16_t addressHigh);
    void vendorWrite(uint16_t command, uint16_t addressLow, uint16_t length, uint16_t addressHigh, const uint8_t buffer[]);
    void vendorRead(uint16_t command, uint16_t addressLow, uint16_t length, uint16_t addressHigh, uint8_t buffer[]);
    void vendorTransfer(uint16_t commandSend, uint16_t addressLowSend, uint16_t lengthSend, uint16_t addressHighSend, const uint8_t bufferSend[],
                        uint16_t commandReceive, uint16_t addressLowReceive, uint16_t lengthReceive, uint16_t addressHighReceive, uint8_t bufferReceive[]);

private:
    void sendCommand(uint16_t command, uint16_t addressLow, uint16_t length, uint16_t addressHigh);
    void sendData(uint16_t length, const uint8_t buffer[]);
    void sendDataVariable(uint32_t chunkSize, uint32_t length, const uint8_t buffer[]);
    void receiveData(uint32_t length, uint8_t buffer[]);
    void receiveDataVariable(uint32_t chunkSize, uint32_t length, uint8_t buffer[]);
    void checkError();

    // Vendor extension access
    IUvcExtension *m_uvcExtension;
    VendorExtensionRealtekFlash m_flash;
    VendorExtensionRealtekI2c m_i2c;

    uint8_t m_buffer[m_maxPayload];
};
