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
 * @file DeviceFmcwAvian.hpp
 *
 * @brief Defines the structure for the Radar Device Controller Module.
 */

#pragma once

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include "../DeviceFmcwBase.hpp"

// Strata
#include <platform/BoardInstance.hpp>

// libAvian
#include <ifxAvian_Driver.hpp>
#include <ifxAvian_IPort.hpp>
#include <ifxAvian_RegisterSet.hpp>
#include <ifxAvian_TimingModel.hpp>
#include <ifxAvian_Types.hpp>

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

struct DeviceFmcwAvian : public DeviceFmcwBase
{
    DeviceFmcwAvian(std::unique_ptr<BoardInstance>&& board);
    IFX_DLL_TEST DeviceFmcwAvian(ifx_Radar_Sensor_t device_type, float reference_clock = 80e6f);
    DeviceFmcwAvian(const DeviceFmcwAvian& other);

    DeviceFmcwAvian(DeviceFmcwAvian&& device) = delete;
    DeviceFmcwAvian& operator=(const DeviceFmcwAvian& deviceFmcwAvian) = delete;
    DeviceFmcwAvian& operator=(const DeviceFmcwAvian&& deviceFmcwAvian) = delete;

    IFX_DLL_TEST ~DeviceFmcwAvian() override;

    void initialize_sensor_info() override;

    ifx_Radar_Sensor_t get_sensor_type() const override;

    float get_minimum_chirp_repetition_time(uint32_t num_samples, float sample_rate_Hz) const override;
    double get_chirp_sampling_range(const ifx_Fmcw_Sequence_Chirp_t* chirp) const override;

    void stop_acquisition() override;
    void start_acquisition() override;

    IFX_DLL_TEST void set_acquisition_sequence(const ifx_Fmcw_Sequence_Element_t* sequence) override;
    IFX_DLL_TEST ifx_Fmcw_Sequence_Element_t* get_acquisition_sequence() const override;

    float get_temperature() override;

    IFX_DLL_TEST std::map<uint16_t, uint32_t>& get_register_list() override;
    void apply_register_list(const std::map<uint16_t, uint32_t>& register_list) override;

    std::map<uint16_t, uint32_t> import_register_list(const char* filename) override;
    void export_register_list(const char* filename, const std::map<uint16_t, uint32_t>& register_list) override;

    IFX_DLL_TEST void load_register_file(const char* filename) override;
    IFX_DLL_TEST void save_register_file(const char* filename) override;


    IFX_DLL_PUBLIC uint32_t export_register_list_legacy(bool set_trigger_bit, uint32_t* register_list);

    IFX_DLL_PUBLIC std::unique_ptr<Infineon::Avian::TimingModel::StateSequence> create_timing_model() const;

protected:
    float get_chirp_duration(const ifx_Fmcw_Sequence_Chirp_t& chirp) const override;

private:
    void set_reference_clock(float reference_clock);
    void detect_reference_clock();

    void generate_register_list();

    std::unique_ptr<Infineon::Avian::HW::IControlPort> m_port;
    std::unique_ptr<Infineon::Avian::Driver> m_driver;
    std::atomic<bool> m_data_started = false;
    std::chrono::steady_clock::time_point m_temperature_expiration_time = {};  // timestamp until the cached temperature value is valid
    float m_temperature_value = 0;                                             // cached temperature value in degrees Celsius

    std::vector<int8_t> m_if_gain_list;
    std::map<uint16_t, uint32_t> m_register_map;
};
