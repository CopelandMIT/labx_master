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
 * @file DeviceMimoseBase.hpp
 *
 * \brief \copybrief gr_device_mimose_base
 *
 * For details refer to \ref gr_device_mimose_base
 */

#pragma once

#include "DeviceMimoseTypes.h"

#include "ifxBase/Cube.h"
#include "ifxRadarDeviceCommon/RadarDeviceCommon.h"

#include <numeric>

struct DeviceMimoseBase
{
    virtual ~DeviceMimoseBase() = default;

    virtual void setConfig(const ifx_Mimose_Config_t* config);
    virtual void getConfig(ifx_Mimose_Config_t* config) const;
    virtual void getDefaultConfig(ifx_Mimose_Config_t* config) const;
    virtual void getDefaultLimits(ifx_Mimose_Config_Limits_t* limits) const;
    virtual void destroy_routine();
    virtual const ifx_Radar_Sensor_Info_t* getSensorInfo();
    virtual ifx_Cube_C_t* createFrame();
    virtual void switchFrameConfiguration(uint16_t activeFrameConfigIndex) = 0;
    virtual void startAcquisition() = 0;
    virtual void stopAcquisition() = 0;
    virtual ifx_Cube_C_t* getNextFrame(ifx_Cube_C_t* frame, ifx_Mimose_Metadata_t* metadata, uint16_t timeoutMillis) = 0;
    virtual void getSensorValues(ifx_Mimose_Sensor_t* sensorValues) = 0;
    virtual size_t getRegisterCount() = 0;
    virtual void getRegisters(uint32_t* registers) = 0;
    virtual void setRegisters(uint32_t* registers, size_t count) = 0;
    virtual uint16_t getRegisterValue(uint16_t register_address) = 0;
    virtual void update_rc_lut() = 0;
    virtual void dumpRegisters(const char* filename) const;
    virtual const ifx_Firmware_Info_t* getFirmwareInformation() const;
    virtual bool checkConfiguration(const ifx_Mimose_Config_t* config, uint16_t frameConfigIndex);
    virtual void setAOCModeAndUpdateConfig(const ifx_Mimose_AOC_Mode_t aocMode[4]) = 0;

    static void getFrameRepetitionTimeLimits(const bool selectedPulseConfigs[4], uint16_t numberOfSamples, uint16_t afcDuration,
                                             float pulseRepetitionTime, float* minFRT, float* maxFRT);
    static void getNumberOfSamplesLimits(const bool selectedPulseConfigs[4], uint16_t* minNumberOfSamples, uint16_t* maxNumberOfSamples);
    static void getPulseRepetitionTimeLimits(const bool selectedPulseConfigs[4], float* minPRT, float* maxPRT);
    static void getAFCFrequencyRange(ifx_Mimose_RF_Band_t band, uint64_t* minRFCenterFrequency, uint64_t* maxRFCenterFrequency);
    inline static uint16_t getNumActivePulseConfigurations(bool const activePcs[4])
    {
        return std::accumulate(activePcs, activePcs + 4u, 0u);
    }

protected:
    DeviceMimoseBase() = default;

    static float calculateMinimumFrameRepetitionTime(const bool selectedPulseConfigs[4],
                                                     uint16_t numOfSamples,
                                                     uint16_t afcDuration,
                                                     float pulseRepetitionTime);

    // attributes
    ifx_Mimose_Config_t m_config = {};
    bool m_frameConfigValid = false;
    uint16_t m_activeFrameIndex = 0;
    ifx_Firmware_Info_t m_firmware_info = {
        /* .description = */ "Mimose dummy device",
        /* .version_major = */ 0,
        /* .version_minor = */ 0,
        /* .version_build = */ 0,
        /* .extendedVersion = */ "Mimose dummy device"};
};
