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

#ifndef APP_COMMON_JSON_HPP
#define APP_COMMON_JSON_HPP


/*
==============================================================================
    1. INCLUDE FILES
==============================================================================
*/

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "ifxAvian/Avian.h"
#include "ifxRadarPresenceSensing/PresenceSensing.h"
#include "ifxRadarSegmentation/RadarSegmentation.h"

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

enum class LowMedHigh
{
    low = 0,
    medium = 1,
    high = 2
};

class ifxJsonConfiguration
{
private:
    nlohmann::ordered_json m_json;
    const nlohmann::ordered_json* m_active = nullptr;
    std::string m_path;

    void set_active_json_object(const std::vector<std::string>& path);
    const nlohmann::ordered_json& get_json_object(const std::string& name) const;

    bool get_bool(const std::string& name) const;
    bool get_bool(const std::string& name, bool default_value) const;

    std::string get_string(const std::string& name) const;
    std::string get_string(const std::string& name, const std::string& default_value) const;

    float get_number(const std::string& name) const;
    float get_number(const std::string& name, float default_value) const;
    float get_number_bounds(const std::string& name, float min, float max) const;

    float get_positive_number(const std::string& name) const;
    float get_positive_number(const std::string& name, float default_value) const;

    uint32_t get_uint32(const std::string& name) const;
    uint32_t get_uint32(const std::string& name, uint32_t default_value) const;
    uint32_t get_uint32_bounds(const std::string& name, uint32_t min, uint32_t max) const;

    uint64_t get_uint64(const std::string& name) const;
    uint64_t get_uint64(const std::string& name, uint64_t default_value) const;
    uint64_t get_uint64_bounds(const std::string& name, uint64_t min, uint64_t max) const;

    uint32_t get_antenna_mask(const std::string& name) const;
    LowMedHigh get_low_med_high(const std::string& name) const;
    ifx_Vector_R_t* get_vector_r(const std::string& name) const;
    uint32_t get_sample_rate() const;

    enum ifx_Avian_MIMO_Mode get_mimo_mode() const;

public:
    ifxJsonConfiguration() = default;

    void load_from_file(const std::string& filename, int type = 0, const char* uuid = "");
    void save_to_file(const std::string& filename);

    bool has_device() const;
    std::vector<std::string> get_device_uuids() const;

    bool has_config_fmcw_scene() const;
    void get_config_fmcw_scene(ifx_Avian_Metrics_t* config_scene);
    void set_config_fmcw_scene(const ifx_Avian_Metrics_t* config);

    bool has_config_fmcw_single_shape() const;
    void get_config_fmcw_single_shape(ifx_Avian_Config_t* config_single_shape);
    void set_config_fmcw_single_shape(const ifx_Avian_Config_t* config);

    bool has_config_segmentation() const;
    void get_config_segmentation(const ifx_Avian_Config_t* device_config, ifx_Segmentation_Config_t* config_segmentation);
    void set_config_segmentation(const ifx_Segmentation_Config_t* config);

    bool has_config_presence_sensing() const;
    void get_config_presence_sensing(ifx_Presence_Sensing_Config_t* config);
    void set_config_presence_sensing(const ifx_Presence_Sensing_Config_t* config);
};


/*
==============================================================================
    4. FUNCTION PROTOTYPES
==============================================================================
*/


#endif  // #ifndef APP_COMMON_JSON_HPP
