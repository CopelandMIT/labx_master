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

#include "DeviceMimose.h"

#include "DeviceMimoseDummy.hpp"
#include "DeviceMimoseImpl.hpp"

#include "ifxBase/Defines.h"
#include "ifxBase/FunctionWrapper.hpp"
#include "ifxBase/internal/List.hpp"
#include "ifxBase/List.h"
#include "ifxBase/Log.h"
#include "ifxBase/Mem.h"

#include "ifxRadarDeviceCommon/internal/RadarDeviceCommon.hpp"

#include <cassert>


int ifx_mimose_api_version(void)
{
    return MIMOSE_API_VERSION;
}

const ifx_Radar_Sensor_Info_t* ifx_mimose_get_sensor_information(ifx_Mimose_Device_t* handle)
{
    return rdk::call_func(handle, &ifx_Mimose_Device_t::getSensorInfo, nullptr);
}

ifx_Mimose_Device_t* ifx_mimose_create(void)
{
    auto selector = [](const ifx_Radar_Sensor_List_Entry_t& entry) {
        return entry.sensor_type == IFX_MIMOSE_BGT24ATR22;
    };

    return rdk::RadarDeviceCommon::open_board_by_selector<DeviceMimose>(selector);
}

ifx_Mimose_Device_t* ifx_mimose_create_by_uuid(const char* uuid)
{
    return rdk::RadarDeviceCommon::open_board_by_uuid<DeviceMimose>(uuid);
}

ifx_Mimose_Device_t* ifx_mimose_create_dummy(void)
{
    return rdk::RadarDeviceCommon::open_device<DeviceMimoseDummy>();
}

void ifx_mimose_destroy(ifx_Mimose_Device_t* handle)
{
    rdk::call_func(handle, &ifx_Mimose_Device_t::destroy_routine);
    // a destructor must not throw exceptions, so no need to wrap the delete
    delete handle;
}

void ifx_mimose_set_config(ifx_Mimose_Device_t* handle, const ifx_Mimose_Config_t* config)
{
    rdk::call_func(handle, &ifx_Mimose_Device_t::setConfig, config);
}

void ifx_mimose_get_config(const ifx_Mimose_Device_t* handle, ifx_Mimose_Config_t* config)
{
    rdk::call_func(handle, &ifx_Mimose_Device_t::getConfig, config);
}

void ifx_mimose_get_config_defaults(const ifx_Mimose_Device_t* handle, ifx_Mimose_Config_t* config)
{
    rdk::call_func(handle, &ifx_Mimose_Device_t::getDefaultConfig, config);
}

void ifx_mimose_switch_frame_configuration(ifx_Mimose_Device_t* handle, uint16_t active_frame_config_index)
{
    rdk::call_func(handle, &ifx_Mimose_Device_t::switchFrameConfiguration, active_frame_config_index);
}

void ifx_mimose_start_acquisition(ifx_Mimose_Device_t* handle)
{
    rdk::call_func(handle, &ifx_Mimose_Device_t::startAcquisition);
}

void ifx_mimose_stop_acquisition(ifx_Mimose_Device_t* handle)
{
    rdk::call_func(handle, &ifx_Mimose_Device_t::stopAcquisition);
}

ifx_Cube_C_t* ifx_mimose_get_next_frame(ifx_Mimose_Device_t* handle, ifx_Cube_C_t* frame, ifx_Mimose_Metadata_t* metadata)
{
    return ifx_mimose_get_next_frame_timeout(handle, frame, metadata, 1100);
}

ifx_Cube_C_t* ifx_mimose_get_next_frame_timeout(ifx_Mimose_Device_t* handle, ifx_Cube_C_t* frame, ifx_Mimose_Metadata_t* metadata, uint16_t timeout_ms)
{
    return rdk::call_func(handle, &ifx_Mimose_Device_t::getNextFrame, nullptr, frame, metadata, timeout_ms);
}

size_t ifx_mimose_get_register_count(ifx_Mimose_Device_t* handle)
{
    return rdk::call_func(handle, &ifx_Mimose_Device_t::getRegisterCount);
}

void ifx_mimose_get_registers(ifx_Mimose_Device_t* handle, uint32_t* registers)
{
    return rdk::call_func(handle, &ifx_Mimose_Device_t::getRegisters, registers);
}

void ifx_mimose_set_registers(ifx_Mimose_Device_t* handle, uint32_t* registers, size_t count)
{
    return rdk::call_func(handle, &ifx_Mimose_Device_t::setRegisters, registers, count);
}

uint16_t ifx_mimose_get_register_value(ifx_Mimose_Device_t* handle, uint16_t register_address)
{
    return rdk::call_func(handle, &ifx_Mimose_Device_t::getRegisterValue, register_address);
}

void ifx_mimose_update_rc_lut(ifx_Mimose_Device_t* handle)
{
    return rdk::call_func(handle, &ifx_Mimose_Device_t::update_rc_lut);
}

void ifx_mimose_get_default_limits(const ifx_Mimose_Device_t* handle, ifx_Mimose_Config_Limits_t* limits)
{
    rdk::call_func(handle, &ifx_Mimose_Device_t::getDefaultLimits, limits);
}

void ifx_mimose_register_dump_to_file(const ifx_Mimose_Device_t* handle, const char* filename)
{
    rdk::call_func(handle, &ifx_Mimose_Device_t::dumpRegisters, filename);
}

void ifx_mimose_get_sensor_values(ifx_Mimose_Device_t* handle, ifx_Mimose_Sensor_t* sensor_values)
{
    rdk::call_func(handle, &ifx_Mimose_Device_t::getSensorValues, sensor_values);
}

ifx_List_t* ifx_mimose_get_list(void)
{
    auto selector = [](const ifx_Radar_Sensor_List_Entry_t& entry) {
        return rdk::RadarDeviceCommon::sensor_is_mimose(entry.sensor_type);
    };

    auto list = rdk::RadarDeviceCommon::get_list(selector);
    return ifx_list_from_vector(list);
}

const ifx_Firmware_Info_t* ifx_mimose_get_firmware_information(ifx_Mimose_Device_t* handle)
{
    return rdk::call_func(handle, &ifx_Mimose_Device_t::getFirmwareInformation);
}

bool ifx_mimose_check_config(ifx_Mimose_Device_t* handle, const ifx_Mimose_Config_t* config, uint16_t frame_configuration_index)
{
    return rdk::call_func(handle, &ifx_Mimose_Device_t::checkConfiguration, false, config, frame_configuration_index);
}

void ifx_mimose_get_frame_repetition_time_limits_s(ifx_Mimose_Device_t* handle, const bool selected_pulse_configs[4], uint16_t number_of_samples,
                                                   uint16_t afc_duration_ct, float prt, float* min_frt, float* max_frt)
{
    rdk::call_func(&ifx_Mimose_Device_t::getFrameRepetitionTimeLimits, selected_pulse_configs, number_of_samples, afc_duration_ct, prt, min_frt, max_frt);
}

void ifx_mimose_get_number_of_samples_limits(ifx_Mimose_Device_t* handle, const bool selected_pulse_configs[4], uint16_t* min_number_of_samples, uint16_t* max_number_of_samples)
{
    rdk::call_func(&ifx_Mimose_Device_t::getNumberOfSamplesLimits, selected_pulse_configs, min_number_of_samples, max_number_of_samples);
}

void ifx_mimose_get_rf_center_frequency_limits_Hz(ifx_Mimose_Device_t* handle, ifx_Mimose_RF_Band_t band,
                                                  uint64_t* min_rf_center_frequency, uint64_t* max_rf_center_frequency)
{
    rdk::call_func(&ifx_Mimose_Device_t::getAFCFrequencyRange, band, min_rf_center_frequency, max_rf_center_frequency);
}

void ifx_mimose_get_pulse_repetition_time_limits_s(ifx_Mimose_Device_t* handle, const bool selected_pulse_configs[4], float* min_prt, float* max_prt)
{
    rdk::call_func(&ifx_Mimose_Device_t::getPulseRepetitionTimeLimits, selected_pulse_configs, min_prt, max_prt);
}

void ifx_mimose_set_automatic_offset_compensation_mode(ifx_Mimose_Device_t* handle, const ifx_Mimose_AOC_Mode_t aoc_mode[4])
{
    rdk::call_func(handle, &ifx_Mimose_Device_t::setAOCModeAndUpdateConfig, aoc_mode);
}
