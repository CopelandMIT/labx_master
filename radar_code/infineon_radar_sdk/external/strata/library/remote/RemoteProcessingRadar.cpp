/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "RemoteProcessingRadar.hpp"

#include <common/Buffer.hpp>
#include <common/Serialization.hpp>
#include <components/radar/TypeSerialization.hpp>
#include <platform/exception/EProtocol.hpp>
#include <universal/components/processing.h>
#include <universal/components/processing/iprocessingradar.h>
#include <universal/components/radar.h>
#include <universal/components/subinterfaces.h>
#include <universal/protocol/protocol_definitions.h>


RemoteProcessingRadar::RemoteProcessingRadar(IVendorCommands *commands, uint8_t id) :
    m_commands {commands, CMD_COMPONENT, COMPONENT_TYPE_PROCESSING_RADAR, id, COMPONENT_SUBIF_DEFAULT}
{
}

void RemoteProcessingRadar::configure(uint8_t dataSource, const IDataProperties_t *dataProperties, const IProcessingRadarInput_t *radarInfo, const IfxRsp_Stages *stages,
                                      const IfxRsp_AntennaCalibration *antennaConfig)
{
    const size_t expectedSize = sizeof(dataSource) + serialized_sizeof(IDataProperties_t()) + serialized_sizeof(IProcessingRadarInput_t()) + serialized_sizeof(IfxRsp_Stages());
    const size_t optionalSize = expectedSize + 2 * serialized_sizeof(IfxRsp_AntennaCalibration());
    uint8_t buf[optionalSize];

    size_t size;
    uint8_t *it = buf;
    it          = hostToSerial(it, dataSource);
    it          = hostToSerial(it, dataProperties);
    it          = hostToSerial(it, radarInfo);
    it          = hostToSerial(it, stages);
    if (antennaConfig)
    {
        it   = hostToSerial(it, &antennaConfig[0]);
        it   = hostToSerial(it, &antennaConfig[1]);
        size = optionalSize;
    }
    else
    {
        size = expectedSize;
    }

    m_commands.vendorWrite(FN_PROCESSING_RADAR_CONFIGURE, static_cast<uint16_t>(size), buf);
}

void RemoteProcessingRadar::doFft(const IfxRsp_Signal *input, const IfxRsp_FftSetting *settings, IfxRsp_Signal *output, uint16_t samples, uint16_t offset, uint8_t dimension, uint8_t format)
{
    constexpr size_t size = serialized_sizeof(IfxRsp_Signal()) + serialized_sizeof(IfxRsp_FftSetting()) + sizeof(samples) + sizeof(offset) + sizeof(dimension) + sizeof(format);
    uint8_t buf[size];
    uint8_t *it = buf;
    it          = hostToSerial(it, input);
    it          = hostToSerial(it, settings);
    it          = hostToSerial(it, samples);
    it          = hostToSerial(it, offset);
    it          = hostToSerial(it, dimension);
    it          = hostToSerial(it, format);
    m_commands.vendorWrite(FN_PROCESSING_RADAR_DO_FFT, static_cast<uint16_t>(size), buf);

    m_commands.vendorReadToStruct(FN_PROCESSING_RADAR_READ_OPERATION_RESULT, output);
}

void RemoteProcessingRadar::doNci(const IfxRsp_Signal *input, uint8_t format, IfxRsp_Signal *output)
{
    constexpr size_t size = serialized_sizeof(IfxRsp_Signal()) + sizeof(format);
    uint8_t buf[size];
    uint8_t *it = buf;
    it          = hostToSerial(it, input);
    it          = hostToSerial(it, format);

    m_commands.vendorWrite(FN_PROCESSING_RADAR_DO_NCI, static_cast<uint16_t>(size), buf);

    m_commands.vendorReadToStruct(FN_PROCESSING_RADAR_READ_OPERATION_RESULT, output);
}

void RemoteProcessingRadar::doThresholding(const IfxRsp_Signal *input, uint8_t dimension, const IfxRsp_ThresholdingSetting *settings, IfxRsp_Signal *output)
{
    constexpr size_t size = serialized_sizeof(IfxRsp_Signal()) + sizeof(dimension) +
                            serialized_sizeof(IfxRsp_ThresholdingSetting());
    uint8_t buf[size];
    uint8_t *it = buf;
    it          = hostToSerial(it, input);
    it          = hostToSerial(it, dimension);
    it          = hostToSerial(it, settings);

    m_commands.vendorWrite(FN_PROCESSING_RADAR_DO_THRESHOLDING, static_cast<uint16_t>(size), buf);

    m_commands.vendorReadToStruct(FN_PROCESSING_RADAR_READ_OPERATION_RESULT, output);
}

void RemoteProcessingRadar::doPsd(const IfxRsp_Signal *input, uint16_t nFft, IfxRsp_Signal *output)
{
    constexpr size_t size = serialized_sizeof(IfxRsp_Signal()) + sizeof(nFft);
    uint8_t buf[size];
    uint8_t *it = buf;
    it          = hostToSerial(it, input);
    it          = hostToSerial(it, nFft);

    m_commands.vendorWrite(FN_PROCESSING_RADAR_DO_PSD, static_cast<uint16_t>(size), buf);

    m_commands.vendorReadToStruct(FN_PROCESSING_RADAR_READ_OPERATION_RESULT, output);
}

void RemoteProcessingRadar::reinitialize()
{
    m_commands.vendorWrite(FN_PROCESSING_RADAR_REINIT);
}

void RemoteProcessingRadar::writeConfigRam(uint16_t offset, uint16_t count, const uint32_t values[])
{
    //No configuration shall be written while it is busy
    if (isBusy())
    {
        reinitialize();
    }

    constexpr uint16_t elemSize = sizeof(values[0]);
    constexpr uint16_t argSize  = sizeof(offset);  //number of additional bytes besides the actual data
    const uint16_t maxCount     = (m_commands.getMaxTransfer() - argSize) / elemSize;
    const size_t payloadSize    = std::min(maxCount, count) * elemSize + argSize;
    strata::buffer<uint8_t> payload(payloadSize);

    while (count > 0)
    {
        const uint16_t wCount = std::min(count, maxCount);
        uint8_t *it           = payload.data();
        it                    = hostToSerial(it, values, values + wCount);
        it                    = hostToSerial(it, offset);

        const uint16_t wLength = wCount * elemSize + argSize;
        m_commands.vendorWrite(FN_PROCESSING_RADAR_WRITE_CONFIG_RAM, wLength, payload.data());

        offset += wCount;
        values += wCount;
        count -= wCount;
    }
}

void RemoteProcessingRadar::writeCustomWindowCoefficients(uint8_t slotNr, uint16_t offset, uint16_t count, const uint32_t coefficients[])
{
    constexpr uint16_t elemSize = sizeof(coefficients[0]);
    constexpr uint16_t argSize  = sizeof(slotNr) + sizeof(offset);  //number of additional bytes besides the actual data
    const uint16_t maxCount     = (m_commands.getMaxTransfer() - argSize) / elemSize;
    const size_t payloadSize    = std::min(maxCount, count) * elemSize + argSize;
    strata::buffer<uint8_t> payload(payloadSize);

    while (count > 0)
    {
        const uint16_t wCount = std::min(count, maxCount);
        uint8_t *it           = payload.data();
        it                    = hostToSerial(it, coefficients, coefficients + wCount);
        it                    = hostToSerial(it, offset);
        it                    = hostToSerial(it, slotNr);

        const uint16_t wLength = wCount * elemSize + argSize;
        m_commands.vendorWrite(FN_PROCESSING_RADAR_WRITE_CUSTOM_WINDOW_COEFFICIENTS, wLength, payload.data());

        offset += wCount;
        coefficients += wCount;
        count -= wCount;
    }
}

void RemoteProcessingRadar::start()
{
    m_commands.vendorWrite(FN_PROCESSING_RADAR_START);
}

bool RemoteProcessingRadar::isBusy()
{
    uint8_t result;
    m_commands.vendorRead(FN_PROCESSING_RADAR_IS_BUSY, result);
    return (result != 0);
}
