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
 * @file DeviceLtr11.cpp
 *
 * @brief Implementation for supporting externs LTR11
 */
#include <cstddef>  // for std::nullptr_t

#include "DeviceLtr11.h"
#include "DeviceLtr11Dummy.hpp"
#include "DeviceLtr11Impl.hpp"

#include "ifxBase/FunctionWrapper.hpp"
#include "ifxBase/internal/List.hpp"
#include "ifxBase/Log.h"
#include "ifxBase/Mem.h"
#include "ifxRadarDeviceCommon/internal/RadarDeviceCommon.hpp"


ifx_List_t* ifx_ltr11_get_list(void)
{
    auto selector = [](const ifx_Radar_Sensor_List_Entry_t& entry) {
        return rdk::RadarDeviceCommon::sensor_is_ltr11(entry.sensor_type);
    };

    auto list = rdk::RadarDeviceCommon::get_list(selector);
    return ifx_list_from_vector(list);
}

ifx_Ltr11_Device_t* ifx_ltr11_create()
{
    auto selector = [](const ifx_Radar_Sensor_List_Entry_t& entry) {
        return entry.sensor_type == IFX_BGT60LTR11AIP;
    };
    return rdk::RadarDeviceCommon::open_board_by_selector<DeviceLtr11>(selector);
}

ifx_Ltr11_Device_t* ifx_ltr11_create_by_uuid(const char* uuid)
{
    return rdk::RadarDeviceCommon::open_board_by_uuid<DeviceLtr11>(uuid);
}

ifx_Ltr11_Device_t* ifx_ltr11_create_dummy()
{
    return rdk::RadarDeviceCommon::open_device<DeviceLtr11Dummy>();
}

void ifx_ltr11_destroy(ifx_Ltr11_Device_t* handle)
{
    delete handle;
}

void ifx_ltr11_set_config(ifx_Ltr11_Device_t* handle, const ifx_Ltr11_Config_t* config)
{
    rdk::call_func(handle, &ifx_Ltr11_Device_t::setConfig, config);
}

void ifx_ltr11_get_config(ifx_Ltr11_Device_t* handle, ifx_Ltr11_Config_t* config)
{
    rdk::call_func(handle, &ifx_Ltr11_Device_t::getConfig, config);
}

void ifx_ltr11_get_config_defaults(ifx_Ltr11_Device_t* handle, ifx_Ltr11_Config_t* config)
{
    *config = handle->getDefaultConfig();
}

void ifx_ltr11_get_limits(ifx_Ltr11_Device_t* handle, ifx_Ltr11_Config_Limits_t* limits)
{
    rdk::call_func(handle, &ifx_Ltr11_Device_t::getLimits, limits);
}

void ifx_ltr11_start_acquisition(ifx_Ltr11_Device_t* handle)
{
    rdk::call_func(handle, &ifx_Ltr11_Device_t::startAcquisition);
}

void ifx_ltr11_stop_acquisition(ifx_Ltr11_Device_t* handle)
{
    rdk::call_func(handle, &ifx_Ltr11_Device_t::stopAcquisition);
}

ifx_Vector_C_t* ifx_ltr11_get_next_frame(ifx_Ltr11_Device_t* handle, ifx_Vector_C_t* frame_data, ifx_Ltr11_Metadata_t* metadata)
{
    return ifx_ltr11_get_next_frame_timeout(handle, frame_data, metadata, 1000);
}

ifx_Vector_C_t* ifx_ltr11_get_next_frame_timeout(ifx_Ltr11_Device_t* handle, ifx_Vector_C_t* frame_data, ifx_Ltr11_Metadata_t* metadata, uint16_t timeout_ms)
{
    return rdk::call_func(handle, &ifx_Ltr11_Device_t::getNextFrame, nullptr, frame_data, metadata, timeout_ms);
}

void ifx_ltr11_register_dump_to_file(ifx_Ltr11_Device_t* handle, const char* filename)
{
    rdk::call_func(handle, &ifx_Ltr11_Device_t::dumpRegisters, filename);
}

const ifx_Radar_Sensor_Info_t* ifx_ltr11_get_sensor_information(ifx_Ltr11_Device_t* handle)
{
    return rdk::call_func(handle, &ifx_Ltr11_Device_t::get_sensor_info);
}

const ifx_Firmware_Info_t* ifx_ltr11_get_firmware_information(ifx_Ltr11_Device_t* handle)
{
    return rdk::call_func(handle, &ifx_Ltr11_Device_t::get_firmware_info);
}

float ifx_ltr11_get_active_mode_power(ifx_Ltr11_Device_t* handle, const ifx_Ltr11_Config_t* config)
{
    return rdk::call_func(handle, &ifx_Ltr11_Device_t::getActiveModePower, config);
}

uint32_t ifx_ltr11_get_sampling_frequency(ifx_Ltr11_Device_t* handle, ifx_Ltr11_PRT_t prt_index)
{
    return rdk::call_func(handle, &ifx_Ltr11_Device_t::getSamplingFrequency, prt_index);
}

bool ifx_ltr11_check_config(ifx_Ltr11_Device_t* handle, const ifx_Ltr11_Config_t* config)
{
    return rdk::call_func(handle, &ifx_Ltr11_Device_t::checkConfig, config);
}
