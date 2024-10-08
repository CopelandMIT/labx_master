/** Copyright (C) 2023 Infineon Technologies AG
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

#include "DeviceMimoseImpl.hpp"

#include "DeviceMimoseConstants.hpp"
#include "DeviceMimoseRegisterConfigurator.hpp"
#include "registers_BGT24ATR22.hpp"

#include "ifxBase/Cube.h"
#include "ifxBase/Exception.hpp"
#include "ifxRadarDeviceCommon/internal/RadarDeviceCommon.hpp"

// strata
#include <common/exception/EException.hpp>
#include <components/interfaces/IRadarAtr22.hpp>
#include <platform/interfaces/IBridgeControl.hpp>
#include <platform/interfaces/IBridgeData.hpp>

#include <array>
#include <cassert>
#include <cstring>
#include <iterator>
#include <numeric>
#include <vector>


namespace {
const std::array<uint16_t, 3> minVersion {2, 5, 3};

constexpr uint16_t RAW_DATA_MEMORY_ADDRESS = 0x3800;
constexpr uint8_t TWO_READOUT_CONFIGS = 2;

const IDataProperties_t properties = {};

void checkFrameDimensions(const ifx_Cube_C_t* frame, const ifx_Mimose_Frame_Config_t& currentConfig)
{
    const auto checkFrameDimensions = (IFX_MDA_DIMENSIONS(frame) == 3)
                                      && (IFX_CUBE_COLS(frame) == DeviceMimoseBase::getNumActivePulseConfigurations(currentConfig.selected_pulse_configs))
                                      && (IFX_CUBE_SLICES(frame) == currentConfig.num_samples);
    if (!checkFrameDimensions)
    {
        throw rdk::exception::frame_size_not_supported();
    }
}

using MemoryRegion = std::pair<const uint16_t*, const uint16_t*>;
using MemoryRegions = std::vector<MemoryRegion>;

inline float toFloat(uint16_t fvalue)
{
    auto value = (fvalue & 0x0FFF);
    return (1.0f * value) / 0xFFF;
}

union PackedAFC
{
    uint32_t afcWords[2];
    uint64_t afcPacked;
};

uint64_t packAfc(uint16_t afcVal, uint16_t afcCounterLow, uint16_t afcCounterHigh, bool afcForward)
{
    const uint32_t afcCounter = (afcCounterHigh << 16) + afcCounterLow;

    if (afcCounter > 0x00FFFFFF)
    {
        return 0;
    }

    PackedAFC packedAfc;

    packedAfc.afcWords[0] = afcCounter;
    packedAfc.afcWords[1] = (afcForward) ? (afcVal + 1) : afcVal;

    return packedAfc.afcPacked;
}

void fillMetaData(ifx_Mimose_Metadata_t* metadata,
                  const uint16_t* aocMemory,
                  const uint16_t* agcMemory,
                  const uint16_t activePulses)
{
    if (metadata == nullptr)
    {
        return;
    }

    *metadata = {};

    // fill the aoc offsets
    {
        for (uint16_t aocIndex = 0; aocIndex < activePulses; ++aocIndex)
        {
            const auto aocPulseOffsetI = aocMemory[aocIndex * 2];
            const auto aocPulseOffsetQ = aocMemory[(aocIndex * 2) + 1];

            int16_t offsetIValue = (aocPulseOffsetI & 0xFF);
            if (aocPulseOffsetI & 0x100)
            {
                offsetIValue *= -1;
            }

            int16_t offsetQValue = (aocPulseOffsetQ & 0xFF);
            if (aocPulseOffsetQ & 0x100)
            {
                offsetQValue *= -1;
            }

            metadata->aoc_offsets[aocIndex][0] = offsetIValue;
            metadata->aoc_offsets[aocIndex][1] = offsetQValue;
        }
    }

    // fill the abb gains
    {
        constexpr uint16_t ABB_GAIN_MASK_BASE_WIDTH = 3;
        constexpr uint16_t ABB_GAIN_MASK_EN_WIDTH = 1;

        uint16_t currentAbbGainMask = 0x07;

        for (uint16_t agcIndex = 0; agcIndex < activePulses; ++agcIndex)
        {
            currentAbbGainMask <<= (ABB_GAIN_MASK_BASE_WIDTH * agcIndex);
            currentAbbGainMask <<= ABB_GAIN_MASK_EN_WIDTH;

            metadata->abb_gains[agcIndex] = static_cast<uint8_t>((agcMemory[0] & currentAbbGainMask));
        }
    }
}

enum RawDataSubType
{
    FirstHalf,
    SecondHalf
};

void fillBuffer(std::vector<uint16_t>& toFill, const uint16_t* rawData, const size_t numSamples, const size_t numPulses, RawDataSubType rawDataSubType)
{
    const auto* begin = rawData;
    const auto* end = begin;

    for (size_t i = 0; i < numPulses; ++i)
    {

        begin = end;
        end += numSamples;

        // the destination buffer offset is the offset at which position the half pulse need to be stored
        size_t destBuffOffset = (rawDataSubType == RawDataSubType::FirstHalf) ? 0 : numSamples;
        destBuffOffset += (i * (numSamples * 2));

        auto* dest = (toFill.data() + destBuffOffset);

        std::copy(begin, end, dest);
    }
}

uint32_t getNextPowerOf2(uint32_t num)
{
    uint32_t pnum = 1;
    while (pnum < num)
        pnum *= 2;

    return ((pnum > 1) ? pnum : 2);
}

// Constants
constexpr uint16_t IQ_SAMPLE_SIZE = 2;  // 2 x uint16_t
constexpr uint16_t DEFAULT_QUEUE_SIZE = 10000;

inline uint16_t getNumberOfHeatingPulses(float minRepetitionTime, float setRepetitionTime, float pulseRepetitionTime)
{

    uint16_t heatingPulsesCount = 0;
    if ((minRepetitionTime + 3 * pulseRepetitionTime <= setRepetitionTime)
        || (minRepetitionTime + 2 * pulseRepetitionTime <= setRepetitionTime))
    {
        heatingPulsesCount = 2;
    }
    else if (minRepetitionTime + 1 * pulseRepetitionTime <= setRepetitionTime)
    {
        heatingPulsesCount = 1;
    }
    return heatingPulsesCount;
}
}  // end of anonymous namespace


DeviceMimose::DeviceMimose(std::unique_ptr<BoardInstance>&& board) :
    m_board(std::move(board)),
    m_atr22(nullptr),
    m_bridgeControl(nullptr),
    m_bridgeData(nullptr),
    m_data(nullptr),
    m_regs(nullptr),
    m_regConfig(nullptr),
    m_dataIndex(0),   // trigger index
    m_dataIndex2(3),  // DRPD trigger index
    m_statusDataIndex(1),
    m_acquisitionStarted(false),
    m_frameBufferSize(0),
    m_statusBufferSize(0),
    m_numSamplesReturned(0),
    m_numSamplesForNextPulseInMem(0),
    m_currentAfc(0),
    m_switchingConf(false),
    m_samplingMode(SamplingMode::FramePausedSampling),
    m_equidistantSamplingTraits {}
{
    if (!m_board)
    {
        throw rdk::exception::no_device();
    }

    if (!m_board->getIBridge() || !m_board->getIBridge()->isConnected())
    {
        throw rdk::exception::communication_error();
    }

    rdk::RadarDeviceCommon::get_firmware_info(m_board.get(), &m_firmware_info);
    if (!rdk::RadarDeviceCommon::is_firmware_version_valid(minVersion, m_firmware_info))
    {
        throw rdk::exception::firmware_version_not_supported();
    }

    m_atr22 = m_board->getComponent<IRadarAtr22>(m_dataIndex);
    if (!m_atr22)
    {
        throw rdk::exception::no_device();
    }

    // probe the shield type
    if (getShieldType() != IFX_MIMOSE_BGT24ATR22)
    {
        throw ::rdk::exception::not_supported();
    }

    m_bridgeControl = m_board->getIBridge()->getIBridgeControl();
    m_bridgeData = m_board->getIBridge()->getIBridgeData();
    m_data = m_bridgeControl->getIData();

    // get the atr22 resources
    m_regs = m_atr22->getIRegisters();

    m_regConfig = std::make_unique<DeviceMimoseRegisterConfigurator>(m_regs);

    m_regConfig->reset();
}

/*virtual*/
DeviceMimose::~DeviceMimose()
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

void DeviceMimose::destroy_routine()
{
    m_regConfig->resetClockConf();
}

void DeviceMimose::setConfig(const ifx_Mimose_Config_t* config)
{
    if (m_acquisitionStarted)
    {
        throw rdk::exception::device_busy();
    }

    DeviceMimoseBase::setConfig(config);

    // setup the configuration
    m_regConfig->resetClockConf();  // this is needed for configuring the RC trim
    setupConfig();

    const auto numActivePulses = getNumActivePulseConfigurations(
        m_config.frame_config[m_activeFrameIndex].selected_pulse_configs);
    m_numSamplesReturned = m_config.frame_config[m_activeFrameIndex].num_samples;
    m_numSamplesForNextPulseInMem = ::getNextPowerOf2(m_numSamplesReturned);

    setupFrameDelays(m_activeFrameIndex, config->clock_config.rc_clock_enabled);

    IFX_LOG_DEBUG("DeviceMimose::setConfig Num of samples %i", m_numSamplesReturned);
    IFX_LOG_DEBUG("DeviceMimose::setConfig Num of samples rounded to power of 2: %i", m_numSamplesForNextPulseInMem);

    m_regConfig->addMemOffsets(m_numSamplesForNextPulseInMem,
                               numActivePulses,
                               (m_samplingMode == SamplingMode::EquidistantSampling));

    const uint16_t rawMemoryReadCount = (m_numSamplesForNextPulseInMem * numActivePulses * IQ_SAMPLE_SIZE);
    if ((rawMemoryReadCount == 0) || (rawMemoryReadCount > (IQ_SAMPLES_MAX_READCOUNT_ALLOWED * IQ_SAMPLE_SIZE)))
    {
        throw rdk::exception::not_supported();
    }

    const auto equidistantSampling = (m_samplingMode == SamplingMode::EquidistantSampling);
    // update the readouts according to the sampling mode
    {
        // default constant readouts - they apply independently which the sampling mode is
        const ReadoutEntry_t frameCounterConf = {BGT24ATR22_FRAME_COUNTER_REG_ADDR, 1};
        const ReadoutEntry_t vcoConf = {BGT24ATR22_VCO_DAC_VALUE_REG_ADDR, 9};
        const ReadoutEntry_t aocConf = {(BGT24ATR22_AOC_CONF_REG_ADDR + 1), 8};  // AOC offsets
        const ReadoutEntry_t agcConf = {BGT24ATR22_PC0_AGC_REG_ADDR, 4};
        // ReadoutDataConfiguration memOffsetsConf = {"memory_offset", BGT24ATR22_MEM_RAW_REG_ADDR, 5};

        ReadoutDataConfiguration defaultReadoutDataConfiguration = {frameCounterConf, vcoConf, aocConf, agcConf};

        if (equidistantSampling)
        {

            const uint16_t rawMemoryReadCountPerPulseHalf = ((IQ_SAMPLE_SIZE * m_numSamplesForNextPulseInMem) / 2u);

            ReadoutDataConfiguration fragmentFirstHalfConfiguration;
            ReadoutDataConfiguration fragmentSecondHalfConfiguration;

            uint16_t rawDataAddrFirstHalf = RAW_DATA_MEMORY_ADDRESS;
            uint16_t rawDataAddrSecondHalf = (RAW_DATA_MEMORY_ADDRESS + rawMemoryReadCountPerPulseHalf);

            // data buffer is arranged as interleaved pulse half sequence
            for (uint32_t iPulse = 0; iPulse < numActivePulses; ++iPulse)
            {

                fragmentFirstHalfConfiguration.push_back({rawDataAddrFirstHalf, rawMemoryReadCountPerPulseHalf});

                fragmentSecondHalfConfiguration.push_back({rawDataAddrSecondHalf, rawMemoryReadCountPerPulseHalf});

                rawDataAddrFirstHalf += (2 * rawMemoryReadCountPerPulseHalf);
                rawDataAddrSecondHalf += (2 * rawMemoryReadCountPerPulseHalf);
            }

            // append default readouts to the 2nd half configurations
            //
            for (const auto& defaultReadoutEntry : defaultReadoutDataConfiguration)
            {
                fragmentSecondHalfConfiguration.push_back(defaultReadoutEntry);
            }

            m_fragmentSpecificReadoutConfigurations = {fragmentFirstHalfConfiguration, fragmentSecondHalfConfiguration};
        }

        // always create the frameSpecificReadoutConfiguration
        //
        m_frameSpecificReadoutConfiguration.clear();
        m_frameSpecificReadoutConfiguration.push_back({RAW_DATA_MEMORY_ADDRESS, rawMemoryReadCount});
        for (const auto& defaultReadoutEntry : defaultReadoutDataConfiguration)
        {
            m_frameSpecificReadoutConfiguration.push_back(defaultReadoutEntry);
        }
    }

    if (equidistantSampling)
    {
        if (m_fragmentSpecificReadoutConfigurations.size() == TWO_READOUT_CONFIGS)
        {
            m_equidistantSamplingTraits.m_frameBufferFirstHalfSize = getFrameBufferSize(m_fragmentSpecificReadoutConfigurations[0]);
            m_equidistantSamplingTraits.m_frameBufferSecondHalfSize = getFrameBufferSize(m_fragmentSpecificReadoutConfigurations[1]);
            m_frameBufferSize = m_equidistantSamplingTraits.m_frameBufferFirstHalfSize + m_equidistantSamplingTraits.m_frameBufferSecondHalfSize;
        }
        else
        {
            throw rdk::exception::not_supported();
        }
    }
    else
    {
        m_frameBufferSize = getFrameBufferSize(m_frameSpecificReadoutConfiguration);
    }

    // setup trigger configuration
    //
    if (m_samplingMode == SamplingMode::FramePausedSampling)
    {
        m_regConfig->addEnableDRDPOnlyAcquisition();
        // m_regConfig->addHardcodedTrigger();
        m_regConfig->addEnableStatusTriggerSignal(BGT24ATR22::BGT24ATR22_TRIGX_CONF_MAP_OUT_OUTPUT_TRIGGER_2);
        m_regConfig->addInterrupt(false, true);
    }
    else
    {
        m_regConfig->addEnableTrigger2();
        m_regConfig->addInterrupt(true, true);
        m_regConfig->addEnableStatusTriggerSignal(BGT24ATR22::BGT24ATR22_TRIGX_CONF_MAP_OUT_OUTPUT_TRIGGER_2);
        enabledAcquisitionTriggerSignal(RawDataTriggerType::PRE_TRIGGER_AND_DRDP);
    }

    setupFrameData();
    setupStatusData();
    setupBridgeData();

    m_regConfig->flushEnqRegisters();
}

void DeviceMimose::switchFrameConfiguration(uint16_t activeFrameConfigIndex)
{
    if (!checkConfiguration(&m_config, activeFrameConfigIndex))
    {
        throw rdk::exception::not_supported();
    }

    if (activeFrameConfigIndex == m_activeFrameIndex)
    {
        return;  // Current config already active
    }

    const auto peviousActiveFrameIndex = m_activeFrameIndex;
    m_activeFrameIndex = activeFrameConfigIndex;
    m_switchingConf = true;

    try
    {
        setConfig(&m_config);
    }
    catch (const rdk::exception::exception& e)
    {
        m_switchingConf = false;
        m_activeFrameIndex = peviousActiveFrameIndex;
        // re-throw the exception
        throw e;
    }

    m_switchingConf = false;
}

void DeviceMimose::startAcquisition()
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
    startSequencer(m_activeFrameIndex);

    m_regConfig->flushEnqRegisters();
    // reset the stored afc
    m_currentAfc = 0;

    if (m_samplingMode == SamplingMode::EquidistantSampling)
    {
        const auto rawDataBufferSize = (m_frameBufferSize / IQ_SAMPLE_SIZE);
        m_equidistantSamplingTraits.m_rawDataBuffer.resize(rawDataBufferSize);  // frame buffer size is expressed in bytes while the raw buffer in uint16's
    }
    m_acquisitionStarted = true;
}

void DeviceMimose::stopAcquisition()
{
    if (!m_acquisitionStarted)
    {
        return;
    }

    stopSequencer();
    stopDataStreaming();
    m_regConfig->flushEnqRegisters();

    m_acquisitionStarted = false;
}

size_t DeviceMimose::getRegisterCount()
{
    return BGT24ATR22::BGT24ATR22_registers.size();
}

void DeviceMimose::getRegisters(uint32_t* registers)
{
    m_regConfig->getRegisters(registers);
}

void DeviceMimose::setRegisters(uint32_t* registers, size_t count)
{
    for (size_t i = 0; i < count; i++)
    {
        auto reg_address = static_cast<uint16_t>(registers[i] >> 16);
        auto reg_value = static_cast<uint16_t>(registers[i] & 0xFFFF);
        m_regConfig->addSetRegister({reg_address, reg_value});
    }
    m_regConfig->flushEnqRegisters();
}

uint16_t DeviceMimose::getRegisterValue(uint16_t register_address)
{
    return m_regConfig->readRegisterValue(register_address);
}

void DeviceMimose::update_rc_lut()
{
    m_regConfig->updateTrimLutRC(IFX_MIMOSE_REF_CLK_HZ_DEFAULT);
}

ifx_Cube_C_t* DeviceMimose::getNextFrame(ifx_Cube_C_t* frame, ifx_Mimose_Metadata_t* metadata, uint16_t timeoutMillis)
{
    if (!m_frameConfigValid)
    {
        throw rdk::exception::not_configured();
    }

    bool frameDataMemoryAllocated;
    if (!frame)
    {
        frame = createFrame();
        frameDataMemoryAllocated = true;
    }
    else
    {
        checkFrameDimensions(frame, m_config.frame_config[m_activeFrameIndex]);
        frameDataMemoryAllocated = false;
    }

    if (!m_acquisitionStarted)
    {
        startAcquisition();
    }

    try
    {
        timeoutMillis = std::max(timeoutMillis, static_cast<uint16_t>(m_config.frame_config[0].frame_repetition_time_s * 1250));
        timeoutMillis = std::max(timeoutMillis, static_cast<uint16_t>(m_config.frame_config[1].frame_repetition_time_s * 1250));
        readRawFrame(frame, metadata, timeoutMillis);
    }
    catch (const rdk::exception::exception& e)
    {
        stopAcquisition();
        if (frameDataMemoryAllocated)
        {
            ifx_cube_destroy_c(frame);
        }
        throw e;
    }

    return frame;
}

void DeviceMimose::getSensorValues(ifx_Mimose_Sensor_t* sensorValues)
{
    const auto currentAfcValue = m_currentAfc.load();

    if (!m_acquisitionStarted || currentAfcValue == 0)
    {
        throw ::rdk::exception::not_possible();
    }

    *sensorValues = {0.0f, 0.0f};

    PackedAFC packedAfc;
    packedAfc.afcPacked = currentAfcValue;

    const auto afcCounter = packedAfc.afcWords[0];
    const auto afcVal = packedAfc.afcWords[1];
    const auto extOscMHz = 1e-6f * EXT_OSC_FREQ_HZ;
    const float clockCycles = afcVal / extOscMHz;
    const float calculatedFreqGHz = (afcCounter * 8) / clockCycles / 1000;

    sensorValues->rf_frequency_Hz = (calculatedFreqGHz * 1e9f);
}

void DeviceMimose::setAnalogBaseBandMode(size_t pulse_index, ifx_Mimose_ABB_type_t abb)
{
    if (pulse_index >= NO_OF_PULSES)
    {
        throw rdk::exception::argument_invalid();
    }

    bool agc = abb == IFX_MIMOSE_ABB_GAIN_AUTOMATIC;
    int gainIndex;
    if (agc)
    {
        gainIndex = 0;
    }
    else
    {
        gainIndex = static_cast<int>(abb);
    }

    m_regConfig->addRegisterABB_gain(pulse_index, agc, gainIndex);
}

void DeviceMimose::setAutomaticOffsetCompensationMode(size_t pulse_index, ifx_Mimose_AOC_Mode_t aoc_mode)
{
    if (pulse_index >= NO_OF_PULSES)
    {
        throw rdk::exception::argument_invalid();
    }

    if (aoc_mode > NO_OF_MODES)
    {
        throw rdk::exception::argument_invalid();
    }

    m_regConfig->addRegisterAOC(pulse_index, static_cast<ifx_Mimose_AOC_Mode_t>(aoc_mode));
}

void DeviceMimose::dumpRegisters(const char* filename) const
{
    // not implemented
}

////////////////////////////////////////
////  protected and private section ////
////////////////////////////////////////

ifx_Radar_Sensor_t DeviceMimose::getShieldType() const
{
    return IFX_MIMOSE_BGT24ATR22;
}

void DeviceMimose::setupConfigPulse(size_t frame_config_index, size_t pulse_index, uint32_t system_clock_Hz)
{
    if (pulse_index >= NO_OF_PULSES)
    {
        throw rdk::exception::argument_invalid();
    }

    auto pulse_repetition_pc_time_sec = m_config.frame_config[frame_config_index].pulse_repetition_time_s;
    auto cPulse = getNumActivePulseConfigurations(m_config.frame_config[frame_config_index].selected_pulse_configs);
    IFX_LOG_DEBUG("DeviceMimose::setupConfigPulse no %i of pulses active on frame index %i", int(pulse_index), frame_config_index);

    pulse_repetition_pc_time_sec /= cPulse;
    auto repetition_pc_time_cycles = static_cast<uint64_t>(std::ceil(pulse_repetition_pc_time_sec * system_clock_Hz));

    m_regConfig->addPulseRepetitionTime(pulse_index, frame_config_index, repetition_pc_time_cycles);
    auto pulse_config = m_config.pulse_config[pulse_index];
    auto channel = pulse_config.channel;
    auto tx_power = pulse_config.tx_power_level;
    auto abb_type = pulse_config.abb_gain_type;
    auto aoc_mode = pulse_config.aoc_mode;
    m_regConfig->addRFChannel(pulse_index, channel, tx_power);
    setAnalogBaseBandMode(pulse_index, abb_type);
    setAutomaticOffsetCompensationMode(pulse_index, aoc_mode);
}

void DeviceMimose::setupConfigFrame(size_t frame_config_index)
{
    if (!m_switchingConf)  // skip the initial configuration if switching
    {
        auto fConfig = m_config.frame_config[frame_config_index];
        auto num_samples_per_frame = fConfig.num_samples;
        auto frame_repetition_time_s = fConfig.frame_repetition_time_s;

        IFX_LOG_DEBUG("DeviceMimose::setupConfigFrame setting config for frame %u, frame period %fs number of samples %u",
                      frame_config_index, frame_repetition_time_s, num_samples_per_frame);
        // frame timing check
        auto minRepTime = DeviceMimoseBase::calculateMinimumFrameRepetitionTime(fConfig.selected_pulse_configs,
                                                                                fConfig.num_samples,
                                                                                m_config.afc_config.afc_duration_ct,
                                                                                fConfig.pulse_repetition_time_s);
        auto heatingPulses = getNumberOfHeatingPulses(minRepTime, fConfig.frame_repetition_time_s, fConfig.pulse_repetition_time_s);
        auto heatingPulse_time = static_cast<float>(heatingPulses) * fConfig.pulse_repetition_time_s;
        if ((heatingPulse_time + minRepTime) >= fConfig.frame_repetition_time_s)
        {
            throw rdk::exception::frame_rate_out_of_range();
        }

        // set frame period
        auto frame_period_clock_cycles = static_cast<uint64_t>(std::round(frame_repetition_time_s
                                                                          * static_cast<float>(m_config.clock_config.system_clock_Hz)));
        m_regConfig->addFramePeriod(frame_config_index, frame_period_clock_cycles);

        // set number of samples
        m_regConfig->addNumberOfSamples(frame_config_index, num_samples_per_frame);

        // set trigger
        const auto active_pc_count = getNumActivePulseConfigurations(fConfig.selected_pulse_configs);

        // need to evaluate this with chip designers
        auto pre_trigger_advance = static_cast<float>(active_pc_count) * 100e-6 + 50e-6
                                   + static_cast<float>(num_samples_per_frame + active_pc_count) * 80e-6;
        auto pre_trigger_count = static_cast<uint32_t>(std::ceil(static_cast<float>(active_pc_count)
                                                                 * pre_trigger_advance / fConfig.pulse_repetition_time_s));
        m_regConfig->addSetPreTrigger(frame_config_index, num_samples_per_frame, active_pc_count, pre_trigger_count);
    }
}


void DeviceMimose::setupConfigAFC(const ifx_Mimose_AFC_Control_t& AFC_config)
{
    IFX_LOG_DEBUG("DeviceMimose::setupConfigAFC setting duration %u, th course %u th fine %u",
                  (unsigned)AFC_config.afc_duration_ct,
                  (unsigned)AFC_config.afc_threshold_course, (unsigned)AFC_config.afc_threshold_fine);
    m_regConfig->addAFC_Duration(AFC_config.afc_duration_ct);
    m_regConfig->addAFC_Threshold(AFC_config.afc_threshold_fine, AFC_config.afc_threshold_course);
    m_regConfig->addAFC_RepetitionPeriod(AFC_config.afc_repeat_count);
    m_regConfig->addAFC_Frequency(AFC_config.rf_center_frequency_Hz, AFC_config.afc_duration_ct);
    // #TODO we need to add afc delay T_AFC register function in the register manipulator
    m_regConfig->addSetRegister({BGT24ATR22_T_AFC_REG_ADDR, BGT24ATR22_T_AFC_DELAY_CONST});
}

uint32_t DeviceMimose::setupConfigClock(const ifx_Mimose_Clock_Config_t& Clock_config)
{
    IFX_LOG_DEBUG("DeviceMimose::Clock Config I2C_CONF.CLK_GATE and CLK_CONF");
    // workaround till a register delay based solution is found for RC clocks less than 9MHz
    uint32_t internal_clock_for_setup = Clock_config.system_clock_Hz;
    if (Clock_config.rc_clock_enabled && (Clock_config.system_clock_Hz < 9000000))
    {
        internal_clock_for_setup = 9000000;
    }

    return m_regConfig->addClockConfigRegisters(Clock_config.reference_clock_Hz,
                                                internal_clock_for_setup,
                                                Clock_config.rc_clock_enabled,
                                                Clock_config.hf_on_time_usec,
                                                Clock_config.system_clock_divider,
                                                Clock_config.system_clock_div_flex,
                                                Clock_config.sys_clk_to_i2c);
}

void DeviceMimose::setupConfig()
{
    IFX_LOG_DEBUG("DeviceMimose::setupConfig set configuration");

    if (!m_switchingConf)
    {
        m_config.clock_config.system_clock_Hz = setupConfigClock(m_config.clock_config);
        setupConfigAFC(m_config.afc_config);
    }

    determineSamplingMode();

    for (size_t i_config = 0; i_config < NO_OF_FRAME_CONFIGURATIONS; i_config++)
    {
        setupConfigFrame(i_config);

        if (!m_switchingConf)
        {
            for (size_t i_PC = 0; i_PC < NO_OF_PULSES; i_PC++)
            {
                if (!m_config.frame_config[i_config].selected_pulse_configs[i_PC])
                {
                    continue;
                }
                setupConfigPulse(i_config, i_PC, m_config.clock_config.system_clock_Hz);
                m_regConfig->addFFT_Len(i_PC, m_config.frame_config[i_config].num_samples);
            }
        }
    }
    m_regConfig->applyFileConfig();
    m_regConfig->addHardcodedConfig();
    IFX_LOG_DEBUG("DeviceMimose::setConfig Configuration ended successfully");
}

void DeviceMimose::setupStatusData()
{
    const auto numActivePulses = getNumActivePulseConfigurations(
        m_config.frame_config[m_activeFrameIndex].selected_pulse_configs);

    const auto totalNumSamples = (m_numSamplesForNextPulseInMem * numActivePulses);
    const auto rawDataAddress = static_cast<uint16_t>(RAW_DATA_MEMORY_ADDRESS + (totalNumSamples * IQ_SAMPLE_SIZE));
    const auto rawDataCount = static_cast<uint16_t>((IQ_SAMPLES_MAX_READCOUNT_ALLOWED - totalNumSamples) * IQ_SAMPLE_SIZE);

    const ReadoutDataConfiguration statusReadoutConfiguration {
        {BGT24ATR22_IR_STATUS_REG_ADDR, 2},
        {rawDataAddress, rawDataCount}};

    m_statusBufferSize = getFrameBufferSize(statusReadoutConfiguration);

    DataSettingsBgtRadar_t settings;
    settings.initialize(statusReadoutConfiguration);
    m_data->configure(m_statusDataIndex, &properties, &settings);
}

void DeviceMimose::setupFrameData()
{
    if (m_samplingMode == SamplingMode::EquidistantSampling)
    {
        if (m_fragmentSpecificReadoutConfigurations.size() != 2)
        {
            throw rdk::exception::not_supported();
        }

        // 1st half / fragment
        {
            const auto& fragmentOneSpecificReadoutConfigurations = m_fragmentSpecificReadoutConfigurations[0];
            DataSettingsBgtRadar_t settingsOne;
            settingsOne.initialize(fragmentOneSpecificReadoutConfigurations);
            m_data->configure(m_dataIndex, &properties, &settingsOne);
        }

        // 2nd half / fragment
        {
            const auto& fragmentTwoSpecificReadoutConfigurations = m_fragmentSpecificReadoutConfigurations[1];
            DataSettingsBgtRadar_t settingsTwo;
            settingsTwo.initialize(fragmentTwoSpecificReadoutConfigurations);
            m_data->configure(m_dataIndex2, &properties, &settingsTwo);
        }
    }
    else
    {
        DataSettingsBgtRadar_t settings;
        settings.initialize(m_frameSpecificReadoutConfiguration);
        m_data->configure(m_dataIndex, &properties, &settings);
    }
}

void DeviceMimose::setupBridgeData()
{
    m_bridgeData->setFrameBufferSize(m_frameBufferSize);
    m_bridgeData->setFrameQueueSize(DEFAULT_QUEUE_SIZE);
}

void DeviceMimose::setupFrameDelays(uint16_t frameConfigIndex, bool rc_clock_enabled)
{
    // sets heating pulses and T_BOOT_REF_CLK
    auto fConfig = m_config.frame_config[frameConfigIndex];
    auto minRepTime = DeviceMimoseBase::calculateMinimumFrameRepetitionTime(fConfig.selected_pulse_configs,
                                                                            fConfig.num_samples,
                                                                            m_config.afc_config.afc_duration_ct,
                                                                            fConfig.pulse_repetition_time_s);
    auto heatingPulses = getNumberOfHeatingPulses(minRepTime, fConfig.frame_repetition_time_s, fConfig.pulse_repetition_time_s);
    m_regConfig->addNoHeatingPulses(frameConfigIndex, heatingPulses);

    // set up T_BOOT_REF_CLK
    if (rc_clock_enabled)
    {
        auto t_afc_time_sec = static_cast<float>(IFX_MIMOSE_RC_T_AFC_CYCLES / static_cast<float>(m_config.clock_config.system_clock_Hz));
        const auto numActivePulses = getNumActivePulseConfigurations(
            m_config.frame_config[m_activeFrameIndex].selected_pulse_configs);
        auto t_boot_ref_clk_sec = static_cast<float>((fConfig.num_samples + heatingPulses) * fConfig.pulse_repetition_time_s
                                                     - (fConfig.pulse_repetition_time_s / static_cast<float>(numActivePulses)) + t_afc_time_sec + IFX_MIMOSE_RC_T_BOOT_VCO_FS_TIME_S
                                                     - IFX_MIMOSE_RC_T_BOOT_REF_CLK_CORRECTION);

        auto t_boot_ref_clk_cycles = static_cast<uint64_t>(t_boot_ref_clk_sec * static_cast<float>(m_config.clock_config.system_clock_Hz));
        m_regConfig->addRCTbootRefClk(t_boot_ref_clk_cycles);
    }
}

void DeviceMimose::startDataStreaming()
{
    m_bridgeData->startStreaming();

    m_data->start(m_dataIndex);
    if (m_samplingMode == SamplingMode::EquidistantSampling)
    {
        m_data->start(m_dataIndex2);
    }
    m_data->start(m_statusDataIndex);
}

void DeviceMimose::stopDataStreaming()
{
    m_data->stop(m_statusDataIndex);
    if (m_samplingMode == SamplingMode::EquidistantSampling)
    {
        m_data->stop(m_dataIndex2);
    }
    m_data->stop(m_dataIndex);

    m_bridgeData->stopStreaming();
}

void DeviceMimose::enabledAcquisitionTriggerSignal(RawDataTriggerType dataTriggerType)
{
    if (dataTriggerType == RawDataTriggerType::DRDP_ONLY)
    {
        m_regConfig->addEnableDRDPOnlyAcquisition();
    }
    else
    {
        const auto triggerCount = m_equidistantSamplingTraits.m_triggerCount;
        m_regConfig->addEnableTriggerAndDRDPAcquisition(uint16_t(triggerCount));
    }
}

void DeviceMimose::startSequencer(uint16_t frameConfigurationIndex)
{
    m_regConfig->addSeqExecute(true, frameConfigurationIndex);
}

void DeviceMimose::stopSequencer()
{
    m_regConfig->addSeqExecute(false, 0);
}

uint32_t DeviceMimose::getFrameBufferSize(const ReadoutDataConfiguration& readoutConfiguration)
{
    uint32_t bufferSize = 0;

    for (const auto& readoutConfig : readoutConfiguration)
    {
        bufferSize += readoutConfig.count;
    }

    return (bufferSize * sizeof(uint16_t));
}

// NOLINTNEXTLINE(misc-no-recursion)
void DeviceMimose::readRawFrame(ifx_Cube_C_t* frame, ifx_Mimose_Metadata_t* metadata, uint16_t timeoutMillis)
{
    IFrame* deviceFrame = m_board->getFrame(timeoutMillis);
    if (!deviceFrame)
    {
        throw rdk::exception::timeout();
    }

    const auto statusCode = deviceFrame->getStatusCode();
    if (statusCode != 0)
    {
        deviceFrame->release();
        switch (statusCode)
        {
            case DataError_FramePoolDepleted:
                throw rdk::exception::insufficient_memory_allocated();
                break;
            case DataError_FrameDropped:
                throw rdk::exception::communication_error();
                break;
            case DataError_LowLevelError:
                throw rdk::exception::frame_acquisition_failed();
                break;
            default:
                throw rdk::exception::error();
                break;
        }
    }

    const auto equidistantSampling = (m_samplingMode == SamplingMode::EquidistantSampling);
    const auto frameChannel = deviceFrame->getVirtualChannel();
    {
        uint32_t expectedFrameSize;
        if (equidistantSampling)
        {
            if (frameChannel == m_dataIndex)
            {
                expectedFrameSize = m_equidistantSamplingTraits.m_frameBufferFirstHalfSize;
            }
            else if (frameChannel == m_dataIndex2)
            {
                expectedFrameSize = m_equidistantSamplingTraits.m_frameBufferSecondHalfSize;
            }
            else
            {
                expectedFrameSize = m_statusBufferSize;
            }
        }
        else
        {
            expectedFrameSize = (frameChannel == m_dataIndex) ? m_frameBufferSize : m_statusBufferSize;
        }
        if (deviceFrame->getDataSize() != expectedFrameSize)
        {
            deviceFrame->release();
            throw rdk::exception::frame_size_not_supported();
        }
    }

    const uint16_t pulsesToRead = DeviceMimoseBase::getNumActivePulseConfigurations(
        m_config.frame_config[m_activeFrameIndex].selected_pulse_configs);
    const auto* samples = reinterpret_cast<const uint16_t*>(deviceFrame->getData());

    if (equidistantSampling)
    {
        if (frameChannel == m_dataIndex)
        {
            ::fillBuffer(m_equidistantSamplingTraits.m_rawDataBuffer, samples, m_numSamplesForNextPulseInMem, pulsesToRead, RawDataSubType::FirstHalf);
            deviceFrame->release();
            return readRawFrame(frame, metadata, timeoutMillis);
        }
        else if (frameChannel == m_dataIndex2)
        {
            ::fillBuffer(m_equidistantSamplingTraits.m_rawDataBuffer, samples, m_numSamplesForNextPulseInMem, pulsesToRead, RawDataSubType::SecondHalf);

            const auto rem = (deviceFrame->getDataSize() / sizeof(uint16_t)) - (static_cast<size_t>(m_numSamplesForNextPulseInMem) * pulsesToRead);
            if (rem > 0)
            {
                const auto fromOffset = (m_numSamplesForNextPulseInMem * pulsesToRead);
                const auto toOffset = (fromOffset + rem);
                std::copy((samples + fromOffset), (samples + toOffset), (m_equidistantSamplingTraits.m_rawDataBuffer.data() + (static_cast<size_t>(m_numSamplesForNextPulseInMem) * pulsesToRead * IQ_SAMPLE_SIZE)));
            }
        }
    }

    const auto totalSampleCount = (m_frameBufferSize / sizeof(uint16_t));

    if (equidistantSampling)
    {
        samples = m_equidistantSamplingTraits.m_rawDataBuffer.data();
        if (totalSampleCount != m_equidistantSamplingTraits.m_rawDataBuffer.size())
        {
            deviceFrame->release();
            throw rdk::exception::internal();
        }
    }

    const auto iqNumSamplesPerPulseMemSize = (m_numSamplesForNextPulseInMem * IQ_SAMPLE_SIZE);  // expressed in uint16_t's as pairs of IQ's (x2)

    constexpr auto RAW_MEM_REGION_INDEX = 0;
    // constexpr auto FRAME_COUNTER_REGION_INDEX = 1;
    constexpr auto VCO_REGION_INDEX = 2;
    constexpr auto AOC_REGION_INDEX = 3;
    constexpr auto AGC_REGION_INDEX = 4;

    // create MemoryRegions
    MemoryRegions dataMemoryRegions;
    try
    {
        dataMemoryRegions.reserve(m_frameSpecificReadoutConfiguration.size());

        const uint16_t* memBegin = samples;
        const uint16_t* memEnd = memBegin;

        uint32_t mapped = 0;

        for (const auto& readoutConf : m_frameSpecificReadoutConfiguration)
        {
            memEnd += readoutConf.count;
            mapped += readoutConf.count;

            if (totalSampleCount >= mapped)
            {
                dataMemoryRegions.emplace_back(std::make_pair(memBegin, memEnd));
            }
            else
            {
                deviceFrame->release();
                throw rdk::exception::insufficient_memory_allocated();
            }
            memBegin = memEnd;
        }
    }
    catch (const std::out_of_range&)
    {
        deviceFrame->release();
        throw rdk::exception::internal();
    }

    // determine the current AFC
    {
        const auto& vcoMemory = dataMemoryRegions[VCO_REGION_INDEX];

        const auto afcValue = vcoMemory.first[2];
        const auto afcCounterLow = vcoMemory.first[3];
        const auto afcCounterHigh = vcoMemory.first[4];

        m_currentAfc = ::packAfc(afcValue, afcCounterLow, afcCounterHigh, true);
    }

    const auto& aocMemory = dataMemoryRegions[AOC_REGION_INDEX];
    const auto& agcMemory = dataMemoryRegions[AGC_REGION_INDEX];
    ::fillMetaData(metadata, aocMemory.first, agcMemory.first, pulsesToRead);

    const auto* pulseMemory = dataMemoryRegions[RAW_MEM_REGION_INDEX].first;

    // perform n reads according to the number of expected pulses
    for (uint16_t pulseIdx = 0; pulseIdx < pulsesToRead; ++pulseIdx)
    {
        for (uint32_t i = 0; i < m_numSamplesReturned; ++i)
        {

            const auto ifiIdx = (i * 2);
            // ifqIdx  = ifiIdx + 1

            const ifx_Float_t I = ::toFloat(pulseMemory[ifiIdx]);
            const ifx_Float_t Q = ::toFloat(pulseMemory[ifiIdx + 1]);

            IFX_CUBE_AT(frame, 0, pulseIdx, i) = ifx_Complex_t {I, Q};
        }
        pulseMemory += iqNumSamplesPerPulseMemSize;
    }
    deviceFrame->release();

    if (pulseMemory != dataMemoryRegions[RAW_MEM_REGION_INDEX].second)
    {
        throw rdk::exception::internal();
    }
}

/* This function returns the time required to read a frame from the memory, including the initialization setup time for the i2c. */
float DeviceMimose::calculateFrameReadoutTime()
{
    // The effective data rate is deduced from the I2c speed in fast mode (400kbit/s) by accounting for the additional clock cycle of the ACK
    const auto effectiveDataRate = 400e3f * 8 / 9;
    const auto& fConfig = m_config.frame_config[m_activeFrameIndex];
    const auto activePcCount = getNumActivePulseConfigurations(fConfig.selected_pulse_configs);
    const auto numSamples = fConfig.num_samples;

    auto defaultSetupTime = activePcCount * (8 /* for device address*/ + 16 /* for memory address*/ + 8 /* for device address*/) / effectiveDataRate;
    auto readoutTime = activePcCount * 32 * numSamples /* 16 bit i sample and 16 bit q sample*/ / effectiveDataRate;
    if (m_samplingMode == SamplingMode::EquidistantSampling)
    {
        readoutTime += 2 * defaultSetupTime;
    }
    else
    {
        readoutTime += defaultSetupTime;
    }

    return readoutTime;
}

/* This function calculates the trigger count to write into the TRIG_COUNT field of the trigger related registers,
 * in order to set a trigger after half of the frame active time, in the case of equidistant mode. */
uint16_t DeviceMimose::calculateTriggerCount()
{
    const auto& fConfig = m_config.frame_config[m_activeFrameIndex];
    const auto activePcCount = getNumActivePulseConfigurations(fConfig.selected_pulse_configs);

    return activePcCount * fConfig.num_samples / 2;
}

void DeviceMimose::determineSamplingMode()
{
    const auto& fConfig = m_config.frame_config[m_activeFrameIndex];
    const auto framePeriodSecs = fConfig.frame_repetition_time_s;
    const auto pulseRepTimeSecs = fConfig.pulse_repetition_time_s;

    const auto minRepTime = DeviceMimoseBase::calculateMinimumFrameRepetitionTime(fConfig.selected_pulse_configs,
                                                                                  fConfig.num_samples,
                                                                                  m_config.afc_config.afc_duration_ct,
                                                                                  pulseRepTimeSecs);

    auto heatingPulseCount = getNumberOfHeatingPulses(minRepTime, fConfig.frame_repetition_time_s, fConfig.pulse_repetition_time_s);
    const auto heatingPulseTimeSecs = heatingPulseCount * pulseRepTimeSecs;
    const auto frameIdleTime = framePeriodSecs - minRepTime;
    const auto frameReadoutTime = calculateFrameReadoutTime();
    const auto activePcCount = getNumActivePulseConfigurations(fConfig.selected_pulse_configs);

    if (((heatingPulseTimeSecs + minRepTime) <= framePeriodSecs) && (frameReadoutTime < framePeriodSecs))
    {
        /* The equidistant sampling mode is chosen if the frame idle time is less than the needed frameReadoutTime.*/
        if (frameIdleTime < frameReadoutTime)
        {
            m_samplingMode = SamplingMode::EquidistantSampling;
            // Nb: Considering the heating pulse count is a bug in the current HW version, which will be fixed in the B1 version.
            // +1 pulse to avoid having another unnecessary trigger at the end of the frame.
            m_equidistantSamplingTraits.m_triggerCount = calculateTriggerCount() + heatingPulseCount * activePcCount + 1;
        }
        else
        {
            m_samplingMode = SamplingMode::FramePausedSampling;
            m_equidistantSamplingTraits.m_triggerCount = 0;
        }
    }
    else
    {
        throw rdk::exception::frame_rate_out_of_range();
    }
}

void DeviceMimose::setAOCModeAndUpdateConfig(const ifx_Mimose_AOC_Mode_t aocMode[4])
{
    const auto numberOfActivePulses = getNumActivePulseConfigurations(m_config.frame_config[m_activeFrameIndex].selected_pulse_configs);
    if (!numberOfActivePulses)
    {
        throw rdk::exception::argument_invalid();
    }

    m_regConfig->reset();

    for (auto pulseIndex = numberOfActivePulses - 1; pulseIndex >= 0; pulseIndex--)
    {
        DeviceMimose::setAutomaticOffsetCompensationMode(pulseIndex, aocMode[pulseIndex]);
        m_config.pulse_config[pulseIndex].aoc_mode = aocMode[pulseIndex];
    }

    m_regConfig->flushEnqRegisters();
}
