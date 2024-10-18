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
 * @file DeviceFmcwBase.hpp
 *
 * @brief Defines the structure for the Radar Device Controller Module.
 */

#pragma once

#include "ifxBase/internal/NonCopyable.hpp"
#include "ifxFmcw/DeviceFmcw.hpp"
#include "ifxRadarDeviceCommon/internal/RadarDeviceCommon.hpp"

#include <string>


struct DeviceFmcwBase : public DeviceFmcw
{
    NONCOPYABLE(DeviceFmcwBase);
    ~DeviceFmcwBase() override = default;

    const ifx_Firmware_Info_t* get_firmware_info() const override;
    const char* get_board_uuid() const override;
    const ifx_Radar_Sensor_Info_t* get_sensor_info() const override;
    ifx_Fmcw_Frame_t* allocate_frame() override;
    ifx_Fmcw_Raw_Frame_t* allocate_raw_frame() override;
    void get_next_frame(ifx_Fmcw_Frame_t* frame, uint16_t timeout_ms) override;
    void get_next_raw_frame(ifx_Fmcw_Raw_Frame_t* frame, uint16_t timeout_ms) override;

    void convert_raw_data_to_float_array(uint32_t num_samples, const uint16_t* raw_data, ifx_Float_t* converted_frame) override;
    void deinterleave_raw_frame(const ifx_Fmcw_Raw_Frame_t* raw_frame, ifx_Fmcw_Raw_Frame_t* deinterleaved_frame) override;
    void view_deinterleaved_frame(ifx_Float_t* converted_frame, ifx_Fmcw_Frame_t* deinterleaved_frame_view) override;
    float get_sequence_duration(const ifx_Fmcw_Sequence_Element_t* sequence) const override;
    IFX_DLL_TEST float get_element_duration(const ifx_Fmcw_Sequence_Element_t* element) const override;

    double get_chirp_sampling_center_frequency(const ifx_Fmcw_Sequence_Chirp_t* chirp) const override;

protected:
    DeviceFmcwBase(ifx_Float_t max_adc_value);
    DeviceFmcwBase(ifx_Float_t max_adc_value, std::unique_ptr<BoardInstance>&& board);

    virtual void initialize_sensor_info() = 0;

    uint16_t calculate_slice_size(uint32_t fifo_size) const;
    void configure_data(uint16_t slice_size, uint16_t readout_address, uint8_t data_format);
    void start_data();
    void stop_data();
    void update_frame_settings();
    void update_defaults_if_not_configured();
    void get_frame_dimensions();
    uint32_t get_buffer_length(uint32_t num_samples) const;
    uint32_t copy_slice_data(uint8_t data_format, const uint8_t* buffer, uint32_t buffer_length, uint16_t* output);

    double get_chirp_sampling_bandwidth(const ifx_Fmcw_Sequence_Chirp_t* chirp) const override;

    ifx_Float_t m_max_adc_value;
    ifx_Firmware_Info_t m_firmware_info;
    ifx_Radar_Sensor_Info_t m_sensor_info;
    std::unique_ptr<BoardInstance> m_board;

    IBridgeData* m_bridge_data;
    uint8_t m_data_index;
    uint8_t m_data_format;
    IData* m_data;

    uint32_t m_num_samples = 0;

private:
    float m_frame_repetition_time_s;
    std::vector<std::array<uint32_t, 3>> m_frame_dimensions;

    uint32_t m_frame_length;
    SmartIFrame m_slice;

    bool m_mimo;  // temporary helper to unblock simple use cases
};
