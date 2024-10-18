/**
 * @copyright 2022 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "BridgeMacro.hpp"

#include <common/Serialization.hpp>
#include <common/exception/EGenericException.hpp>
#include <platform/exception/EProtocol.hpp>
#include <platform/exception/EProtocolFunction.hpp>
#include <universal/protocol/protocol_definitions.h>

#include <fstream>


namespace
{
    static inline uint32_t getPadding(uint32_t length)
    {
        return (0 - length) % sizeof(uint32_t);
    }
}


BridgeMacro::BridgeMacro(IBridge *realBridge) :
    m_realBridge {realBridge},
    m_enabled {false},
    m_recording {false}
{
    m_realBridge->getIBridgeControl()->getIVendorCommands()->vendorReadParameters(REQ_MACRO, REQ_MACRO_REQUEST_BUFFER_WVALUE, 0, m_maxLength);
    m_requests.reserve(m_maxLength);
    reset();
}

BridgeMacro::~BridgeMacro()
{
}

bool BridgeMacro::isConnected()
{
    return m_realBridge->isConnected();
}

void BridgeMacro::openConnection()
{
    m_realBridge->openConnection();
}

void BridgeMacro::closeConnection()
{
    m_realBridge->closeConnection();
}

IBridgeData *BridgeMacro::getIBridgeData()
{
    return m_realBridge->getIBridgeData();
}

IBridgeControl *BridgeMacro::getIBridgeControl()
{
    return this;
}

IVendorCommands *BridgeMacro::getIVendorCommands()
{
    return this;
}

void BridgeMacro::getBoardInfo(IBridgeControl::BoardInfo_t &boardInfo)
{
    m_realBridge->getIBridgeControl()->getBoardInfo(boardInfo);
}

void BridgeMacro::checkVersion()
{
    m_realBridge->getIBridgeControl()->checkVersion();
}

const IBridgeControl::VersionInfo_t &BridgeMacro::getVersionInfo()
{
    return m_realBridge->getIBridgeControl()->getVersionInfo();
}

const std::string &BridgeMacro::getVersionString()
{
    return m_realBridge->getIBridgeControl()->getVersionString();
}

const std::string &BridgeMacro::getExtendedVersionString()
{
    return m_realBridge->getIBridgeControl()->getExtendedVersionString();
}

const IBridgeControl::Uuid_t &BridgeMacro::getUuid()
{
    return m_realBridge->getIBridgeControl()->getUuid();
}

const std::string &BridgeMacro::getUuidString()
{
    return m_realBridge->getIBridgeControl()->getUuidString();
}

void BridgeMacro::activateBootloader()
{
    m_realBridge->getIBridgeControl()->activateBootloader();
}

void BridgeMacro::setDefaultTimeout()
{
    m_realBridge->getIBridgeControl()->setDefaultTimeout();
}

uint16_t BridgeMacro::getMaxTransfer() const
{
    return m_realBridge->getIBridgeControl()->getMaxTransfer();
}

IData *BridgeMacro::getIData()
{
    return m_realBridge->getIBridgeControl()->getIData();
}

IGpio *BridgeMacro::getIGpio()
{
    return m_realBridge->getIBridgeControl()->getIGpio();
}

II2c *BridgeMacro::getII2c()
{
    return m_realBridge->getIBridgeControl()->getII2c();
}

ISpi *BridgeMacro::getISpi()
{
    return m_realBridge->getIBridgeControl()->getISpi();
}

IFlash *BridgeMacro::getIFlash()
{
    return m_realBridge->getIBridgeControl()->getIFlash();
}

IMemory<uint32_t> *BridgeMacro::getIMemory()
{
    return m_realBridge->getIBridgeControl()->getIMemory();
}

uint32_t BridgeMacro::getProtocolVersion() const
{
    return m_realBridge->getIBridgeControl()->getIVendorCommands()->getProtocolVersion();
}

void BridgeMacro::sendRequest(uint8_t bmReqType, uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength, const uint8_t buffer[])
{
    const auto padding         = getPadding(wLength);
    const uint32_t requestSize = m_commandHeaderSize + wLength + padding;
    if (requestSize > m_maxLength - m_requests.size())
    {
        throw EGenericException("Capacity of internal macro buffer has been exceeded");
    }

    m_requests.insert(m_requests.end(), {bmReqType,
                                         bRequest,
                                         static_cast<uint8_t>(wValue),
                                         static_cast<uint8_t>(wValue >> 8),
                                         static_cast<uint8_t>(wIndex),
                                         static_cast<uint8_t>(wIndex >> 8),
                                         static_cast<uint8_t>(wLength),
                                         static_cast<uint8_t>(wLength >> 8)});

    if (bmReqType == VENDOR_REQ_READ)
    {
        wLength = 0;
    }

    if (wLength)
    {
        m_requests.insert(m_requests.end(), buffer, buffer + wLength);
        m_requests.insert(m_requests.end(), padding, 0);  // ensure 32-bit buffer alignment
    }
}

void BridgeMacro::vendorWrite(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength, const uint8_t buffer[])
{
    if (!m_enabled)
    {
        m_realBridge->getIBridgeControl()->getIVendorCommands()->vendorWrite(bRequest, wValue, wIndex, wLength, buffer);
        return;
    }

    if (m_recording)
    {
        sendRequest(VENDOR_REQ_WRITE, bRequest, wValue, wIndex, wLength, buffer);
    }
    else
    {
        wLength = 0;
        receiveResponse(VENDOR_REQ_WRITE, bRequest, wLength, nullptr);
    }
}

void BridgeMacro::vendorRead(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength, uint8_t buffer[])
{
    if (!m_enabled)
    {
        m_realBridge->getIBridgeControl()->getIVendorCommands()->vendorRead(bRequest, wValue, wIndex, wLength, buffer);
        return;
    }

    if (m_recording)
    {
        sendRequest(VENDOR_REQ_READ, bRequest, wValue, wIndex, wLength, nullptr);
    }
    else
    {
        receiveResponse(VENDOR_REQ_READ, bRequest, wLength, buffer);
    }
}

void BridgeMacro::vendorTransfer(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLengthSend, const uint8_t bufferSend[], uint16_t &wLengthReceive, uint8_t bufferReceive[])
{
    if (!m_enabled)
    {
        m_realBridge->getIBridgeControl()->getIVendorCommands()->vendorTransfer(bRequest, wValue, wIndex, wLengthSend, bufferSend, wLengthReceive, bufferReceive);
        return;
    }

    if (m_recording)
    {
        sendRequest(VENDOR_REQ_TRANSFER, bRequest, wValue, wIndex, wLengthSend, bufferSend);
    }
    else
    {
        receiveResponse(VENDOR_REQ_TRANSFER, bRequest, wLengthReceive, bufferReceive);
    }
}

void BridgeMacro::receiveResponse(uint8_t bmReqType, uint8_t bRequest, uint16_t &wLength, uint8_t buffer[])
{
    if (m_response == m_responses.end())
    {
        throw EGenericException("Internal macro buffer does not contain any responses");
    }
    if (m_response + m_responseHeaderSize > m_responses.end())
    {
        throw EGenericException("Internal macro buffer contains an incomplete response header");
    }

    const uint8_t &bmResType = *m_response++;
    const uint8_t &bStatus   = *m_response++;
    if (bmResType != bmReqType)
    {
        throw EProtocol("Request response type error", (bmResType << 8) | bmReqType);
    }

    const auto maxLength = wLength;
    wLength              = serialToHost<uint16_t>(m_response);
    m_response += sizeof(wLength);

    if (bmReqType == VENDOR_REQ_READ)
    {
        if (wLength != maxLength)
        {
            throw EProtocol("Read request response length error", (wLength << 16) | maxLength);
        }
    }
    if (wLength > maxLength)
    {
        throw EProtocol("Request response too long for buffer", (wLength << 16) | (bmReqType << 8) | bRequest);
    }
    if (wLength != 0)
    {
        const auto padding = getPadding(wLength);
        if (m_response + wLength + padding > m_responses.end())
        {
            throw EGenericException("Internal macro buffer contains an incomplete response payload");
        }

        std::copy(m_response, m_response + wLength, buffer);
        m_response += wLength;
        m_response += padding;  // ensure 32-bit buffer alignment
    }

    if (bStatus)
    {
        throw EProtocolFunction(bStatus);
    }
}

void BridgeMacro::reset()
{
    m_requests.clear();
    m_responses.clear();
}

void BridgeMacro::execute()
{
    if (m_requests.empty())
    {
        throw EGenericException("Internal macro buffer does not contain any commands to be executed");
    }

    // transmit requests to device
    const auto maxTransferSize = getMaxTransfer();
    uint8_t *ptr               = m_requests.data();
    auto bufferSize            = m_requests.size();
    while (bufferSize)
    {
        const auto transferSize = (bufferSize > maxTransferSize) ? maxTransferSize : bufferSize;
        m_realBridge->getIBridgeControl()->getIVendorCommands()->vendorWrite(REQ_MACRO, REQ_MACRO_REQUEST_BUFFER_WVALUE, 0, transferSize, ptr);
        bufferSize -= transferSize;
        ptr += transferSize;
    }

    // execute requests on device
    m_realBridge->getIBridgeControl()->getIVendorCommands()->vendorWrite(REQ_MACRO, REQ_MACRO_REQUEST_BUFFER_WVALUE, 0);
}

void BridgeMacro::saveRecording(const char *filename)
{
    if (m_requests.empty())
    {
        throw EGenericException("Internal macro buffer does not contain any commands to be saved");
    }

    std::ofstream outFile;

    if (filename != nullptr)
    {
        if (filename[0])
        {
            outFile.open(filename, std::ios::out | std::ios::binary);
        }
    }

    outFile.write(reinterpret_cast<char *>(m_requests.data()), m_requests.size());
    outFile.close();
}

void BridgeMacro::loadRecording(const char *filename)
{
    enableRecording();

    std::ifstream inFile;

    if (filename != nullptr)
    {
        if (filename[0])
        {
            inFile.open(filename, std::ios::in | std::ios::binary);
        }
    }

    m_requests.assign((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
    inFile.close();
}

void BridgeMacro::fetchResults()
{
    uint32_t responseSize;
    m_realBridge->getIBridgeControl()->getIVendorCommands()->vendorReadParameters(REQ_MACRO, REQ_MACRO_RESPONSE_BUFFER_WVALUE, 0, responseSize);
    m_responses.resize(responseSize);
    m_response = m_responses.data();

    const auto maxTransferSize = getMaxTransfer();
    uint8_t *buffer            = m_response;
    while (responseSize)
    {
        uint16_t transferSize = (responseSize > maxTransferSize) ? maxTransferSize : responseSize;
        m_realBridge->getIBridgeControl()->getIVendorCommands()->vendorTransfer(REQ_MACRO, REQ_MACRO_RESPONSE_BUFFER_WVALUE, 0, 0, NULL, transferSize, buffer);
        responseSize -= transferSize;
        buffer += transferSize;
    }
}

void BridgeMacro::enableRecording()
{
    m_enabled   = true;
    m_recording = true;
    reset();
}

void BridgeMacro::enablePlayback()
{
    execute();
    reset();
    fetchResults();
    m_enabled   = true;
    m_recording = false;
}

void BridgeMacro::enableNormalMode()
{
    m_enabled = false;
    reset();
}

void BridgeMacro::wait(uint32_t microseconds)
{
    vendorWriteParameters(REQ_MACRO, REQ_MACRO_EXTENDED_FUNC_WVALUE, REQ_MACRO_EXTENDED_FUNC_DELAY_WINDEX, microseconds);
}
