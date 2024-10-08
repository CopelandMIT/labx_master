/**
 * \file ifxAvian_DeviceTraits.cpp
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

// ---------------------------------------------------------------------------- includes
#include "ifxAvian_DeviceTraits.hpp"
#include "Driver/registers_BGT120TR24E.h"
#include "Driver/registers_BGT60TR11D.h"
#include "Driver/registers_BGT60TRxxC.h"
#include "Driver/registers_BGT60TRxxD.h"
#include "Driver/registers_BGT60TRxxE.h"

#include <stdexcept>

// ---------------------------------------------------------------------------- namespaces
namespace Infineon {
namespace Avian {

// ---------------------------------------------------------------------------- get
const Device_Traits& Device_Traits::get(Device_Type device_type)
{
    switch (device_type)
    {
        case Device_Type::BGT60TR13C:
            {
                static const Device_Traits s_Traits =
                    {
                        /*.description = */ "BGT60TR13C FMCW Radar Sensor",
                        /*.allowed_rf_range = */ {58000000, 63500000},
                        /*.num_tx_antennas = */ 1,
                        /*.num_rx_antennas = */ 3,
                        /*.num_registers = */ BGT60TRxxC_NUM_REGISTERS,
                        /*.cs_register_layout = */ Channel_Set_Layout::Version1,
                        /*.has_extra_startup_delays = */ false,
                        /*.has_ref_frequency_doubler = */ false,
                        /*.pll_pre_divider = */ 8,
                        /*.pll_div_set_80M = */ 20,
                        /*.pll_div_set_76M8 = */ 21,
                        /*.has_sadc = */ true,
                        /*.has_explicit_sadc_bg_div_control = */ false,
                        /*.wu_register_type = */ Wu_Register_Type::None,
                        /*.supports_tx_toggling = */ false,
                        /*.has_programmable_fifo_power_mode = */ false,
                        /*.has_programmable_pad_driver = */ false,
                        /*.has_programmable_pullup_resistors = */ false,
                        /*.has_device_id = */ false,
                        /*.has_reordered_register_layout = */ false,
                        /*.has_internal_oscillator = */ false,
                        /*.has_local_oscillator_frequency_doubler*/ false,
                        /*.fifo_size = */ 8192,
                        /*.sadc_power_channels = */ {{{'\x04', '\x03'}, {'\0', '\0'}}},
                        /*.hpf_cutoff_settings = */ {20000, 45000, 70000, 80000, -1, -1, -1},
                        /*.aaf_cutoff_settings = */ {500000, -1, -1, -1, -1},
                        /*.hp_gain_settings = */ {18, 30, -1},
                        /*.vga_gain_settings = */ {0, 5, 10, 15, 20, 25, 30, -1}};
                return s_Traits;
            }

        case Device_Type::BGT60ATR24C:
            {
                static const Device_Traits s_Traits =
                    {
                        /*.description = */ "BGT60ATR24C FMCW Radar Sensor",
                        /*.allowed_rf_range = */ {58000000, 63500000},
                        /*.num_tx_antennas = */ 2,
                        /*.num_rx_antennas = */ 4,
                        /*.num_registers = */ 0x62u,
                        /*.cs_register_layout = */ Channel_Set_Layout::Version1,
                        /*.has_extra_startup_delays = */ false,
                        /*.has_ref_frequency_doubler = */ false,
                        /*.pll_pre_divider = */ 8,
                        /*.pll_div_set_80M = */ 20,
                        /*.pll_div_set_76M8 = */ 21,
                        /*.has_sadc = */ true,
                        /*.has_explicit_sadc_bg_div_control = */ true,
                        /*.wu_register_type = */ Wu_Register_Type::None,
                        /*.supports_tx_toggling = */ false,
                        /*.has_programmable_fifo_power_mode = */ false,
                        /*.has_programmable_pad_driver = */ false,
                        /*.has_programmable_pullup_resistors = */ false,
                        /*.has_device_id = */ true,
                        /*.has_reordered_register_layout = */ false,
                        /*.has_internal_oscillator = */ false,
                        /*.has_local_oscillator_frequency_doubler*/ false,
                        /*.fifo_size = */ 8192,
                        /*.sadc_power_channels = */ {{{'\x0C', '\x0B'}, {'\x0E', '\x0D'}}},
                        /*.hpf_cutoff_settings = */ {20000, 45000, 70000, 80000, -1, -1, -1},
                        /*.aaf_cutoff_settings = */ {500000, -1, -1, -1, -1},
                        /*.hp_gain_settings = */ {18, 30, -1},
                        /*.vga_gain_settings = */ {0, 5, 10, 15, 20, 25, 30, -1}};
                return s_Traits;
            }

        case Device_Type::BGT60UTR13D:
            {
                static const Device_Traits s_Traits =
                    {
                        /*.description = */ "BGT60UTR13DAIP FMCW Radar Sensor",
                        /*.allowed_rf_range = */ {57800000, 63500000},
                        /*.num_tx_antennas = */ 1,
                        /*.num_rx_antennas = */ 3,
                        /*.num_registers = */ BGT60TRxxD_NUM_REGISTERS,
                        /*.cs_register_layout = */ Channel_Set_Layout::Version2,
                        /*.has_extra_startup_delays = */ true,
                        /*.has_ref_frequency_doubler = */ true,
                        /*.pll_pre_divider = */ 8,
                        /*.pll_div_set_80M = */ 20,
                        /*.pll_div_set_76M8 = */ 21,
                        /*.has_sadc = */ true,
                        /*.has_explicit_sadc_bg_div_control = */ true,
                        /*.wu_register_type = */ Wu_Register_Type::None,
                        /*.supports_tx_toggling = */ true,
                        /*.has_programmable_fifo_power_mode = */ true,
                        /*.has_programmable_pad_driver = */ true,
                        /*.has_programmable_pullup_resistors = */ false,
                        /*.has_device_id = */ true,
                        /*.has_reordered_register_layout = */ false,
                        /*.has_internal_oscillator = */ false,
                        /*.has_local_oscillator_frequency_doubler*/ false,
                        /*.fifo_size = */ 8192,
                        /*.sadc_power_channels = */ {{{'\x04', '\x03'}, {'\0', '\0'}}},
                        /*.hpf_cutoff_settings = */ {20000, 40000, 80000, 140000, 160000, -1, -1},
                        /*.aaf_cutoff_settings = */ {600000, 1000000, -1, -1, -1},
                        /*.hp_gain_settings = */ {18, 30, -1},
                        /*.vga_gain_settings = */ {0, 5, 10, 15, 20, 25, 30, -1}};
                return s_Traits;
            }

        case Device_Type::BGT60TR12E:
            {
                static const Device_Traits s_Traits =
                    {
                        /*.description = */ "BGT60TR12E FMCW Radar Sensor",
                        /*.allowed_rf_range = */ {58000000, 63500000},
                        /*.num_tx_antennas = */ 1,
                        /*.num_rx_antennas = */ 2,
                        /*.num_registers = */ BGT60TRxxE_NUM_REGISTERS,
                        /*.cs_register_layout = */ Channel_Set_Layout::Version2,
                        /*.has_extra_startup_delays = */ true,
                        /*.has_ref_frequency_doubler = */ true,
                        /*.pll_pre_divider = */ 8,
                        /*.pll_div_set_80M = */ 20,
                        /*.pll_div_set_76M8 = */ 21,
                        /*.has_sadc = */ true,
                        /*.has_explicit_sadc_bg_div_control = */ true,
                        /*.wu_register_type = */ Wu_Register_Type::T_WU_Only,
                        /*.supports_tx_toggling = */ true,
                        /*.has_programmable_fifo_power_mode = */ true,
                        /*.has_programmable_pad_driver = */ true,
                        /*.has_programmable_pullup_resistors = */ true,
                        /*.has_device_id = */ true,
                        /*.has_reordered_register_layout = */ true,
                        /*.has_internal_oscillator = */ false,
                        /*.has_local_oscillator_frequency_doubler*/ false,
                        /*.fifo_size = */ 8192,
                        /*.sadc_power_channels = */ {{{'\x04', '\x03'}, {'\0', '\0'}}},
                        /*.hpf_cutoff_settings = */ {20000, 40000, 100000, 160000, 200000, -1, -1},
                        /*.aaf_cutoff_settings = */ {600000, 1000000, -1, -1, -1},
                        /*.hp_gain_settings = */ {18, 30, -1},
                        /*.vga_gain_settings = */ {0, 5, 10, 15, 20, 25, 30, -1}};
                return s_Traits;
            }

        case Device_Type::BGT60UTR11AIP:
            {
                static const Device_Traits s_Traits =
                    {
                        /*.description = */ "BGT60UTR11AIP FMCW Radar Sensor",
                        /*.allowed_rf_range = */ {57400000, 63000000},
                        /*.num_tx_antennas = */ 1,
                        /*.num_rx_antennas = */ 1,
                        /*.num_registers = */ BGT60TR11D_NUM_REGISTERS,
                        /*.cs_register_layout = */ Channel_Set_Layout::Version2,
                        /*.has_extra_startup_delays = */ true,
                        /*.has_ref_frequency_doubler = */ true,
                        /*.pll_pre_divider = */ 8,
                        /*.pll_div_set_80M = */ 20,
                        /*.pll_div_set_76M8 = */ 21,
                        /*.has_sadc = */ false,
                        /*.has_explicit_sadc_bg_div_control = */ true,
                        /*.wu_register_type = */ Wu_Register_Type::T_WU_Only,
                        /*.supports_tx_toggling = */ true,
                        /*.has_programmable_fifo_power_mode = */ true,
                        /*.has_programmable_pad_driver = */ true,
                        /*.has_programmable_pullup_resistors = */ true,
                        /*.has_device_id = */ true,
                        /*.has_reordered_register_layout = */ true,
                        /*.has_internal_oscillator = */ false,
                        /*.has_local_oscillator_frequency_doubler*/ false,
                        /*.fifo_size = */ 2048,
                        /*.sadc_power_channels = */ {{{'\0', '\0'}, {'\0', '\0'}}},
                        /*.hpf_cutoff_settings = */ {20000, 40000, 80000, 140000, 160000, -1, -1},
                        /*.aaf_cutoff_settings = */ {600000, 1000000, -1, -1, -1},
                        /*.hp_gain_settings = */ {18, 30, -1},
                        /*.vga_gain_settings = */ {0, 5, 10, 15, 20, 25, 30, -1}};
                return s_Traits;
            }

        case Device_Type::BGT120UTR13E:
            {
                static const Device_Traits s_Traits =
                    {
                        /*.description = */ "BGT120UTR13E FMCW Radar Sensor",
                        /*.allowed_rf_range = */ {112500000, 127500000},
                        /*.num_tx_antennas = */ 1,
                        /*.num_rx_antennas = */ 3,
                        /*.num_registers = */ BGT60TRxxE_NUM_REGISTERS,
                        /*.cs_register_layout = */ Channel_Set_Layout::Version2,
                        /*.has_extra_startup_delays = */ true,
                        /*.has_ref_frequency_doubler = */ true,
                        /*.pll_pre_divider = */ 16,
                        /*.pll_div_set_80M = */ 20,
                        /*.pll_div_set_76M8 = */ 21,
                        /*.has_sadc = */ true,
                        /*.has_explicit_sadc_bg_div_control = */ true,
                        /*.wu_register_type = */ Wu_Register_Type::T_WU_Only,
                        /*.supports_tx_toggling = */ true,
                        /*.has_programmable_fifo_power_mode = */ true,
                        /*.has_programmable_pad_driver = */ true,
                        /*.has_programmable_pullup_resistors = */ true,
                        /*.has_device_id = */ true,
                        /*.has_reordered_register_layout = */ true,
                        /*.has_internal_oscillator = */ false,
                        /*.has_local_oscillator_frequency_doubler*/ false,
                        /*.fifo_size = */ 8192,
                        /*.sadc_power_channels = */ {{{'\x04', '\x03'}, {'\0', '\0'}}},
                        /*.hpf_cutoff_settings = */ {20000, 40000, 100000, 160000, 200000, -1, -1},
                        /*.aaf_cutoff_settings = */ {600000, 1000000, -1, -1, -1},
                        /*.hp_gain_settings = */ {18, 30, -1},
                        /*.vga_gain_settings = */ {0, 5, 10, 15, 20, 25, 30, -1}};
                return s_Traits;
            }

        case Device_Type::BGT24LTR24:
            {
                static const Device_Traits s_Traits =
                    {
                        /*.description = */ "BGT24LTR24 FMCW Radar Sensor",
                        /*.allowed_rf_range = */ {24000000, 24250000},
                        /*.num_tx_antennas = */ 2,
                        /*.num_rx_antennas = */ 4,
                        /*.num_registers = */ 0x62u,
                        /*.cs_register_layout = */ Channel_Set_Layout::Version1,
                        /*.has_extra_startup_delays = */ false,
                        /*.has_ref_frequency_doubler = */ false,
                        /*.pll_pre_divider = */ 4,
                        /*.pll_div_set_80M = */ 15,
                        /*.pll_div_set_76M8 = */ 16,
                        /*.has_sadc = */ true,
                        /*.has_explicit_sadc_bg_div_control = */ true,
                        /*.wu_register_type = */ Wu_Register_Type::None,
                        /*.supports_tx_toggling = */ false,
                        /*.has_programmable_fifo_power_mode = */ false,
                        /*.has_programmable_pad_driver = */ false,
                        /*.has_programmable_pullup_resistors = */ false,
                        /*.has_device_id = */ true,
                        /*.has_reordered_register_layout = */ false,
                        /*.has_internal_oscillator = */ false,
                        /*.has_local_oscillator_frequency_doubler*/ false,
                        /*.fifo_size = */ 8192,
                        /*.sadc_power_channels = */ {{{'\x0C', '\x0B'}, {'\x0E', '\x0D'}}},
                        /*.hpf_cutoff_settings = */ {20000, 45000, 70000, 80000, -1, -1, -1},
                        /*.aaf_cutoff_settings = */ {500000, -1, -1, -1, -1},
                        /*.hp_gain_settings = */ {18, 30, -1},
                        /*.vga_gain_settings = */ {0, 5, 10, 15, 20, 25, 30, -1}};
                return s_Traits;
            }

        case Device_Type::BGT120UTR24:
            {
                static const Device_Traits s_Traits =
                    {
                        /*.description = */ "BGT120UTR24 FMCW Radar Sensor",
                        /*.allowed_rf_range = */ {112500000, 127500000},
                        /*.num_tx_antennas = */ 2,
                        /*.num_rx_antennas = */ 4,
                        /*.num_registers = */ BGT120TR24E_NUM_REGISTERS,
                        /*.cs_register_layout = */ Channel_Set_Layout::Version3,
                        /*.has_extra_startup_delays = */ true,
                        /*.has_ref_frequency_doubler = */ true,
                        /*.pll_pre_divider = */ 16,
                        /*.pll_div_set_80M = */ 20,
                        /*.pll_div_set_76M8 = */ 21,
                        /*.has_sadc = */ true,
                        /*.has_explicit_sadc_bg_div_control = */ true,
                        /*.wu_register_type = */ Wu_Register_Type::T_WU_and_Offset,
                        /*.supports_tx_toggling = */ true,
                        /*.has_programmable_fifo_power_mode = */ true,
                        /*.has_programmable_pad_driver = */ true,
                        /*.has_programmable_pullup_resistors = */ true,
                        /*.has_device_id = */ true,
                        /*.has_reordered_register_layout = */ true,
                        /*.has_internal_oscillator = */ true,
                        /*.has_local_oscillator_frequency_doubler*/ true,
                        /*.fifo_size = */ 8192,
                        /*.sadc_power_channels = */ {{{'\x04', '\x03'}, {'\x0E', '\x0D'}}},
                        /*.hpf_cutoff_settings = */ {20000, 40000, 100000, 160000, 200000, 800000, -1},
                        /*.aaf_cutoff_settings = */ {500000, 1000000, 1500000, 1800000, -1},
                        /*.hp_gain_settings = */ {18, 30, -1},
                        /*.vga_gain_settings = */ {0, 5, 10, 15, 20, 25, 30, -1}};
                return s_Traits;
            }

        case Device_Type::BGT60ATR24E:
            {
                static const Device_Traits s_Traits =
                    {
                        /*.description = */ "BGT60ATR24E FMCW Radar Sensor",
                        /*.allowed_rf_range = */ {58000000, 63500000},
                        /*.num_tx_antennas = */ 2,
                        /*.num_rx_antennas = */ 4,
                        /*.num_registers = */ BGT120TR24E_NUM_REGISTERS,
                        /*.cs_register_layout = */ Channel_Set_Layout::Version3,
                        /*.has_extra_startup_delays = */ true,
                        /*.has_ref_frequency_doubler = */ true,
                        /*.pll_pre_divider = */ 8,
                        /*.pll_div_set_80M = */ 20,
                        /*.pll_div_set_76M8 = */ 21,
                        /*.has_sadc = */ true,
                        /*.has_explicit_sadc_bg_div_control = */ true,
                        /*.wu_register_type = */ Wu_Register_Type::T_WU_and_Offset,
                        /*.supports_tx_toggling = */ true,
                        /*.has_programmable_fifo_power_mode = */ true,
                        /*.has_programmable_pad_driver = */ true,
                        /*.has_programmable_pullup_resistors = */ true,
                        /*.has_device_id = */ true,
                        /*.has_reordered_register_layout = */ true,
                        /*.has_internal_oscillator = */ true,
                        /*.has_local_oscillator_frequency_doubler*/ false,
                        /*.fifo_size = */ 8192,
                        /*.sadc_power_channels = */ {{{'\x04', '\x03'}, {'\x0E', '\x0D'}}},
                        /*.hpf_cutoff_settings = */ {20000, 40000, 100000, 160000, 200000, 800000, -1},
                        /*.aaf_cutoff_settings = */ {500000, 1000000, 1500000, 1800000, -1},
                        /*.hp_gain_settings = */ {18, 30, -1},
                        /*.vga_gain_settings = */ {0, 5, 10, 15, 20, 25, 30, -1}};
                return s_Traits;
            }
        case Device_Type::BGT24LTR13E:
            {
                static const Device_Traits s_Traits =
                    {
                        /*.description = */ "BGT24LTR13E FMCW Radar Sensor",
                        /*.allowed_rf_range = */ {24000000, 24250000},
                        /*.num_tx_antennas = */ 1,
                        /*.num_rx_antennas = */ 3,
                        /*.num_registers = */ BGT120TR24E_NUM_REGISTERS,
                        /*.cs_register_layout = */ Channel_Set_Layout::Version3,
                        /*.has_extra_startup_delays = */ true,
                        /*.has_ref_frequency_doubler = */ true,
                        /*.pll_pre_divider = */ 4,
                        /*.pll_div_set_80M = */ 15,
                        /*.pll_div_set_76M8 = */ 16,
                        /*.has_sadc = */ true,
                        /*.has_explicit_sadc_bg_div_control = */ true,
                        /*.wu_register_type = */ Wu_Register_Type::T_WU_and_Offset,
                        /*.supports_tx_toggling = */ true,
                        /*.has_programmable_fifo_power_mode = */ true,
                        /*.has_programmable_pad_driver = */ true,
                        /*.has_programmable_pullup_resistors = */ true,
                        /*.has_device_id = */ true,
                        /*.has_reordered_register_layout = */ true,
                        /*.has_internal_oscillator = */ true,
                        /*.has_local_oscillator_frequency_doubler*/ false,
                        /*.fifo_size = */ 8192,
                        /*.sadc_power_channels = */ {{{'\x04', '\x03'}, {'\x0E', '\x0D'}}},
                        /*.hpf_cutoff_settings = */ {20000, 40000, 100000, 160000, 200000, 800000, -1},
                        /*.aaf_cutoff_settings = */ {500000, 1000000, 1500000, 1800000, -1},
                        /*.hp_gain_settings = */ {18, 30, -1},
                        /*.vga_gain_settings = */ {0, 5, 10, 15, 20, 25, 30, -1}};
                return s_Traits;
            }

        default:
            {
                throw std::runtime_error("Unsupported device type.");
            }
    }
}

/* ------------------------------------------------------------------------ */
}  // namespace Avian
}  // namespace Infineon

/* --- End of File -------------------------------------------------------- */
