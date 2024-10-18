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
 * @file DeviceMimoseDummy.hpp
 *
 * \brief \copybrief gr_device_mimose_dummy
 *
 * For details refer to \ref gr_device_mimose_dummy
 */

#pragma once

#include "DeviceMimoseBase.hpp"

#include <vector>


struct DeviceMimoseDummy : public DeviceMimoseBase
{
    DeviceMimoseDummy();

    void setConfig(const ifx_Mimose_Config_t* config) override;
    void switchFrameConfiguration(uint16_t activeFrameConfigIndex) override;
    void startAcquisition() override;
    void stopAcquisition() override;
    ifx_Cube_C_t* getNextFrame(ifx_Cube_C_t* frame, ifx_Mimose_Metadata_t* metadata, uint16_t timeoutMillis) override;
    void getSensorValues(ifx_Mimose_Sensor_t* sensorValues) override;
    void setAOCModeAndUpdateConfig(const ifx_Mimose_AOC_Mode_t aocMode[4]) override;
    size_t getRegisterCount() override;
    void getRegisters(uint32_t* registers) override;
    void setRegisters(uint32_t* registers, size_t count) override;
    uint16_t getRegisterValue(uint16_t register_address) override;
    void update_rc_lut() override;

private:
    void prepareWaveGeneration(uint16_t numOfSamples,
                               uint32_t frameRepetitionTimeMillis,
                               uint32_t pulseRepetitionTimeMillis);

    std::vector<ifx_Float_t> generateWave();

    uint64_t m_delay;

    // Wave generation attributes
    //
    uint32_t m_sampleRate;
    uint32_t m_frameNumOfSamples;
    float m_amplitude;
    uint32_t m_frequency;
    float m_phase;
    float m_time;
    float m_deltaTime;
};
