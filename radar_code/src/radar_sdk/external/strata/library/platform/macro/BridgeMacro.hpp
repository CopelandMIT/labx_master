/**
 * @copyright 2022 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <platform/interfaces/IBridge.hpp>

#include <vector>


class BridgeMacro :
    public IBridge,
    public IVendorCommands,
    private IBridgeControl
{
public:
    BridgeMacro(IBridge *realBridge);
    ~BridgeMacro();

    //IBridge
    bool isConnected() override;
    void openConnection() override;
    void closeConnection() override;
    IBridgeControl *getIBridgeControl() override;
    IBridgeData *getIBridgeData() override;

    //IBridgeControl
    IVendorCommands *getIVendorCommands() override;
    void checkVersion() override;
    void setDefaultTimeout() override;
    uint16_t getMaxTransfer() const override;
    IData *getIData() override;
    IGpio *getIGpio() override;
    II2c *getII2c() override;
    ISpi *getISpi() override;
    IFlash *getIFlash() override;
    IMemory<uint32_t> *getIMemory() override;

    void getBoardInfo(IBridgeControl::BoardInfo_t &boardInfo) override;
    const VersionInfo_t &getVersionInfo() override;
    const std::string &getVersionString() override;
    const std::string &getExtendedVersionString() override;
    const Uuid_t &getUuid() override;
    const std::string &getUuidString() override;
    void activateBootloader() override;

    //IVendorCommands
    uint32_t getProtocolVersion() const override;
    void vendorWrite(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength, const uint8_t buffer[]) override;
    void vendorRead(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength, uint8_t buffer[]) override;
    void vendorTransfer(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLengthSend, const uint8_t bufferSend[], uint16_t &wLengthReceive, uint8_t bufferReceive[]) override;

    void loadRecording(const char *filename);
    void saveRecording(const char *filename);

    void enableRecording();
    void enablePlayback();
    void enableNormalMode();

    void wait(uint32_t microseconds);

private:
    void sendRequest(uint8_t bmReqType, uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wHeaderLength, const uint8_t buffer[]);
    void receiveResponse(uint8_t bmReqType, uint8_t bRequest, uint16_t &wLength, uint8_t buffer[]);

    void reset();
    void execute();
    void fetchResults();

    uint32_t m_maxLength;
    std::vector<uint8_t> m_requests;
    strata::buffer<uint8_t> m_responses;
    uint8_t *m_response;

    IBridge *m_realBridge;

    bool m_enabled;
    bool m_recording;

protected:
    constexpr static const uint16_t m_commandHeaderSize  = 8;
    constexpr static const uint16_t m_responseHeaderSize = 4;
};
