/**
 * @copyright 2020 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "VendorExtensionCypress.hpp"

#include <platform/exception/EProtocol.hpp>
#include <platform/exception/EProtocolFunction.hpp>
#include <platform/exception/EUvcProperty.hpp>


#ifdef STRATA_LEGACY_PROTOCOL_3
    #undef VENDOR_REQ_WRITE
    #undef VENDOR_REQ_READ

    #define VENDOR_REQ_WRITE vendorReqWrite
    #define VENDOR_REQ_READ  vendorReqRead
#endif


VendorExtensionCypress::VendorExtensionCypress(IUvcExtension *uvcExtension) :
    BridgeProtocol(this),
    m_uvcExtension {uvcExtension}
{
    m_variableSizeSupport = m_uvcExtension->variableSizeSupport();
    if (!m_variableSizeSupport)
    {
        m_fixedBuffer.resize(m_maxPayload);
    }
}

void VendorExtensionCypress::setDefaultTimeout()
{
}

uint16_t VendorExtensionCypress::getMaxTransfer() const
{
    return m_maxPayload;
}

void VendorExtensionCypress::sendRequest(uint8_t bmReqType, uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength, const uint8_t buffer[])
{
    if ((bmReqType != VENDOR_REQ_READ) && (wLength > 0) && (wLength < 56))
    {
        uint8_t setupPacket[56 + 8];
        setupPacket[56 + 0] = bmReqType;
        setupPacket[56 + 1] = bRequest;

        setupPacket[56 + 2] = static_cast<uint8_t>(wValue);
        setupPacket[56 + 3] = static_cast<uint8_t>(wValue >> 8);
        setupPacket[56 + 4] = static_cast<uint8_t>(wIndex);
        setupPacket[56 + 5] = static_cast<uint8_t>(wIndex >> 8);
        setupPacket[56 + 6] = static_cast<uint8_t>(wLength);
        setupPacket[56 + 7] = static_cast<uint8_t>(wLength >> 8);
        std::copy(buffer, buffer + wLength, setupPacket);

        m_uvcExtension->setProperty(UVC_XU_PROPERTY_DATA_SETUP_56_8, sizeof(setupPacket), setupPacket);
    }
    else
    {
        const uint8_t setupPacket[8] = {
            bmReqType,
            bRequest,
            static_cast<uint8_t>(wValue),
            static_cast<uint8_t>(wValue >> 8),
            static_cast<uint8_t>(wIndex),
            static_cast<uint8_t>(wIndex >> 8),
            static_cast<uint8_t>(wLength),
            static_cast<uint8_t>(wLength >> 8),
        };

        m_uvcExtension->setProperty(UVC_XU_PROPERTY_SETUP, sizeof(setupPacket), setupPacket);
        if ((bmReqType != VENDOR_REQ_READ) && (wLength > 0))
        {
            m_uvcExtension->setProperty(UVC_XU_PROPERTY_DATA_VARIABLE, wLength, buffer);
        }
    }
}

void VendorExtensionCypress::vendorWrite(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength, const uint8_t buffer[])
{
    std::lock_guard<IUvcExtension> lock(*m_uvcExtension);
    sendRequest(VENDOR_REQ_WRITE, bRequest, wValue, wIndex, wLength, buffer);
    checkStatus();
}

void VendorExtensionCypress::vendorRead(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength, uint8_t buffer[])
{
    std::lock_guard<IUvcExtension> lock(*m_uvcExtension);
    sendRequest(VENDOR_REQ_READ, bRequest, wValue, wIndex, wLength, nullptr);
    try
    {
        getDataProperty(wLength, buffer);
    }
    catch (const EUvcProperty &e)
    {
        // We had an error, e.g. stall. Now we check what the status of the firmware is.
        // If the firmware is fine, i.e. no exception, then we reraise the exception we just got.
        // Because in that case there might have been a different issue.
        checkStatus();
        throw e;
    }
}

void VendorExtensionCypress::vendorTransfer(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLengthSend, const uint8_t bufferSend[], uint16_t &wLengthReceive, uint8_t bufferReceive[])
{
    std::lock_guard<IUvcExtension> lock(*m_uvcExtension);
    sendRequest(VENDOR_REQ_TRANSFER, bRequest, wValue, wIndex, wLengthSend, bufferSend);

    const auto maxLength = wLengthReceive;
    wLengthReceive       = checkStatus();
    if (wLengthReceive > maxLength)
    {
        throw EProtocol("Request response too long for buffer", (wLengthReceive << 16) | (VENDOR_REQ_TRANSFER << 8) | bRequest);
    }

    getDataProperty(wLengthReceive, bufferReceive);
}

uint16_t VendorExtensionCypress::checkStatus()
{
    uint8_t setupPacket[8];

    m_uvcExtension->getProperty(UVC_XU_PROPERTY_SETUP, 8, setupPacket);
    if (setupPacket[2])
    {
        throw EProtocolFunction(setupPacket[4]);
    }
    return (setupPacket[7] << 8) | setupPacket[6];  //return the length out of the packet
}

void VendorExtensionCypress::getDataProperty(uint16_t length, uint8_t buffer[])
{
    if (m_variableSizeSupport)
    {
        m_uvcExtension->getProperty(UVC_XU_PROPERTY_DATA_VARIABLE, length, buffer);
    }
    else
    {
        switch (length)
        {
            case 64:
                m_uvcExtension->getProperty(UVC_XU_PROPERTY_DATA_64, 64, buffer);
                break;
            case 512:
                m_uvcExtension->getProperty(UVC_XU_PROPERTY_DATA_512, 512, buffer);
                break;
            case 4096:
                m_uvcExtension->getProperty(UVC_XU_PROPERTY_DATA_4096, 4096, buffer);
                break;
            default:
                if (length <= 64)
                {
                    m_uvcExtension->getProperty(UVC_XU_PROPERTY_DATA_64, 64, m_fixedBuffer.data());
                }
                else if (length <= 512)
                {
                    m_uvcExtension->getProperty(UVC_XU_PROPERTY_DATA_512, 512, m_fixedBuffer.data());
                }
                else
                {
                    m_uvcExtension->getProperty(UVC_XU_PROPERTY_DATA_4096, 4096, m_fixedBuffer.data());
                }
                std::copy(m_fixedBuffer.data(), m_fixedBuffer.data() + length, buffer);
                break;
        }
    }
}
