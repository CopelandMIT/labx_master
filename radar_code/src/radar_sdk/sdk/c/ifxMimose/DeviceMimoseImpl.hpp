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


/**
 * @file DeviceMimoseImpl.hpp
 *
 * \brief \copybrief gr_device_mimose
 *
 * For details refer to \ref gr_device_mimose
 */

#pragma once

#include "DeviceMimoseBase.hpp"

#include "ifxBase/internal/NonCopyable.hpp"

// strata
#include <components/interfaces/IRegisters.hpp>
#include <platform/BoardInstance.hpp>
#include <universal/types/DataSettingsBgtRadar.h>

#include <atomic>
#include <vector>


class IRadarAtr22;
class IProtocolAtr22;

/*
 * @brief The structure holds the settings to configure for the equidistant sampling mode.
 */
struct EquidistantSamplingTraits
{
    uint16_t m_triggerCount;
    uint32_t m_frameBufferFirstHalfSize;
    uint32_t m_frameBufferSecondHalfSize;
    std::vector<uint16_t> m_rawDataBuffer;
};


class DeviceMimoseRegisterConfigurator;

struct DeviceMimose : public DeviceMimoseBase
{
    NONCOPYABLE(DeviceMimose);
    DeviceMimose(std::unique_ptr<BoardInstance>&& board);
    ~DeviceMimose() override;

    void setConfig(const ifx_Mimose_Config_t* config) override;
    void destroy_routine() override;
    void switchFrameConfiguration(uint16_t activeFrameConfigIndex) override;
    void startAcquisition() override;
    void stopAcquisition() override;
    ifx_Cube_C_t* getNextFrame(ifx_Cube_C_t* frame, ifx_Mimose_Metadata_t* metadata, uint16_t timeoutMillis) override;
    void getSensorValues(ifx_Mimose_Sensor_t* sensorValues) override;
    void dumpRegisters(const char* filename) const override;
    size_t getRegisterCount() override;
    void getRegisters(uint32_t* registers) override;
    void setRegisters(uint32_t* registers, size_t count) override;
    uint16_t getRegisterValue(uint16_t register_address) override;
    void update_rc_lut() override;
    void setAOCModeAndUpdateConfig(const ifx_Mimose_AOC_Mode_t aocMode[4]) override;

private:
    using ReadoutDataConfiguration = std::vector<ReadoutEntry_t>;

    enum class SamplingMode
    {
        FramePausedSampling,
        EquidistantSampling
    };

    std::unique_ptr<BoardInstance> m_board;

    IRadarAtr22* m_atr22;
    IBridgeControl* m_bridgeControl;
    IBridgeData* m_bridgeData;
    IData* m_data;
    IRegisters<uint16_t, uint16_t>* m_regs;

    std::array<ReadoutDataConfiguration, 2> m_fragmentSpecificReadoutConfigurations;
    ReadoutDataConfiguration m_frameSpecificReadoutConfiguration;

    std::unique_ptr<DeviceMimoseRegisterConfigurator> m_regConfig;

    uint8_t m_dataIndex;
    uint8_t m_dataIndex2;
    uint8_t m_statusDataIndex;

    std::atomic<bool> m_acquisitionStarted;

    uint32_t m_frameBufferSize;
    uint32_t m_statusBufferSize;

    uint16_t m_numSamplesReturned;
    uint16_t m_numSamplesForNextPulseInMem;

    std::atomic<uint64_t> m_currentAfc;

    bool m_switchingConf;

    SamplingMode m_samplingMode;
    EquidistantSamplingTraits m_equidistantSamplingTraits;

    ifx_Radar_Sensor_t getShieldType() const;

    void determineSamplingMode();

    void setupConfig();
    uint32_t setupConfigClock(const ifx_Mimose_Clock_Config_t& Clock_config);
    void setupConfigPulse(size_t frame_config_index, size_t pulse_index, uint32_t system_clock_Hz);
    void setupConfigFrame(size_t frame_config_index);
    void setupConfigAFC(const ifx_Mimose_AFC_Control_t& AFC_config);

    void setupFrameDelays(uint16_t frameConfigIndex, bool rc_clock_enabled);

    void setupFrameData();
    void setupStatusData();

    void setupBridgeData();

    void startDataStreaming();
    void stopDataStreaming();

    void enableInterruptTargetDetectPC0();

    enum RawDataTriggerType
    {
        DRDP_ONLY,
        PRE_TRIGGER_AND_DRDP
    };
    void enabledAcquisitionTriggerSignal(RawDataTriggerType dataTriggerType);

    void startSequencer(uint16_t frameConfigurationIndex);
    void stopSequencer();

    void setAnalogBaseBandMode(size_t pulse_index, ifx_Mimose_ABB_type_t abb);
    void setAutomaticOffsetCompensationMode(size_t pulse_index, ifx_Mimose_AOC_Mode_t aoc_mode);

    float calcAFC_additionalFrameTime();
    float calculateFrameReadoutTime();
    uint16_t calculateTriggerCount();

    IFrame* getFrame(uint16_t timeoutMillis);
    static uint32_t getFrameBufferSize(const ReadoutDataConfiguration& readoutConfiguration);

    void readRawFrame(ifx_Cube_C_t* frame, ifx_Mimose_Metadata_t* metadata, uint16_t timeoutMillis);
};
