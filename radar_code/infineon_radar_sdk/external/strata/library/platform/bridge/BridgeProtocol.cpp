/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "BridgeProtocol.hpp"
#include <common/Logger.hpp>
#include <common/Serialization.hpp>
#include <platform/exception/EProtocol.hpp>
#include <platform/exception/EProtocolFunction.hpp>


BridgeProtocol::BridgeProtocol(VendorCommandsImpl *commands) :
#ifdef STRATA_LEGACY_PROTOCOL_3
    vendorReqWrite {VENDOR_REQ_WRITE},
    vendorReqRead {VENDOR_REQ_READ},
    vendorReqTransfer {VENDOR_REQ_TRANSFER},
#endif
    m_commands(commands),
    m_I2c(m_commands),
    m_Gpio(m_commands),
    m_Spi(m_commands),
    m_Memory(m_commands),
    m_Flash(m_commands),
    m_Data(m_commands)
{
}

IVendorCommands *BridgeProtocol::getIVendorCommands()
{
    return m_commands;
}

IGpio *BridgeProtocol::getIGpio()
{
    return &m_Gpio;
}

II2c *BridgeProtocol::getII2c()
{
    return &m_I2c;
}

IData *BridgeProtocol::getIData()
{
    return &m_Data;
}

ISpi *BridgeProtocol::getISpi()
{
    return &m_Spi;
}

IFlash *BridgeProtocol::getIFlash()
{
    return &m_Flash;
}

IMemory<uint32_t> *BridgeProtocol::getIMemory()
{
    return &m_Memory;
}

void BridgeProtocol::setDefaultTimeout()
{
    m_commands->setDefaultTimeout();
}

uint16_t BridgeProtocol::getMaxTransfer() const
{
    return m_commands->getMaxTransfer();
}

IVendorCommands *BridgeProtocol::getIVendorCommands() const
{
    return m_commands;
}

void BridgeProtocol::readVersionInfo()
{
    m_commands->readVersionInfo(m_versionInfo.data(), m_versionInfo.size());
}

void BridgeProtocol::readExtendedVersion()
{
    constexpr uint16_t maxLength = 256;
    uint8_t buffer[maxLength];

    uint16_t wLength = maxLength;
    m_commands->vendorTransfer(REQ_BOARD_INFO, REQ_BOARD_INFO_EXTENDED_VERSION_WVALUE, 0, 0, nullptr, wLength, buffer);
    buffer[wLength - 1] = 0;  // make sure the string is terminated to avoid memory leak
    m_extendedVersionString.assign(reinterpret_cast<const char *>(buffer), wLength);
}

void BridgeProtocol::checkVersion()
{
#ifdef STRATA_LEGACY_PROTOCOL_3
    try
    {
        readVersionInfo();
    }
    catch (const EProtocolFunction &e)
    {
        if (e.code() == STATUS_REQUEST_TYPE_INVALID)
        {
            vendorReqRead     = VENDOR_REQ_READ_LEGACY;
            vendorReqWrite    = VENDOR_REQ_WRITE_LEGACY;
            vendorReqTransfer = VENDOR_REQ_TRANSFER_LEGACY;

            readVersionInfo();
        }
        else
        {
            throw;
        }
    }
#else
    readVersionInfo();
#endif

    LOG(DEBUG) << "Board firmware version = " << std::dec << m_versionInfo[0] << "." << m_versionInfo[1] << "." << m_versionInfo[2] << "." << m_versionInfo[3];
    LOG(DEBUG) << "Board protocol version = " << m_versionInfo[4] << "." << m_versionInfo[5];

    const uint16_t &boardVer = m_versionInfo[4];
    const uint16_t hostVer   = PROTOCOL_VERSION_MAJOR;

#ifdef STRATA_LEGACY_PROTOCOL_3
    if ((boardVer < hostVer) && (boardVer != 3))
#else
    if (boardVer < hostVer)
#endif
    {
        std::string msg("Incompatible protocol version! Please update board with firmware using protocol version ");
        msg.append(std::to_string(PROTOCOL_VERSION_MAJOR));
        msg.append(".");
        msg.append(std::to_string(PROTOCOL_VERSION_MINOR));
        msg.append(" (it has ");
        msg.append(std::to_string(m_versionInfo[4]));
        msg.append(".");
        msg.append(std::to_string(m_versionInfo[5]));
        msg.append(")");
        throw EProtocol(msg.c_str());
    }
    else if (boardVer > hostVer)
    {
        std::string msg("Protcol version mismatch! Please use Host Software with protocol version ");
        msg.append(std::to_string(m_versionInfo[4]));
        msg.append(".");
        msg.append(std::to_string(m_versionInfo[5]));
        msg.append(" (this is ");
        msg.append(std::to_string(PROTOCOL_VERSION_MAJOR));
        msg.append(".");
        msg.append(std::to_string(PROTOCOL_VERSION_MINOR));
        msg.append(")");
        throw EProtocol(msg.c_str());
    }
}

void BridgeProtocol::getBoardInfo(BoardInfo_t &buffer)
{
    uint16_t wLength = sizeof(buffer);

#ifdef STRATA_LEGACY_PROTOCOL_3
    try
    {
        m_commands->vendorTransfer(REQ_BOARD_INFO, REQ_BOARD_INFO_BOARD_INFO_WVALUE, 0, 0, nullptr, wLength, buffer.data());
    }
    catch (const EException &e)
    {
        if (e.code() == STATUS_REQUEST_TYPE_INVALID || e.code() == STATUS_HEADER_INCOMPLETE)
        {
            vendorReqRead     = VENDOR_REQ_READ_LEGACY;
            vendorReqWrite    = VENDOR_REQ_WRITE_LEGACY;
            vendorReqTransfer = VENDOR_REQ_TRANSFER_LEGACY;

            try
            {
                wLength = sizeof(buffer);
                m_commands->vendorTransfer(REQ_BOARD_INFO, REQ_BOARD_INFO_BOARD_INFO_WVALUE, 0, 0, nullptr, wLength, buffer.data());
            }
            catch (const EProtocolFunction &e)
            {
                if (e.code() == STATUS_REQUEST_WVALUE_INVALID || e.code() == STATUS_COMMAND_ID_INVALID || e.code() == STATUS_REQUEST_INVALID)
                {
                    // try reading fixed size info
                    wLength = 32;
                    m_commands->vendorRead(REQ_BOARD_INFO, 0, 0, wLength, buffer.data());
                }
                else
                {
                    throw;
                }
            }
        }
        else
        {
            throw;
        }
    }
#else
    m_commands->vendorTransfer(REQ_BOARD_INFO, REQ_BOARD_INFO_BOARD_INFO_WVALUE, 0, 0, nullptr, wLength, buffer.data());
#endif

    buffer[wLength - 1] = 0;  // make sure the string is terminated to avoid memory leak
}

void BridgeProtocol::getLastError(uint8_t buffer[4])
{
    m_commands->vendorRead(REQ_BOARD_INFO, REQ_BOARD_INFO_ERROR_INFO_WVALUE, REQ_BOARD_INFO_ERROR_INFO_LAST_ERROR_WINDEX, sizeof(*buffer) * 4, buffer);
}

void BridgeProtocol::getDetailedError(int32_t errors[], size_t count)
{
    const auto wLength = static_cast<uint16_t>(sizeof(*errors) * count);
    m_commands->vendorRead(REQ_BOARD_INFO, REQ_BOARD_INFO_ERROR_INFO_WVALUE, REQ_BOARD_INFO_ERROR_INFO_DETAILED_ERROR_WINDEX, wLength, errors);
}

void BridgeProtocol::activateBootloader()
{
#ifdef STRATA_LEGACY_PROTOCOL
    try
    {
        m_commands->vendorWrite(REQ_BOARD_INFO, REQ_BOARD_INFO_BOOTLOADER_WVALUE, 0);
    }
    catch (const EProtocolFunction &e)
    {
        if (e.code() == STATUS_REQUEST_WVALUE_INVALID)
        {
            constexpr auto REQ_CUSTOM_BOOT_LOADER_WVALUE              = 0x0042;
            constexpr auto REQ_CUSTOM_BOOT_LOADER_WINDEX_FUNCTION_CHK = 0x0001;
            constexpr auto REQ_CUSTOM_BOOT_LOADER_WINDEX_ACTIVATE     = 0x0002;

            m_commands->vendorRead(REQ_CUSTOM, REQ_CUSTOM_BOOT_LOADER_WVALUE, REQ_CUSTOM_BOOT_LOADER_WINDEX_FUNCTION_CHK, 0, nullptr);
            m_commands->vendorWrite(REQ_CUSTOM, REQ_CUSTOM_BOOT_LOADER_WVALUE, REQ_CUSTOM_BOOT_LOADER_WINDEX_ACTIVATE);
        }
        else
        {
            throw;
        }
    }
#else
    m_commands->vendorWrite(REQ_BOARD_INFO, REQ_BOARD_INFO_BOOTLOADER_WVALUE, 0);
#endif
}

const std::string &BridgeProtocol::getExtendedVersionString()
{
    if (m_extendedVersionString.empty())
    {
        readExtendedVersion();
    }

    return m_extendedVersionString;
}

void BridgeProtocol::readUuid()
{
    try
    {
        m_commands->vendorRead(REQ_BOARD_INFO, REQ_BOARD_INFO_UUID_WVALUE, 0, static_cast<uint16_t>(m_uuid.size()), m_uuid.data());
    }
    catch (const EProtocolFunction &e)
    {
        if ((e.code() == E_NOT_IMPLEMENTED) || (e.code() == STATUS_REQUEST_WVALUE_INVALID))
        {
            // the connected board does not support the UUID request, call the default implementation
            BridgeControl::readUuid();
        }
        else
        {
            throw;
        }
    }
}
