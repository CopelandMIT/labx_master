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
 * @file DeviceMimoseConstants.h
 *
 * \brief \copybrief gr_device_mimose_constants
 *
 * For details refer to \ref gr_device_mimose_constants
 */

#pragma once

#include "DeviceMimoseTypes.h"

#include <iterator>


static const size_t NO_OF_FRAME_CONFIGURATIONS = std::size(((ifx_Mimose_Config_t*)nullptr)->frame_config);
static const size_t NO_OF_MODES = 3;
static const size_t NO_OF_PULSES = std::size(((ifx_Mimose_Config_t*)nullptr)->pulse_config);
static const uint32_t EXT_OSC_FREQ_HZ = 38'400'000u;
static const uint32_t INT_OSC_FREQ_HZ = 12'500'000u;
static constexpr uint16_t IQ_SAMPLES_MAX_READCOUNT_ALLOWED = 640;  // max number of samples allowed for multiple pulse configuration
static const uint16_t ATR22_AFC_VAL = 320;

constexpr uint16_t MIN_NUM_SAMPLES_ALLOWED_ONE_PULSE = 1;        // to evaluate risks, was 2 before
constexpr uint16_t MIN_NUM_SAMPLES_ALLOWED_MULTIPLE_PULSES = 1;  // to evaluate risks, was 32 before
constexpr uint16_t MAX_NUM_SAMPLES_ONE_PULSE = 256;
constexpr uint16_t MAX_NUM_SAMPLES_MULTIPLE_PULSES = 128;

// limits related
constexpr uint16_t IFX_MIMOSE_NUM_ACTIVE_PULSES_DEF = 1;

constexpr uint8_t IFX_MIMOSE_TX_POWER_LEVEL_MIN = 0;
constexpr uint8_t IFX_MIMOSE_TX_POWER_LEVEL_MAX = 63;
constexpr uint8_t IFX_MIMOSE_TX_POWER_LEVEL_DEF = 50;

constexpr uint16_t IFX_MIMOSE_NUM_SAMPLES_MIN = MIN_NUM_SAMPLES_ALLOWED_ONE_PULSE;
constexpr uint16_t IFX_MIMOSE_NUM_SAMPLES_MAX = MAX_NUM_SAMPLES_ONE_PULSE;
constexpr uint16_t IFX_MIMOSE_NUM_SAMPLES_DEF = 128;

constexpr float IFX_MIMOSE_PULSE_REPETITION_TIME_S_MIN = 130e-6f;   // single pulse limit (also global) in seconds
constexpr float IFX_MIMOSE_PULSE_REPETITION_TIME_S_MAX = 2000e-6f;  // Maximum register allowed value = 2047*(2e31)
constexpr float IFX_MIMOSE_PULSE_REPETITION_TIME_S_DEFAULT = 1000e-6f;

constexpr uint64_t IFX_MIMOSE_RF_CENTER_FREQ_HZ_MIN_BAND_100MHz = 24'150'000'000ull;
constexpr uint64_t IFX_MIMOSE_RF_CENTER_FREQ_HZ_MIN_BAND_200MHz = 24'050'000'000ull;
constexpr uint64_t IFX_MIMOSE_RF_CENTER_FREQ_HZ_MAX = 24'250'000'000ull;
constexpr uint64_t IFX_MIMOSE_RF_CENTER_FREQ_HZ_DEF = 24'240'0000'00ull;

constexpr uint16_t IFX_MIMOSE_AFC_DURATION_CT_MIN = 100;
constexpr uint16_t IFX_MIMOSE_AFC_DURATION_CT_MAX = 1000;
constexpr uint16_t IFX_MIMOSE_AFC_DURATION_CT_DEF = 115;

constexpr uint16_t IFX_MIMOSE_AFC_THRESHOLD_COURSE_MIN = 1;
constexpr uint16_t IFX_MIMOSE_AFC_THRESHOLD_COURSE_MAX = 5;
constexpr uint16_t IFX_MIMOSE_AFC_THRESHOLD_COURSE_DEF = 3;

constexpr uint16_t IFX_MIMOSE_AFC_THRESHOLD_FINE_MIN = 5;
constexpr uint16_t IFX_MIMOSE_AFC_THRESHOLD_FINE_MAX = 50;
constexpr uint16_t IFX_MIMOSE_AFC_THRESHOLD_FINE_DEF = 10;

constexpr uint8_t IFX_MIMOSE_AFC_PERIOD_MIN = 0;
constexpr uint8_t IFX_MIMOSE_AFC_PERIOD_MAX = 255;
constexpr uint8_t IFX_MIMOSE_AFC_PERIOD_DEF = 16;

constexpr ifx_Mimose_ABB_type_t IFX_MIMOSE_ABB_TYPE_DEF = IFX_MIMOSE_ABB_GAIN_384;
constexpr ifx_Mimose_AOC_Mode_t IFX_MIMOSE_AOC_MODE_DEF = IFX_MIMOSE_AOC_OFFSET_VALUE_IS_FULLY_HANDLED_BY_THE_FSM_2;

constexpr float IFX_MIMOSE_FRAME_REPETITION_TIME_S_MIN = 10e-3f;
constexpr float IFX_MIMOSE_FRAME_REPETITION_TIME_S_MAX = 2000e-3f;
constexpr float IFX_MIMOSE_FRAME_REPETITION_TIME_S_DEFAULT = 256e-3f;

constexpr uint32_t IFX_MIMOSE_REF_CLK_HZ_DEFAULT = 38400000;
constexpr uint32_t IFX_MIMOSE_HF_ONTIME_US_DEFAULT = 1;

constexpr uint16_t IFX_MIMOSE_RC_T_AFC_CYCLES = 2;
constexpr float IFX_MIMOSE_RC_T_BOOT_VCO_FS_TIME_S = 1e-4f;  // 100us
constexpr float IFX_MIMOSE_RC_T_BOOT_REF_CLK_CORRECTION = 25e-5f;
