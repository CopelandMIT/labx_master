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

#include "DeviceLtr11Base.hpp"
#include "ifxBase/Exception.hpp"

#include <cassert>
#include <cmath>

namespace {
const float avgIdleTimeCurrentmA = 0.8f;                       // Average Idle Time Current in mA
const uint8_t activePulseTimeUs = 15;                          // Active time in us
const uint8_t avgActiveTimeCurrentmA[4] = {100, 142, 83, 91};  // Average Current for entire pulse duration in mA
const float voltageV = 1.5f;                                   // The outputs from MMIC (Tdet and Pdet pins) are at the voltage level of 1.5 V

// Device constants and default limits
constexpr uint64_t LTR11_FREQUENCY_DEFAULT = 61'044'000'000ull;
constexpr uint64_t LTR11_FREQUENCY_DEFAULT_JAPAN = 60'540'000'000ull;

constexpr uint64_t LTR11_FREQUENCY_MIN = 61'044'000'000ull;
constexpr uint64_t LTR11_FREQUENCY_MAX = 61'452'000'000ull;
constexpr uint64_t LTR11_FREQUENCY_MIN_JAPAN = 60'540'000'000ull;
constexpr uint64_t LTR11_FREQUENCY_MAX_JAPAN = 60'948'000'000ull;

constexpr uint64_t LTR11_FREQUENCY_STEP = 2'400'000ull;

static_assert(LTR11_FREQUENCY_DEFAULT <= LTR11_FREQUENCY_MAX);
static_assert(LTR11_FREQUENCY_MIN <= LTR11_FREQUENCY_DEFAULT);

static_assert(LTR11_FREQUENCY_DEFAULT_JAPAN <= LTR11_FREQUENCY_MAX_JAPAN);
static_assert(LTR11_FREQUENCY_MIN_JAPAN <= LTR11_FREQUENCY_DEFAULT_JAPAN);

constexpr uint16_t LTR11_INTERNAL_DETECTOR_THRESHOLD_MIN = 0;
constexpr uint16_t LTR11_INTERNAL_DETECTOR_THRESHOLD_MAX = (1 << 12) - 1;
constexpr uint16_t LTR11_INTERNAL_DETECTOR_THRESHOLD_DEFAULT = IFX_LTR11_INTERNAL_DETECTOR_THRESHOLD_80;

static_assert(LTR11_INTERNAL_DETECTOR_THRESHOLD_DEFAULT <= LTR11_INTERNAL_DETECTOR_THRESHOLD_MAX);
static_assert(LTR11_INTERNAL_DETECTOR_THRESHOLD_MIN <= LTR11_INTERNAL_DETECTOR_THRESHOLD_DEFAULT);

// number of samples
constexpr uint16_t LTR11_NUM_SAMPLES_MIN = 16;
constexpr uint16_t LTR11_NUM_SAMPLES_MAX = 1024;
constexpr uint16_t LTR11_NUM_SAMPLES_DEFAULT = 256;

static_assert(LTR11_NUM_SAMPLES_DEFAULT <= LTR11_NUM_SAMPLES_MAX);
static_assert(LTR11_NUM_SAMPLES_MIN <= LTR11_NUM_SAMPLES_DEFAULT);

ifx_Ltr11_Config_t defaultConfig = {
    IFX_LTR11_SPI_PULSE_MODE,                  /* .mode */
    0,                                         /* .rf_frequency_Hz */
    LTR11_NUM_SAMPLES_DEFAULT,                 /* .num_samples */
    LTR11_INTERNAL_DETECTOR_THRESHOLD_DEFAULT, /* .internal_detector_threshold */
    IFX_LTR11_PRT_500us,                       /* .prt */
    IFX_LTR11_PULSE_WIDTH_5us,                 /* .pulse_width */
    IFX_LTR11_TX_POWER_LEVEL_4_5dBm,           /* .tx_power_level */
    IFX_LTR11_RX_IF_GAIN_50dB,                 /* .rx_if_gain */
    IFX_LTR11_APRT_FACTOR_1,                   /* .aprt_factor */
    IFX_LTR11_HOLD_TIME_1s,                    /* .hold_time */
    false,                                     /* .disable_internal_detector */
};
}  // namespace

template <typename T>
static bool checkInRange(T min, T x, T max)
{
    return min <= x && x <= max;
}

template <typename T>
static bool checkStep(T x, T step)
{
    return (x % step) == 0;
}

void DeviceLtr11Base::getConfig(ifx_Ltr11_Config_t* config)
{
    if (!config)
    {
        throw rdk::exception::argument_null();
    }
    *config = m_config;
}

void DeviceLtr11Base::setConfig(const ifx_Ltr11_Config_t* config)
{
    auto validConfig = checkConfig(config);
    if (!validConfig)
    {
        throw rdk::exception::argument_invalid();
    }

    m_frameConfigValid = validConfig;
    m_config = *config;
    const auto aprtFactor = aprtFactorValue(config->aprt_factor);
    const auto num_samples = config->num_samples;
    const auto prt = prtIndexToUs(config->prt);
    const auto pulseWidth = config->pulse_width;

    m_activePower = getPowerConsumption(pulseWidth, prt);
    m_lowPower = getPowerConsumption(pulseWidth, aprtFactor * prt);
    /* Tolerance of 10us, to cover the timestamp delta measurement deviation (dependant on the potential
     *  prt measurement deviations) when the chip is in active mode.*/
    const decltype(m_timestampThreshold) tolerance = 10;
    m_timestampThreshold = prt * num_samples + tolerance;
}

void DeviceLtr11Base::getLimits(ifx_Ltr11_Config_Limits_t* limits)
{
    if (!limits)
    {
        throw rdk::exception::argument_invalid();
    }

    uint64_t freqMin;
    uint64_t freqMax;

    if (m_bandJapan)
    {
        freqMin = LTR11_FREQUENCY_MIN_JAPAN;
        freqMax = LTR11_FREQUENCY_MAX_JAPAN;
    }
    else
    {
        freqMin = LTR11_FREQUENCY_MIN;
        freqMax = LTR11_FREQUENCY_MAX;
    }

    *limits = {
        {freqMin, freqMax, LTR11_FREQUENCY_STEP},                                      /* .rf_frequency_Hz */
        {LTR11_NUM_SAMPLES_MIN, LTR11_NUM_SAMPLES_MAX},                                /* .num_samples */
        {LTR11_INTERNAL_DETECTOR_THRESHOLD_MIN, LTR11_INTERNAL_DETECTOR_THRESHOLD_MAX} /* .internal_detector_threshold */
    };
}

bool DeviceLtr11Base::checkConfig(const ifx_Ltr11_Config_t* config)
{
    ifx_Ltr11_Config_Limits_t limits;
    getLimits(&limits);

    return checkInRange(limits.rf_frequency_Hz.min, config->rf_frequency_Hz, limits.rf_frequency_Hz.max)
           && checkStep(config->rf_frequency_Hz, limits.rf_frequency_Hz.step)
           && checkInRange(limits.internal_detector_threshold.min, config->internal_detector_threshold, limits.internal_detector_threshold.max)
           && checkInRange(limits.num_samples.min, config->num_samples, limits.num_samples.max);
}

ifx_Ltr11_Config_t DeviceLtr11Base::getDefaultConfig()
{
    if (m_bandJapan)
    {
        defaultConfig.rf_frequency_Hz = LTR11_FREQUENCY_DEFAULT_JAPAN;
    }
    else
    {
        defaultConfig.rf_frequency_Hz = LTR11_FREQUENCY_DEFAULT;
    }

    return defaultConfig;
}

const ifx_Radar_Sensor_Info_t* DeviceLtr11Base::get_sensor_info()
{
    if (m_bandJapan)
    {
        m_sensor_info.min_rf_frequency_Hz = LTR11_FREQUENCY_MIN_JAPAN;
        m_sensor_info.max_rf_frequency_Hz = LTR11_FREQUENCY_MAX_JAPAN;
    }
    else
    {
        m_sensor_info.min_rf_frequency_Hz = LTR11_FREQUENCY_MIN;
        m_sensor_info.max_rf_frequency_Hz = LTR11_FREQUENCY_MAX;
    }
    m_sensor_info.description = "BGT60LTR11AIP";
    m_sensor_info.num_tx_antennas = 1;
    m_sensor_info.num_rx_antennas = 1;
    m_sensor_info.max_tx_power = 7;
    m_sensor_info.device_id = 0;
    m_sensor_info.adc_resolution_bits = 8;

    return &m_sensor_info;
}

const ifx_Firmware_Info_t* DeviceLtr11Base::get_firmware_info() const
{
    return &m_firmware_info;
}

float DeviceLtr11Base::getActiveModePower(const ifx_Ltr11_Config_t* config)
{
    if (!config)
    {
        throw rdk::exception::argument_invalid();
    }

    const auto pulseWidth = config->pulse_width;
    const auto prt = prtIndexToUs(config->prt);

    return getPowerConsumption(pulseWidth, prt);
}

float DeviceLtr11Base::getPowerConsumption(ifx_Ltr11_PulseWidth_t pulseWidth, uint16_t prtValue)
{
    return voltageV * (avgActiveTimeCurrentmA[pulseWidth] * 1e-3f * activePulseTimeUs + avgIdleTimeCurrentmA * 1e-3f * (prtValue - activePulseTimeUs)) / prtValue;
}

uint16_t DeviceLtr11Base::prtIndexToUs(ifx_Ltr11_PRT_t prtIndex)
{
    switch (prtIndex)
    {
        case IFX_LTR11_PRT_250us:
            return 250;
        case IFX_LTR11_PRT_500us:
            return 500;
        case IFX_LTR11_PRT_1000us:
            return 1000;
        case IFX_LTR11_PRT_2000us:
            return 2000;
        default:
            throw rdk::exception::argument_invalid();
    }
}

uint8_t DeviceLtr11Base::aprtFactorValue(ifx_Ltr11_APRT_Factor_t aprtFactorIndex)
{
    switch (aprtFactorIndex)
    {
        case IFX_LTR11_APRT_FACTOR_16:
            return 16;
        case IFX_LTR11_APRT_FACTOR_8:
            return 8;
        case IFX_LTR11_APRT_FACTOR_4:
            return 4;
        case IFX_LTR11_APRT_FACTOR_2:
            return 2;
        case IFX_LTR11_APRT_FACTOR_1:
            return 1;
        default:
            throw rdk::exception::argument_invalid();
    }
}

uint32_t DeviceLtr11Base::getSamplingFrequency(ifx_Ltr11_PRT_t prtIndex)
{
    return 1000000u / prtIndexToUs(prtIndex);
}
