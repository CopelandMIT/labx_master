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
 * @file DeviceCwAvian.hpp
 *
 * @brief Defines the structure for the Radar Device Module.
 */

#pragma once

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include "../DeviceCwBase.hpp"
#include "../DeviceCwTypes.h"
#include "ifxAvian_CwController.hpp"
#include "ifxAvian_Utilities.hpp"
#include "ifxBase/Matrix.h"

// Strata
#include <platform/BoardInstance.hpp>


// libAvian

#include <atomic>
#include <chrono>
#include <memory>

/*
==============================================================================
   1. FORWARD DECLARATIONS
==============================================================================
*/

/*
==============================================================================
   2. DEFINITIONS
==============================================================================
*/

/*
==============================================================================
   3. TYPES
==============================================================================
*/

/**
 * @brief A handle for an instance of DeviceControl module, see DeviceControl.h.
 */

/*
==============================================================================
   4. FUNCTION PROTOTYPES
==============================================================================
*/

struct DeviceCwAvian : public DeviceCwBase
{
public:
    DeviceCwAvian(std::unique_ptr<BoardInstance>&& board);
    DeviceCwAvian(ifx_Radar_Sensor_t device_type);
    DeviceCwAvian(const DeviceCwAvian& deviceCwAvian);

    DeviceCwAvian(DeviceCwAvian&& device) = delete;
    DeviceCwAvian& operator=(const DeviceCwAvian& deviceCwAvian) = delete;
    DeviceCwAvian& operator=(const DeviceCwAvian&& deviceCwAvian) = delete;

    ~DeviceCwAvian() override;

    void initialize_sensor_info() override;

    bool is_signal_active() override;
    void start_signal() override;
    void stop_signal() override;

    void set_baseband_config(const ifx_Cw_Baseband_Config_t* config) override;
    const ifx_Cw_Baseband_Config_t* get_baseband_config() override;

    void set_adc_config(const ifx_Cw_Adc_Config_t* config) override;
    const ifx_Cw_Adc_Config_t* get_adc_config() override;

    void set_test_signal_generator_config(const ifx_Cw_Test_Signal_Generator_Config_t* config) override;
    const ifx_Cw_Test_Signal_Generator_Config_t* get_test_signal_generator_config() override;

    float measure_temperature() override;
    float measure_tx_power(uint32_t antenna) override;

    ifx_Matrix_R_t* capture_frame(ifx_Matrix_R_t* frame) override;

    ifx_Radar_Sensor_t get_sensor_type() const override;

    std::map<uint16_t, uint32_t>& get_register_list() override;
    void apply_register_list(const std::map<uint16_t, uint32_t>& register_list) override;

    std::map<uint16_t, uint32_t> import_register_list(const char* filename) override;
    void export_register_list(const char* filename, const std::map<uint16_t, uint32_t>& register_list) override;

    void load_register_file(const char* filename) override;
    void save_register_file(const char* filename) override;

    /*----------------------------------------------------------------------------*/
    std::unique_ptr<Infineon::Avian::HW::IControlPort> m_avian_port;
    std::unique_ptr<Infineon::Avian::Continuous_Wave_Controller> m_cw_controller;

    std::vector<int8_t> m_if_gain_list;
    Infineon::Avian::Device_Type m_device_type;
    ifx_Cw_Baseband_Config_t m_baseband_config;
    ifx_Cw_Adc_Config_t m_adc_config;
    ifx_Cw_Test_Signal_Generator_Config_t m_test_signal_config;

private:
    uint32_t get_tx_antenna_mask() const;
    uint32_t get_rx_antenna_mask() const;
    uint32_t get_tx_antenna_enabled_count() const;
    uint32_t get_rx_antenna_enabled_count() const;

    void generate_register_list();

    std::map<uint16_t, uint32_t> m_register_map;
};
