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

/**
 * @file bgt60-configurator-cli.cpp
 *
 * @brief This is a small tool that allows to convert a device configuration
 *        in form of a json file into a register list.
 *
 */

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include <iostream>

#include <cstring>
#include <fstream>
#include <string>

#include "ifxAvian/Avian.h"
#include "ifxBase/Base.h"

#include "ifxRadarDeviceCommon/internal/RadarDeviceCommon.hpp"

#include "argparse.h"
#include "json.h"

static const char* const usage[] = {
    "bgt60-configurator-cli [options] [[--] args]",
    "bgt60-configurator-cli [options]",
    nullptr,
};

int main(int argc, char* argv[])
{
    int display_version = 0;
    const char* config_path = nullptr;
    const char* output_path = nullptr;
    const char* device_str = nullptr;

    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_GROUP("Options"),
        OPT_BOOLEAN('v', "version", &display_version, "Displays version information.", nullptr, 0, 0),
        OPT_STRING('d', "device", &device_str, "Selects device [BGT60TR13C, BGT60ATR24C, BGT60UTR13D, BGT60UTR11AIP]", nullptr, 0, 0),
        OPT_STRING('c', "config", &config_path, "Path to the radar sensor configuration file.", nullptr, 0, 0),
        OPT_STRING('o', "output", &output_path, "Path to the generated C code file.", nullptr, 0, 0),
        OPT_END(),
    };

    struct argparse argparse;
    argparse_init(&argparse, options, usage, 0);
    argparse_describe(&argparse, "\nProvides the command-line interface for generating the BGT60TRxx driver configurator output C files.", "\n");
    argc = argparse_parse(&argparse, argc, argv);

    if (display_version != 0)
    {
        std::cout << "BGT60TRxx Configurator, SDK version: " << ifx_sdk_get_version_string_full() << std::endl;
        return EXIT_SUCCESS;
    }

    // this needs refactoring, see HMI-3735
    ifx_Radar_Sensor_t sensor_type;
    if (device_str == nullptr)
    {
        sensor_type = IFX_AVIAN_BGT60TR13C;
    }
    else
    {
        sensor_type = rdk::RadarDeviceCommon::string_to_sensor(device_str);

        if (sensor_type == IFX_AVIAN_UNKNOWN || !rdk::RadarDeviceCommon::sensor_is_avian(sensor_type))
        {
            std::cout << "Error: Unknown sensor or sensor not supported." << std::endl;
            return EXIT_FAILURE;
        }
    }

    ifx_Avian_Device_t* device = ifx_avian_create_dummy(sensor_type);
    if (!device)
    {
        std::cout << "Error: Cannot create dummy device: " << ifx_error_to_string(ifx_error_get()) << std::endl;
        return EXIT_FAILURE;
    }

    if (config_path == nullptr)
    {
        std::cout << "Error: Configuration file path is not set. Use the --config argument to set the configuration file." << std::endl;
        return EXIT_FAILURE;
    }

    std::ifstream config_file(config_path);
    if (config_file.fail())
    {
        std::cout << "The configuration file " << config_path << " does not exist. Skipped code generation." << std::endl;
        return EXIT_FAILURE;
    }

    ifx_Avian_Config_t config;
    ifx_json_t* json = ifx_json_create_from_file(config_path);
    if (json == nullptr)
    {
        std::cout << "Error: Cannot create JSON structure from configuration file" << std::endl;
        return EXIT_FAILURE;
    }

    if (ifx_json_has_config_single_shape(json))
    {
        if (!ifx_json_get_device_config_single_shape(json, &config))
        {
            std::cout << "Error: Cannot parse fmcw_single_shape configuration: " << ifx_json_get_error(json) << std::endl;
            return EXIT_FAILURE;
        }
    }
    else if (ifx_json_has_config_scene(json))
    {
        ifx_Avian_Metrics_t scene_config;
        if (!ifx_json_get_device_config_scene(json, &scene_config))
        {
            std::cout << "Error: Cannot parse fmcw_scene configuration: " << ifx_json_get_error(json) << std::endl;
            return EXIT_FAILURE;
        }

        ifx_avian_metrics_to_config(device, &scene_config, false, &config);
        if (ifx_error_get() != IFX_OK)
        {
            std::cout << "Error: Cannot convert scene to device configuration: " << ifx_error_to_string(ifx_error_get()) << std::endl;
            return EXIT_FAILURE;
        }
    }
    else
    {
        std::cout << "Error: Configuration file format not supported." << std::endl;
        return EXIT_FAILURE;
    }

    ifx_avian_set_config(device, &config);
    if (ifx_error_get() != IFX_OK)
    {
        std::cout << "Error: Device configuration invalid: " << ifx_error_to_string(ifx_error_get()) << std::endl;
        return EXIT_FAILURE;
    }

    bool set_trigger_bit = false;
    char* register_list_object = ifx_avian_get_register_list_string(device, set_trigger_bit);

    if (output_path == nullptr)
    {
        std::cout << register_list_object << std::endl;
    }
    else
    {
        std::ofstream output(output_path);
        if (output.is_open())
        {
            output << "/* XENSIV BGT60TRXX register configurator, SDK version" << ifx_sdk_get_version_string_full() << " */" << std::endl;
            output << std::endl;

            output << register_list_object;

            output.close();
        }
        else
        {
            std::cout << "Error: Could not open " << output_path << " for writing register configuration." << std::endl;
            ifx_mem_free(register_list_object);
            return EXIT_FAILURE;
        }
    }

    ifx_mem_free(register_list_object);

    return EXIT_SUCCESS;
}
