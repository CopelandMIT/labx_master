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
#include "DeviceCwAvian.hpp"

// Strata
#include <platform/NamedMemory.hpp>

// libAvian
#include "ifxAvian_DeviceTraits.hpp"
#include "ifxAvian_StrataUtilities.hpp"
#include "ifxAvian_Types.hpp"
#include "ifxAvian_Utilities.hpp"
#include "ports/ifxAvian_DummyPort.hpp"
#include "ports/ifxAvian_StrataPort.hpp"

#include <cmath>  // for std::round

/*
==============================================================================
   2. LOCAL DEFINITIONS
==============================================================================
*/

using namespace Infineon::Avian;

namespace {

// maximum tx power
constexpr uint8_t BGT60TRXX_MAX_TX_POWER = 31;
// maximum numbers of samples per chirp
constexpr uint32_t MAX_NUM_SAMPLES_PER_CHIRP = 4095;

// minimum adc sampling rate in Hz
constexpr float MIN_ADC_SAMPLING_RATE_HZ = 78.201e3f;
// maximum adc sampling rate in Hz
constexpr float MAX_ADC_SAMPLING_RATE_HZ = 4e6f;

// Amplification factor that is applied to the IF signal before sampling; Valid range: [18-60]dB
constexpr int8_t IFX_IF_GAIN_DB_LOWER_LIMIT = 18;
constexpr int8_t IFX_IF_GAIN_DB_UPPER_LIMIT = 60;

uint32_t hp_gain_to_dB(const Hp_Gain hp_gain)
{
    switch (hp_gain)
    {
        case Hp_Gain::_18dB:
            return 18;
        case Hp_Gain::_30dB:
            return 30;
        default:
            throw rdk::exception::argument_invalid();
    }
}

uint32_t vga_gain_to_dB(const Vga_Gain vga_gain)
{
    switch (vga_gain)
    {
        case Vga_Gain::_0dB:
            return 0;
        case Vga_Gain::_5dB:
            return 5;
        case Vga_Gain::_10dB:
            return 10;
        case Vga_Gain::_15dB:
            return 15;
        case Vga_Gain::_25dB:
            return 25;
        case Vga_Gain::_30dB:
            return 30;
        default:
            throw rdk::exception::argument_invalid();
    }
}

uint32_t adc_sampling_time_to_ns(const Adc_Sample_Time adc_sampling_time)
{
    switch (adc_sampling_time)
    {
        case Adc_Sample_Time::_50ns:
            return 50;
        case Adc_Sample_Time::_100ns:
            return 100;
        case Adc_Sample_Time::_200ns:
            return 200;
        case Adc_Sample_Time::_400ns:
            return 400;
        default:
            throw rdk::exception::argument_invalid();
    }
}

Adc_Sample_Time ns_to_adc_sampling_time(uint32_t time_ns)
{
    switch (time_ns)
    {
        case 50:
            return Adc_Sample_Time::_50ns;
        case 100:
            return Adc_Sample_Time::_100ns;
        case 200:
            return Adc_Sample_Time::_200ns;
        case 400:
            return Adc_Sample_Time::_400ns;
        default:
            throw rdk::exception::argument_invalid();
    }
}

uint32_t adc_oversampling_to_factor(const Adc_Oversampling oversampling)
{
    switch (oversampling)
    {
        case Adc_Oversampling::Off:
            return 0;
        case Adc_Oversampling::_2x:
            return 2;
        case Adc_Oversampling::_4x:
            return 4;
        case Adc_Oversampling::_8x:
            return 8;
        default:
            throw rdk::exception::argument_invalid();
    }
}

Adc_Oversampling oversampling_factor_to_adc_oversampling(uint32_t oversampling_factor)
{
    switch (oversampling_factor)
    {
        case 0:
            return Adc_Oversampling::Off;
        case 2:
            return Adc_Oversampling::_2x;
        case 4:
            return Adc_Oversampling::_4x;
        case 8:
            return Adc_Oversampling::_8x;
        default:
            throw rdk::exception::argument_invalid();
    }
}

Adc_Tracking subconversion_to_adc_tracking(uint16_t additional_subconversions)
{
    switch (additional_subconversions)
    {
        case 0:
            return Adc_Tracking::None;
        case 1:
            return Adc_Tracking::_1_Subconversion;
        case 3:
            return Adc_Tracking::_3_Subconversions;
        case 7:
            return Adc_Tracking::_7_Subconversions;
        default:
            throw rdk::exception::argument_invalid();
    }
}

uint16_t adc_tracking_to_subconversion(Adc_Tracking adc_tracking)
{
    switch (adc_tracking)
    {
        case Adc_Tracking::None:
            return 0;
        case Adc_Tracking::_1_Subconversion:
            return 1;
        case Adc_Tracking::_3_Subconversions:
            return 3;
        case Adc_Tracking::_7_Subconversions:
            return 7;
        default:
            throw rdk::exception::argument_invalid();
    }
}

}  // namespace

/*
==============================================================================
   3. LOCAL TYPES
==============================================================================
*/

/*
==============================================================================
   4. FUNCTION PROTOTYPES
==============================================================================
*/

/*
==============================================================================
   5. LOCAL FUNCTION PROTOTYPES
==============================================================================
*/

DeviceCwAvian::DeviceCwAvian(std::unique_ptr<BoardInstance>&& board) :
    DeviceCwBase(std::move(board))
{
    if (!m_board)
    {
        throw rdk::exception::no_device();
    }

    // Checks internally that we are really connected to a board with an Avian sensor
    m_avian_port = std::make_unique<StrataPort>(m_board.get());

    m_cw_controller = std::make_unique<Continuous_Wave_Controller>(*m_avian_port);

    get_baseband_config();
    get_adc_config();

    //! TODO Check if parameters okay
    m_test_signal_config.frequency_Hz = m_cw_controller->get_test_signal_frequency();
    m_test_signal_config.mode = static_cast<ifx_Cw_Test_Signal_Generator_Mode>(m_cw_controller->get_test_signal_generator_mode());

    /* Initialize data component related member variables */
    m_data_index = 0;
    m_data = m_board->getIBridge()->getIBridgeControl()->getIData();
    m_bridge_data = m_board->getIBridge()->getIBridgeData();
    m_device_type = m_cw_controller->get_device_type();

    DeviceCwAvian::initialize_sensor_info();
}

DeviceCwAvian::DeviceCwAvian(ifx_Radar_Sensor_t device_type)
{
    m_avian_port = std::make_unique<DummyPort>();
    const Driver driver = Driver(*m_avian_port, static_cast<Device_Type>(device_type));
    m_cw_controller = std::make_unique<Continuous_Wave_Controller>(*m_avian_port, driver);

    /* Initialize data component related member variables */
    m_data_index = 0;
    m_device_type = m_cw_controller->get_device_type();

    DeviceCwAvian::initialize_sensor_info();
}

DeviceCwAvian::DeviceCwAvian(const DeviceCwAvian& deviceCwAvian) :
    DeviceCwBase()  // NOLINT(readability-redundant-member-init)
{
    m_avian_port = std::make_unique<DummyPort>();
    m_cw_controller = std::make_unique<Continuous_Wave_Controller>(*m_avian_port, deviceCwAvian.m_cw_controller->get_driver());

    /* Initialize data component related member variables */
    m_data_index = 0;
    m_device_type = m_cw_controller->get_device_type();

    DeviceCwAvian::initialize_sensor_info();
}

DeviceCwAvian::~DeviceCwAvian()
{
    try
    {
        DeviceCwAvian::stop_signal();
    }
    catch (...)
    {
    }
}

void DeviceCwAvian::initialize_sensor_info()
{
    const auto& device_traits = Device_Traits::get(m_device_type);
    m_sensor_info.description = device_traits.description;
    m_sensor_info.num_tx_antennas = device_traits.num_tx_antennas;
    m_sensor_info.num_rx_antennas = device_traits.num_rx_antennas;
    m_sensor_info.max_tx_power = BGT60TRXX_MAX_TX_POWER;
    auto range = m_cw_controller->get_continuous_wave_frequency_range();
    m_sensor_info.min_rf_frequency_Hz = range.first;
    m_sensor_info.max_rf_frequency_Hz = range.second;
    m_sensor_info.device_id = 0;
    m_sensor_info.min_adc_sampling_rate = MIN_ADC_SAMPLING_RATE_HZ;
    m_sensor_info.max_adc_sampling_rate = MAX_ADC_SAMPLING_RATE_HZ;
    m_sensor_info.max_num_samples_per_chirp = MAX_NUM_SAMPLES_PER_CHIRP;
    m_sensor_info.hp_cutoff_list = device_traits.hpf_cutoff_settings.data();
    m_sensor_info.lp_cutoff_list = device_traits.aaf_cutoff_settings.data();

    for (const auto& hp_gain : device_traits.hp_gain_settings)
    {
        if (hp_gain == -1)
        {
            break;
        }
        for (const auto& vga_gain : device_traits.vga_gain_settings)
        {
            if (vga_gain == -1)
            {
                break;
            }
            m_if_gain_list.emplace_back(hp_gain + vga_gain);
        }
    }
    std::sort(m_if_gain_list.begin(), m_if_gain_list.end());
    m_if_gain_list.push_back(-1);
    m_sensor_info.if_gain_list = m_if_gain_list.data();
}

bool DeviceCwAvian::is_signal_active()
{
    return m_cw_controller->is_continuous_wave_enabled();
}

void DeviceCwAvian::start_signal()
{
    m_cw_controller->enable_continuous_wave(true);
}

void DeviceCwAvian::stop_signal()
{
    m_cw_controller->enable_continuous_wave(false);
}

void DeviceCwAvian::set_baseband_config(const ifx_Cw_Baseband_Config_t* config)
{
    using namespace Infineon::Avian;
    const auto& if_gain_dB = config->if_gain_dB;
    const auto hp_cutoff_Hz = config->hp_cutoff_Hz;

    // check range of if_gain_dB
    if ((if_gain_dB < IFX_IF_GAIN_DB_LOWER_LIMIT) || (if_gain_dB > IFX_IF_GAIN_DB_UPPER_LIMIT))
        throw rdk::exception::error::exception(IFX_ERROR_IF_GAIN_OUT_OF_RANGE);

    // Choose hp_gain as high as possible
    const auto hp_gain = (if_gain_dB >= 30)
                             ? Hp_Gain::_30dB
                             : Hp_Gain::_18dB;

    // VGA gain can be 0dB, 5dB, 10dB, ..., 30dB.
    // Choose vga_gain such that hp_gain + vga_gain is as close to if_gain_dB
    // as possible.
    const int gain_left_dB = if_gain_dB - ((hp_gain == Hp_Gain::_30dB) ? 30 : 18);
    const div_t div = std::div(gain_left_dB, 5);                     // 5 because of the steps of 5dB
    const auto vga_gain = (div.rem >= 3)
                              ? static_cast<Vga_Gain>(div.quot + 1)  // round up (hp_gain+vga_gain > if_gain_dB)
                              : static_cast<Vga_Gain>(div.quot);     // round down (hp_gain+vga_gain <= if_gain_dB)
    bool was_enabled = is_signal_active();
    if (was_enabled)
    {
        stop_signal();
    }

    m_cw_controller->set_vga_gain(vga_gain);
    m_cw_controller->set_hp_gain(hp_gain);
    m_cw_controller->set_hp_cutoff(hp_cutoff_Hz);
    m_cw_controller->set_continuous_wave_power(config->tx_power_level);

    m_cw_controller->set_continuous_wave_frequency(static_cast<double>(config->start_frequency_Hz));
    m_cw_controller->set_aaf_cutoff(config->lp_cutoff_Hz);
    m_baseband_config.lp_cutoff_Hz = config->lp_cutoff_Hz;
    m_baseband_config.if_gain_dB = config->if_gain_dB;
    m_baseband_config.hp_cutoff_Hz = config->hp_cutoff_Hz;

    uint8_t antenna_index = 0;
    for (uint32_t tx_bitmask = config->tx_mask; tx_bitmask != 0; tx_bitmask >>= 1)
    {
        bool enable = (tx_bitmask & 1u) != 0;
        m_cw_controller->enable_tx_antenna(antenna_index, enable);
        antenna_index += 1;
    }

    antenna_index = 0;
    for (uint32_t rx_bitmask = config->rx_mask; rx_bitmask != 0; rx_bitmask >>= 1)
    {
        bool enable = (rx_bitmask & 1u) != 0;
        m_cw_controller->enable_rx_antenna(antenna_index, enable);
        antenna_index += 1;
    }

    m_cw_controller->set_number_of_samples(config->num_samples);
    m_cw_controller->set_sampling_rate(config->sample_rate_Hz);

    if (was_enabled)
    {
        start_signal();
    }
}

const ifx_Cw_Baseband_Config_t* DeviceCwAvian::get_baseband_config()
{
    const auto vga_gain = static_cast<Vga_Gain>(m_cw_controller->get_vga_gain());
    const auto vga_gain_dB = vga_gain_to_dB(vga_gain);
    const auto hp_gain = static_cast<Hp_Gain>(m_cw_controller->get_hp_gain());
    const auto hp_gain_dB = hp_gain_to_dB(hp_gain);
    m_baseband_config.if_gain_dB = vga_gain_dB + hp_gain_dB;

    m_baseband_config.hp_cutoff_Hz = m_cw_controller->get_hp_cutoff();
    m_baseband_config.lp_cutoff_Hz = m_cw_controller->get_aaf_cutoff();
    m_baseband_config.start_frequency_Hz = m_cw_controller->get_continuous_wave_frequency();

    m_baseband_config.num_samples = m_cw_controller->get_number_of_samples();
    m_baseband_config.sample_rate_Hz = m_cw_controller->get_sampling_rate();

    m_baseband_config.rx_mask = get_rx_antenna_mask();
    m_baseband_config.tx_mask = get_tx_antenna_mask();
    m_baseband_config.tx_power_level = m_cw_controller->get_continuous_wave_power();

    return &m_baseband_config;
}

void DeviceCwAvian::set_adc_config(const ifx_Cw_Adc_Config_t* config)
{
    using namespace Infineon::Avian;
    auto sampling_time = ns_to_adc_sampling_time(config->sample_and_hold_time_ns);
    m_cw_controller->set_adc_sample_time(sampling_time);

    auto oversampling = oversampling_factor_to_adc_oversampling(config->oversampling_factor);
    m_cw_controller->set_adc_oversampling(oversampling);

    m_cw_controller->set_adc_double_msb_time(config->double_msb_time);

    auto additional_subconversions = subconversion_to_adc_tracking(config->additional_subconversions);
    m_cw_controller->set_adc_tracking(additional_subconversions);

    m_adc_config.additional_subconversions = config->additional_subconversions;
    m_adc_config.sample_and_hold_time_ns = config->sample_and_hold_time_ns;
    m_adc_config.oversampling_factor = config->oversampling_factor;
    m_adc_config.double_msb_time = config->double_msb_time;
}

const ifx_Cw_Adc_Config_t* DeviceCwAvian::get_adc_config()
{
    auto sampling_time = m_cw_controller->get_adc_sample_time();
    m_adc_config.sample_and_hold_time_ns = adc_sampling_time_to_ns(sampling_time);

    auto oversampling_factor = m_cw_controller->get_adc_oversampling();
    m_adc_config.oversampling_factor = adc_oversampling_to_factor(oversampling_factor);

    auto adc_tracking = m_cw_controller->get_adc_tracking();
    m_adc_config.additional_subconversions = adc_tracking_to_subconversion(adc_tracking);

    m_adc_config.double_msb_time = m_cw_controller->get_adc_double_msb_time();

    return &m_adc_config;
}

void DeviceCwAvian::set_test_signal_generator_config(const ifx_Cw_Test_Signal_Generator_Config_t* config)
{
    using namespace Infineon::Avian;
    m_cw_controller->set_test_signal_frequency(config->frequency_Hz);
    m_cw_controller->set_test_signal_generator_mode(static_cast<Continuous_Wave_Controller::Test_Signal_Generator_Mode>(config->mode));

    m_test_signal_config.frequency_Hz = config->frequency_Hz;
    m_test_signal_config.mode = config->mode;
}

const ifx_Cw_Test_Signal_Generator_Config_t* DeviceCwAvian::get_test_signal_generator_config()
{
    return &m_test_signal_config;
}

float DeviceCwAvian::measure_temperature()
{
    return m_cw_controller->measure_temperature();
}

float DeviceCwAvian::measure_tx_power(const uint32_t antenna)
{
    return m_cw_controller->measure_tx_power(antenna);
}

ifx_Matrix_R_t* DeviceCwAvian::capture_frame(ifx_Matrix_R_t* frame)
{
    if (frame == nullptr)
    {
        frame = ifx_mat_create_r(get_tx_antenna_enabled_count() * get_rx_antenna_enabled_count(),
                                 m_cw_controller->get_number_of_samples());
    }
    if (ifx_error_get() != IFX_OK)
    {
        return frame;
    }

    std::map<uint32_t, std::vector<ifx_Float_t>> signals;
    uint8_t num_rx = 0;
    signals = m_cw_controller->capture_rx_signals();

    for (const auto& keyvalue : signals)
    {
        for (uint32_t sample = 0; sample < keyvalue.second.size(); sample++)
        {
            IFX_MAT_AT(frame, num_rx, sample) = keyvalue.second[sample];
        }
        num_rx++;
    }
    return frame;
}

ifx_Radar_Sensor_t DeviceCwAvian::get_sensor_type() const
{
    return static_cast<ifx_Radar_Sensor_t>(m_cw_controller->get_device_type());
}

void DeviceCwAvian::generate_register_list()
{
    const auto& driver = m_cw_controller->get_driver();
    auto avian_registers = driver.get_device_configuration().get_configuration_sequence(false);

    m_register_map.clear();
    for (auto spi_command : avian_registers)
    {
        const uint16_t address = spi_command >> 25;
        const uint32_t value = spi_command & 0x00FFFFFF;
        m_register_map.insert(std::make_pair(address, value));
    }
}

std::map<uint16_t, uint32_t>& DeviceCwAvian::get_register_list()
{
    generate_register_list();
    return m_register_map;
}

std::map<uint16_t, uint32_t> DeviceCwAvian::import_register_list(const char* filename)
{
    // Each entry in the retrieved  register map alwayas refers to layout index 0.
    // Therefore, provide one dummy layout with no bitfields in it (an empty map).
    std::vector<NamedMemory<uint16_t, uint32_t>::Layout> dummy_layout {{"dummy_layout", {}}};
    auto named_memory = std::make_unique<NamedMemory<uint16_t, uint32_t>>(nullptr, get_register_map(m_device_type), dummy_layout);

    std::map<uint16_t, uint32_t> register_list;
    named_memory->loadConfig(filename, register_list);

    return register_list;
}

void DeviceCwAvian::export_register_list(const char* filename, const std::map<uint16_t, uint32_t>& register_list)
{
    // Each entry in the retrieved register map alwayas refers to layout index 0.
    // Therefore, provide one dummy layout with with no bitfields in it (an empty map).
    std::vector<NamedMemory<uint16_t, uint32_t>::Layout> dummy_layout {{"dummy_layout", {}}};
    auto named_memory = std::make_unique<NamedMemory<uint16_t, uint32_t>>(nullptr, get_register_map(m_device_type), dummy_layout);

    named_memory->saveConfig(filename, register_list);
}

void DeviceCwAvian::load_register_file(const char* filename)
{
    const auto register_list = import_register_list(filename);
    apply_register_list(register_list);
}

void DeviceCwAvian::save_register_file(const char* filename)
{
    auto& register_list = get_register_list();
    export_register_list(filename, register_list);
}

uint32_t DeviceCwAvian::get_tx_antenna_mask() const
{
    const uint32_t NUM_TX_ANTENNAS = m_cw_controller->get_number_of_tx_antennas();

    uint32_t tx_mask = 0;
    for (uint32_t antenna = 0; antenna < NUM_TX_ANTENNAS; antenna++)
    {
        if (m_cw_controller->is_tx_antenna_enabled(antenna))
            tx_mask |= (0x1 << antenna);
    }

    return tx_mask;
}

uint32_t DeviceCwAvian::get_rx_antenna_mask() const
{
    const uint32_t NUM_RX_ANTENNAS = m_cw_controller->get_number_of_rx_antennas();

    uint32_t rx_mask = 0;
    for (uint32_t antenna = 0; antenna < NUM_RX_ANTENNAS; antenna++)
    {
        if (m_cw_controller->is_rx_antenna_enabled(antenna))
            rx_mask |= (0x1 << antenna);
    }

    return rx_mask;
}

uint32_t DeviceCwAvian::get_tx_antenna_enabled_count() const
{
    const uint32_t NUM_TX_ANTENNAS = m_cw_controller->get_number_of_tx_antennas();

    uint32_t tx_enabled_count = 0;
    for (uint32_t antenna = 0; antenna < NUM_TX_ANTENNAS; antenna++)
    {
        if (m_cw_controller->is_tx_antenna_enabled(antenna))
            tx_enabled_count += 1;
    }

    return tx_enabled_count;
}

uint32_t DeviceCwAvian::get_rx_antenna_enabled_count() const
{
    const uint32_t NUM_RX_ANTENNAS = m_cw_controller->get_number_of_rx_antennas();

    uint32_t rx_enabled_count = 0;
    for (uint32_t antenna = 0; antenna < NUM_RX_ANTENNAS; antenna++)
    {
        if (m_cw_controller->is_rx_antenna_enabled(antenna))
            rx_enabled_count += 1;
    }

    return rx_enabled_count;
}

void DeviceCwAvian::apply_register_list(const std::map<uint16_t, uint32_t>& register_list)
{
    throw rdk::exception::not_supported();
}
