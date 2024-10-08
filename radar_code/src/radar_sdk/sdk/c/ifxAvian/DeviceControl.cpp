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

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include "DeviceControl.h"

#include "ifxBase/internal/List.hpp"
#include "ifxBase/internal/Util.h"
#include "ifxBase/Mem.h"
#include "ifxFmcw/avian/DeviceFmcwAvian.hpp"

#include <ifxAvian_DeviceTraits.hpp>

/*
==============================================================================
   2. LOCAL DEFINITIONS
==============================================================================
*/

/*
==============================================================================
   3. LOCAL TYPES
==============================================================================
*/

/*
==============================================================================
   5. LOCAL FUNCTION PROTOTYPES
==============================================================================
*/

/*
==============================================================================
   6. LOCAL FUNCTIONS
==============================================================================
*/

/*
==============================================================================
   7. EXPORTED FUNCTIONS
==============================================================================
*/

ifx_List_t* ifx_avian_get_list_by_sensor_type(ifx_Radar_Sensor_t sensor_type)
{
    return ifx_fmcw_get_list_by_sensor_type(sensor_type);
}

//----------------------------------------------------------------------------

ifx_List_t* ifx_avian_get_list()
{
    return ifx_fmcw_get_list();
}

//----------------------------------------------------------------------------

ifx_Avian_Device_t* ifx_avian_create_by_port(const char* port)
{
    return ifx_fmcw_create_by_port(port);
}

//----------------------------------------------------------------------------

ifx_Avian_Device_t* ifx_avian_create_dummy(ifx_Radar_Sensor_t sensor_type)
{
    return ifx_fmcw_create_dummy(sensor_type);
}

//----------------------------------------------------------------------------

ifx_Avian_Device_t* ifx_avian_create()
{
    return ifx_fmcw_create();
}

//----------------------------------------------------------------------------

ifx_Avian_Device_t* ifx_avian_create_by_uuid(const char* uuid)
{
    return ifx_fmcw_create_by_uuid(uuid);
}

//----------------------------------------------------------------------------

char* ifx_avian_get_register_list_string(ifx_Avian_Device_t* handle, bool set_trigger_bit)
{
    ifx_Avian_Config_t config;
    ifx_avian_get_config(handle, &config);

    std::stringstream string_cpp;

    const uint32_t num_rx_antennas = ifx_util_popcount(config.rx_mask);
    const uint32_t num_tx_antennas = ifx_util_popcount(config.tx_mask);

    string_cpp << "#ifndef XENSIV_BGT60TRXX_CONF_H" << std::endl;
    string_cpp << "#define XENSIV_BGT60TRXX_CONF_H" << std::endl;
    string_cpp << std::endl;

    const char* sensor_type_str = rdk::RadarDeviceCommon::sensor_to_string(ifx_avian_get_sensor_type(handle));

    string_cpp << "#define XENSIV_BGT60TRXX_CONF_DEVICE (XENSIV_DEVICE_" << sensor_type_str << ")" << std::endl;
    string_cpp << "#define XENSIV_BGT60TRXX_CONF_START_FREQ_HZ (" << config.start_frequency_Hz << ")" << std::endl;
    string_cpp << "#define XENSIV_BGT60TRXX_CONF_END_FREQ_HZ (" << config.end_frequency_Hz << ")" << std::endl;
    string_cpp << "#define XENSIV_BGT60TRXX_CONF_NUM_SAMPLES_PER_CHIRP (" << config.num_samples_per_chirp << ")" << std::endl;
    string_cpp << "#define XENSIV_BGT60TRXX_CONF_NUM_CHIRPS_PER_FRAME (" << config.num_chirps_per_frame << ")" << std::endl;
    string_cpp << "#define XENSIV_BGT60TRXX_CONF_NUM_RX_ANTENNAS (" << num_rx_antennas << ")" << std::endl;
    string_cpp << "#define XENSIV_BGT60TRXX_CONF_NUM_TX_ANTENNAS (" << num_tx_antennas << ")" << std::endl;
    string_cpp << "#define XENSIV_BGT60TRXX_CONF_SAMPLE_RATE (" << config.sample_rate_Hz << ")" << std::endl;
    string_cpp << "#define XENSIV_BGT60TRXX_CONF_CHIRP_REPETITION_TIME_S (" << config.chirp_repetition_time_s << ")" << std::endl;
    string_cpp << "#define XENSIV_BGT60TRXX_CONF_FRAME_REPETITION_TIME_S (" << config.frame_repetition_time_s << ")" << std::endl;

    // get number of registers and allocate enough size in vector
    const uint32_t num_registers = ifx_avian_export_register_list(handle, set_trigger_bit, nullptr);
    std::vector<uint32_t> register_list(num_registers);

    // export register list
    ifx_avian_export_register_list(handle, set_trigger_bit, register_list.data());

    if (num_registers)
    {
        const auto size = num_registers;

        string_cpp << "#define XENSIV_BGT60TRXX_CONF_NUM_REGS (" << size << ")\n\n";
        string_cpp << "#if defined(XENSIV_BGT60TRXX_CONF_IMPL)\n";
        string_cpp << "const uint32_t register_list[] = {\n";
        for (uint32_t i = 0; i < size; i++)
        {
            string_cpp << "    0x" << std::hex << register_list[i] << "UL,\n";
        }
        string_cpp << "};\n";
        string_cpp << "#endif /* XENSIV_BGT60TRXX_CONF_IMPL */\n";
    }

    string_cpp << "\n";
    string_cpp << "#endif /* XENSIV_BGT60TRXX_CONF_H */\n";

    const auto std_str = string_cpp.str();
    const auto length = std_str.size() + 1;
    auto* outstring_c = static_cast<char*>(ifx_mem_calloc(length, sizeof(char)));
    if (outstring_c)
    {
        std::copy(std_str.c_str(), std_str.c_str() + length, outstring_c);
    }
    else
    {
        ifx_error_set(IFX_ERROR_MEMORY_ALLOCATION_FAILED);
    }

    return outstring_c;
}

//----------------------------------------------------------------------------

void ifx_avian_import_register_list(ifx_Avian_Device_t* handle, const uint32_t* register_list, size_t num_elements)
{
    IFX_ERR_BRK_NULL(register_list);

    // mapping from spi command (uint32_t) to register address and value
    std::map<uint16_t, uint32_t> register_map;
    for (size_t i = 0; i < num_elements; i++)
    {
        // combined register address and register entry
        const uint32_t entry = register_list[i];

        // register address (most significand 7 bits)
        const uint16_t address = entry >> 25;

        // register value (least significand 24 bits)
        const uint32_t value = entry & 0xFFFFFF;

        register_map.insert(std::make_pair(address, value));
    }

    rdk::call_func(handle, &DeviceFmcw::apply_register_list, register_map);
}

void ifx_avian_load_register_file(ifx_Avian_Device_t* handle, const char* filename)
{
    auto* device_avian = dynamic_cast<DeviceFmcwAvian*>(handle);
    rdk::call_func(device_avian, &DeviceFmcwAvian::load_register_file, filename);
}

void ifx_avian_save_register_file(ifx_Avian_Device_t* handle, const char* filename)
{
    auto* device_avian = dynamic_cast<DeviceFmcwAvian*>(handle);
    rdk::call_func(device_avian, &DeviceFmcwAvian::save_register_file, filename);
}

//----------------------------------------------------------------------------

uint32_t ifx_avian_export_register_list(ifx_Avian_Device_t* handle, bool set_trigger_bit, uint32_t* register_list)
{
    auto* device_avian = dynamic_cast<DeviceFmcwAvian*>(handle);
    if (!device_avian)
    {
        ifx_error_set(IFX_ERROR_NOT_POSSIBLE);
        return 0;
    }

    return rdk::call_func(device_avian, &DeviceFmcwAvian::export_register_list_legacy, set_trigger_bit, register_list);
}

//----------------------------------------------------------------------------

ifx_Radar_Sensor_t ifx_avian_get_sensor_type(const ifx_Avian_Device_t* handle)
{
    return ifx_fmcw_get_sensor_type(handle);
}

//----------------------------------------------------------------------------

ifx_List_t* ifx_avian_get_hp_cutoff_list(const ifx_Avian_Device_t* handle)
{

    const auto* sensor_info = ifx_fmcw_get_sensor_information(handle);
    std::vector<int32_t> hp_cutoff_vector;
    for (size_t i = 0; sensor_info->hp_cutoff_list[i] != -1; i++)
    {
        hp_cutoff_vector.emplace_back(sensor_info->hp_cutoff_list[i]);
    }

    auto* list = ifx_list_from_vector(hp_cutoff_vector);
    return list;
}

//----------------------------------------------------------------------------

ifx_List_t* ifx_avian_get_aaf_cutoff_list(const ifx_Avian_Device_t* handle)
{
    const auto* sensor_info = ifx_fmcw_get_sensor_information(handle);
    std::vector<int32_t> lp_cutoff_vector;
    for (size_t i = 0; sensor_info->lp_cutoff_list[i] != -1; i++)
    {
        lp_cutoff_vector.emplace_back(sensor_info->lp_cutoff_list[i]);
    }

    auto* list = ifx_list_from_vector(lp_cutoff_vector);
    return list;
}

//----------------------------------------------------------------------------

void ifx_avian_set_config(ifx_Avian_Device_t* handle, const ifx_Avian_Config_t* config)
{
    /*
     * The new FMCW API offers a convenience structure that allows to specify
     * an acquisition sequence with the same parameters, as the Avian API does.
     * The provided configuration is translated to the new structure.
     */
    ifx_Fmcw_Simple_Sequence_Config_t translated_config;
    translated_config.frame_repetition_time_s = config->frame_repetition_time_s;
    translated_config.chirp_repetition_time_s = config->chirp_repetition_time_s;
    translated_config.num_chirps = config->num_chirps_per_frame;
    translated_config.chirp.sample_rate_Hz = static_cast<float>(config->sample_rate_Hz);
    translated_config.chirp.rx_mask = config->rx_mask;
    translated_config.chirp.tx_mask = config->tx_mask;
    translated_config.chirp.tx_power_level = config->tx_power_level;
    translated_config.chirp.if_gain_dB = config->if_gain_dB;
    translated_config.chirp.start_frequency_Hz = static_cast<double>(config->start_frequency_Hz);
    translated_config.chirp.end_frequency_Hz = static_cast<double>(config->end_frequency_Hz);
    translated_config.chirp.num_samples = config->num_samples_per_chirp;
    translated_config.chirp.hp_cutoff_Hz = config->hp_cutoff_Hz;
    translated_config.chirp.lp_cutoff_Hz = config->aaf_cutoff_Hz;
    translated_config.tdm_mimo = config->mimo_mode == IFX_MIMO_TDM;

    /*
     * From the translated configuration a temporary sequence structure is
     * created and passed to the DeviceFmcwAvian object. Afterwards that
     * sequence structure is not needed any longer.
     */
    auto* sequence = ifx_fmcw_create_simple_sequence(&translated_config);
    if (ifx_error_get() == IFX_OK)
    {
        ifx_fmcw_set_acquisition_sequence(handle, sequence);
    }
    ifx_fmcw_destroy_sequence(sequence);
}

//----------------------------------------------------------------------------

void ifx_avian_get_config(ifx_Avian_Device_t* handle, ifx_Avian_Config_t* config)
{
    auto* sequence = ifx_fmcw_get_acquisition_sequence(handle);

    /*
     * The acquisition sequence must consist only of a frame loop, a chirp loop
     * and a chirp element. Since the sequence is set only through this API
     * this should always be the case. Anyhow, for safety the sequence is
     * checked for the expected structure.
     */
    const ifx_Fmcw_Sequence_Loop_t* frame_loop = nullptr;
    const ifx_Fmcw_Sequence_Loop_t* chirp_loop = nullptr;
    const ifx_Fmcw_Sequence_Chirp_t* chirp = nullptr;
    const ifx_Fmcw_Sequence_Element_t* additional_element = nullptr;

    IFX_ERR_BRK_NULL(sequence);
    IFX_ERR_BRF_COND((sequence->type != IFX_SEQ_LOOP)
                         || (sequence->next_element != nullptr),
                     IFX_ERROR_ARGUMENT_INVALID);
    frame_loop = &sequence->loop;

    IFX_ERR_BRF_COND((frame_loop->sub_sequence == nullptr)
                         || (frame_loop->sub_sequence->type != IFX_SEQ_LOOP)
                         || (frame_loop->sub_sequence->next_element != nullptr),
                     IFX_ERROR_ARGUMENT_INVALID);
    chirp_loop = &frame_loop->sub_sequence->loop;

    IFX_ERR_BRF_COND((chirp_loop->sub_sequence == nullptr)
                         || (chirp_loop->sub_sequence->type != IFX_SEQ_CHIRP),
                     IFX_ERROR_ARGUMENT_INVALID);
    chirp = &chirp_loop->sub_sequence->chirp;
    additional_element = chirp_loop->sub_sequence->next_element;

    /*
     * Now that the structure is checked, the parameters can be extracted from
     * the sequence.
     */
    config->sample_rate_Hz = static_cast<uint32_t>(chirp->sample_rate_Hz);
    config->rx_mask = chirp->rx_mask;
    config->tx_mask = chirp->tx_mask;
    config->tx_power_level = chirp->tx_power_level;
    config->if_gain_dB = static_cast<uint8_t>(chirp->if_gain_dB);
    config->start_frequency_Hz = static_cast<uint64_t>(chirp->start_frequency_Hz);
    config->end_frequency_Hz = static_cast<uint64_t>(chirp->end_frequency_Hz);
    config->num_samples_per_chirp = chirp->num_samples;
    config->hp_cutoff_Hz = chirp->hp_cutoff_Hz;
    config->aaf_cutoff_Hz = chirp->lp_cutoff_Hz;

    config->chirp_repetition_time_s = chirp_loop->repetition_time_s;
    config->num_chirps_per_frame = chirp_loop->num_repetitions;

    config->frame_repetition_time_s = frame_loop->repetition_time_s;

    /*
     * Usually there is only one chirp in the sequence, but in case of TDM MIMO
     * there are multiple chirps with equal parameters except the TX mask,
     * which is different for each shape. In this case the TX masks from all
     * chirps must be joined to one mask that contains all active TX antennas.
     */
    config->mimo_mode = (additional_element != nullptr) ? IFX_MIMO_TDM
                                                        : IFX_MIMO_OFF;

    while (additional_element != nullptr)
    {
        const ifx_Fmcw_Sequence_Chirp_t* additional_chirp = nullptr;

        /*
         * Wait elements are ignored, because the Avian FSM has hard coded delays
         * at the end of a chirp. As a result in MIMO TDM mode there is always a
         * little delay between the chirps.
         */
        if (additional_element->type == IFX_SEQ_DELAY)
        {
            additional_element = additional_element->next_element;
            continue;
        }

        IFX_ERR_BRF_COND((additional_element->type != IFX_SEQ_CHIRP),
                         IFX_ERROR_ARGUMENT_INVALID);
        additional_chirp = &additional_element->chirp;

        IFX_ERR_BRF_COND((config->sample_rate_Hz != additional_chirp->sample_rate_Hz)
                             || (config->rx_mask != additional_chirp->rx_mask)
                             || (config->tx_power_level != additional_chirp->tx_power_level)
                             || (config->if_gain_dB != static_cast<uint8_t>(additional_chirp->if_gain_dB))
                             || (config->start_frequency_Hz != static_cast<uint64_t>(additional_chirp->start_frequency_Hz))
                             || (config->end_frequency_Hz != static_cast<uint64_t>(additional_chirp->end_frequency_Hz))
                             || (config->num_samples_per_chirp != additional_chirp->num_samples)
                             || (config->hp_cutoff_Hz != static_cast<uint32_t>(additional_chirp->hp_cutoff_Hz))
                             || (config->aaf_cutoff_Hz != static_cast<uint32_t>(additional_chirp->lp_cutoff_Hz)),
                         IFX_ERROR_ARGUMENT_INVALID);

        config->tx_mask |= additional_chirp->tx_mask;

        additional_element = additional_element->next_element;
    }

fail:
    ifx_fmcw_destroy_sequence(sequence);
}

//----------------------------------------------------------------------------

void ifx_avian_get_config_defaults(ifx_Avian_Device_t* handle, ifx_Avian_Config_t* config)
{
    IFX_ERR_BRK_NULL(handle);
    IFX_ERR_BRK_NULL(config);

    const auto sensor_type = ifx_avian_get_sensor_type(handle);
    auto* dummy = ifx_avian_create_dummy(sensor_type);

    ifx_avian_get_config(dummy, config);
}

//----------------------------------------------------------------------------


void ifx_avian_get_temperature(ifx_Avian_Device_t* handle, float* temperature_celsius)
{
    *temperature_celsius = ifx_fmcw_get_temperature(handle);
}


//----------------------------------------------------------------------------

void ifx_avian_destroy(ifx_Avian_Device_t* handle)
{
    ifx_fmcw_destroy(handle);
}

//----------------------------------------------------------------------------

void ifx_avian_start_acquisition(ifx_Avian_Device_t* handle)
{
    ifx_fmcw_start_acquisition(handle);
}

//----------------------------------------------------------------------------

void ifx_avian_stop_acquisition(ifx_Avian_Device_t* handle)
{
    ifx_fmcw_stop_acquisition(handle);
}


//----------------------------------------------------------------------------

ifx_Cube_R_t* ifx_avian_get_next_frame_timeout(ifx_Avian_Device_t* handle, ifx_Cube_R_t* frame, uint16_t timeout_ms)
{
    SmartFmcwFrame fmcwFrame(ifx_fmcw_allocate_frame(handle));
    IFX_ERR_RETV(frame);

    IFX_ERR_BRV_COND((fmcwFrame->cubes[0]->dimensions != 3), IFX_ERROR_DIMENSION_MISMATCH, frame);
    if (fmcwFrame->num_cubes > 1)
    {
        for (uint32_t i = 1; i < fmcwFrame->num_cubes; i++)
        {
            IFX_ERR_BRV_COND((fmcwFrame->cubes[i]->dimensions != 3), IFX_ERROR_DIMENSION_MISMATCH, frame);
            for (uint8_t d = 0; d < 3; d++)
            {
                IFX_ERR_BRV_COND((fmcwFrame->cubes[i]->shape[d] != fmcwFrame->cubes[0]->shape[d]), IFX_ERROR_DIMENSION_MISMATCH, frame);
            }
        }
        if (frame)
        {
            IFX_ERR_BRV_COND((frame->dimensions != 3), IFX_ERROR_DIMENSION_MISMATCH, frame);
            IFX_ERR_BRV_COND((frame->shape[0] != fmcwFrame->cubes[0]->shape[0] * fmcwFrame->num_cubes), IFX_ERROR_DIMENSION_MISMATCH, frame);
            for (uint8_t d = 1; d < 3; d++)
            {
                IFX_ERR_BRV_COND((frame->shape[d] != fmcwFrame->cubes[0]->shape[d]), IFX_ERROR_DIMENSION_MISMATCH, frame);
            }
        }
    }

    ifx_fmcw_get_next_frame_timeout(handle, fmcwFrame.get(), timeout_ms);
    IFX_ERR_RETV(frame);

    if (fmcwFrame->num_cubes == 1)
    {
        // transfer ownership of pointer to caller
        if (!frame)
        {
            // there was no frame given, we simply extract the pointer from fmcwFrame
            frame = fmcwFrame->cubes[0];
            fmcwFrame->cubes[0] = nullptr;
        }
        else
        {
            // there was a frame given, so if it contains data, free the memory
            // then we can copy the mda structure to the given frame (which includes the pointer)
            // (The pointer in the copied-from structure needs to be reset then)
            if (IFX_MDA_OWNS_DATA(frame))
            {
                ifx_mem_aligned_free(IFX_MDA_DATA(frame));
            }
            IFX_MDA_FLAGS(frame) |= IFX_MDA_FLAG_OWNS_DATA;
            *frame = *fmcwFrame->cubes[0];
            fmcwFrame->cubes[0]->data = nullptr;
        }
    }
    else
    {
        const auto num_rx_per_cube = fmcwFrame->cubes[0]->shape[0];
        const auto num_chirps = fmcwFrame->cubes[0]->shape[1];
        const auto num_samples_per_chirp = fmcwFrame->cubes[0]->shape[2];

        const auto num_rx = num_rx_per_cube * fmcwFrame->num_cubes;
        if (!frame)
        {
            frame = ifx_cube_create_r(num_rx, num_chirps, num_samples_per_chirp);
            IFX_ERR_RETV(frame);
        }

        auto* frame_ptr = frame->data;
        const auto data_per_cube = num_rx_per_cube * num_chirps * num_samples_per_chirp;
        for (uint32_t i = 0; i < fmcwFrame->num_cubes; i++)
        {
            std::copy(fmcwFrame->cubes[i]->data, fmcwFrame->cubes[i]->data + data_per_cube, frame_ptr);
            frame_ptr += data_per_cube;
        }
    }

    // convert data from range -1..1 to range 0..1
    auto* first = IFX_MDA_DATA(frame);
    const auto* last = first + ifx_mda_elements_r(frame);
    while (first < last)
    {
        const auto sample = (*first + 1.0f) / 2.0f;
        *first++ = sample;
    }

    return frame;
}

//----------------------------------------------------------------------------

ifx_Cube_R_t* ifx_avian_get_next_frame(ifx_Avian_Device_t* handle, ifx_Cube_R_t* frame)
{
    return ifx_avian_get_next_frame_timeout(handle, frame, 10000);
}

//----------------------------------------------------------------------------

const char* ifx_avian_get_board_uuid(const ifx_Avian_Device_t* handle)
{
    return ifx_fmcw_get_board_uuid(handle);
}

//----------------------------------------------------------------------------

const ifx_Radar_Sensor_Info_t* ifx_avian_get_sensor_information(const ifx_Avian_Device_t* handle)
{
    return ifx_fmcw_get_sensor_information(handle);
}

//----------------------------------------------------------------------------

const ifx_Firmware_Info_t* ifx_avian_get_firmware_information(const ifx_Avian_Device_t* handle)
{
    return ifx_fmcw_get_firmware_information(handle);
}

//----------------------------------------------------------------------------

float ifx_avian_get_minimum_chirp_repetition_time(const ifx_Avian_Device_t* handle, uint32_t num_samples, uint32_t sample_rate_Hz)
{
    return ifx_fmcw_get_minimum_chirp_repetition_time(handle, num_samples, static_cast<float>(sample_rate_Hz));
}

//----------------------------------------------------------------------------

double ifx_avian_get_sampling_center_frequency(ifx_Device_Fmcw_t* handle, ifx_Avian_Config_t* config)
{
    ifx_Fmcw_Sequence_Chirp_t chirp;
    chirp.start_frequency_Hz = static_cast<double>(config->start_frequency_Hz);
    chirp.end_frequency_Hz = static_cast<double>(config->end_frequency_Hz);

    return ifx_fmcw_get_chirp_sampling_center_frequency(handle, &chirp);
}

//----------------------------------------------------------------------------

double ifx_avian_get_sampling_bandwidth(ifx_Device_Fmcw_t* handle, ifx_Avian_Config_t* config)
{
    ifx_Fmcw_Sequence_Chirp_t chirp;
    chirp.start_frequency_Hz = static_cast<double>(config->start_frequency_Hz);
    chirp.end_frequency_Hz = static_cast<double>(config->end_frequency_Hz);

    return ifx_fmcw_get_chirp_sampling_bandwidth(handle, &chirp);
}
