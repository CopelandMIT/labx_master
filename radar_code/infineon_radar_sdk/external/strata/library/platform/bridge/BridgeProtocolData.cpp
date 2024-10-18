/**
 * @copyright 2020 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "BridgeProtocolData.hpp"
#include <common/Buffer.hpp>
#include <components/radar/TypeSerialization.hpp>
#include <platform/interfaces/IVendorCommands.hpp>
#include <universal/protocol/protocol_definitions.h>


BridgeProtocolData::BridgeProtocolData(IVendorCommands *commands) :
    m_commands(commands)
{
}

void BridgeProtocolData::configure(uint8_t index, const IDataProperties_t *dataProperties, const uint8_t *settings, const uint16_t settingsSize)
{
    const auto length = static_cast<uint16_t>(serialized_sizeof(IDataProperties_t())) + settingsSize;
    strata::buffer<uint8_t> buf(length);
    uint8_t *it = buf.data();
    it          = hostToSerial(it, dataProperties);
    std::copy(settings, settings + settingsSize, it);

    m_commands->vendorWrite(REQ_DATA,
                            REQ_DATA_CONFIGURE,
                            index,
                            length,
                            buf.data());
}

void BridgeProtocolData::start(uint8_t index)
{
    m_commands->vendorWrite(REQ_DATA,
                            REQ_DATA_START,
                            index);
}

void BridgeProtocolData::stop(uint8_t index)
{
    m_commands->vendorWrite(REQ_DATA,
                            REQ_DATA_STOP,
                            index);
}

uint32_t BridgeProtocolData::getStatusFlags(uint8_t index)
{
    uint32_t flags;
    m_commands->vendorRead(REQ_DATA,
                           REQ_DATA_STATUS_FLAGS,
                           index,
                           sizeof(flags),
                           &flags);
    return flags;
}
