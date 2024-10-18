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
 * @file DeviceLtr11Base.hpp
 *
 * \brief \copybrief gr_device_ltr11_base
 *
 * For details refer to \ref gr_device_ltr11_base
 */

#pragma once

#include "DeviceLtr11Types.h"
#include "ifxBase/internal/NonCopyable.hpp"
#include "ifxBase/Vector.h"
#include "ifxRadarDeviceCommon/RadarDeviceCommon.h"


struct DeviceLtr11Base
{
public:
    virtual ~DeviceLtr11Base() = default;

    virtual ifx_Ltr11_Config_t getDefaultConfig();

    virtual void getConfig(ifx_Ltr11_Config_t* config);
    virtual void setConfig(const ifx_Ltr11_Config_t* config);
    virtual void getLimits(ifx_Ltr11_Config_Limits_t* limits);
    virtual void dumpRegisters(const char* filename) = 0;

    virtual void startAcquisition() = 0;
    virtual void stopAcquisition() = 0;

    virtual ifx_Vector_C_t* getNextFrame(ifx_Vector_C_t* frame, ifx_Ltr11_Metadata_t* metadata, uint16_t timeoutMs) = 0;

    const ifx_Radar_Sensor_Info_t* get_sensor_info();
    const ifx_Firmware_Info_t* get_firmware_info() const;
    float getActiveModePower(const ifx_Ltr11_Config_t* config);
    uint32_t getSamplingFrequency(ifx_Ltr11_PRT_t prtIndex);
    bool checkConfig(const ifx_Ltr11_Config_t* config);

protected:
    ifx_Ltr11_Config_t m_config;
    ifx_Firmware_Info_t m_firmware_info;
    float m_activePower = 0;
    float m_lowPower = 0;
    uint64_t m_timestampThreshold = 0;
    bool m_bandJapan = false;
    bool m_frameConfigValid = false;

    uint16_t prtIndexToUs(ifx_Ltr11_PRT_t prtIndex);
    uint8_t aprtFactorValue(ifx_Ltr11_APRT_Factor_t aprtFactorIndex);

private:
    ifx_Radar_Sensor_Info_t m_sensor_info = {}; /**< Sensor information */
    float getPowerConsumption(ifx_Ltr11_PulseWidth_t pulseWidth, uint16_t prtValue);
};
