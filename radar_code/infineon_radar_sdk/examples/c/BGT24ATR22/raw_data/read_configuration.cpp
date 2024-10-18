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

#include "frame_configuration.h"
#include "ifxBase/Log.h"
#include "nlohmann/json.hpp"
#include <algorithm>
#include <cstdint>
#include <fstream>


bool read_configuration(const char* file_name, frame_configuration_t* configuration)
{
    bool res = false;

    if (!file_name || !configuration)
    {
        return res;
    }

    std::ifstream ifs;
    ifs.open(file_name, std::ios_base::in);
    res = (ifs.good() && ifs.is_open());

    if (!res)
    {
        return res;
    }

    try
    {
        nlohmann::json json = nlohmann::json::parse(ifs);

        const auto frame_config_obj = json["frame_config"];
        const auto numActivePulses = frame_config_obj["num_active_pulses"].get<uint16_t>();
        const auto numSamples = frame_config_obj["num_samples"].get<uint16_t>();
        const auto frameRepTime = frame_config_obj["frame_rep_time"].get<float>();
        const auto pulseRepTime = frame_config_obj["pulse_rep_time"].get<float>();

        res = ((numActivePulses > 0)
               && (numSamples > 0)
               && (frameRepTime > 0.0f)
               && (pulseRepTime > 0.0f));

        if (res)
        {
            std::fill_n(configuration->pulse_configs, sizeof(configuration->pulse_configs), false);
            std::fill_n(configuration->pulse_configs, numActivePulses, true);
            configuration->num_samples = numSamples;
            configuration->frame_repetition_time_s = frameRepTime;
            configuration->pulse_repetition_time_s = pulseRepTime;
        }
    }
    catch (nlohmann::json::parse_error&)
    {
        IFX_LOG_ERROR("Could not parse input configuration file, could not recognize format.");
    }
    catch (std::out_of_range&)
    {
        IFX_LOG_ERROR("Could not parse input configuration file, could not recognize attribute(s).");
    }

    return res;
}
