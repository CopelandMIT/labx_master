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

#include "ifxFmcw/DeviceFmcwTypes.h"
#include <map>
#include <memory>
#include <vector>


struct DeviceFmcw
{
    /* These abstract virtual member functions are implemented in the base class */
    virtual ~DeviceFmcw() = default;
    virtual const char* get_board_uuid() const = 0;
    virtual const ifx_Firmware_Info_t* get_firmware_info() const = 0;
    virtual const ifx_Radar_Sensor_Info_t* get_sensor_info() const = 0;
    virtual ifx_Radar_Sensor_t get_sensor_type() const = 0;
    virtual void get_next_frame(ifx_Fmcw_Frame_t* frame, uint16_t timeout_ms) = 0;
    virtual void get_next_raw_frame(ifx_Fmcw_Raw_Frame_t* frame, uint16_t timeout_ms) = 0;
    virtual ifx_Fmcw_Frame_t* allocate_frame() = 0;
    virtual ifx_Fmcw_Raw_Frame_t* allocate_raw_frame() = 0;
    virtual void convert_raw_data_to_float_array(uint32_t num_samples, const uint16_t* raw_data, ifx_Float_t* converted_frame) = 0;
    virtual void deinterleave_raw_frame(const ifx_Fmcw_Raw_Frame_t* raw_frame, ifx_Fmcw_Raw_Frame_t* deinterleaved_frame) = 0;
    virtual void view_deinterleaved_frame(ifx_Float_t* converted_frame, ifx_Fmcw_Frame_t* deinterleaved_frame_view) = 0;
    virtual float get_element_duration(const ifx_Fmcw_Sequence_Element_t* element) const = 0;
    virtual float get_sequence_duration(const ifx_Fmcw_Sequence_Element_t* sequence) const = 0;

    /* These abstract virtual members must be implemented by the derived class */
    virtual float get_temperature() = 0;
    virtual float get_chirp_duration(const ifx_Fmcw_Sequence_Chirp_t& chirp) const = 0;
    virtual float get_minimum_chirp_repetition_time(uint32_t num_samples, float sample_rate_Hz) const = 0;
    virtual double get_chirp_sampling_range(const ifx_Fmcw_Sequence_Chirp_t* chirp) const = 0;
    virtual double get_chirp_sampling_bandwidth(const ifx_Fmcw_Sequence_Chirp_t* chirp) const = 0;
    virtual double get_chirp_sampling_center_frequency(const ifx_Fmcw_Sequence_Chirp_t* chirp) const = 0;

    virtual void stop_acquisition() = 0;
    virtual void start_acquisition() = 0;

    virtual void set_acquisition_sequence(const ifx_Fmcw_Sequence_Element_t* sequence) = 0;
    virtual ifx_Fmcw_Sequence_Element_t* get_acquisition_sequence() const = 0;

    virtual std::map<uint16_t, uint32_t>& get_register_list() = 0;
    virtual void apply_register_list(const std::map<uint16_t, uint32_t>& register_list) = 0;

    virtual std::map<uint16_t, uint32_t> import_register_list(const char* filename) = 0;
    virtual void export_register_list(const char* filename, const std::map<uint16_t, uint32_t>& register_list) = 0;

    virtual void load_register_file(const char* filename) = 0;
    virtual void save_register_file(const char* filename) = 0;
};


namespace Fmcw {

IFX_DLL_PUBLIC void destroy_frame(ifx_Fmcw_Frame_t* frame);
IFX_DLL_PUBLIC void destroy_raw_frame(ifx_Fmcw_Raw_Frame_t* frame);

}  // namespace Fmcw

struct ifx_Fmcw_Raw_Frame_t_Deleter
{
    void operator()(ifx_Fmcw_Raw_Frame_t* frame)
    {
        Fmcw::destroy_raw_frame(frame);
    }
};

struct ifx_Fmcw_Frame_t_Deleter
{
    void operator()(ifx_Fmcw_Frame_t* frame)
    {
        Fmcw::destroy_frame(frame);
    }
};


using SmartFmcwRawFrame = std::unique_ptr<ifx_Fmcw_Raw_Frame_t, ifx_Fmcw_Raw_Frame_t_Deleter>;
using SmartFmcwFrame = std::unique_ptr<ifx_Fmcw_Frame_t, ifx_Fmcw_Frame_t_Deleter>;
