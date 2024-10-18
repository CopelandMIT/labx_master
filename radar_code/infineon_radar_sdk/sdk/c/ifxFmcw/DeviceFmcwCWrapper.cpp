/* ===========================================================================
** Copyright (C) 2021 - 2022 Infineon Technologies AG
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

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include "avian/DeviceFmcwAvian.hpp"
#include "DeviceFmcw.h"

#include "ifxBase/FunctionWrapper.hpp"
#include "ifxBase/internal/List.hpp"

#include <platform/NamedMemory.hpp>


/*
==============================================================================
   2. LOCAL DEFINITIONS
==============================================================================
*/

/*
==============================================================================
   3. LOCAL TYPES
==============================================================================
*/

/*
==============================================================================
   5. LOCAL FUNCTION PROTOTYPES
==============================================================================
*/

/*
==============================================================================
   6. LOCAL FUNCTIONS
==============================================================================
*/


/*
==============================================================================
   7. EXPORTED FUNCTIONS
==============================================================================
*/

ifx_List_t* ifx_fmcw_get_list_by_sensor_type(ifx_Radar_Sensor_t sensor_type)
{
    auto selector = [&sensor_type](const ifx_Radar_Sensor_List_Entry_t& entry) {
        return entry.sensor_type == sensor_type;
    };

    auto list = rdk::RadarDeviceCommon::get_list(selector);
    return ifx_list_from_vector(list);
}

//----------------------------------------------------------------------------

const ifx_Firmware_Info_t* ifx_fmcw_get_firmware_information(const ifx_Device_Fmcw_t* handle)
{
    return rdk::call_func(handle, &ifx_Device_Fmcw_t::get_firmware_info);
}

//----------------------------------------------------------------------------

const ifx_Radar_Sensor_Info_t* ifx_fmcw_get_sensor_information(const ifx_Device_Fmcw_t* handle)
{
    return rdk::call_func(handle, &ifx_Device_Fmcw_t::get_sensor_info);
}

//----------------------------------------------------------------------------

ifx_List_t* ifx_fmcw_get_list()
{
    using namespace rdk::RadarDeviceCommon;
    auto selector = [](const ifx_Radar_Sensor_List_Entry_t& entry) {
        return sensor_is_avian(entry.sensor_type);
    };

    auto list = get_list(selector);
    return ifx_list_from_vector(list);
}

//----------------------------------------------------------------------------

const char* ifx_fmcw_get_board_uuid(const ifx_Device_Fmcw_t* handle)
{
    return rdk::call_func(handle, &ifx_Device_Fmcw_t::get_board_uuid);
}

//----------------------------------------------------------------------------

ifx_Device_Fmcw_t* ifx_fmcw_create_by_port(const char* port)
{
    // if port is NULL, call ifx_fmcw_create. This ensures the previous behavior.
    if (port == nullptr)
        return ifx_fmcw_create();

    return rdk::RadarDeviceCommon::open_board_by_port<DeviceFmcwAvian>(port);
}

//----------------------------------------------------------------------------

ifx_Device_Fmcw_t* ifx_fmcw_create_dummy(ifx_Radar_Sensor_t sensor_type)
{
    if (rdk::RadarDeviceCommon::sensor_is_avian(sensor_type))
    {
        return rdk::RadarDeviceCommon::open_device<DeviceFmcwAvian>(sensor_type);
    }
    else
    {
        return nullptr;
    }
}

//----------------------------------------------------------------------------

ifx_Device_Fmcw_t* ifx_fmcw_create_dummy_from_device(const ifx_Device_Fmcw_t* handle)
{
    const auto* avian_instance = dynamic_cast<const DeviceFmcwAvian*>(handle);
    if (avian_instance)
    {
        return rdk::RadarDeviceCommon::open_device<DeviceFmcwAvian>(*avian_instance);
    }

    return nullptr;
}

ifx_Device_Fmcw_t* ifx_fmcw_create()
{
    auto selector = [](const ifx_Radar_Sensor_List_Entry_t& entry) {
        return rdk::RadarDeviceCommon::sensor_is_avian(entry.sensor_type);
    };
    ifx_Device_Fmcw_t *device = rdk::RadarDeviceCommon::open_board_by_selector<DeviceFmcwAvian>(selector);

    return device;
}

//----------------------------------------------------------------------------

ifx_Device_Fmcw_t* ifx_fmcw_create_by_uuid(const char* uuid)
{
    using namespace rdk::RadarDeviceCommon;

    if (!uuid)
    {
        ifx_error_set(IFX_ERROR_ARGUMENT_NULL);
        return nullptr;
    }
    auto board = open_by_uuid(uuid);

    ifx_Radar_Sensor_t sensor_type;
    const auto found = get_sensor_type(board, sensor_type);
    if (!found)
    {
        return nullptr;
    }

    if (sensor_is_avian(sensor_type))
    {
        return open_board<DeviceFmcwAvian>(std::move(board));
    }

    return nullptr;
}

//----------------------------------------------------------------------------

// ifx_Device_Fmcw_t* ifx_fmcw_create_ish(void)
// {
//     return rdk::RadarDeviceCommon::open_device<DeviceFmcwAvian>();
// }

//----------------------------------------------------------------------------

void ifx_fmcw_destroy(ifx_Device_Fmcw_t* handle)
{
    delete handle;
}

//----------------------------------------------------------------------------

ifx_Radar_Sensor_t ifx_fmcw_get_sensor_type(const ifx_Device_Fmcw_t* handle)
{
    return rdk::call_func(handle, &ifx_Device_Fmcw_t::get_sensor_type);
}

//----------------------------------------------------------------------------

float ifx_fmcw_get_temperature(ifx_Device_Fmcw_t* handle)
{
    return (rdk::call_func(handle, &ifx_Device_Fmcw_t::get_temperature));
}

//----------------------------------------------------------------------------

float ifx_fmcw_get_element_duration(const ifx_Device_Fmcw_t* handle, const ifx_Fmcw_Sequence_Element_t* element)
{
    return (rdk::call_func(handle, &ifx_Device_Fmcw_t::get_element_duration, element));
}

//----------------------------------------------------------------------------

float ifx_fmcw_get_sequence_duration(const ifx_Device_Fmcw_t* handle, const ifx_Fmcw_Sequence_Element_t* sequence)
{
    return (rdk::call_func(handle, &ifx_Device_Fmcw_t::get_sequence_duration, sequence));
}

//----------------------------------------------------------------------------

void ifx_fmcw_stop_acquisition(ifx_Device_Fmcw_t* handle)
{
    rdk::call_func(handle, &ifx_Device_Fmcw_t::stop_acquisition);
}

//----------------------------------------------------------------------------

void ifx_fmcw_start_acquisition(ifx_Device_Fmcw_t* handle)
{
    rdk::call_func(handle, &ifx_Device_Fmcw_t::start_acquisition);
}

//----------------------------------------------------------------------------

ifx_Fmcw_Frame_t* ifx_fmcw_allocate_frame(ifx_Device_Fmcw_t* handle)
{
    return (rdk::call_func(handle, &ifx_Device_Fmcw_t::allocate_frame));
}

//----------------------------------------------------------------------------

ifx_Fmcw_Raw_Frame_t* ifx_fmcw_allocate_raw_frame(ifx_Device_Fmcw_t* handle)
{
    return (rdk::call_func(handle, &ifx_Device_Fmcw_t::allocate_raw_frame));
}

//----------------------------------------------------------------------------

void ifx_fmcw_get_next_frame(ifx_Device_Fmcw_t* handle, ifx_Fmcw_Frame_t* frame)
{
    ifx_fmcw_get_next_frame_timeout(handle, frame, 10000);
}

//----------------------------------------------------------------------------

void ifx_fmcw_get_next_frame_timeout(ifx_Device_Fmcw_t* handle, ifx_Fmcw_Frame_t* frame, uint16_t timeout_ms)
{
    rdk::call_func(handle, &ifx_Device_Fmcw_t::get_next_frame, frame, timeout_ms);
}

//----------------------------------------------------------------------------

void ifx_fmcw_get_next_raw_frame(ifx_Device_Fmcw_t* handle, ifx_Fmcw_Raw_Frame_t* frame)
{
    ifx_fmcw_get_next_raw_frame_timeout(handle, frame, 10000);
}

//----------------------------------------------------------------------------

void ifx_fmcw_get_next_raw_frame_timeout(ifx_Device_Fmcw_t* handle, ifx_Fmcw_Raw_Frame_t* frame, uint16_t timeout_ms)
{
    rdk::call_func(handle, &ifx_Device_Fmcw_t::get_next_raw_frame, frame, timeout_ms);
}

//----------------------------------------------------------------------------

void ifx_fmcw_destroy_frame(ifx_Fmcw_Frame_t* frame)
{
    rdk::call_func(&Fmcw::destroy_frame, frame);
}

//----------------------------------------------------------------------------

void ifx_fmcw_destroy_raw_frame(ifx_Fmcw_Raw_Frame_t* frame)
{
    rdk::call_func(&Fmcw::destroy_raw_frame, frame);
}

//----------------------------------------------------------------------------

void ifx_fmcw_set_acquisition_sequence(ifx_Device_Fmcw_t* handle, const ifx_Fmcw_Sequence_Element_t* sequence)
{
    rdk::call_func(handle, &ifx_Device_Fmcw_t::set_acquisition_sequence, sequence);
}

//----------------------------------------------------------------------------

ifx_Fmcw_Sequence_Element_t* ifx_fmcw_get_acquisition_sequence(ifx_Device_Fmcw_t* handle)
{
    return (rdk::call_func(handle, &ifx_Device_Fmcw_t::get_acquisition_sequence));
}

//----------------------------------------------------------------------------

void ifx_fmcw_save_register_file(ifx_Device_Fmcw_t* handle, const char* filename)
{
    return (rdk::call_func(handle, &ifx_Device_Fmcw_t::save_register_file, filename));
}

//----------------------------------------------------------------------------

void ifx_fmcw_load_register_file(ifx_Device_Fmcw_t* handle, const char* filename)
{
    rdk::call_func(handle, &ifx_Device_Fmcw_t::load_register_file, filename);
}

//----------------------------------------------------------------------------

float ifx_fmcw_get_minimum_chirp_repetition_time(const ifx_Device_Fmcw_t* handle, uint32_t num_samples, float sample_rate_Hz)
{
    return rdk::call_func(handle, &ifx_Device_Fmcw_t::get_minimum_chirp_repetition_time, num_samples, sample_rate_Hz);
}

//----------------------------------------------------------------------------

double ifx_fmcw_get_chirp_sampling_bandwidth(ifx_Device_Fmcw_t* handle, const ifx_Fmcw_Sequence_Chirp_t* chirp)
{
    return rdk::call_func(handle, &ifx_Device_Fmcw_t::get_chirp_sampling_bandwidth, chirp);
}

//----------------------------------------------------------------------------

double ifx_fmcw_get_chirp_sampling_center_frequency(ifx_Device_Fmcw_t* handle, const ifx_Fmcw_Sequence_Chirp_t* chirp)
{
    return rdk::call_func(handle, &ifx_Device_Fmcw_t::get_chirp_sampling_center_frequency, chirp);
}

//----------------------------------------------------------------------------

void ifx_fmcw_convert_raw_data_to_float_array(ifx_Device_Fmcw_t* handle, uint32_t num_samples, const uint16_t* raw_data, ifx_Float_t* converted_frame)
{
    return rdk::call_func(handle, &ifx_Device_Fmcw_t::convert_raw_data_to_float_array, num_samples, raw_data, converted_frame);
}

//----------------------------------------------------------------------------

void ifx_fmcw_view_deinterleaved_frame(ifx_Device_Fmcw_t* handle, ifx_Float_t* converted_frame, ifx_Fmcw_Frame_t* deinterleaved_frame_view)
{
    return rdk::call_func(handle, &ifx_Device_Fmcw_t::view_deinterleaved_frame, converted_frame, deinterleaved_frame_view);
}

//----------------------------------------------------------------------------

void ifx_fmcw_deinterleave_raw_frame(ifx_Device_Fmcw_t* handle, const ifx_Fmcw_Raw_Frame_t* raw_frame, ifx_Fmcw_Raw_Frame_t* deinterleaved_frame)
{
    return rdk::call_func(handle, &ifx_Device_Fmcw_t::deinterleave_raw_frame, raw_frame, deinterleaved_frame);
}
