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

#include "DeviceMimoseBase.hpp"

#include "DeviceMimoseConstants.hpp"
#include "DeviceMimoseRegisterConfigurator.hpp"

#include "ifxBase/Exception.hpp"
#include "ifxBase/Utils.hpp"
#include "registers_BGT24ATR22.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iterator>
#include <numeric>
#include <vector>

namespace {  // Default limits instance
ifx_Mimose_Config_Limits_t g_absoluteLimits = {IFX_MIMOSE_TX_POWER_LEVEL_MIN, IFX_MIMOSE_TX_POWER_LEVEL_MAX,
                                               IFX_MIMOSE_NUM_SAMPLES_MIN, IFX_MIMOSE_NUM_SAMPLES_MAX,
                                               IFX_MIMOSE_PULSE_REPETITION_TIME_S_MIN, IFX_MIMOSE_PULSE_REPETITION_TIME_S_MAX,
                                               IFX_MIMOSE_FRAME_REPETITION_TIME_S_MIN, IFX_MIMOSE_FRAME_REPETITION_TIME_S_MAX,
                                               MIN(IFX_MIMOSE_RF_CENTER_FREQ_HZ_MIN_BAND_100MHz, IFX_MIMOSE_RF_CENTER_FREQ_HZ_MIN_BAND_200MHz), IFX_MIMOSE_RF_CENTER_FREQ_HZ_MAX,
                                               IFX_MIMOSE_AFC_DURATION_CT_MIN, IFX_MIMOSE_AFC_DURATION_CT_MAX,
                                               IFX_MIMOSE_AFC_THRESHOLD_COURSE_MIN, IFX_MIMOSE_AFC_THRESHOLD_COURSE_MAX,
                                               IFX_MIMOSE_AFC_THRESHOLD_FINE_MIN, IFX_MIMOSE_AFC_THRESHOLD_FINE_MAX};


ifx_Mimose_Pulse_Config_t defaultPulseConfiguration_ANT_A_MAX = {
    IFX_MIMOSE_CHANNEL_TX1_RX1,
    IFX_MIMOSE_TX_POWER_LEVEL_DEF,
    IFX_MIMOSE_ABB_TYPE_DEF,
    IFX_MIMOSE_AOC_MODE_DEF};

ifx_Mimose_Pulse_Config_t defaultPulseConfiguration_ANT_B_MAX = {
    IFX_MIMOSE_CHANNEL_TX2_RX2,
    IFX_MIMOSE_TX_POWER_LEVEL_DEF,
    IFX_MIMOSE_ABB_TYPE_DEF,
    IFX_MIMOSE_AOC_MODE_DEF};

ifx_Mimose_Pulse_Config_t defaultPulseConfiguration_ANT_A_DEF = {
    IFX_MIMOSE_CHANNEL_TX1_RX2,
    IFX_MIMOSE_TX_POWER_LEVEL_DEF,
    IFX_MIMOSE_ABB_TYPE_DEF,
    IFX_MIMOSE_AOC_MODE_DEF};

ifx_Mimose_Pulse_Config_t defaultPulseConfiguration_ANT_B_DEF = {
    IFX_MIMOSE_CHANNEL_TX2_RX1,
    IFX_MIMOSE_TX_POWER_LEVEL_DEF,
    IFX_MIMOSE_ABB_TYPE_DEF,
    IFX_MIMOSE_AOC_MODE_DEF};

ifx_Mimose_AFC_Control_t defaultAfcControl = {
    IFX_MIMOSE_RF_BAND_ISM_SRR_24GHZ_100MHz,
    IFX_MIMOSE_RF_CENTER_FREQ_HZ_DEF,
    IFX_MIMOSE_AFC_DURATION_CT_DEF,
    IFX_MIMOSE_AFC_THRESHOLD_COURSE_DEF,
    IFX_MIMOSE_AFC_THRESHOLD_FINE_DEF,
    IFX_MIMOSE_AFC_PERIOD_DEF,
    IFX_MIMOSE_AFC_REPEAT_COUNT_1};

// default frame configuration
ifx_Mimose_Frame_Config_t defaultFrameConfiguration = {
    IFX_MIMOSE_FRAME_REPETITION_TIME_S_DEFAULT,
    IFX_MIMOSE_PULSE_REPETITION_TIME_S_DEFAULT,
    {true, false, false, false},
    IFX_MIMOSE_NUM_SAMPLES_DEF,
};

ifx_Mimose_Clock_Config_t defaultClockConfiguration = {
    IFX_MIMOSE_REF_CLK_HZ_DEFAULT,    // reference_clock_Hz
    IFX_MIMOSE_REF_CLK_HZ_DEFAULT,    // system_clock_Hz
    false,                            // rc_clock_enabled
    IFX_MIMOSE_HF_ONTIME_US_DEFAULT,  // hf_on_time_usec
    0,                                // system_clock_divider
    false,                            // system_clock_div_flex
    false,                            // sys_clk_to_i2c
};

const ifx_Mimose_Config_t defaultConfiguration = {
    {defaultPulseConfiguration_ANT_A_MAX, defaultPulseConfiguration_ANT_B_MAX, defaultPulseConfiguration_ANT_A_DEF, defaultPulseConfiguration_ANT_B_DEF},
    {defaultFrameConfiguration, {/* zero config */}},
    defaultAfcControl,
    defaultClockConfiguration};

bool checkAFCFrequency(const ifx_Mimose_AFC_Control_t& afcConfig)
{
    auto validBand = (afcConfig.band == IFX_MIMOSE_RF_BAND_ISM_SRR_24GHZ_100MHz) || (afcConfig.band == IFX_MIMOSE_RF_BAND_ISM_SRR_24GHZ_200MHz);
    if (!validBand)
    {
        return false;
    }
    uint64_t minRFCenterFrequency;
    uint64_t maxRFCenterFrequency;
    DeviceMimoseBase::getAFCFrequencyRange(afcConfig.band, &minRFCenterFrequency, &maxRFCenterFrequency);

    return checkInRange(afcConfig.rf_center_frequency_Hz, minRFCenterFrequency, maxRFCenterFrequency);
}

bool checkAbbGainType(const ifx_Mimose_ABB_type_t& ABBGainType)
{
    // ABB AUTO AGC is buggy in current silicon, we need to make sure it is not the set type
    return (ABBGainType != IFX_MIMOSE_ABB_GAIN_AUTOMATIC);
}

bool checkPulseConfiguration(const ifx_Mimose_Pulse_Config_t& pulseConfiguration, const ifx_Mimose_Config_Limits_t& limits)
{
    return checkInRange(pulseConfiguration.tx_power_level, limits.min_tx_power_level, limits.max_tx_power_level)
           && checkAbbGainType(pulseConfiguration.abb_gain_type);
}

bool checkFrameRepetitionTime(float frt, float minFRT, float maxFRT)
{
    return checkInRange(frt, minFRT, maxFRT);
}

bool checkSelectedPulseConfigs(const bool selectedPulseConfigs[4], const ifx_Mimose_Pulse_Config_t* pulseConfigurations, const ifx_Mimose_Config_Limits_t& limits)
{
    // At least one of the pulse configs is active
    const auto numberOfActivePulses = DeviceMimoseBase::getNumActivePulseConfigurations(selectedPulseConfigs);
    if (!numberOfActivePulses)
    {
        return false;
    }

    for (size_t i = 0; i < numberOfActivePulses; i++)
    {
        if (!checkPulseConfiguration(pulseConfigurations[i], limits))
        {
            return false;
        }
    }

    return true;
}

bool checkPulseRepetitionTime(float prt, float minPRT, float maxPRT)
{
    return checkInRange(prt, minPRT, maxPRT);
}

bool checkNumberOfSamples(uint16_t numberOfSamples, uint16_t minNumberOfSamples, uint16_t maxNumberOfSamples)
{
    return checkInRange(numberOfSamples, minNumberOfSamples, maxNumberOfSamples);
}

bool checkFrameConfiguration(const ifx_Mimose_Frame_Config_t& frameConfig, const ifx_Mimose_Config_Limits_t& limits,
                             const ifx_Mimose_AFC_Control_t& afcConfig, const ifx_Mimose_Pulse_Config_t pulseConfigurations[4])
{
    return checkNumberOfSamples(frameConfig.num_samples, limits.min_num_samples, limits.max_num_samples)
           && checkSelectedPulseConfigs(frameConfig.selected_pulse_configs, pulseConfigurations, limits)
           && checkPulseRepetitionTime(frameConfig.pulse_repetition_time_s, limits.min_pulse_repetition_time_s, limits.max_pulse_repetition_time_s)
           && checkFrameRepetitionTime(frameConfig.frame_repetition_time_s, limits.min_frame_repetition_time_s, limits.max_frame_repetition_time_s);
}
}  // end of anonymous namespace

void DeviceMimoseBase::setConfig(const ifx_Mimose_Config_t* config)
{
    auto validConfig = checkConfiguration(config, m_activeFrameIndex);
    if (!validConfig)
    {
        throw rdk::exception::argument_invalid();
    }

    m_frameConfigValid = validConfig;
    m_config = *config;
}

void DeviceMimoseBase::getConfig(ifx_Mimose_Config_t* config) const
{
    if (!config)
    {
        throw rdk::exception::argument_null();
    }
    if (!m_frameConfigValid)
    {
        throw rdk::exception::not_configured();
    }
    *config = m_config;
}

void DeviceMimoseBase::getDefaultConfig(ifx_Mimose_Config_t* config) const
{
    if (!config)
    {
        throw rdk::exception::argument_null();
    }

    *config = defaultConfiguration;
}

void DeviceMimoseBase::getDefaultLimits(ifx_Mimose_Config_Limits_t* limits) const
{
    if (!limits)
    {
        throw rdk::exception::argument_null();
    }

    *limits = g_absoluteLimits;
}

void DeviceMimoseBase::destroy_routine()
{
    // override by DeviceMimoseImpl
}

const ifx_Radar_Sensor_Info_t* DeviceMimoseBase::getSensorInfo()
{
    static const int8_t empty_list8[] = {-1};
    static const int32_t empty_list32[] = {-1};
    static const ifx_Radar_Sensor_Info_t info = {
        /* .description = */ "BGT24ATR22 radar sensor",
        /* min_rf_frequency_Hz = */ 24'000'000'000,
        /* max_rf_frequency_Hz = */ 24'500'000'000,
        /* .num_tx_antennas = */ 2,
        /* .num_rx_antennas = */ 2,
        /* .max_tx_power = */ 31,
        /* .device_id = */ 0,
        /* .max_num_samples_per_chirp = */ MAX_NUM_SAMPLES_ONE_PULSE,
        /* .min_adc_sampling_rate =*/20'000'000,
        /* .max_adc_sampling_rate= */ 40'000'000,
        /* .adc_resolution_bits*/ 12,
        /*.hp_cutoff_list = */ empty_list32,
        /*.lp_cutoff_list = */ empty_list32,
        /*.if_gain_list = */ empty_list8};

    return &info;
}

ifx_Cube_C_t* DeviceMimoseBase::createFrame()
{
    const auto& frameConfiguration = m_config.frame_config[m_activeFrameIndex];

    const auto num_samples_per_frame = frameConfiguration.num_samples;
    const auto num_pulse_configurations_per_frame = getNumActivePulseConfigurations(frameConfiguration.selected_pulse_configs);
    auto* cube_ptr = ifx_cube_create_c(1, num_pulse_configurations_per_frame, num_samples_per_frame);
    if (!cube_ptr)
    {
        throw rdk::exception::memory_allocation_failed();
    }

    return cube_ptr;
}

bool DeviceMimoseBase::checkConfiguration(const ifx_Mimose_Config_t* config, uint16_t frameConfigIndex)
{
    // Check if the given frame config is empty
    if (config->frame_config[frameConfigIndex].frame_repetition_time_s == 0.0f)
    {
        return false;
    }
    // check if system clock divider exceeds 4 bits
    if (config->clock_config.system_clock_divider > 15)
    {
        return false;
    }
    const ifx_Mimose_Frame_Config_t frameConfig = config->frame_config[frameConfigIndex];
    ifx_Mimose_Config_Limits_t limits;
    getDefaultLimits(&limits);
    getNumberOfSamplesLimits(frameConfig.selected_pulse_configs, &limits.min_num_samples, &limits.max_num_samples);
    limits.min_frame_repetition_time_s = calculateMinimumFrameRepetitionTime(frameConfig.selected_pulse_configs, frameConfig.num_samples, config->afc_config.afc_duration_ct, frameConfig.pulse_repetition_time_s);

    return checkFrameConfiguration(config->frame_config[frameConfigIndex], limits, config->afc_config, config->pulse_config)
           && checkAFCFrequency(config->afc_config);
}

/* static */
void DeviceMimoseBase::dumpRegisters(const char* filename) const
{
    IFX_IGNORE(filename);
    throw rdk::exception::device_not_supported();
}

const ifx_Firmware_Info_t* DeviceMimoseBase::getFirmwareInformation() const
{
    return &m_firmware_info;
}

/* This function calculates the minimum FRT (Minimum Frame Repetition Time), given the listed arguments.*/
float DeviceMimoseBase::calculateMinimumFrameRepetitionTime(const bool selectedPulseConfigs[4],
                                                            uint16_t numOfSamples,
                                                            uint16_t afcDuration,
                                                            float pulseRepetitionTime)
{
    auto numberOfActivePulses = getNumActivePulseConfigurations(selectedPulseConfigs);

    const auto maxAdcConversionTime = 30e-6f;  // depends on sysclk, this calculation is based on RC clock 12MHz
    // TODO (@benhmidaines): adapt calculation of adc conversion time to the actual clock, currently w.c.e.t is used.

    const auto singlePulseBootupTime = 100e-6f;  // This is default bootup time for VCO, from registers (vco_bootup)
    const auto pulseDurationTime = DeviceMimoseRegisterConfigurator::calcPulseDurationTime(BGT24ATR22_RXABB_HF_ON_T_REG_RST);
    const auto afcAdditionalFrameTime = DeviceMimoseRegisterConfigurator::calcAFC_additionalFrameTime(afcDuration);
    const auto sampleTime = pulseDurationTime + pulseRepetitionTime * numOfSamples - pulseRepetitionTime / numberOfActivePulses;

    const auto maxHeatingPulses = 2.0f;
    float minFrameRepetitionTime = sampleTime + afcAdditionalFrameTime + singlePulseBootupTime + maxAdcConversionTime
                                   + maxHeatingPulses * pulseRepetitionTime;

    if (minFrameRepetitionTime < pulseRepetitionTime * numOfSamples)
    {
        minFrameRepetitionTime = pulseRepetitionTime * numOfSamples;
    }

    return minFrameRepetitionTime;
}

void DeviceMimoseBase::getNumberOfSamplesLimits(const bool selectedPulseConfigs[4], uint16_t* minNumberOfSamples, uint16_t* maxNumberOfSamples)
{
    auto numberOfActivePulses = getNumActivePulseConfigurations(selectedPulseConfigs);
    *minNumberOfSamples = ((numberOfActivePulses > 1) ? MIN_NUM_SAMPLES_ALLOWED_MULTIPLE_PULSES : MIN_NUM_SAMPLES_ALLOWED_ONE_PULSE);
    *maxNumberOfSamples = ((numberOfActivePulses > 1) ? MAX_NUM_SAMPLES_MULTIPLE_PULSES : MAX_NUM_SAMPLES_ONE_PULSE);
}

void DeviceMimoseBase::getAFCFrequencyRange(ifx_Mimose_RF_Band_t band, uint64_t* minRFCenterFrequency, uint64_t* maxRFCenterFrequency)
{
    if (band == IFX_MIMOSE_RF_BAND_ISM_SRR_24GHZ_100MHz)
    {
        *minRFCenterFrequency = IFX_MIMOSE_RF_CENTER_FREQ_HZ_MIN_BAND_100MHz;
        *maxRFCenterFrequency = IFX_MIMOSE_RF_CENTER_FREQ_HZ_MAX;
    }
    else
    {
        *minRFCenterFrequency = IFX_MIMOSE_RF_CENTER_FREQ_HZ_MIN_BAND_200MHz;
        *maxRFCenterFrequency = IFX_MIMOSE_RF_CENTER_FREQ_HZ_MAX;
    }
}

void DeviceMimoseBase::getFrameRepetitionTimeLimits(const bool selectedPulseConfigs[4], uint16_t numberOfSamples, uint16_t afcDuration,
                                                    float pulseRepetitionTime, float* minFRT, float* maxFRT)
{
    auto numberOfActivePulses = getNumActivePulseConfigurations(selectedPulseConfigs);
    if (!numberOfActivePulses)
    {
        throw rdk::exception::argument_invalid();
    }

    uint16_t minNumberOfSamples;
    uint16_t maxNumberOfSamples;
    getNumberOfSamplesLimits(selectedPulseConfigs, &minNumberOfSamples, &maxNumberOfSamples);
    if (!checkNumberOfSamples(numberOfSamples, minNumberOfSamples, maxNumberOfSamples))
    {
        throw rdk::exception::argument_invalid();
    }

    float minPRT;
    float maxPRT;
    getPulseRepetitionTimeLimits(selectedPulseConfigs, &minPRT, &maxPRT);
    if (!checkPulseRepetitionTime(pulseRepetitionTime, minPRT, maxPRT))
    {
        throw rdk::exception::argument_invalid();
    }

    *minFRT = DeviceMimoseBase::calculateMinimumFrameRepetitionTime(selectedPulseConfigs, numberOfSamples, afcDuration, pulseRepetitionTime);
    *maxFRT = IFX_MIMOSE_FRAME_REPETITION_TIME_S_MAX;
}

void DeviceMimoseBase::getPulseRepetitionTimeLimits(const bool selectedPulseConfigs[4], float* minPRT, float* maxPRT)
{
    auto numberOfActivePulses = getNumActivePulseConfigurations(selectedPulseConfigs);
    if (!numberOfActivePulses)
    {
        throw rdk::exception::argument_invalid();
    }
    *minPRT = g_absoluteLimits.min_pulse_repetition_time_s * numberOfActivePulses;
    *maxPRT = g_absoluteLimits.max_pulse_repetition_time_s;
}
