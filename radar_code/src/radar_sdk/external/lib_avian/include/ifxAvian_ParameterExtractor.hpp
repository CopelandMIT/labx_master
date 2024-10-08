/**
 * \file ifxAvian_ParameterExtractor.hpp
 */
/* ===========================================================================
** Copyright (C) 2016-2023 Infineon Technologies AG
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

#pragma once

// ---------------------------------------------------------------------------- includes
#include "ifxAvian_Types.hpp"
#include "value_conversion/ifxAvian_TimingConversion.hpp"
#include <cstdint>
#include <map>
#include <stdexcept>
#include <vector>

namespace Infineon {
namespace Avian {

struct Device_Traits;

}  // namespace Avian
}  // namespace Infineon

// ---------------------------------------------------------------------------- namespaces
namespace Infineon {
namespace Avian {

// ---------------------------------------------------------------------------- Parameter_Extractor
class Parameter_Extractor
{
public:
    class Error;

    Parameter_Extractor(const std::vector<uint32_t>& registers,
                        Device_Type device_type = Device_Type::Unknown);
    Parameter_Extractor(std::map<uint8_t, uint32_t> registers,
                        Device_Type device_type = Device_Type::Unknown);

    // Global parameters
    Device_Type get_device_type() const;
    Reference_Clock_Frequency get_reference_clock() const;
    uint16_t get_slice_size() const;
    Frame_Definition get_frame_definition() const;
    Adc_Configuration get_adc_configuration() const;
    Chirp_Timing get_chirp_timing() const;
    Startup_Timing get_startup_timing() const;
    Power_Down_Configuration get_idle_configuration() const;
    Power_Down_Configuration get_deep_sleep_configuration() const;

    // Global parameters introduced with 'generation D'
    Startup_Delays get_startup_delays() const;
    Duty_Cycle_Correction_Settings get_duty_cycle_correction() const;
    Fifo_Power_Mode get_fifo_power_mode() const;
    Pad_Driver_Mode get_pad_driver_mode() const;

    // Global parameters introduced with 'generation E'
    Pullup_Resistor_Configuration get_pullup_resistor_configuration() const;

    // Global parameters introduced with BGT60UTR11AIP
    uint32_t get_power_sens_delay() const;
    bool get_power_sens_enabled() const;
    bool get_temperature_sens_enabled() const;

    // Chirp parameters
    Fmcw_Configuration get_fmcw_configuration(uint8_t shape) const;
    Frame_Format get_frame_format(uint8_t shape, bool down) const;
    Tx_Mode get_tx_mode(uint8_t shape, bool down) const;
    Baseband_Configuration get_baseband_configuration(uint8_t shape,
                                                      bool down) const;
    uint32_t get_chirp_end_delay(uint8_t shape, bool down) const;

    // Chirp parameters introduced with 'generation D'
    Anti_Alias_Filter_Settings get_anti_alias_filter_settings(uint8_t shape,
                                                              bool down) const;

    // Methods to access the imported register set
    bool has_register(uint8_t address) const;
    uint32_t get_register_value(uint8_t address) const;

private:
    uint32_t get_register(uint8_t address, const char* function_name) const;
    void detect_type_and_clock();
    inline uint32_t get_clock_frequency() const;
    uint64_t to_100ps(uint32_t tr_x, uint32_t tr_x_mul) const;
    Power_Down_Configuration extract_power_down_config(uint32_t reg_CSCx,
                                                       uint32_t reg_CSx_0) const;
    void check_shape_enabled(uint8_t shape) const;
    bool is_second_chirp(uint8_t shape, bool down) const;
    uint32_t get_num_samples(uint8_t shape, bool down) const;

    std::map<uint8_t, uint32_t> m_registers;
    Device_Type m_device_type;
    Reference_Clock_Frequency m_reference_clock;
};

//--------------------------------------------------------------------------- RegisterImporter::Error
class Parameter_Extractor::Error : public std::runtime_error
{
public:
    using runtime_error::runtime_error;
};

/* ------------------------------------------------------------------------ */
}  // namespace Avian
}  // namespace Infineon

/* --- End of File -------------------------------------------------------- */
