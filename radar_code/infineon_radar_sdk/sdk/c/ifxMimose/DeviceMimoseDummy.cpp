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

#include "DeviceMimoseDummy.hpp"

#include "ifxBase/Complex.h"
#include "ifxBase/Defines.h"

#include <chrono>
#include <cmath>
#include <limits>
#include <thread>


namespace {
uint64_t estimateFrameTimeDelay()
{
    // TODO
    return 50;
}
}  // namespace


DeviceMimoseDummy::DeviceMimoseDummy()
{
    // use default configuration
    DeviceMimoseBase::getDefaultConfig(&m_config);

    prepareWaveGeneration(m_config.frame_config[0].num_samples,
                          static_cast<uint32_t>(m_config.frame_config[0].frame_repetition_time_s * 1000),
                          static_cast<uint32_t>(m_config.frame_config[0].pulse_repetition_time_s * 1000));

    m_delay = ::estimateFrameTimeDelay();
}

void DeviceMimoseDummy::setConfig(const ifx_Mimose_Config_t* config)
{
    DeviceMimoseBase::setConfig(config);

    prepareWaveGeneration(m_config.frame_config[0].num_samples,
                          static_cast<uint32_t>(m_config.frame_config[0].frame_repetition_time_s * 1000),
                          static_cast<uint32_t>(m_config.frame_config[0].pulse_repetition_time_s * 1000));

    m_delay = ::estimateFrameTimeDelay();
}

void DeviceMimoseDummy::switchFrameConfiguration(uint16_t activeFrameConfigIndex)
{
    IFX_IGNORE(activeFrameConfigIndex);

    // nullop
}

void DeviceMimoseDummy::startAcquisition()
{
    // null op
}

void DeviceMimoseDummy::stopAcquisition()
{
    // null op
}

ifx_Cube_C_t* DeviceMimoseDummy::getNextFrame(ifx_Cube_C_t* frame, ifx_Mimose_Metadata_t* metadata, uint16_t timeoutMillis)
{
    IFX_IGNORE(timeoutMillis);
    IFX_IGNORE(metadata);

    if (frame == nullptr)
    {
        frame = createFrame();
    }

    const uint32_t num_pulse_configurations_per_frame = IFX_CUBE_ROWS(frame);
    const uint32_t num_samples_per_frame = IFX_CUBE_COLS(frame);

    for (uint32_t pulse = 0; pulse < num_pulse_configurations_per_frame; ++pulse)
    {
        // generate wave per each pulse
        const auto frameBuffer = generateWave();

        for (uint32_t sample = 0; sample < num_samples_per_frame; ++sample)
        {
            const auto real = frameBuffer.at(sample);  // I
            const auto imag = 0.0f;                    // Q

            IFX_CUBE_AT(frame, pulse, sample, 0) = IFX_COMPLEX_DEF(real, imag);
        }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(m_delay));

    return frame;
}


void DeviceMimoseDummy::getSensorValues(ifx_Mimose_Sensor_t* sensorValues)
{
    if (sensorValues != nullptr)
    {
        sensorValues->rf_frequency_Hz = 24'125'000'000.0f;
        sensorValues->temperature = 25;
    }
}

// private section //

void DeviceMimoseDummy::prepareWaveGeneration(const uint16_t numOfSamples, const uint32_t frameRepetitionTimeMillis, const uint32_t pulseRepetitionTimeMillis)
{
    m_sampleRate = (numOfSamples * static_cast<uint32_t>(1000 / frameRepetitionTimeMillis));
    m_frameNumOfSamples = numOfSamples;

    m_amplitude = 0.5;
    m_frequency = 500;
    m_phase = 0.0;
    m_deltaTime = 1.0f / m_sampleRate;
    m_time = m_deltaTime;  // let the first wave also be
}

size_t DeviceMimoseDummy::getRegisterCount()
{
    return 0;
}

void DeviceMimoseDummy::getRegisters(uint32_t* registers)
{
    // null op
}

void DeviceMimoseDummy::setRegisters(uint32_t* registers, size_t count)
{
    // null op
}

uint16_t DeviceMimoseDummy::getRegisterValue(uint16_t register_address)
{
    return 0;
}

void DeviceMimoseDummy::update_rc_lut()
{
    // null op
}

std::vector<ifx_Float_t> DeviceMimoseDummy::generateWave()
{
    if (m_time >= std::numeric_limits<float>::max())
    {
        m_time = 0.0;
    }

    std::vector<ifx_Float_t> frameBuffer;
    frameBuffer.resize(m_frameNumOfSamples);

    // generate sin wave
    for (auto& sample : frameBuffer)
    {
        sample = m_amplitude * std::sin(2 * IFX_PI * m_frequency * m_time + m_phase);
        m_time += m_deltaTime;
    }

    return frameBuffer;
}

void DeviceMimoseDummy::setAOCModeAndUpdateConfig(const ifx_Mimose_AOC_Mode_t aocMode[4])
{
    // null op
}
