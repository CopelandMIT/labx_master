/* ===========================================================================
** Copyright (C) 2023 Infineon Technologies AG
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**
** 1. Redistributions of source code must retain the above copyright notice,
**    this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. Neither the name of the copyright holder nor the names of its
**    contributors may be used to endorse or promote products derived from
**    this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
** LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
** CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
** INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
** CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
** ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
** POSSIBILITY OF SUCH DAMAGE.
** ===========================================================================
*/

#include "DeviceLtr11Impl.hpp"

#include <common/exception/EException.hpp>
#include <common/Finally.hpp>
#include <components/interfaces/IRadarLtr11.hpp>
#include <platform/interfaces/IBridgeControl.hpp>
#include <platform/interfaces/IBridgeData.hpp>

#include "ifxBase/Complex.h"
#include "ifxBase/Exception.hpp"
#include "ifxBase/Vector.h"
#include "ifxRadarDeviceCommon/internal/RadarDeviceCommon.hpp"


namespace {
constexpr uint16_t DEFAULT_QUEUE_SIZE = 4096;

constexpr uint16_t BGT60LTR11_ADC_RSL_IFI_REG40_REG_ADDR = 0x28;
constexpr uint16_t BGT60LTR11_STS_CHIP_VER_REG56_REG_ADDR = 0x38;
constexpr uint16_t DATA_LTR11_DETECTOR_OUTPUT_VIRTUAL_ADDR = 0xFF;

constexpr uint8_t IFX_LTR11_DETECTOR_OUTPUT_MOTION_MASK = 1 << 0;
constexpr uint8_t IFX_LTR11_DETECTOR_OUTPUT_DIRECTION_MASK = 1 << 1;

constexpr std::array<uint16_t, 3> minVersion = {2, 5, 4};

using ReadoutDataConfiguration = std::vector<ReadoutEntry_t>;

const IDataProperties_t properties = {};

const ReadoutDataConfiguration frameReadoutConfiguration {
    {BGT60LTR11_ADC_RSL_IFI_REG40_REG_ADDR, 2},    // ADC Ifi Samples
    {BGT60LTR11_STS_CHIP_VER_REG56_REG_ADDR, 1},   // Amplitude
    {DATA_LTR11_DETECTOR_OUTPUT_VIRTUAL_ADDR, 1},  // Detector Output
};

uint32_t getFrameSize()
{
    uint32_t frameSize = 0;

    for (const auto& readoutConfig : frameReadoutConfiguration)
    {
        frameSize += (readoutConfig.count);
    }

    return (frameSize * sizeof(uint16_t));
}

float normalize(uint16_t fvalue)
{
    /* For LTR11, internal ADC, which is physically an 8bit ADC,
     * is used. The result is 10 bit wide, but only bit9-bit2 are
     * significant, hence bit1-bit0 and bit15-bit10 are discarded.
     */
    auto value = (fvalue & 0x3FC) >> 2;
    return (1.0f * value) / 0xFF;
}

}  // end of anonymous namespace


DeviceLtr11::DeviceLtr11(std::unique_ptr<BoardInstance>&& board) :
    m_board {std::move(board)},
    m_radarLtr11 {nullptr},
    m_bridgeData {nullptr},
    m_data {nullptr},
    m_protocol {nullptr},
    m_registers {nullptr},
    m_dataIndex {0},
    m_frameSize {::getFrameSize()},
    m_acquisitionStarted {false},
    m_frameCounter {0},
    m_averagePower {0}
{
    if (!m_board)
    {
        throw rdk::exception::no_device();
    }

    if (!m_board->getIBridge() || !m_board->getIBridge()->isConnected())
    {
        throw rdk::exception::communication_error();
    }

    m_radarLtr11 = m_board->getComponent<IRadarLtr11>(m_dataIndex);
    if (m_radarLtr11 == nullptr)
    {
        throw rdk::exception::no_device();
    }

    m_bridgeData = m_board->getIBridge()->getIBridgeData();
    m_data = m_board->getIBridge()->getIBridgeControl()->getIData();

    // get the ltr11 resources
    m_protocol = m_radarLtr11->getIProtocolLtr11();
    m_registers = m_radarLtr11->getIRegisters();

    m_registerConfigurator = std::make_unique<DeviceLtr11RegisterConfigurator>(m_registers);

    m_bandJapan = m_registerConfigurator->isJapanBand();

    /* check if firmware is valid */
    {
        rdk::RadarDeviceCommon::get_firmware_info(m_board.get(), &m_firmware_info);
        if (!rdk::RadarDeviceCommon::is_firmware_version_valid(minVersion, m_firmware_info))
            throw rdk::exception::firmware_version_not_supported();
    }
}

DeviceLtr11::~DeviceLtr11()
{
    try
    {
        stopAcquisition();
    }
    catch (...)
    {
        // It might happen that stopAcquisition throws an exception in case
        // the device is no longer present.
        //
        // As a destructor must not throw exceptions, we ignore the exception here.
        //
        // Anyhow, if the device is longer present, it is also not necessary to
        // stop the acquisition.
    }
}

void DeviceLtr11::setConfig(const ifx_Ltr11_Config_t* config)
{
    if (m_acquisitionStarted)
    {
        throw rdk::exception::not_supported();
    }

    DeviceLtr11Base::setConfig(config);

    softReset();

    setMisoArbitration(0);

    m_registerConfigurator->addSetDefaultRegistersList();
    m_registerConfigurator->addSetPulseConfig(m_config.prt, m_config.pulse_width, m_config.tx_power_level);
    m_registerConfigurator->addSetHoldTime(m_config.hold_time);
    m_registerConfigurator->addSetDetectorThreshold_AdaptivPRT(m_config.internal_detector_threshold, m_config.aprt_factor);
    m_registerConfigurator->addSetRX_IF_Gain(m_config.rx_if_gain);
    m_registerConfigurator->addSetRFCenterFrequency(m_config.rf_frequency_Hz);
    m_registerConfigurator->addSetMode(m_config.mode);
    if (config->disable_internal_detector)
    {
        m_registerConfigurator->disableInternalDetector();
    }

    m_registerConfigurator->flushEnqRegisters();

    setMisoArbitration(DeviceLtr11Base::prtIndexToUs(m_config.prt));

    setupFrameData();
    setupBridgeData();
}

void DeviceLtr11::startAcquisition()
{
    if (m_acquisitionStarted)
    {
        return;
    }
    if (!m_frameConfigValid)
    {
        throw rdk::exception::not_configured();
    }
    startDataStreaming();

    // Re-initialize member variables needed to compute the metadata
    m_timestampPrev.reset();
    m_frameCounter = 0;
    m_averagePower = 0;

    m_acquisitionStarted = true;
}

void DeviceLtr11::stopAcquisition()
{
    if (!m_acquisitionStarted)
    {
        return;
    }

    stopDataStreaming();
    m_acquisitionStarted = false;
}

ifx_Vector_C_t* DeviceLtr11::getNextFrame(ifx_Vector_C_t* frameData, ifx_Ltr11_Metadata_t* metadata, uint16_t timeoutMs)
{

    if (!m_frameConfigValid)
    {
        throw ::rdk::exception::error();
    }
    if (!metadata)
    {
        throw ::rdk::exception::argument_null();
    }
    if (!timeoutMs)
    {
        throw ::rdk::exception::argument_invalid();
    }

    bool frameDataMemoryAllocated;  // Flag indicating when true: frame memory allocated in getNextFrame method
    if (!frameData)
    {
        // allocate memory for frame
        frameData = ifx_vec_create_c(getNumberOfSamples());
        if (!frameData)
            throw rdk::exception::memory_allocation_failed();
        frameDataMemoryAllocated = true;
    }
    else
    {
        if (IFX_MDA_DIMENSIONS(frameData) != 1 || IFX_VEC_LEN(frameData) != getNumberOfSamples())
            throw rdk::exception::dimension_mismatch();
        frameDataMemoryAllocated = false;
    }

    if (!m_acquisitionStarted)
    {
        startAcquisition();
    }

    try
    {
        readNextFrame(frameData, metadata, timeoutMs);
    }
    catch (const rdk::exception::exception& e)
    {
        stopAcquisition();
        if (frameDataMemoryAllocated)
            ifx_vec_destroy_c(frameData);
        throw e;
    }

    return frameData;
}

void DeviceLtr11::softReset()
{
    m_radarLtr11->getIPinsLtr11()->reset();
}

uint32_t DeviceLtr11::determineBufferSize() const
{
    if ((getNumberOfSamples() == 0) || (getNumberOfSamples() > IFX_LTR11_MAX_ALLOWED_NUM_SAMPLES))
    {
        throw rdk::exception::not_supported();
    }

    return (m_frameSize * getNumberOfSamples());
}

void DeviceLtr11::setupFrameData()
{
    DataSettingsBgtRadar_t settings;
    settings.initialize(frameReadoutConfiguration, getNumberOfSamples());
    m_data->configure(m_dataIndex, &properties, &settings);
}

void DeviceLtr11::setupBridgeData()
{
    m_bridgeData->setFrameBufferSize(m_frameSize * getNumberOfSamples());
    m_bridgeData->setFrameQueueSize(DEFAULT_QUEUE_SIZE);
}

void DeviceLtr11::setMisoArbitration(uint16_t prt /* = 0*/)
{
    m_protocol->setMisoArbitration(prt);
}

void DeviceLtr11::startDataStreaming()
{
    m_bridgeData->startStreaming();
    m_data->start(m_dataIndex);
}

void DeviceLtr11::stopDataStreaming()
{
    m_data->stop(m_dataIndex);
    m_bridgeData->stopStreaming();
}

uint16_t DeviceLtr11::getNumberOfSamples() const
{
    return m_config.num_samples;
}

void DeviceLtr11::readNextFrame(ifx_Vector_C_t* frameData, ifx_Ltr11_Metadata_t* metadata, uint16_t timeoutMs)
{
    const auto frameBufferSize = determineBufferSize();

    auto* deviceFrame = m_board->getFrame(timeoutMs);

    if (deviceFrame == nullptr)
    {
        throw rdk::exception::timeout();
    }

    auto cleanup = stdext::finally(
        [&deviceFrame]() {
            deviceFrame->release();
        });

    const auto statusCode = deviceFrame->getStatusCode();
    switch (statusCode)
    {
        case DataError_NoError:
            break;
        case DataError_FramePoolDepleted:
        case DataError_FrameQueueTrimmed:
        case DataError_FrameDropped:
            throw rdk::exception::frame_acquisition_failed();
            break;
        default:
            throw rdk::exception::error();
            return;
    }

    if (deviceFrame->getDataSize() != frameBufferSize)
    {
        throw rdk::exception::dimension_mismatch();
    }

    auto frameTimestamp = deviceFrame->getTimestamp();
    if (m_timestampPrev)
    {
        metadata->active = (frameTimestamp - *m_timestampPrev) < m_timestampThreshold;
    }
    else
    {
        /* There is no previous timestamp for the first frame, and the chip should be by
         * default in active mode. Hence, for the first frame, active should be initialized
         * to true (similarly the average power should be the active mode power). */
        metadata->active = true;
        m_averagePower = m_activePower;
    }
    m_timestampPrev = frameTimestamp;

    float currentPower;
    if (metadata->active)
    {
        currentPower = m_activePower;
    }
    else
    {
        currentPower = m_lowPower;
    }
    m_averagePower += (currentPower - m_averagePower) / ++m_frameCounter;

    const auto frameStepping = (m_frameSize / sizeof(uint16_t));
    const auto detectorOutputIndex = 3;  // detector output index in data readout

    const auto* dataAsUint = reinterpret_cast<const uint16_t*>(deviceFrame->getData());

    const auto numberOfSamples = getNumberOfSamples();
    for (uint32_t i = 0; i < numberOfSamples; ++i)
    {

        const auto ifiIdx = (i * frameStepping);
        // ifqIdx  = ifiIdx + 1

        const ifx_Float_t I = ::normalize(dataAsUint[ifiIdx]);
        const ifx_Float_t Q = ::normalize(dataAsUint[ifiIdx + 1]);

        IFX_VEC_AT(frameData, i) = IFX_COMPLEX_DEF(I, Q);
    }

    metadata->motion = (dataAsUint[(numberOfSamples - 1) * frameStepping + detectorOutputIndex] & IFX_LTR11_DETECTOR_OUTPUT_MOTION_MASK) == IFX_LTR11_DETECTOR_OUTPUT_MOTION_MASK;
    metadata->direction = (dataAsUint[(numberOfSamples - 1) * frameStepping + detectorOutputIndex] & IFX_LTR11_DETECTOR_OUTPUT_DIRECTION_MASK) == IFX_LTR11_DETECTOR_OUTPUT_DIRECTION_MASK;
    metadata->avg_power = m_averagePower;
}

void DeviceLtr11::dumpRegisters(const char* filename)
{
    m_registerConfigurator->dumpRegisters(filename);
}
