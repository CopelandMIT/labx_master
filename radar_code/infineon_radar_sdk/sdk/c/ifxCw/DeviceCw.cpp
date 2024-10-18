/* ===========================================================================
** Copyright (C) 2021 Infineon Technologies AG
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

#include "ifxBase/Error.h"
#include "ifxBase/FunctionWrapper.hpp"
#include "ifxBase/internal/List.hpp"
#include "ifxBase/List.h"
#include "ifxBase/Types.h"
#include "ifxBase/Vector.h"

#include "DeviceCw.h"

#include "ifxAvian_CwController.hpp"
#include "ifxAvian_Utilities.hpp"
#include <platform/exception/EConnection.hpp>
#include <stdexcept>

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

#include "avian/DeviceCwAvian.hpp"
#include "DeviceCw.h"
#include <ifxAvian_CwController.hpp>

ifx_Device_Cw_t* ifx_cw_create()
{
    using namespace rdk::RadarDeviceCommon;

    auto selector = [](const ifx_Radar_Sensor_List_Entry_t& entry) {
        return sensor_is_avian(entry.sensor_type);
    };

    ifx_Device_Cw_t* device = open_board_by_selector<DeviceCwAvian>(selector);

    return device;
}

ifx_Device_Cw_t* ifx_cw_create_by_uuid(const char* uuid)
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
        return open_board<DeviceCwAvian>(std::move(board));
    }

    return nullptr;
}

ifx_Device_Cw_t* ifx_cw_create_dummy(ifx_Radar_Sensor_t sensor_type)
{
    if (rdk::RadarDeviceCommon::sensor_is_avian(sensor_type))
    {
        return rdk::RadarDeviceCommon::open_device<DeviceCwAvian>(sensor_type);
    }
    else
    {
        return nullptr;
    }
}

ifx_Device_Cw_t* ifx_cw_create_dummy_from_device(const ifx_Device_Cw_t* handle)
{
    const auto* avian_instance = dynamic_cast<const DeviceCwAvian*>(handle);
    if (avian_instance)
    {
        return rdk::RadarDeviceCommon::open_device<DeviceCwAvian>(*avian_instance);
    }

    return nullptr;
}

void ifx_cw_destroy(ifx_Device_Cw_t* handle)
{
    delete handle;
}

ifx_List_t* ifx_cw_get_list()
{
    auto selector = [](const ifx_Radar_Sensor_List_Entry_t& entry) {
        return rdk::RadarDeviceCommon::sensor_is_avian(entry.sensor_type);
    };

    auto list = rdk::RadarDeviceCommon::get_list(selector);
    return ifx_list_from_vector(list);
}

const ifx_Firmware_Info_t* ifx_cw_get_firmware_information(const ifx_Device_Cw_t* handle)
{
    return rdk::call_func(handle, &ifx_Device_Cw_t::get_firmware_info);
}

bool ifx_cw_is_signal_active(ifx_Device_Cw_t* handle)
{
    return rdk::call_func(handle, &ifx_Device_Cw_t::is_signal_active);
}

void ifx_cw_start_signal(ifx_Device_Cw_t* handle)
{
    return rdk::call_func(handle, &ifx_Device_Cw_t::start_signal);
}

void ifx_cw_stop_signal(ifx_Device_Cw_t* handle)
{
    return rdk::call_func(handle, &ifx_Device_Cw_t::stop_signal);
}

const ifx_Radar_Sensor_Info_t* ifx_cw_get_sensor_information(const ifx_Device_Cw_t* handle)
{
    return rdk::call_func(handle, &ifx_Device_Cw_t::get_sensor_info);
}

void ifx_cw_set_baseband_config(ifx_Device_Cw_t* handle, const ifx_Cw_Baseband_Config_t* config)
{
    rdk::call_func(handle, &ifx_Device_Cw_t::set_baseband_config, config);
}

const ifx_Cw_Baseband_Config_t* ifx_cw_get_baseband_config(ifx_Device_Cw_t* handle)
{
    return rdk::call_func(handle, &ifx_Device_Cw_t::get_baseband_config);
}

void ifx_cw_set_adc_config(ifx_Device_Cw_t* handle, const ifx_Cw_Adc_Config_t* config)
{
    rdk::call_func(handle, &ifx_Device_Cw_t::set_adc_config, config);
}

const ifx_Cw_Adc_Config_t* ifx_cw_get_adc_config(ifx_Device_Cw_t* handle)
{
    return rdk::call_func(handle, &ifx_Device_Cw_t::get_adc_config);
}

void ifx_cw_set_test_signal_generator_config(ifx_Device_Cw_t* handle, const ifx_Cw_Test_Signal_Generator_Config_t* config)
{
    rdk::call_func(handle, &ifx_Device_Cw_t::set_test_signal_generator_config, config);
}

const ifx_Cw_Test_Signal_Generator_Config_t* ifx_cw_get_test_signal_generator_config(ifx_Device_Cw_t* handle)
{
    return rdk::call_func(handle, &ifx_Device_Cw_t::get_test_signal_generator_config);
}

float ifx_cw_measure_temperature(ifx_Device_Cw_t* handle)
{
    return rdk::call_func(handle, &ifx_Device_Cw_t::measure_temperature);
}

float ifx_cw_measure_tx_power(ifx_Device_Cw_t* handle, const uint32_t antenna)
{
    return rdk::call_func(handle, &ifx_Device_Cw_t::measure_tx_power, antenna);
}

ifx_Matrix_R_t* ifx_cw_capture_frame(ifx_Device_Cw_t* handle, ifx_Matrix_R_t* frame)
{
    return rdk::call_func(handle, &ifx_Device_Cw_t::capture_frame, frame);
}

ifx_Radar_Sensor_t ifx_cw_get_sensor_type(const ifx_Device_Cw_t* handle)
{
    return rdk::call_func(handle, &ifx_Device_Cw_t::get_sensor_type);
}

void ifx_cw_save_register_file(ifx_Device_Cw_t* handle, const char* filename)
{
    rdk::call_func(handle, &ifx_Device_Cw_t::save_register_file, filename);
}

void ifx_cw_load_register_file(ifx_Device_Cw_t* handle, const char* filename)
{
    rdk::call_func(handle, &ifx_Device_Cw_t::load_register_file, filename);
}
