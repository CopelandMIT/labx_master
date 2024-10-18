/**
 * \file ifxAvian_CwController.cpp
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
#include "ifxAvian_CwController.hpp"
#include "Driver/registers_BGT120TR24E.h"
#include "Driver/registers_BGT60TR11D.h"
#include "Driver/registers_BGT60TRxxC.h"
#include "Driver/registers_BGT60TRxxD.h"
#include "Driver/registers_BGT60TRxxE.h"
#include "ifxAvian_DataConverter.hpp"
#include "ifxAvian_DeviceTraits.hpp"
#include "ifxAvian_SensorMeter.hpp"
#include "ifxAvian_Utilities.hpp"
#include <algorithm>
#include <cmath>
#include <condition_variable>
#include <mutex>
#include <stdexcept>


// Define template for clamp because std::clamp is only available with C++17.
template <class T>
constexpr const T& clamp(const T& value, const T& low, const T& high)
{
    if (value < low)
        return low;
    else if (value > high)
        return high;
    else
        return value;
}

// ---------------------------------------------------------------------------- namespaces
namespace Infineon {
namespace Avian {

// ---------------------------------------------------------------------------- Constant_Wave_Controller
Continuous_Wave_Controller::Continuous_Wave_Controller(HW::IControlPort& port) :
    Continuous_Wave_Controller(port, Driver::create_driver(port))
{}

// ---------------------------------------------------------------------------- Constant_Wave_Controller
Continuous_Wave_Controller::Continuous_Wave_Controller(HW::IControlPort& port,
                                                   const Driver& driver_template) :
    Continuous_Wave_Controller(port, std::make_unique<Driver>(port, driver_template))
{}

// ---------------------------------------------------------------------------- Constant_Wave_Controller
Continuous_Wave_Controller::Continuous_Wave_Controller(HW::IControlPort& port,
                                                   std::unique_ptr<Driver> driver) :
    m_port(port),
    m_driver(std::move(driver)),
    m_continuous_wave_frequency(60.0e9),
    m_continuous_wave_power(0),
    m_test_signal_mode(Test_Signal_Generator_Mode::Off),
    m_test_signal_frequency_divider(200 /* 400kHz at 80MHz ref clock*/),
    m_continuous_wave_enabled(false),
    m_tx_mask(1),
    m_rx_mask(1),
    m_num_samples(64),
    m_toggle_commands {0}
{
    /*
     * A frame with just one chirp per frame is defined.That frame type is used
     * for data acquisition during CW mode. More elaborate modes are not
     * supported.
     */
    Frame_Definition frame_def =
        {
            /* shapes*/
            {
                {1, Power_Mode::Stay_Active, 0},
                {0, Power_Mode::Stay_Active, 0},
                {0, Power_Mode::Stay_Active, 0},
                {0, Power_Mode::Stay_Active, 0}},
            /* shape set */
            {1, Power_Mode::Stay_Active, 0},
            /* num_frames */
            0};
    m_driver->set_frame_definition(&frame_def);
    m_driver->select_shape_to_configure(0, false);
    m_driver->set_chirp_end_delay(0);
}

// ---------------------------------------------------------------------------- ~Constant_Wave_Controller
Continuous_Wave_Controller::~Continuous_Wave_Controller()
{
    try
    {
        enable_continuous_wave(false);
    }
    catch (...)
    {
        /*
         * In case radar device is unplugged, the method enable_continuous_wave() throws an exception because it
         * is not possible to communicate to the device. As there is no way to catch an exception from a destructor,
         * we catch the exception here.
         */
    }
}
// ---------------------------------------------------------------------------- get_device_type
Device_Type Continuous_Wave_Controller::get_device_type() const
{
    return m_driver->get_device_type();
}

// ---------------------------------------------------------------------------- enable_continuous_wave
void Continuous_Wave_Controller::enable_continuous_wave(bool enable)
{
    m_continuous_wave_enabled = enable;
    if (enable)
    {
        auto& device_traits = Device_Traits::get(m_driver->get_device_type());

        uint32_t frequency_kHz = unsigned(m_continuous_wave_frequency / 1000.0);

        // First the driver is used to configure the chip for normal operation
        Frame_Format cfg_frame {};
        cfg_frame.num_chirps_per_frame = 1;
        cfg_frame.rx_mask = uint8_t(m_rx_mask.to_ulong());
        cfg_frame.num_samples_per_chirp = m_num_samples;
        m_driver->set_frame_format(&cfg_frame);

        Fmcw_Configuration cfg_fmcw {};
        cfg_fmcw.lower_frequency_kHz = frequency_kHz;
        cfg_fmcw.upper_frequency_kHz = frequency_kHz;
        cfg_fmcw.shape_type = Shape_Type::Saw_Up;
        cfg_fmcw.tx_power = uint8_t(m_continuous_wave_power);
        m_driver->set_fmcw_configuration(&cfg_fmcw);

        m_driver->set_tx_mode(m_tx_mask[0]
                                  ? (m_tx_mask[1] ? Tx_Mode::Alternating
                                                  : Tx_Mode::Tx1_Only)
                                  : (m_tx_mask[1] ? Tx_Mode::Tx2_Only
                                                  : Tx_Mode::Off));

        m_driver->set_slice_size(uint16_t(m_num_samples * m_rx_mask.count()));

        /*
         * Afterwards the register set generated by the driver is modified
         * to adjust it continuous wave mode.
         */
        auto registers = m_driver->get_device_configuration();

        // TX power sensors are enabled for measurement.
        registers.set(registers[BGT60TRxxC_REG_CS1_U_0]
                      | BGT60TRxxC_SET(CS1_U_0, PD1_EN, 1)
                      | BGT60TRxxC_SET(CS1_U_0, PD2_EN, 1));

        /*
         * CW mode is enabled
         * - Setting the CW mode bit makes the state machine wait for a new
         *   trigger before going to the next state.
         * - Setting the BYPRMPEN lets PLL stay at start frequency instead of
         *   generating a ramp.
         */
        registers.set(BGT60TRxxC_SET(PDFT0, BYPRMPEN, 1));
        registers.set(registers[BGT60TRxxC_REG_MAIN]
                      | BGT60TRxxC_SET(MAIN, CW_MODE, 1));

        /*
         * If the device does not have an SADC, MADC must be enabled to be
         * ready for temperature and power measurement. MADC can't be enabled
         * directly, it must be enabled implicitly by enabling at least one RX
         * channel.
         */
        if (!device_traits.has_sadc)
        {
            registers.set(registers[BGT60TRxxC_REG_CS1_U_1]
                          | BGT60TRxxC_SET(CS1_U_1, BBCH_SEL, 1));
        }

        // By default toggle command sequence is cleared.
        m_toggle_commands[0] = 0;

        // Test signal generator is configured.
        if (m_test_signal_mode == Test_Signal_Generator_Mode::Off)
        {
            registers.set(BGT60TRxxC_SET(RFT0, RFTSIGCLK_DIV_EN, 0));
        }
        else if (m_test_signal_mode == Test_Signal_Generator_Mode::Test_Baseband)
        {
            registers.set(BGT60TRxxC_SET(RFT0, RFTSIGCLK_DIV,
                                         m_test_signal_frequency_divider)
                          | BGT60TRxxC_SET(RFT0, RFTSIGCLK_DIV_EN, 1)
                          | BGT60TRxxC_SET(RFT0, TEST_SIG_RF_EN, 0)
                          | BGT60TRxxC_SET(RFT0, TEST_SIG_IF_EN, m_rx_mask.to_ulong()));

            // For baseband testing, RX mixers are disabled
            uint32_t reg_cs1u = registers[BGT60TRxxC_REG_CS1_U_0];
            if (m_rx_mask[0])
                reg_cs1u &= ~BGT60TRxxC_CS1_U_0_RX1LOBUF_EN_msk
                            & ~BGT60TRxxC_CS1_U_0_RX1MIX_EN_msk;
            if (m_rx_mask[1])
                reg_cs1u &= ~BGT60TRxxC_CS1_U_0_RX2LOBUF_EN_msk
                            & ~BGT60TRxxC_CS1_U_0_RX2MIX_EN_msk;
            if (m_rx_mask[2])
                reg_cs1u &= ~BGT60TRxxC_CS1_U_0_RX3LOBUF_EN_msk
                            & ~BGT60TRxxC_CS1_U_0_RX3MIX_EN_msk;
            if (m_rx_mask[3])
                reg_cs1u &= ~BGT60TRxxC_CS1_U_0_RX4LOBUF_EN_msk
                            & ~BGT60TRxxC_CS1_U_0_RX4MIX_EN_msk;
            registers.set(BGT60TRxxC_REG_CS1_U_0, reg_cs1u);
        }
        else if (device_traits.supports_tx_toggling)
        {
            auto command_RFT0 = BGT60TRxxC_SET(RFT0, RFTSIGCLK_DIV,
                                               m_test_signal_frequency_divider)
                                | BGT60TRxxC_SET(RFT0, RFTSIGCLK_DIV_EN, 1)
                                | BGT60TRxxC_SET(RFT0, TEST_SIG_IF_EN, 0);

            if (m_test_signal_mode == Test_Signal_Generator_Mode::Toggle_Tx_Enable)
            {
                command_RFT0 |= BGT60TRxxD_SET(RFT0, RF_TEST_MODE, 1)
                                | BGT60TRxxC_SET(RFT0, TEST_SIG_RF_EN, 0);
            }
            else if (m_test_signal_mode == Test_Signal_Generator_Mode::Toggle_Dac_Value)
            {
                command_RFT0 |= BGT60TRxxD_SET(RFT0, RF_TEST_MODE, 2)
                                | BGT60TRxxC_SET(RFT0, TEST_SIG_RF_EN, 0);
            }
            else if (m_test_signal_mode == Test_Signal_Generator_Mode::Rx_Self_Test)
            {
                command_RFT0 |= BGT60TRxxD_SET(RFT0, RF_TEST_MODE, 3)
                                | BGT60TRxxC_SET(RFT0, TEST_SIG_RF_EN, 1);
            }
            registers.set(command_RFT0);
        }
        else
        {
            /*
             * If the Avian device does not support the selected generator
             * mode. The test signal is generated by toggling the according
             * bit field via SPI, while data is acquired. In this case the test
             * signal frequency is ignored. The resulting test signal frequency
             * depends only on SPI clock rate.
             * At this point the commands for toggling a bit field are setup.
             * The actual toggling happens in the method capture_rx_signals.
             */
            if (m_test_signal_mode == Test_Signal_Generator_Mode::Toggle_Tx_Enable)
            {
                m_toggle_commands[1] = registers[BGT60TRxxC_REG_CS1_U_0]
                                       | BGT60TRxxC_SET(CS1_U_0, TX1_EN, 0);
                m_toggle_commands[0] = m_toggle_commands[1]
                                       & ~BGT60TRxxC_CS1_U_0_TX1_EN_msk;
            }
            else if (m_test_signal_mode == Test_Signal_Generator_Mode::Toggle_Dac_Value)
            {
                m_toggle_commands[1] = registers[BGT60TRxxC_REG_CS1_U_1]
                                       | BGT60TRxxC_SET(CS1_U_1, TX1_DAC, 0);
                m_toggle_commands[0] = m_toggle_commands[1]
                                       & ~BGT60TRxxC_CS1_U_1_TX1_DAC_msk;
            }
            else if (m_test_signal_mode == Test_Signal_Generator_Mode::Rx_Self_Test)
            {
                m_toggle_commands[0] = BGT60TRxxC_SET(RFT0, TEST_SIG_RF_EN, 1);
                m_toggle_commands[1] = BGT60TRxxC_SET(RFT0, TEST_SIG_RF_EN, 0);
            }
        }

        // The final configuration is sent to the Avian device.
        m_port.generate_reset_sequence();
        initialize_reference_clock(m_port, m_driver->get_clock_config_command());
        registers.send_to_device(m_port, false);

        // Power amplifiers are enabled.
        if (!go_to_active_state())
        {
            m_port.generate_reset_sequence();
            m_continuous_wave_enabled = false;
            throw std::runtime_error("A hardware failure occurred.");
        }
    }
    else
    {
        m_port.generate_reset_sequence();
    }
}

// ---------------------------------------------------------------------------- go_to_active_state
bool Continuous_Wave_Controller::go_to_active_state()
{
    auto& device_traits = Device_Traits::get(m_driver->get_device_type());

    auto trigger = m_driver->get_device_configuration()[BGT60TRxxC_REG_MAIN]
                   | BGT60TRxxC_SET(MAIN, FRAME_START, 1)
                   | BGT60TRxxC_SET(MAIN, CW_MODE, 1);
    auto read_status = device_traits.has_reordered_register_layout
                           ? BGT60TRxxE_REGISTER_READ_CMD(STAT0)
                           : BGT60TRxxC_REGISTER_READ_CMD(STAT0);

    for (unsigned i = 0; i < 10000; ++i)
    {
        m_port.send_commands(&trigger, 1);

        HW::Spi_Response_t status_register;
        m_port.send_commands(&read_status, 1, &status_register);

        if (BGT60TRxxC_EXTRACT(STAT0, PM, status_register) == 1)
            return true;
    }

    // Active power state hasn't been reached.
    return false;
}

// ---------------------------------------------------------------------------- is_continuous_wave_enabled
bool Continuous_Wave_Controller::is_continuous_wave_enabled() const
{
    return m_continuous_wave_enabled;
}

// ---------------------------------------------------------------------------- get_continuous_wave_frequency_range
Continuous_Wave_Controller::Range<double> Continuous_Wave_Controller::get_continuous_wave_frequency_range() const
{
    Device_Info device_info {};
    m_driver->get_device_info(&device_info);

    return Range<double>(device_info.min_rf_frequency_kHz * 1000.0,
                         device_info.max_rf_frequency_kHz * 1000.0);
}

// ---------------------------------------------------------------------------- set_continuous_wave_frequency
void Continuous_Wave_Controller::set_continuous_wave_frequency(double frequency_Hz)
{
    auto range = get_continuous_wave_frequency_range();
    m_continuous_wave_frequency = clamp(frequency_Hz, range.first, range.second);

    // By re-enabling continuous wave mode, settings are applied.
    if (m_continuous_wave_enabled)
        enable_continuous_wave(true);
}

// ---------------------------------------------------------------------------- get_continuous_wave_frequency
double Continuous_Wave_Controller::get_continuous_wave_frequency() const
{
    return m_continuous_wave_frequency;
}

// ---------------------------------------------------------------------------- get_continuous_wave_power_range
Continuous_Wave_Controller::Range<unsigned> Continuous_Wave_Controller::get_continuous_wave_power_range() const
{
    Device_Info device_info {};
    m_driver->get_device_info(&device_info);

    return Range<unsigned>(0, device_info.max_tx_power);
}

// ---------------------------------------------------------------------------- set_continuous_wave_power
void Continuous_Wave_Controller::set_continuous_wave_power(unsigned dac_value)
{
    auto range = get_continuous_wave_power_range();
    m_continuous_wave_power = clamp(dac_value, range.first, range.second);

    // By re-enabling continuous wave mode, settings are applied.
    if (m_continuous_wave_enabled)
        enable_continuous_wave(true);
}

// ---------------------------------------------------------------------------- get_continuous_wave_power
unsigned Continuous_Wave_Controller::get_continuous_wave_power() const
{
    return m_continuous_wave_power;
}

// ---------------------------------------------------------------------------- get_number_of_tx_antennas
unsigned Continuous_Wave_Controller::get_number_of_tx_antennas() const
{
    Device_Info device_info {};
    m_driver->get_device_info(&device_info);
    return device_info.num_tx_antennas;
}

// ---------------------------------------------------------------------------- enable_tx_antenna
void Continuous_Wave_Controller::enable_tx_antenna(unsigned antenna, bool enable)
{
    if (antenna >= get_number_of_tx_antennas())
        throw std::runtime_error("Selected TX antenna does not exist.");

    m_tx_mask[antenna] = enable;

    // By re-enabling continuous wave mode, settings are applied.
    if (m_continuous_wave_enabled)
        enable_continuous_wave(true);
}

// ---------------------------------------------------------------------------- is_tx_antenna_enabled
bool Continuous_Wave_Controller::is_tx_antenna_enabled(unsigned antenna) const
{
    return m_tx_mask[antenna];
}

// ---------------------------------------------------------------------------- set_test_signal_generator_mode
void Continuous_Wave_Controller::set_test_signal_generator_mode(Test_Signal_Generator_Mode mode)
{
    m_test_signal_mode = mode;

    // By re-enabling continuous wave mode, settings are applied.
    if (m_continuous_wave_enabled)
        enable_continuous_wave(true);
}

// ---------------------------------------------------------------------------- get_test_signal_generator_mode
Continuous_Wave_Controller::Test_Signal_Generator_Mode Continuous_Wave_Controller::get_test_signal_generator_mode() const
{
    return m_test_signal_mode;
}

// ---------------------------------------------------------------------------- get_test_signal_frequency_range
Continuous_Wave_Controller::Range<float> Continuous_Wave_Controller::get_test_signal_frequency_range() const
{
    Reference_Clock_Frequency ref_clock {};
    m_driver->get_reference_clock_frequency(&ref_clock);
    float reference_clock = 80.0e6;
    if ((ref_clock == Reference_Clock_Frequency::_76_8MHz)
        || (ref_clock == Reference_Clock_Frequency::_38_4MHz))
        reference_clock = 76.8e6;

    const uint32_t uMaxDivider = BGT60TRxxC_RFT0_RFTSIGCLK_DIV_msk
                                 >> BGT60TRxxC_RFT0_RFTSIGCLK_DIV_pos;
    return Range<float>(reference_clock / uMaxDivider, reference_clock / 2);
}

// ---------------------------------------------------------------------------- set_test_signal_frequency
void Continuous_Wave_Controller::set_test_signal_frequency(float frequency_Hz)
{
    Reference_Clock_Frequency ref_clock {};
    m_driver->get_reference_clock_frequency(&ref_clock);
    float reference_clock = 80.0e6;
    if ((ref_clock == Reference_Clock_Frequency::_76_8MHz)
        || (ref_clock == Reference_Clock_Frequency::_38_4MHz))
        reference_clock = 76.8e6;

    auto range = get_test_signal_frequency_range();
    frequency_Hz = clamp(frequency_Hz, range.first, range.second);

    m_test_signal_frequency_divider = std::lround(reference_clock
                                                  / frequency_Hz);

    // By re-enabling continuous wave mode, settings are applied.
    if (m_continuous_wave_enabled)
        enable_continuous_wave(true);
}

// ---------------------------------------------------------------------------- get_test_signal_frequency
float Continuous_Wave_Controller::get_test_signal_frequency() const
{
    Reference_Clock_Frequency ref_clock {};
    m_driver->get_reference_clock_frequency(&ref_clock);
    float reference_clock = 80.0e6;
    if ((ref_clock == Reference_Clock_Frequency::_76_8MHz)
        || (ref_clock == Reference_Clock_Frequency::_38_4MHz))
        reference_clock = 76.8e6;

    return reference_clock / m_test_signal_frequency_divider;
}

// ---------------------------------------------------------------------------- get_number_of_rx_antennas
unsigned Continuous_Wave_Controller::get_number_of_rx_antennas() const
{
    Device_Info device_info {};
    m_driver->get_device_info(&device_info);
    return device_info.num_rx_antennas;
}

// ---------------------------------------------------------------------------- enable_rx_antenna
void Continuous_Wave_Controller::enable_rx_antenna(unsigned antenna, bool enable)
{
    if (antenna >= get_number_of_rx_antennas())
        throw std::runtime_error("Selected RX antenna does not exist.");

    m_rx_mask[antenna] = enable;

    // By re-enabling continuous wave mode, settings are applied.
    if (m_continuous_wave_enabled)
        enable_continuous_wave(true);
}

// ---------------------------------------------------------------------------- is_rx_antenna_enabled
bool Continuous_Wave_Controller::is_rx_antenna_enabled(unsigned antenna) const
{
    return m_rx_mask[antenna];
}

// ---------------------------------------------------------------------------- get_number_of_samples_range
Continuous_Wave_Controller::Range<unsigned> Continuous_Wave_Controller::get_number_of_samples_range() const
{
    const uint32_t uMaxNumSamples = BGT60TRxxC_PLL1_3_APU_msk
                                    >> BGT60TRxxC_PLL1_3_APU_pos;
    return Range<unsigned>(1, uMaxNumSamples);
}

// ---------------------------------------------------------------------------- set_number_of_samples
void Continuous_Wave_Controller::set_number_of_samples(unsigned num_samples)
{
    auto range = get_number_of_samples_range();
    m_num_samples = uint16_t(clamp(num_samples, range.first, range.second));

    // By re-enabling continuous wave mode, settings are applied.
    if (m_continuous_wave_enabled)
        enable_continuous_wave(true);
}

// ---------------------------------------------------------------------------- get_number_of_samples
unsigned Continuous_Wave_Controller::get_number_of_samples() const
{
    return m_num_samples;
}

// ---------------------------------------------------------------------------- get_sampling_rate_range
Continuous_Wave_Controller::Range<float> Continuous_Wave_Controller::get_sampling_rate_range() const
{
    Reference_Clock_Frequency ref_clock {};
    m_driver->get_reference_clock_frequency(&ref_clock);
    float reference_clock = 80.0e6;

    if ((ref_clock == Reference_Clock_Frequency::_76_8MHz)
        || (ref_clock == Reference_Clock_Frequency::_38_4MHz))
        reference_clock = 76.8e6;

    // Read current ADC configuration and calculation number of cycles per conversion
    Adc_Configuration adc_config;
    m_driver->get_adc_configuration(&adc_config);

    return Range<float>(get_min_sampling_rate(reference_clock),
                        get_max_sampling_rate(adc_config.sample_time,
                                              adc_config.tracking,
                                              adc_config.double_msb_time,
                                              adc_config.oversampling,
                                              reference_clock));
}

// ---------------------------------------------------------------------------- set_sampling_rate
void Continuous_Wave_Controller::set_sampling_rate(float frequency_Hz)
{
    auto range = get_sampling_rate_range();
    frequency_Hz = clamp(frequency_Hz, range.first, range.second);

    m_driver->set_adc_samplerate(uint32_t(frequency_Hz));

    // By re-enabling continuous wave mode, settings are applied.
    if (m_continuous_wave_enabled)
        enable_continuous_wave(true);
}

// ---------------------------------------------------------------------------- get_sampling_rate
float Continuous_Wave_Controller::get_sampling_rate() const
{
    uint32_t sampling_rate {};
    m_driver->get_adc_samplerate(&sampling_rate);
    return float(sampling_rate);
}

// ---------------------------------------------------------------------------- capture_rx_signals
std::map<unsigned, std::vector<float>> Continuous_Wave_Controller::capture_rx_signals()
{
    auto& device_traits = Device_Traits::get(m_driver->get_device_type());

    // First it's checked if data can be acquired.
    if (!m_continuous_wave_enabled)
        throw std::runtime_error("continuous wave is not active.");
    if (m_rx_mask == 0)
        throw std::runtime_error("No RX antenna selected.");

    auto read_port = dynamic_cast<HW::IReadPort<HW::Packed_Raw_Data_t>*>(&m_port);
    if (read_port == nullptr)
        throw std::runtime_error("The provided port does not support data acquisition.");

    // Now a memory block is allocated to store received raw data.
    auto num_rx_antennas = m_rx_mask.count();
    size_t raw_block_size = m_num_samples * num_rx_antennas;
    std::vector<uint16_t> raw_data(raw_block_size);

    /*
     * Usually, the data receive callbacks is invoked in a separate thread.
     * Therefore some synchronization objects are initialized.
     */
    volatile bool data_received = false;
    std::condition_variable receive_notifier;
    std::mutex wait_guard;

    /*
     * The data converter is used as a wrapper around the Avian port and takes
     * care for data unpacking. After starting the converter and assigning a
     * buffer to it, it is ready to receive acquired data. The callback data
     * does nothing more than unblocking the waiting main thread.
     */
    DataConverter<uint16_t> converter(*read_port);
    converter.start_reader(m_driver->get_burst_prefix(), raw_block_size,
                           [&](uint32_t) -> void {
                               {
                                   std::unique_lock<std::mutex> lock(wait_guard);
                                   data_received = true;
                               }
                               receive_notifier.notify_one();
                           });
    converter.set_buffer(raw_data.data());

    /*
     * For Avian devices without SADC the MADC input may be set to temperature
     * or power sensor channel, so the input must be set to RX channel.
     */
    if (!device_traits.has_sadc)
    {
        auto command = BGT60TR11D_SET(ADC1, SENSOR_SEL, 0)
                       | (m_driver->get_device_configuration()[BGT60TR11D_REG_ADC1]
                          & ~BGT60TR11D_ADC1_SENSOR_SEL_msk);
        m_port.send_commands(&command, 1);
    }

    /*
     * The SPI sequence to trigger acquisition is prepared here.
     * Usually this sequence contains only a single write command to set the
     * trigger bit in main register. If a toggle sequence has been setup to
     * emulate certain test generator modes, that sequence is appended
     * repeatedly for the duration of acquisition.
     */
    std::vector<HW::Spi_Command_t> spi_commands;
    spi_commands.push_back(m_driver->get_device_configuration()[BGT60TRxxC_REG_MAIN]
                           | BGT60TRxxC_SET(MAIN, FRAME_START, 1)
                           | BGT60TRxxC_SET(MAIN, CW_MODE, 1));

    if (m_toggle_commands[0] != 0)
    {
        // A toggle cycle means two command words. Maximum SPI speed is assumed.
        const float spi_clock_rate = 50.0e6f;
        const float num_bits_per_cycle = 64;
        const float cycle_duration = num_bits_per_cycle / spi_clock_rate;

        uint32_t adc_sampling_rate = 0;
        m_driver->get_adc_samplerate(&adc_sampling_rate);
        const float acquisition_duration = float(m_num_samples)
                                           / float(adc_sampling_rate);

        // The number of cycles needed is rounded up.
        auto num_required_cycles = size_t(acquisition_duration
                                          / cycle_duration)
                                   + 1;

        /*
         * Some Infineon evaluation boards have some trouble, when the message
         * containing SPI commands becomes too large. As a workaround the
         * sequence is truncated.
         */
        num_required_cycles = std::min<size_t>(num_required_cycles, 60);

        spi_commands.reserve(2 * num_required_cycles);
        for (unsigned i = 0; i < num_required_cycles; ++i)
        {
            spi_commands.insert(spi_commands.end(),
                                m_toggle_commands.begin(),
                                m_toggle_commands.end());
        }
    }

    /*
     * After starting the ADC the execution blocks and waits for data. The
     * receive callback handler above will unblock this thread.
     *
     * After data has been received calling get_frame_info brings the Avian
     * state machine back to the point it was before the acquisition. The
     * frame info structure can be ignored.
     */
    std::unique_lock<std::mutex> lock(wait_guard);

    m_port.send_commands(spi_commands.data(), spi_commands.size());

    receive_notifier.wait_for(lock, std::chrono::seconds(1),
                              [&]() { return data_received; });

    if (!data_received || !go_to_active_state())
    {
        m_port.generate_reset_sequence();
        m_continuous_wave_enabled = false;
        throw std::runtime_error("A hardware failure occurred.");
    }

    /*
     * Finally raw data is de-interleaved and converted to floating point
     * numbers in the range -1...1.
     */
    std::map<unsigned, std::vector<float>> rx_signals;
    unsigned start_index = 0;
    for (unsigned i = 0; i < get_number_of_rx_antennas(); ++i)
    {
        if (!is_rx_antenna_enabled(i))
            continue;

        /*
         * A read pointer is initialized to point at the first sample of the
         * current RX antenna signal.
         */
        auto raw_pointer = raw_data.data() + start_index;
        ++start_index;

        /*
         * A output vector created and filled with the current RX antenna's
         * signal. Maximum 12 bit ADC Range 0...4095 is scaled to -1...1.
         */
        auto& signal = rx_signals.emplace(i, m_num_samples).first->second;
        for (auto& sample : signal)
        {
            sample = *raw_pointer * (2.f / 4095.f) - 1.f;
            raw_pointer += num_rx_antennas;
        }
    }

    return rx_signals;
}

// ---------------------------------------------------------------------------- set_hp_gain
void Continuous_Wave_Controller::set_hp_gain(Hp_Gain gain)
{
    Baseband_Configuration baseband_configuration {};
    m_driver->get_baseband_configuration(&baseband_configuration);
    baseband_configuration.hp_gain_1 = gain;
    baseband_configuration.hp_gain_2 = gain;
    baseband_configuration.hp_gain_3 = gain;
    baseband_configuration.hp_gain_4 = gain;
    m_driver->set_baseband_configuration(&baseband_configuration);

    // By re-enabling continuous wave mode, settings are applied.
    if (m_continuous_wave_enabled)
        enable_continuous_wave(true);
}

// ---------------------------------------------------------------------------- get_hp_gain
Hp_Gain Continuous_Wave_Controller::get_hp_gain() const
{
    Baseband_Configuration baseband_configuration {};
    m_driver->get_baseband_configuration(&baseband_configuration);
    return baseband_configuration.hp_gain_1;
}

// ---------------------------------------------------------------------------- set_hp_cutoff
void Continuous_Wave_Controller::set_hp_cutoff(int32_t cutoff_Hz)
{
    Baseband_Configuration baseband_configuration {};
    m_driver->get_baseband_configuration(&baseband_configuration);
    baseband_configuration.hp_cutoff_1_Hz = cutoff_Hz;
    baseband_configuration.hp_cutoff_2_Hz = cutoff_Hz;
    baseband_configuration.hp_cutoff_3_Hz = cutoff_Hz;
    baseband_configuration.hp_cutoff_4_Hz = cutoff_Hz;
    auto error = m_driver->set_baseband_configuration(&baseband_configuration);
    if (error != Driver::Error::OK)
        throw std::runtime_error("The specified cutoff frequency is not supported.");

    // By re-enabling continuous wave mode, settings are applied.
    if (m_continuous_wave_enabled)
        enable_continuous_wave(true);
}

// ---------------------------------------------------------------------------- get_hp_cutoff
int32_t Continuous_Wave_Controller::get_hp_cutoff() const
{
    Baseband_Configuration baseband_configuration {};
    m_driver->get_baseband_configuration(&baseband_configuration);
    return baseband_configuration.hp_cutoff_1_Hz;
}

// ---------------------------------------------------------------------------- set_vga_gain
void Continuous_Wave_Controller::set_vga_gain(Vga_Gain gain)
{
    Baseband_Configuration baseband_configuration {};
    m_driver->get_baseband_configuration(&baseband_configuration);
    baseband_configuration.vga_gain_1 = gain;
    baseband_configuration.vga_gain_2 = gain;
    baseband_configuration.vga_gain_3 = gain;
    baseband_configuration.vga_gain_4 = gain;
    m_driver->set_baseband_configuration(&baseband_configuration);

    // By re-enabling continuous wave mode, settings are applied.
    if (m_continuous_wave_enabled)
        enable_continuous_wave(true);
}

// ---------------------------------------------------------------------------- get_vga_gain
Vga_Gain Continuous_Wave_Controller::get_vga_gain() const
{
    Baseband_Configuration baseband_configuration {};
    m_driver->get_baseband_configuration(&baseband_configuration);
    return baseband_configuration.vga_gain_1;
}

// ---------------------------------------------------------------------------- set_aaf_cutoff
void Continuous_Wave_Controller::set_aaf_cutoff(int32_t cutoff_Hz)
{
    Anti_Alias_Filter_Settings aaf_settings {};
    aaf_settings.frequency1_Hz = cutoff_Hz;
    aaf_settings.frequency2_Hz = cutoff_Hz;
    aaf_settings.frequency3_Hz = cutoff_Hz;
    aaf_settings.frequency4_Hz = cutoff_Hz;
    auto error = m_driver->set_anti_alias_filter_settings(&aaf_settings);
    if (error != Driver::Error::OK)
        throw std::runtime_error("The specified cutoff frequency is not supported.");

    // By re-enabling continuous wave mode, settings are applied.
    if (m_continuous_wave_enabled)
        enable_continuous_wave(true);
}

// ---------------------------------------------------------------------------- get_aaf_cutoff
int32_t Continuous_Wave_Controller::get_aaf_cutoff() const
{
    Anti_Alias_Filter_Settings aaf_settings {};
    m_driver->get_anti_alias_filter_settings(&aaf_settings);
    return aaf_settings.frequency1_Hz;
}

// ---------------------------------------------------------------------------- set_adc_sample_time
void Continuous_Wave_Controller::set_adc_sample_time(Adc_Sample_Time sample_time)
{
    Adc_Configuration adc_configuration {};
    m_driver->get_adc_configuration(&adc_configuration);
    adc_configuration.sample_time = sample_time;
    if (m_driver->set_adc_configuration(&adc_configuration)
        != Driver::Error::OK)
    {
        adc_configuration.samplerate_Hz = 0;
        m_driver->set_adc_configuration(&adc_configuration);
    }

    // By re-enabling continuous wave mode, settings are applied.
    if (m_continuous_wave_enabled)
        enable_continuous_wave(true);
}

// ---------------------------------------------------------------------------- get_adc_sample_time
Adc_Sample_Time Continuous_Wave_Controller::get_adc_sample_time() const
{
    Adc_Configuration adc_configuration {};
    m_driver->get_adc_configuration(&adc_configuration);
    return adc_configuration.sample_time;
}

// ---------------------------------------------------------------------------- set_adc_tracking
void Continuous_Wave_Controller::set_adc_tracking(Adc_Tracking tracking)
{
    Adc_Configuration adc_configuration {};
    m_driver->get_adc_configuration(&adc_configuration);
    adc_configuration.tracking = tracking;
    if (m_driver->set_adc_configuration(&adc_configuration) != Driver::Error::OK)
    {
        adc_configuration.samplerate_Hz = 0;
        m_driver->set_adc_configuration(&adc_configuration);
    }

    // By re-enabling continuous wave mode, settings are applied.
    if (m_continuous_wave_enabled)
        enable_continuous_wave(true);
}

// ---------------------------------------------------------------------------- get_adc_tracking
Adc_Tracking Continuous_Wave_Controller::get_adc_tracking() const
{
    Adc_Configuration adc_configuration {};
    m_driver->get_adc_configuration(&adc_configuration);
    return adc_configuration.tracking;
}

// ---------------------------------------------------------------------------- set_adc_oversampling
void Continuous_Wave_Controller::set_adc_oversampling(Adc_Oversampling oversampleing)
{
    Adc_Configuration adc_configuration {};
    m_driver->get_adc_configuration(&adc_configuration);
    adc_configuration.oversampling = oversampleing;
    if (m_driver->set_adc_configuration(&adc_configuration) != Driver::Error::OK)
    {
        adc_configuration.samplerate_Hz = 0;
        m_driver->set_adc_configuration(&adc_configuration);
    }

    // By re-enabling continuous wave mode, settings are applied.
    if (m_continuous_wave_enabled)
        enable_continuous_wave(true);
}

// ---------------------------------------------------------------------------- get_adc_oversampling
Adc_Oversampling Continuous_Wave_Controller::get_adc_oversampling() const
{
    Adc_Configuration adc_configuration {};
    m_driver->get_adc_configuration(&adc_configuration);
    return adc_configuration.oversampling;
}

// ---------------------------------------------------------------------------- set_adc_double_msb_time
void Continuous_Wave_Controller::set_adc_double_msb_time(bool double_time)
{
    Adc_Configuration adc_configuration {};
    m_driver->get_adc_configuration(&adc_configuration);
    adc_configuration.double_msb_time = double_time;
    if (m_driver->set_adc_configuration(&adc_configuration)
        != Driver::Error::OK)
    {
        adc_configuration.samplerate_Hz = 0;
        m_driver->set_adc_configuration(&adc_configuration);
    }

    // By re-enabling continuous wave mode, settings are applied.
    if (m_continuous_wave_enabled)
        enable_continuous_wave(true);
}

// ---------------------------------------------------------------------------- get_adc_double_msb_time
bool Continuous_Wave_Controller::get_adc_double_msb_time() const
{
    Adc_Configuration adc_configuration {};
    m_driver->get_adc_configuration(&adc_configuration);
    return adc_configuration.double_msb_time;
}

// ---------------------------------------------------------------------------- measure_temperature
float Continuous_Wave_Controller::measure_temperature()
{
    Sensor_Meter meter(m_port, m_driver->get_device_type());
    if (m_continuous_wave_enabled)
        return meter.measure_temperature();
    else
        return meter.wake_up_and_measure_temperature(*m_driver);
}

// ---------------------------------------------------------------------------- measure_tx_power
float Continuous_Wave_Controller::measure_tx_power(unsigned antenna,
                                                 Power_Detector detector)
{
    // First, it's checked if the selected antenna and power detector exist.
    using Cs_Layout = Device_Traits::Channel_Set_Layout;
    auto& device_traits = Device_Traits::get(m_driver->get_device_type());
    if (antenna >= device_traits.num_tx_antennas)
        throw std::runtime_error("Selected TX antenna does not exist.");
    if ((device_traits.cs_register_layout != Cs_Layout::Version3)
        && (detector != Power_Detector::Forward))
    {
        throw std::runtime_error("The selected power detector is not"
                                 " present in this Avian device");
    }

    if (m_continuous_wave_enabled)
    {
        /*
         * The selected power detector is enabled by setting or clearing the
         * according bit field.
         */
        if (device_traits.cs_register_layout == Cs_Layout::Version3)
        {
            auto registers = m_driver->get_device_configuration();
            auto spi_word = registers[BGT120TR24E_REG_CS1_U_0];
            uint32_t value = (detector == Power_Detector::Reflection) ? 1 : 0;
            if (antenna == 1)
                spi_word |= BGT120TR24E_SET(CS1_U_0, PD2_SEL, value);
            else
                spi_word |= BGT120TR24E_SET(CS1_U_0, PD1_SEL, value);
            m_port.send_commands(&spi_word, 1);
        }

        Sensor_Meter meter(m_port, m_driver->get_device_type());
        return meter.measure_tx_power(antenna);
    }
    else
        return -std::numeric_limits<float>::infinity();
}

const Driver& Continuous_Wave_Controller::get_driver()
{
    if (!m_driver)
    {
        throw std::runtime_error("Driver was not initialized");
    }
    return *m_driver;
}

/* ------------------------------------------------------------------------ */
}  // namespace Avian
}  // namespace Infineon

/* --- End of File -------------------------------------------------------- */
