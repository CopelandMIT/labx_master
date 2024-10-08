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

/**
 * @internal
 * @file DeviceFmcw.hpp
 *
 * @brief Defines the structure for the Radar Device Controller Module.
 */

#pragma once

#include "DeviceCwTypes.h"
#include "ifxBase/Matrix.h"
#include <map>
#include <vector>


struct DeviceCw
{
    virtual ~DeviceCw() = default;

    /* These abstract virtual members implemented in the base class */
    virtual const ifx_Firmware_Info_t* get_firmware_info() const = 0;
    virtual const ifx_Radar_Sensor_Info_t* get_sensor_info() const = 0;
    virtual ifx_Radar_Sensor_t get_sensor_type() const = 0;

    virtual void initialize_sensor_info() = 0;

    /* These abstract virtual members must be implemented by the base class derived class */
    virtual bool is_signal_active() = 0;

    virtual void start_signal() = 0;
    virtual void stop_signal() = 0;

    virtual void set_baseband_config(const ifx_Cw_Baseband_Config_t* config) = 0;
    virtual const ifx_Cw_Baseband_Config_t* get_baseband_config() = 0;

    virtual void set_adc_config(const ifx_Cw_Adc_Config_t* config) = 0;
    virtual const ifx_Cw_Adc_Config_t* get_adc_config() = 0;

    virtual void set_test_signal_generator_config(const ifx_Cw_Test_Signal_Generator_Config_t* config) = 0;
    virtual const ifx_Cw_Test_Signal_Generator_Config_t* get_test_signal_generator_config() = 0;

    virtual float measure_temperature() = 0;
    virtual float measure_tx_power(uint32_t antenna) = 0;

    virtual ifx_Matrix_R_t* capture_frame(ifx_Matrix_R_t* frame) = 0;

    virtual std::map<uint16_t, uint32_t>& get_register_list() = 0;
    virtual void apply_register_list(const std::map<uint16_t, uint32_t>& register_list) = 0;

    virtual std::map<uint16_t, uint32_t> import_register_list(const char* filename) = 0;
    virtual void export_register_list(const char* filename, const std::map<uint16_t, uint32_t>& register_list) = 0;

    virtual void load_register_file(const char* filename) = 0;
    virtual void save_register_file(const char* filename) = 0;
};
