/**
 * @file ifxRadar_BGT60TRxx.cpp
 */
/* ===========================================================================
** Copyright (C) 2017-2023 Infineon Technologies AG
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
#include "_configuration.h"
#include "ifxAvian_CwController.hpp"
#include "ifxAvian_DeviceTraits.hpp"
#include "ifxAvian_Driver.hpp"
#include "ifxAvian_SensorMeter.hpp"
#include "ifxAvian_Utilities.hpp"
#include "registers_BGT60TR11D.h"
#include "registers_BGT60TRxxC.h"
#include "registers_BGT60TRxxD.h"
#include "registers_BGT60TRxxE.h"
#include "value_conversion/ifxAvian_RfConversion.hpp"

#include <algorithm>
#include <array>
#include <limits>
#include <memory>  // for make_unique and unique_ptr

// ---------------------------------------------------------------------------- namespaces
namespace Infineon {
namespace Avian {

using namespace Value_Conversion;
using Cs_Layout_t = Device_Traits::Channel_Set_Layout;

// ---------------------------------------------------------------------------- cutoff_to_reg
template <typename ARRAY_TYPE>
static uint8_t cutoff_to_reg(ARRAY_TYPE& cutoff_values, int32_t cutoff_Hz)
{
    uint8_t reg = 0;
    while (cutoff_Hz > cutoff_values[reg])
    {
        if ((reg + 1u == cutoff_values.size()) || (cutoff_values[reg + 1] == -1))
            break;

        auto double_middle = ((cutoff_values[reg] + cutoff_values[reg + 1]));
        if ((2 * cutoff_Hz) >= double_middle)
            ++reg;
        else
            break;
    }
    return reg;
}

// ---------------------------------------------------------------------------- get_frequency_range
void Driver::get_frequency_range(uint32_t* min_freq_kHz, uint32_t* max_freq_kHz) const
{
    auto& device_traits = Device_Traits::get(m_device_type);

    /*
     * First the maximum programmable RF range is calculated. If limited RF is
     * not desired, that's it.
     */
    RF_Converter converter(m_reference_clock_freq_Hz, m_pll_div_set,
                           device_traits.pll_pre_divider);
    *min_freq_kHz = converter.pll_to_freq(-0x800000);
    *max_freq_kHz = converter.pll_to_freq(0x7FFFFF);

#ifndef UNLIMITED_RF
    /* Set limited RF frequency range to not violate regulation rules. */
    using namespace std;
    *min_freq_kHz = max(*min_freq_kHz,
                        device_traits.allowed_rf_range.first);
    *max_freq_kHz = min(*max_freq_kHz,
                        device_traits.allowed_rf_range.second);
#endif
}

// ---------------------------------------------------------------------------- get_madc_acquisition_cycles
uint16_t Driver::get_madc_acquisition_cycles(const Adc_Configuration* config)
{
    uint16_t num_cycles = 0;

    switch (config->sample_time)
    {
        case Adc_Sample_Time::_50ns: num_cycles += 4; break;
        case Adc_Sample_Time::_100ns: num_cycles += 8; break;
        case Adc_Sample_Time::_200ns: num_cycles += 16; break;
        case Adc_Sample_Time::_400ns: num_cycles += 32; break;
    }

    /* check tracking */
    switch (config->tracking)
    {
        case Adc_Tracking::None: num_cycles += 0; break;
        case Adc_Tracking::_1_Subconversion: num_cycles += 8; break;
        case Adc_Tracking::_3_Subconversions: num_cycles += 24; break;
        case Adc_Tracking::_7_Subconversions: num_cycles += 56; break;
    }

    /* add cycles for conversion time */
    num_cycles += config->double_msb_time ? 17 : 16;

    /* oversampling */
    switch (config->oversampling)
    {
        case Adc_Oversampling::Off: break;
        case Adc_Oversampling::_2x: num_cycles *= 2; break;
        case Adc_Oversampling::_4x: num_cycles *= 4; break;
        case Adc_Oversampling::_8x: num_cycles *= 8; break;
    }

    return num_cycles;
}

// ---------------------------------------------------------------------------- check_power_sens_timing
Driver::Error Driver::check_power_sens_timing()
{
    uint32_t switch_cycles =
        (uint32_t)cycles_from_100ps(BGT60TR11D_MADC_IN_SWITCH_TIME_100ps,
                                    m_reference_clock_freq_Hz);
    Adc_Configuration adc_config;
    get_adc_configuration(&adc_config);

    /*
     * The total delay between chirp start and power measurement must be long
     * enough to allow switching the MADC input to power sensor channel.
     * ==> T_PAEN + T_PSSTART >= T_SW
     */
    if (m_pa_delay_reg * 8
            + (uint32_t)m_power_sens_delay_reg * 8 + 2
        < switch_cycles)
        return Error::NO_SWITCH_TIME_MADC_POWER;

    /*
     * After power measurement there must be enough time to switch back to the
     * RX channel before the IF sampling starts.
     * ==> T_SSTART >= T_PSSTART + T_SW + T_CONV
     */
    if (m_power_sens_delay_reg * 8 + 2 + switch_cycles
            + get_madc_acquisition_cycles(&adc_config)
        > 8 * (uint32_t)m_adc_delay_reg + 1)
        return Error::NO_SWITCH_TIME_MADC_RX;

    return Error::OK;
}

// ---------------------------------------------------------------------------- check_temperature_sens_timing
Driver::Error Driver::check_temperature_sens_timing()
{
    uint32_t T_SW =
        (uint32_t)cycles_from_100ps(BGT60TR11D_MADC_IN_SWITCH_TIME_100ps,
                                    m_reference_clock_freq_Hz);
    Adc_Configuration adc_config;
    get_adc_configuration(&adc_config);
    uint32_t T_CONV = get_madc_acquisition_cycles(&adc_config);

    uint32_t T_START = m_pre_chirp_delay_reg * 8 /* pre-chirp delay */
                       + 7                       /* syncing between FSM and PLL */
                       + 3;                      /* extra cycles from PLL state machine */

    uint32_t T_END = m_post_chirp_delay_reg * 8  /* post chirp delay */
                     + 5;                        /* extra cycles from PLL state machine */

    auto reg_PACR2 = m_current_configuration[BGT60TRxxC_REG_PACR2];
    uint8_t fast_down_mode = BGT60TRxxC_EXTRACT(PACR2, FSTDNEN, reg_PACR2);
    uint32_t T_FSDNTMR = BGT60TRxxC_EXTRACT(PACR2, FSDNTMR, reg_PACR2);
    if (T_FSDNTMR == 0)
    {
        T_FSDNTMR = (fast_down_mode == 1) ? 40 : (fast_down_mode == 2) ? 56
                                             : (fast_down_mode == 3)   ? 24
                                                                       : 0;
    }
    else
        T_FSDNTMR += 1;

    uint32_t T_PAEN = 8 * m_pa_delay_reg;
    uint32_t T_SSTART = 8 * m_adc_delay_reg + 1;

    uint32_t ADC_DIV = m_adc_sample_rate_divider;

    uint8_t shp;
    for (shp = 0; shp < 4; ++shp)
    {
        uint32_t T_EDx = 0;
        uint32_t APx = 0;

        if (m_shape[shp].num_repetitions == 0)
            break;

        PLL_Bitfield_Set pll_bitfields;
        setup_pll_bitfields(&m_shape[shp], &pll_bitfields);

        uint32_t T_PLL = T_START + T_END;

        if (m_shape[shp].shape_type == Shape_Type::Saw_Up)
        {
            APx = m_shape[shp].num_samples_up;
            T_PLL += pll_bitfields.rtu * 8;
            T_PLL += T_FSDNTMR;
            T_EDx = pll_bitfields.tedu * 8 + ((pll_bitfields.tedu != 0) ? 5 : 2)
                    + pll_bitfields.tedd * 8 + ((pll_bitfields.tedd != 0) ? 5 : 2);
        }
        else if (m_shape[shp].shape_type == Shape_Type::Saw_Down)
        {
            APx = m_shape[shp].num_samples_down;
            T_PLL += pll_bitfields.rtu * 8;
            T_PLL += T_FSDNTMR;
            T_EDx = pll_bitfields.tedu * 8 + ((pll_bitfields.tedu != 0) ? 5 : 2)
                    + pll_bitfields.tedd * 8 + ((pll_bitfields.tedd != 0) ? 5 : 2);
        }
        else if (m_shape[shp].shape_type == Shape_Type::Tri_Up)
        {
            T_PLL += pll_bitfields.rtd * 8;
            APx = m_shape[shp].num_samples_down;
            T_EDx = pll_bitfields.tedd * 8 + ((pll_bitfields.tedd != 0) ? 5 : 2);
        }
        else if (m_shape[shp].shape_type == Shape_Type::Tri_Down)
        {
            T_PLL += pll_bitfields.rtd * 8;
            APx = m_shape[shp].num_samples_up;
            T_EDx = pll_bitfields.tedd * 8 + ((pll_bitfields.tedd != 0) ? 5 : 2);
        }

        /*
         * dnop: On a closer look it seems that this constraint could be
         * reduced to T_END < T_SW. This should be double checked, before
         * applying any change here.
         */
        if (T_PLL < T_PAEN + T_SSTART + APx * ADC_DIV + T_SW)
            return Error::NO_SWITCH_TIME_MADC_TEMP;

        if (T_EDx < T_CONV)
            return Error::NO_MEASURE_TIME_MADC_TEMP;
    }
    return Error::OK;
}

// ---------------------------------------------------------------------------- create_driver
std::unique_ptr<Driver> Driver::create_driver(HW::IControlPort& port)
{
    /* start with a chip reset */
    port.generate_reset_sequence();

#if 1
    auto device_type = read_device_type(port);
    if (device_type == Device_Type::Unknown)
        return nullptr;

#else
    /* Support startup without RF board. */
    auto device_type = BGT60UTR11D;
#endif

    return std::make_unique<Driver>(port, device_type);
}

// ---------------------------------------------------------------------------- Driver
Driver::Driver(HW::IControlPort& port, Device_Type device_type) :
    m_port(port),
    m_device_type(device_type),
    m_pll_div_set(Device_Traits::get(m_device_type).pll_div_set_80M),
    m_reference_clock_freq_Hz(80000000),
    m_enable_frequency_doubler(false),
    m_adc_sample_rate_divider(0),
    m_adc_sample_time(Adc_Sample_Time::_50ns),
    m_adc_tracking(Adc_Tracking::None),
    m_adc_double_msb_time(false),
    m_adc_oversampling(Adc_Oversampling::Off),
    m_pre_chirp_delay_reg(0),
    m_post_chirp_delay_reg(0),
    m_pa_delay_reg(0),
    m_adc_delay_reg(0),
    m_time_wake_up {},
    m_time_init0 {},
    m_time_init1 {},
    m_idle_settings {},
    m_deep_sleep_settings {},
    m_currently_selected_shape(0),
    m_shape {},
    m_channel_set {},
    m_num_set_repetitions(0),
    m_frame_end_power_mode(Power_Mode::Stay_Active),
    m_frame_end_delay {},
    m_num_frames_before_stop(0),
    m_fifo_power_mode(Fifo_Power_Mode::Always_On),
    m_pad_driver_mode(Pad_Driver_Mode::Normal),
    m_bandgap_delay_reg(0),
    m_madc_delay_reg(0),
    m_pll_enable_delay_reg(0),
    m_pll_divider_delay_reg(0),
    m_dc_correction {Duty_Cycle_Correction_Mode::In_Out, 0, 0, 0},
    m_pullup_configuration {},
    m_power_sens_delay_reg(0),
    m_power_sensing_enabled(false),
    m_temperature_sensing_enabled(false),
    m_slice_size(4096),
    m_easy_mode_buffer_size(0),
    m_reset_state(true),
    m_current_mode(MODE_NORMAL),
    m_tx_power {}
{
    /* get default settings */
    set_default_parameters();

    /* update derived parameters */
    for (unsigned i = 0; i < 8; ++i)
    {
        Channel_Set_Settings* channel_set = &m_channel_set[i];
        channel_set->num_rx_antennas = count_antennas(channel_set->rx_mask);
    }

    /* initialize array of measured TX power values */
    for (unsigned i = 0; i < 16; ++i)
        m_tx_power[i / 2][i & 1] = 0x80000000;

    /* program BGT60TRxx device */
    update_spi_register_set();
}

// ---------------------------------------------------------------------------- Driver
Driver::Driver(HW::IControlPort& port, const Driver& source) :
    m_port(port)
{
    *this = source;
}

// ---------------------------------------------------------------------------- operator=
const Driver& Driver::operator=(const Driver& source)
{
    m_device_type = source.m_device_type;
    m_pll_div_set = source.m_pll_div_set;
    m_reference_clock_freq_Hz = source.m_reference_clock_freq_Hz;
    m_enable_frequency_doubler = source.m_enable_frequency_doubler;
    m_adc_sample_rate_divider = source.m_adc_sample_rate_divider;
    m_adc_sample_time = source.m_adc_sample_time;
    m_adc_tracking = source.m_adc_tracking;
    m_adc_double_msb_time = source.m_adc_double_msb_time;
    m_adc_oversampling = source.m_adc_oversampling;
    m_pre_chirp_delay_reg = source.m_pre_chirp_delay_reg;
    m_post_chirp_delay_reg = source.m_post_chirp_delay_reg;
    m_pa_delay_reg = source.m_pa_delay_reg;
    m_adc_delay_reg = source.m_adc_delay_reg;
    m_time_wake_up = source.m_time_wake_up;
    m_time_init0 = source.m_time_init0;
    m_time_init1 = source.m_time_init1;
    m_idle_settings = source.m_idle_settings;
    m_deep_sleep_settings = source.m_deep_sleep_settings;
    m_currently_selected_shape = source.m_currently_selected_shape;
    std::copy(source.m_shape, source.m_shape + 4, m_shape);
    std::copy(source.m_channel_set, source.m_channel_set + 8, m_channel_set);
    m_num_set_repetitions = source.m_num_set_repetitions;
    m_frame_end_power_mode = source.m_frame_end_power_mode;
    m_frame_end_delay = source.m_frame_end_delay;
    m_num_frames_before_stop = source.m_num_frames_before_stop;
    m_fifo_power_mode = source.m_fifo_power_mode;
    m_pad_driver_mode = source.m_pad_driver_mode;
    m_bandgap_delay_reg = source.m_bandgap_delay_reg;
    m_madc_delay_reg = source.m_madc_delay_reg;
    m_pll_enable_delay_reg = source.m_pll_enable_delay_reg;
    m_pll_divider_delay_reg = source.m_pll_divider_delay_reg;
    m_dc_correction = source.m_dc_correction;
    m_pullup_configuration = source.m_pullup_configuration;
    m_oscillator_configuration = source.m_oscillator_configuration;
    m_power_sens_delay_reg = source.m_power_sens_delay_reg;
    m_power_sensing_enabled = source.m_power_sensing_enabled;
    m_temperature_sensing_enabled = source.m_temperature_sensing_enabled;
    m_slice_size = source.m_slice_size;
    m_easy_mode_buffer_size = source.m_easy_mode_buffer_size;
    m_reset_state = source.m_reset_state;
    m_current_mode = source.m_current_mode;
    m_current_configuration = source.m_current_configuration;
    m_reg_modifications = source.m_reg_modifications;

    for (unsigned i = 0; i < 16; ++i)
        m_tx_power[i / 2][i & 1] = 0x80000000;

    return *this;
}

// ---------------------------------------------------------------------------- set_reference_clock_frequency
Driver::Error Driver::set_reference_clock_frequency(Reference_Clock_Frequency frequency)
{
    auto& device_traits = Device_Traits::get(m_device_type);

    /* Set internal parameters derived from the reference clock */
    if (frequency == Reference_Clock_Frequency::_80MHz)
    {
        m_reference_clock_freq_Hz = 80000000;
        m_pll_div_set = device_traits.pll_div_set_80M;
        m_enable_frequency_doubler = 0;
    }
    else if (frequency == Reference_Clock_Frequency::_76_8MHz)
    {
        m_reference_clock_freq_Hz = 76800000;
        m_pll_div_set = device_traits.pll_div_set_76M8;
        m_enable_frequency_doubler = 0;
    }
    else if (frequency == Reference_Clock_Frequency::_40MHz)
    {
        if (!device_traits.has_ref_frequency_doubler)
            return Error::UNSUPPORTED_FREQUENCY;

        m_reference_clock_freq_Hz = 80000000;
        m_pll_div_set = device_traits.pll_div_set_80M;
        m_enable_frequency_doubler = 1;
    }
    else if (frequency == Reference_Clock_Frequency::_38_4MHz)
    {
        if (!device_traits.has_ref_frequency_doubler)
            return Error::UNSUPPORTED_FREQUENCY;

        m_reference_clock_freq_Hz = 76800000;
        m_pll_div_set = device_traits.pll_div_set_76M8;
        m_enable_frequency_doubler = 1;
    }
    else
    {
        return Error::UNSUPPORTED_FREQUENCY;
    }

    /* Make sure that frequency range is still in the allowed range */
    uint32_t min_rf_frequency;
    uint32_t max_rf_frequency;
    get_frequency_range(&min_rf_frequency, &max_rf_frequency);
    for (unsigned i = 0; i < 4; ++i)
    {
        if (m_shape[i].lower_frequency_kHz < min_rf_frequency)
            m_shape[i].lower_frequency_kHz = min_rf_frequency;
        else if (m_shape[i].lower_frequency_kHz > max_rf_frequency)
            m_shape[i].lower_frequency_kHz = max_rf_frequency;

        if (m_shape[i].upper_frequency_kHz < min_rf_frequency)
            m_shape[i].upper_frequency_kHz = min_rf_frequency;
        else if (m_shape[i].upper_frequency_kHz > max_rf_frequency)
            m_shape[i].upper_frequency_kHz = max_rf_frequency;
    }

    /* Recalculate the chip configuration */
    update_spi_register_set();

    return Error::OK;
}

// ---------------------------------------------------------------------------- get_reference_clock_frequency
Driver::Error Driver::get_reference_clock_frequency(Reference_Clock_Frequency* frequency) const
{
    if (m_reference_clock_freq_Hz == 80000000)
    {
        *frequency = m_enable_frequency_doubler
                         ? Reference_Clock_Frequency::_40MHz
                         : Reference_Clock_Frequency::_80MHz;
        return Error::OK;
    }
    else if (m_reference_clock_freq_Hz == 76800000)
    {
        *frequency = m_enable_frequency_doubler
                         ? Reference_Clock_Frequency::_38_4MHz
                         : Reference_Clock_Frequency::_76_8MHz;
        return Error::OK;
    }

    return Error::UNSUPPORTED_FREQUENCY;
}

// ---------------------------------------------------------------------------- get_device_info
Driver::Error Driver::get_device_info(Device_Info* device_info) const
{
    auto& device_traits = Device_Traits::get(m_device_type);

    /* features of the sensor */
    /* ---------------------- */
    device_info->description = device_traits.description;
    device_info->num_tx_antennas = device_traits.num_tx_antennas;
    device_info->num_rx_antennas = device_traits.num_rx_antennas;
    device_info->num_temp_sensors = 1;
    device_info->max_tx_power = BGT60TRXX_MAX_TX_POWER;
    get_frequency_range(&device_info->min_rf_frequency_kHz,
                        &device_info->max_rf_frequency_kHz);

    /* data format */
    /* ----------- */
    device_info->interleaved_rx = 1;

    return Error::OK;
}

// ---------------------------------------------------------------------------- get_temperature
Driver::Error Driver::get_temperature(int32_t* temperature_001C) const
{
    Sensor_Meter meter(m_port, m_device_type);

    if (m_reset_state)
    {
        *temperature_001C = int32_t(1000.f * meter.wake_up_and_measure_temperature(*this));
    }
    else
    {
        *temperature_001C = int32_t(1000.f * meter.get_recently_measured_temperature());
    }
    return Error::OK;
}

// ---------------------------------------------------------------------------- get_tx_power
Driver::Error Driver::get_tx_power(uint8_t tx_antenna, int32_t* tx_power_001dBm)
{
    auto& device_traits = Device_Traits::get(m_device_type);

    /* Check number of TX antenna */
    if (tx_antenna >= device_traits.num_tx_antennas)
    {
        return Error::SENSOR_DOES_NOT_EXIST;
    }

    /*
     * when execution comes to this point, antenna test mode is off
     *
     * Basically the last measured values should be returned. The code
     * 0x80000000 indicates that remembered values in invalid and
     * measurement must be repeated.
     * Measurement is only available in reset state, otherwise skip
     * measurement and return this invalid code, which virtually means
     * -infinity.
     */
    auto& requested_tx_power = m_tx_power[m_currently_selected_shape][tx_antenna];

    if ((requested_tx_power == (int32_t)0x80000000) && m_reset_state)
    {
        Sensor_Meter meter(m_port, m_device_type);

        /*
         * In reset state, the sensor must be started using the CW mode.
         * Before a measurement can be done. The measurement is done in
         * two steps, because TX power is not constant over frequency, so
         * TX power is measured at upper and lower frequency and the
         * average voltage difference is converted to dBm.
         */
        const auto& shape = m_shape[m_currently_selected_shape / 2];
        const auto& channel = m_channel_set[m_currently_selected_shape];

        try
        {
            /*
             * TX power can only be measured in continuous wave mode,
             * therefore a continuous wave controller is created and
             * configured according to the parameters of the currently
             * selected chirp.
             */
            Continuous_Wave_Controller cw_controller(m_port, *this);
            cw_controller.enable_tx_antenna(0, (channel.tx_mode == Tx_Mode::Tx1_Only)
                                                   || (channel.tx_mode == Tx_Mode::Alternating));
            if (device_traits.num_tx_antennas >= 2)
            {
                cw_controller.enable_tx_antenna(1, (channel.tx_mode == Tx_Mode::Tx2_Only)
                                                       || (channel.tx_mode == Tx_Mode::Alternating));
            }
            cw_controller.set_continuous_wave_power(unsigned(channel.tx_power));

            // TX power is measured at the lower and upper RF frequency.
            cw_controller.set_continuous_wave_frequency(shape.lower_frequency_kHz * 1000.0);
            cw_controller.enable_continuous_wave(true);
            auto lower_tx_power = cw_controller.measure_tx_power(tx_antenna);

            cw_controller.set_continuous_wave_frequency(shape.upper_frequency_kHz * 1000.0);
            auto upper_tx_power = cw_controller.measure_tx_power(tx_antenna);

            /*
             * TX power may be slightly different at lower and upper
             * frequency. Only one value can be returned, so the average of
             * the TX power at the corner frequencies is calculated.
             * Finally the value is converted to milli dBm.
             */
            if ((lower_tx_power != std::numeric_limits<float>::infinity())
                && (upper_tx_power != std::numeric_limits<float>::infinity()))
            {
                auto tx_power = 0.5f * (lower_tx_power + upper_tx_power);
                requested_tx_power = int32_t(1000.f * tx_power);
            }
        }
        catch (...)
        {
            return Error::INCOMPATIBLE_MODE;
        }
    }
    /* finally return value */
    *tx_power_001dBm = requested_tx_power;

    return Error::OK;
}

// ---------------------------------------------------------------------------- get_chirp_duration
Driver::Error Driver::get_chirp_duration(uint32_t* chirp_duration_ns) const
{
    uint32_t rtu = 0;
    uint32_t rtd = 0;
    uint32_t num_ramp_steps;

    /* extract PLL parameter from currently selected shape */
    /*
     * The currently selected shape and chirp may be disabled. In this case
     * the according registers are not programmed. Assuming zero here is
     * valid: If the chirp is not active, a duration of zero makes sense.
     */
    uint8_t register_offset = 8 * (m_currently_selected_shape / 2);
    if (m_current_configuration.is_defined(BGT60TRxxC_REG_PLL1_2 + register_offset))
    {
        rtu = m_current_configuration[BGT60TRxxC_REG_PLL1_2 + register_offset];
        rtu &= 0x003FFF;
    }
    if (m_current_configuration.is_defined(BGT60TRxxC_REG_PLL1_6 + register_offset))
    {
        rtd = m_current_configuration[BGT60TRxxC_REG_PLL1_6 + register_offset];
        rtd &= 0x003FFF;
    }

    /* now select up or down chirp depending on current settings */
    if (m_currently_selected_shape & 1)
    {
        /* down chirp */
        if ((m_shape[m_currently_selected_shape / 2].shape_type == Shape_Type::Saw_Up)
            || (m_shape[m_currently_selected_shape / 2].shape_type == Shape_Type::Tri_Up))
        {
            num_ramp_steps = rtd;
        }
        else
        {
            num_ramp_steps = rtu;
        }
    }
    else
    {
        /* up chirp */
        if ((m_shape[m_currently_selected_shape / 2].shape_type == Shape_Type::Saw_Up)
            || (m_shape[m_currently_selected_shape / 2].shape_type == Shape_Type::Tri_Up))
        {
            num_ramp_steps = rtu;
        }
        else
        {
            num_ramp_steps = rtd;
        }
    }

    /* one ramp step is 8 cycles */
    *chirp_duration_ns =
        (uint32_t)(cycles_to_100ps(num_ramp_steps * 8,
                                   m_reference_clock_freq_Hz)
                   / 10);

    return Error::OK;
}

// ---------------------------------------------------------------------------- get_min_frame_interval
Driver::Error Driver::get_min_frame_interval(uint32_t* min_frame_interval_us) const
{
    uint64_t num_cycles;

    /* when not in easy mode, this information is not available */
    if ((m_current_mode & MODE_EASY) == 0)
    {
        *min_frame_interval_us = 0;
        return Error::OK;
    }

    /* count the number of cycles per shape */
    /* ------------------------------------ */
    if ((m_shape[0].shape_type == Shape_Type::Tri_Up)
        || (m_shape[0].shape_type == Shape_Type::Tri_Down))
    {
        /* a shape consists of up and down chirp */
        uint32_t reg_value;

        /* pre-chirp delay */
        num_cycles = m_pre_chirp_delay_reg * 8; /* pre-chirp delay */
        num_cycles += 7;                        /* syncing between FSM and PLL */
        num_cycles += 3;                        /* extra cycles from PLL state machine */

        /* post chirp delay */
        num_cycles += m_post_chirp_delay_reg * 8; /* post-chirp delay */
        num_cycles += 5;                          /* extra cycles from PLL state machine */

        /* pre- and post-delay is applied after up and down chirp */
        num_cycles *= 2;

        /* ramp time up chirp */
        reg_value = m_current_configuration[BGT60TRxxC_REG_PLL1_2];
        num_cycles += BGT60TRxxC_EXTRACT(PLL1_2, RTU, reg_value) * 8;

        /* chirp end delay (after up chirp) */
        /* two cycles are for syncing between PLL and FSM */
        /* three cycles for chirp end delay timer in FSM */
        reg_value = BGT60TRxxC_EXTRACT(PLL1_2, T_EDU, reg_value);
        num_cycles += reg_value * 8;
        num_cycles += (reg_value != 0) ? 5 : 2;

        /* ramp time up chirp */
        reg_value = m_current_configuration[BGT60TRxxC_REG_PLL1_6];
        num_cycles += BGT60TRxxC_EXTRACT(PLL1_6, RTD, reg_value) * 8;

        /* chirp end delay (after down chirp) */
        /* two cycles are for syncing between PLL and FSM */
        /* three cycles for chirp end delay timer in FSM */
        reg_value = BGT60TRxxC_EXTRACT(PLL1_6, T_EDD, reg_value);
        num_cycles += reg_value * 8;
        num_cycles += (reg_value != 0) ? 5 : 2;
    }
    else
    {
        /* a shape consists of only one chirp */
        uint32_t reg_value;
        uint32_t fast_down_mode;

        /* pre-chirp delay */
        num_cycles = m_pre_chirp_delay_reg * 8; /* pre-chirp delay */
        num_cycles += 7;                        /* syncing between FSM and PLL */
        num_cycles += 3;                        /* extra cycles from PLL state machine */

        /* post chirp delay */
        num_cycles += m_post_chirp_delay_reg * 8; /* post chirp delay */
        num_cycles += 5;                          /* extra cycles from PLL state machine */

        /* ramp time up chirp */
        reg_value = m_current_configuration[BGT60TRxxC_REG_PLL1_2];
        num_cycles += BGT60TRxxC_EXTRACT(PLL1_2, RTU, reg_value) * 8;

        /* chirp end delay (after up chirp) */
        /* two cycles are for syncing between PLL and FSM */
        /* three cycles for chirp end delay timer in FSM */
        reg_value = BGT60TRxxC_EXTRACT(PLL1_2, T_EDU, reg_value);
        num_cycles += reg_value * 8;
        num_cycles += (reg_value != 0) ? 5 : 2;

        /* chirp end delay (after down chirp) */
        /* two cycles are for syncing between PLL and FSM */
        /* three cycles for chirp end delay timer in FSM */
        reg_value = m_current_configuration[BGT60TRxxC_REG_PLL1_6];
        reg_value = BGT60TRxxC_EXTRACT(PLL1_6, T_EDD, reg_value);
        num_cycles += reg_value * 8;
        num_cycles += (reg_value != 0) ? 5 : 2;

        /* fast down ramp time */
        reg_value = m_current_configuration[BGT60TRxxC_REG_PACR2];
        fast_down_mode = BGT60TRxxC_EXTRACT(PACR2, FSTDNEN, reg_value);

        if (fast_down_mode != 0)
        {
            uint32_t down_cycles = BGT60TRxxC_EXTRACT(PACR2, FSDNTMR,
                                                      reg_value);

            /* set default values hard coded in PLL */
            if (down_cycles == 0)
            {
                down_cycles = (fast_down_mode == 1)   ? 40
                              : (fast_down_mode == 2) ? 56
                              : (fast_down_mode == 3) ? 24
                                                      : 0;
            }
            else
            {
                down_cycles += 1;
            }

            num_cycles += down_cycles;
        }
    }

    /* multiply shape time by number of shapes */
    num_cycles *= m_shape[0].num_repetitions
                  + m_shape[1].num_repetitions
                  + m_shape[2].num_repetitions
                  + m_shape[3].num_repetitions;

    /* add shape end delays (only of next shape group is used) */
    if (m_shape[1].num_repetitions > 0)
    {
        num_cycles += m_shape[0].post_delay.get_clock_cycles();
    }

    if (m_shape[2].num_repetitions > 0)
    {
        num_cycles += m_shape[1].post_delay.get_clock_cycles();
    }

    if (m_shape[3].num_repetitions > 0)
    {
        num_cycles += m_shape[2].post_delay.get_clock_cycles();
    }

    /* add settle time at frame begin */
    num_cycles += m_time_init0.get_clock_cycles()
                  + m_time_init1.get_clock_cycles();

    /* convert to micro seconds (always round up) */
    num_cycles = cycles_to_100ps(num_cycles,
                                 m_reference_clock_freq_Hz);
    *min_frame_interval_us = (uint32_t)(num_cycles / 10000)
                             + ((num_cycles % 10000 != 0) ? 1 : 0);

    return Error::OK;
}

// ---------------------------------------------------------------------------- set_frame_format
Driver::Error Driver::set_frame_format(const Frame_Format* frame_format)
{
    auto& device_traits = Device_Traits::get(m_device_type);

    uint8_t num_antennas;
    Shape_Settings* shape;
    Channel_Set_Settings* channel_set;

    /* check if configuration is valid */
    /* ------------------------------- */

    /* BGT60TRxx has 4 RX antennas, don't try to activate more than this */
    if ((frame_format->rx_mask & ~((1 << device_traits.num_rx_antennas) - 1)) != 0)
    {
        return Error::ANTENNA_DOES_NOT_EXIST;
    }

    /* count number of activated antennas */
    num_antennas = count_antennas(frame_format->rx_mask);

    /* Check number of samples per chirp. */
    if ((frame_format->num_samples_per_chirp > BGT60TRXX_MAX_NUM_SAMPLES)
        || (frame_format->num_samples_per_chirp == 0))
    {
        return Error::UNSUPPORTED_FRAME_FORMAT;
    }

    /*
     * BGT60TR11D state machine gets stuck if rx_mask = 0 and temperature
     * sensing is enabled. If rx_mask is about to become 0, it is made sure
     * that chirp is not enabled.
     */
    if (!device_traits.has_sadc && m_temperature_sensing_enabled
        && (frame_format->rx_mask == 0))
    {
        shape = &m_shape[m_currently_selected_shape / 2];

        if (shape->num_repetitions != 0)
        {
            if ((shape->shape_type != Shape_Type::Saw_Down)
                && ((m_currently_selected_shape & 1) == 0))
                return Error::TEMP_SENSING_WITH_NO_RX;

            if ((shape->shape_type != Shape_Type::Saw_Up)
                && ((m_currently_selected_shape & 1) == 1))
                return Error::TEMP_SENSING_WITH_NO_RX;
        }
    }

    if (m_current_mode & MODE_EASY)
    {
        Fmcw_Configuration fmcw_settings;
        Tx_Mode tx_mode;
        Error error;
        size_t frame_size;

        /* get current settings */
        get_fmcw_configuration(&fmcw_settings);
        get_tx_mode(&tx_mode);

        /* In easy mode, frame size is FIFO slice size, so check the frame
         * size is a multiple of 4 (which is the slice size condition)
         */
        frame_size = frame_format->num_samples_per_chirp * num_antennas
                     * frame_format->num_chirps_per_frame;

        if ((frame_size & 0x03) != 0)
        {
            return Error::UNSUPPORTED_FRAME_FORMAT;
        }

        /* check if frame size is small enough that internal FIFO can at least
         * hold 2 frames.
         */
        if ((frame_size + 2) * 2 > m_easy_mode_buffer_size)
        {
            return Error::UNSUPPORTED_FRAME_FORMAT;
        }

        error = setup_easy_mode_frame(frame_format,
                                      &fmcw_settings,
                                      tx_mode);
        m_slice_size = (uint16_t)frame_size;

        if (error != Error::OK)
        {
            return error;
        }
    }
    else
    {
        /*
         * Number of chirps per frame must be 1. Number of Repetitions is set
         * in radar_setFrameSequence.
         */
        if (frame_format->num_chirps_per_frame != 1)
        {
            return Error::UNSUPPORTED_FRAME_FORMAT;
        }

        /* remember frame format */
        /* --------------------- */
        shape = &m_shape[m_currently_selected_shape / 2];
        channel_set = &m_channel_set[m_currently_selected_shape];

        if (m_currently_selected_shape & 1)
        {
            shape->num_samples_down = (uint16_t)frame_format->num_samples_per_chirp;
        }
        else
        {
            shape->num_samples_up = (uint16_t)frame_format->num_samples_per_chirp;
        }

        channel_set->rx_mask = frame_format->rx_mask;
        channel_set->num_rx_antennas = num_antennas;
    }

    /* update SPI registers of BGT60TR24 chip */
    /* --------------------------------------- */
    update_spi_register_set();

    return Error::OK;
}

// ---------------------------------------------------------------------------- get_frame_format
Driver::Error Driver::get_frame_format(Frame_Format* frame_format) const
{
    /* return current frame format of driver object */
    auto& shape = m_shape[m_currently_selected_shape / 2];
    auto& channel_set = m_channel_set[m_currently_selected_shape];

    frame_format->num_samples_per_chirp =
        (m_currently_selected_shape & 1) ? shape.num_samples_down
                                         : shape.num_samples_up;
    frame_format->rx_mask = channel_set.rx_mask;

    /* get number of chirps per frame */
    if (m_current_mode & MODE_EASY)
    {
        /* Check the case of alternating TX in sawtooth mode
         * (this is only allowed when number of repetitions is 1)
         */
        uint8_t i;
        frame_format->num_chirps_per_frame = 0;

        for (i = 0; i < 4; ++i)
        {
            frame_format->num_chirps_per_frame +=
                m_shape[i].num_repetitions;
        }

        if ((m_shape[0].shape_type == Shape_Type::Tri_Up)
            || (m_shape[0].shape_type == Shape_Type::Tri_Down))
        {
            frame_format->num_chirps_per_frame *= 2;
        }
    }
    else
    {
        frame_format->num_chirps_per_frame = 1;
    }

    return Error::OK;
}

// ---------------------------------------------------------------------------- set_fmcw_configuration
Driver::Error Driver::set_fmcw_configuration(const Fmcw_Configuration* fmcw_settings)
{
    auto& device_traits = Device_Traits::get(m_device_type);

    Shape_Settings* shape;
    Channel_Set_Settings* channel_set;
    Fmcw_Configuration previous_settings;

    /* check if configuration is valid */
    /* ------------------------------- */
    uint32_t min_rf_frequency;
    uint32_t max_rf_frequency;
    get_frequency_range(&min_rf_frequency, &max_rf_frequency);

    if ((fmcw_settings->lower_frequency_kHz < min_rf_frequency)
        || (fmcw_settings->lower_frequency_kHz > max_rf_frequency)
        || (fmcw_settings->upper_frequency_kHz < min_rf_frequency)
        || (fmcw_settings->upper_frequency_kHz > max_rf_frequency)
        || (fmcw_settings->lower_frequency_kHz > fmcw_settings->upper_frequency_kHz))
        return Error::FREQUENCY_OUT_OF_RANGE;

    if ((fmcw_settings->shape_type != Shape_Type::Saw_Up)
        && (fmcw_settings->shape_type != Shape_Type::Saw_Down)
        && (fmcw_settings->shape_type != Shape_Type::Tri_Up)
        && (fmcw_settings->shape_type != Shape_Type::Tri_Down))
        return Error::UNSUPPORTED_DIRECTION;

    if (fmcw_settings->tx_power > BGT60TRXX_MAX_TX_POWER)
        return Error::POWER_OUT_OF_RANGE;

    /*
     * BGT60TR11D state machine gets stuck if rx_mask = 0 and temperature
     * sensing is enabled. Therefore it must be checked if a chirp with
     * rx_mask = 0 gets enabled here.
     */
    if (!device_traits.has_sadc && m_temperature_sensing_enabled)
    {
        if (m_shape[m_currently_selected_shape / 2].num_repetitions != 0)
        {
            if ((fmcw_settings->shape_type != Shape_Type::Saw_Down)
                && (m_channel_set[m_currently_selected_shape & 0xFE].rx_mask == 0))
                return Error::TEMP_SENSING_WITH_NO_RX;

            if ((fmcw_settings->shape_type != Shape_Type::Saw_Up)
                && (m_channel_set[m_currently_selected_shape | 0x01].rx_mask == 0))
                return Error::TEMP_SENSING_WITH_NO_RX;
        }
    }

    /* remember current FMCW settings for the case of an error roll back */
    get_fmcw_configuration(&previous_settings);

    /* remember FMCW configuration */
    /* --------------------------- */
    if (m_current_mode & MODE_EASY)
    {
        Frame_Format frame_format;
        Tx_Mode tx_mode;

        get_tx_mode(&tx_mode);
        get_frame_format(&frame_format);

        Error error = setup_easy_mode_frame(&frame_format,
                                            fmcw_settings,
                                            tx_mode);

        if (error != Error::OK)
        {
            return error;
        }
    }
    else
    {
        shape = &m_shape[m_currently_selected_shape / 2];
        shape->shape_type = fmcw_settings->shape_type;
        shape->lower_frequency_kHz = fmcw_settings->lower_frequency_kHz;
        shape->upper_frequency_kHz = fmcw_settings->upper_frequency_kHz;

        /* set TX power to both, up chirp and down chirp of a shape */
        channel_set = &m_channel_set[m_currently_selected_shape & 0xFE];
        channel_set->tx_power = fmcw_settings->tx_power;
        channel_set = &m_channel_set[m_currently_selected_shape | 0x01];
        channel_set->tx_power = fmcw_settings->tx_power;
    }

    /* finally check if temperature sensing timing constraints are met */
    if (!device_traits.has_sadc && m_temperature_sensing_enabled)
    {
        Error error_code = check_temperature_sens_timing();
        if (error_code != Error::OK)
        {
            set_fmcw_configuration(&previous_settings);
            return error_code;
        }
    }

    /* update SPI registers of BGT60TR24 chip */
    /* --------------------------------------- */
    update_spi_register_set();

    /* tag remembered TX power values as invalid */
    m_tx_power[m_currently_selected_shape & 0xFE][0] = 0x80000000;
    m_tx_power[m_currently_selected_shape & 0xFE][1] = 0x80000000;
    m_tx_power[m_currently_selected_shape | 0x01][0] = 0x80000000;
    m_tx_power[m_currently_selected_shape | 0x01][1] = 0x80000000;

    return Error::OK;
}

// ---------------------------------------------------------------------------- get_fmcw_configuration
Driver::Error Driver::get_fmcw_configuration(Fmcw_Configuration* configuration) const
{
    /* return current FMCW configuration of driver object (return TX power
     * from up chirp, down chirp value is equal) */
    auto& shape = m_shape[m_currently_selected_shape / 2];
    auto& channel_set = m_channel_set[m_currently_selected_shape & 0xFE];

    configuration->shape_type = shape.shape_type;
    configuration->lower_frequency_kHz = shape.lower_frequency_kHz;
    configuration->upper_frequency_kHz = shape.upper_frequency_kHz;
    configuration->tx_power = channel_set.tx_power;

    return Error::OK;
}

// ---------------------------------------------------------------------------- get_bandwidth_per_second
Driver::Error Driver::get_bandwidth_per_second(uint32_t* bandwidth_per_second_MHz_s) const
{
    auto& device_traits = Device_Traits::get(m_device_type);

    int32_t rsu = 0;
    int32_t rsd = 0;
    int32_t ramp_step;

    /* extract PLL parameter from currently selected shape */
    uint8_t register_offset = 8 * (m_currently_selected_shape / 2);
    if (m_current_configuration.is_defined(BGT60TRxxC_REG_PLL1_1 + register_offset))
        rsu = m_current_configuration[BGT60TRxxC_REG_PLL1_1 + register_offset];
    if (m_current_configuration.is_defined(BGT60TRxxC_REG_PLL1_5 + register_offset))
        rsd = m_current_configuration[BGT60TRxxC_REG_PLL1_5 + register_offset];

    if (m_currently_selected_shape & 1)
    {
        const int32_t sign_extension_mask = 0xFF000000;

        /* down chirp */
        if ((m_shape[m_currently_selected_shape / 2].shape_type == Shape_Type::Saw_Up)
            || (m_shape[m_currently_selected_shape / 2].shape_type == Shape_Type::Tri_Up))
        {
            /* replace command byte by sign extension and invert sign */
            ramp_step = -(rsd | sign_extension_mask);
        }
        else
        {
            /* replace command byte by sign extension and invert sign */
            ramp_step = -(rsu | sign_extension_mask);
        }
    }
    else
    {
        /* up chirp */
        if ((m_shape[m_currently_selected_shape / 2].shape_type == Shape_Type::Saw_Up)
            || (m_shape[m_currently_selected_shape / 2].shape_type == Shape_Type::Tri_Up))
        {
            /* ramp step is positive, just remove command byte */
            ramp_step = rsu & 0x00FFFFFF;
        }
        else
        {
            /* ramp step is positive, just remove command byte */
            ramp_step = rsd & 0x00FFFFFF;
        }
    }

    /* convert value according to section 3.4.3 of BGT60TRxx data sheet */
    /*
     * The frequency increment for each cycle is
     * RSU = (fSTEP/(REFERENCE_OSC_FREQ_Hz * PLL_PRE_DIV)) * 2^20. This
     * increment is applied every 1/REFERENCE_OSC_FREQ_Hz seconds.
     * The bandwidth increment in Hz/s is:
     * BWs = (RSU / 2^20) * (REFERENCE_OSC_FREQ_Hz * PLL_PRE_DIV)
     *       * REFERENCE_OSC_FREQ_Hz
     * Do another division by 1e6 to convert to MHz per second.
     */
    double bandwidth = double(ramp_step) / double(1l << 20)
                       * m_reference_clock_freq_Hz
                       * device_traits.pll_pre_divider;

    *bandwidth_per_second_MHz_s = uint32_t(bandwidth
                                           * m_reference_clock_freq_Hz * 1.0e-6);

    return Error::OK;
}

// ---------------------------------------------------------------------------- set_adc_samplerate
Driver::Error Driver::set_adc_samplerate(uint32_t samplerate_Hz)
{
    Adc_Configuration new_configuration;
    new_configuration.samplerate_Hz = samplerate_Hz;
    new_configuration.sample_time = m_adc_sample_time;
    new_configuration.tracking = m_adc_tracking;
    new_configuration.double_msb_time = m_adc_double_msb_time;
    new_configuration.oversampling = m_adc_oversampling;

    return set_adc_configuration(&new_configuration);
}

// ---------------------------------------------------------------------------- get_adc_samplerate
Driver::Error Driver::get_adc_samplerate(uint32_t* samplerate_Hz) const
{
    /* return current ADC sample rate of driver object */
    *samplerate_Hz = ((m_reference_clock_freq_Hz * 2)
                          / (uint32_t)m_adc_sample_rate_divider
                      + 1)
                     / 2;

    return Error::OK;
}

// ---------------------------------------------------------------------------- set_adc_configuration
Driver::Error Driver::set_adc_configuration(const Adc_Configuration* configuration)
{
    auto& device_traits = Device_Traits::get(m_device_type);

    uint16_t min_num_cycles = 0;

    /* remember current parameters for error rollback */
    Adc_Sample_Time prev_sample_time = m_adc_sample_time;
    Adc_Tracking prev_tracking = m_adc_tracking;
    uint8_t prev_double_msb_time = m_adc_double_msb_time;
    Adc_Oversampling prev_oversampling = m_adc_oversampling;
    uint16_t prev_sample_rate_divider = m_adc_sample_rate_divider;

    /* check if configuration is valid */
    /* ------------------------------- */
    /* calculate minimum number of ADC cycles on the fly, it's needed to check
     * sample rate */

    /* check sample time */
    switch (configuration->sample_time)
    {
        case Adc_Sample_Time::_50ns: min_num_cycles += 4; break;
        case Adc_Sample_Time::_100ns: min_num_cycles += 8; break;
        case Adc_Sample_Time::_200ns: min_num_cycles += 16; break;
        case Adc_Sample_Time::_400ns: min_num_cycles += 32; break;
        default:
            return Error::UNDEFINED_ADC_SAMPLE_TIME;
    }

    /* check tracking */
    switch (configuration->tracking)
    {
        case Adc_Tracking::None: min_num_cycles += 0; break;
        case Adc_Tracking::_1_Subconversion: min_num_cycles += 8; break;
        case Adc_Tracking::_3_Subconversions: min_num_cycles += 24; break;
        case Adc_Tracking::_7_Subconversions: min_num_cycles += 56; break;
        default:
            return Error::UNDEFINED_TRACKING_MODE;
    }

    /* add cycles for conversion time */
    min_num_cycles += configuration->double_msb_time ? 17 : 16;

    /* oversampling */
    switch (configuration->oversampling)
    {
        case Adc_Oversampling::Off: break;
        case Adc_Oversampling::_2x: min_num_cycles *= 2; break;
        case Adc_Oversampling::_4x: min_num_cycles *= 4; break;
        case Adc_Oversampling::_8x: min_num_cycles *= 8; break;
        default:
            return Error::UNDEFINED_ADC_OVERSAMPLING;
    }

    /* check ADC sampling rate */
    if (configuration->samplerate_Hz != 0)
    {
        /* calculate divider to generate sample rate from reference
         * oscillator (round value to find the nearest sampling rate) */
        uint16_t sampe_rate_divider =
            (uint16_t)(((m_reference_clock_freq_Hz * 2)
                            / configuration->samplerate_Hz
                        + 1)
                       / 2);

        if ((sampe_rate_divider > BGT60TRXX_MAX_ADC_RATE_DIVIDER)
            || (sampe_rate_divider < min_num_cycles))
        {
            return Error::SAMPLERATE_OUT_OF_RANGE;
        }

        /* divider is in valid range, so remember it */
        m_adc_sample_rate_divider = sampe_rate_divider;
    }
    else
    {
        /* if sampling rate 0 was specified, choose maximum possible
         * sampling rate */
        m_adc_sample_rate_divider = min_num_cycles;
    }

    /* remember configuration */
    /* -----------------------*/
    m_adc_sample_time = configuration->sample_time;
    m_adc_tracking = configuration->tracking;
    m_adc_double_msb_time = configuration->double_msb_time;
    m_adc_oversampling = configuration->oversampling;

    /* finally check if power sensing timing constraints are met */
    if (!device_traits.has_sadc)
    {
        if (m_power_sensing_enabled)
        {
            Error error_code = check_power_sens_timing();
            if (error_code != Error::OK)
            {
                m_adc_sample_time = prev_sample_time;
                m_adc_tracking = prev_tracking;
                m_adc_double_msb_time = prev_double_msb_time;
                m_adc_oversampling = prev_oversampling;
                m_adc_sample_rate_divider = prev_sample_rate_divider;
                return error_code;
            }
        }

        if (m_temperature_sensing_enabled)
        {
            Error error_code = check_temperature_sens_timing();
            if (error_code != Error::OK)
            {
                m_adc_sample_time = prev_sample_time;
                m_adc_tracking = prev_tracking;
                m_adc_double_msb_time = prev_double_msb_time;
                m_adc_oversampling = prev_oversampling;
                m_adc_sample_rate_divider = prev_sample_rate_divider;
                return error_code;
            }
        }
    }

    /* update SPI registers of BGT60TR24 chip */
    /* --------------------------------------- */
    update_spi_register_set();

    return Error::OK;
}

// ---------------------------------------------------------------------------- get_adc_configuration
Driver::Error Driver::get_adc_configuration(Adc_Configuration* configuration) const
{
    /* return current ADC configuration of driver object */
    get_adc_samplerate(&configuration->samplerate_Hz);
    configuration->sample_time = m_adc_sample_time;
    configuration->tracking = m_adc_tracking;
    configuration->double_msb_time = m_adc_double_msb_time;
    configuration->oversampling = m_adc_oversampling;

    return Error::OK;
}

// ---------------------------------------------------------------------------- set_tx_mode
Driver::Error Driver::set_tx_mode(Tx_Mode tx_mode)
{
    auto& device_traits = Device_Traits::get(m_device_type);

    Channel_Set_Settings* channel_set;

    /* check if configuration is valid */
    /* ------------------------------- */
    if ((tx_mode != Tx_Mode::Off)
        && (tx_mode != Tx_Mode::Tx1_Only))
    {
        if (device_traits.num_tx_antennas < 2)
        {
            return Error::UNSUPPORTED_TX_MODE;
        }
        else if ((tx_mode != Tx_Mode::Tx2_Only)
                 && (tx_mode != Tx_Mode::Alternating))
        {
            return Error::UNSUPPORTED_TX_MODE;
        }
    }

    /* in easy mode, do a special treatment */
    /* ------------------------------------ */
    if (m_current_mode & MODE_EASY)
    {
        Fmcw_Configuration fmcw_settings;
        Frame_Format frame_format;
        Error error;

        /* direction can be taken from first shape */
        get_frame_format(&frame_format);
        get_fmcw_configuration(&fmcw_settings);

        error = setup_easy_mode_frame(&frame_format,
                                      &fmcw_settings,
                                      tx_mode);

        if (error != Error::OK)
        {
            return error;
        }
    }
    else
    {
        /* remember TX mode */
        /* ---------------- */
        channel_set = &m_channel_set[m_currently_selected_shape];
        channel_set->tx_mode = tx_mode;
    }

    /* update SPI registers of BGT60TR24 chip */
    /* --------------------------------------- */
    update_spi_register_set();

    /* tag remembered TX power values as invalid */
    m_tx_power[m_currently_selected_shape][0] = 0x80000000;
    m_tx_power[m_currently_selected_shape][1] = 0x80000000;

    return Error::OK;
}

// ---------------------------------------------------------------------------- get_tx_mode
Driver::Error Driver::get_tx_mode(Tx_Mode* tx_mode) const
{
    /* return current TX mode of driver object */
    if (m_current_mode & MODE_EASY)
    {
        /* in easy mode start with TX mode from first shape */
        *tx_mode = m_channel_set[0].tx_mode;

        /* check for alternating mode */
        /* in alternating mode, not all channel sets are configured equally */
        if ((m_channel_set[0].tx_mode != m_channel_set[1].tx_mode)
            || (m_channel_set[0].tx_mode != m_channel_set[2].tx_mode))
        {
            *tx_mode = Tx_Mode::Alternating;
        }
    }
    else
    {
        /* in standard mode, return TX mode from currently selected shape */
        auto& channel_set = m_channel_set[m_currently_selected_shape];
        *tx_mode = channel_set.tx_mode;
    }

    return Error::OK;
}

// ---------------------------------------------------------------------------- set_baseband_configuration
Driver::Error Driver::set_baseband_configuration(const Baseband_Configuration* configuration)
{
    auto& device_traits = Device_Traits::get(m_device_type);

    /* converting reset timer period to register value */
    /* ----------------------------------------------- */
    auto timer_period_reg =
        uint32_t(cycles_from_100ps(configuration->reset_period_100ps,
                                   m_reference_clock_freq_Hz));
    if (device_traits.cs_register_layout != Cs_Layout_t::Version1)
    {
        /*
         * Since BGT60TR13D the base band reset timer bit field counts in steps
         * of 8 cycles rather than cycles.
         */
        timer_period_reg = (timer_period_reg + 4) / 8;
    }

    /* check if configuration is valid */
    /* ------------------------------- */
    if (((configuration->hp_gain_1 != Hp_Gain::_18dB)
         && (configuration->hp_gain_1 != Hp_Gain::_30dB))
        || ((configuration->hp_gain_2 != Hp_Gain::_18dB)
            && (configuration->hp_gain_2 != Hp_Gain::_30dB))
        || ((configuration->hp_gain_3 != Hp_Gain::_18dB)
            && (configuration->hp_gain_3 != Hp_Gain::_30dB))
        || ((configuration->hp_gain_4 != Hp_Gain::_18dB)
            && (configuration->hp_gain_4 != Hp_Gain::_30dB)))
    {
        return Error::UNSUPPORTED_HP_GAIN;
    }

    if (((configuration->vga_gain_1 != Vga_Gain::_0dB)
         && (configuration->vga_gain_1 != Vga_Gain::_5dB)
         && (configuration->vga_gain_1 != Vga_Gain::_10dB)
         && (configuration->vga_gain_1 != Vga_Gain::_15dB)
         && (configuration->vga_gain_1 != Vga_Gain::_20dB)
         && (configuration->vga_gain_1 != Vga_Gain::_25dB)
         && (configuration->vga_gain_1 != Vga_Gain::_30dB))
        || ((configuration->vga_gain_2 != Vga_Gain::_0dB)
            && (configuration->vga_gain_2 != Vga_Gain::_5dB)
            && (configuration->vga_gain_2 != Vga_Gain::_10dB)
            && (configuration->vga_gain_2 != Vga_Gain::_15dB)
            && (configuration->vga_gain_2 != Vga_Gain::_20dB)
            && (configuration->vga_gain_2 != Vga_Gain::_25dB)
            && (configuration->vga_gain_2 != Vga_Gain::_30dB))
        || ((configuration->vga_gain_3 != Vga_Gain::_0dB)
            && (configuration->vga_gain_3 != Vga_Gain::_5dB)
            && (configuration->vga_gain_3 != Vga_Gain::_10dB)
            && (configuration->vga_gain_3 != Vga_Gain::_15dB)
            && (configuration->vga_gain_3 != Vga_Gain::_20dB)
            && (configuration->vga_gain_3 != Vga_Gain::_25dB)
            && (configuration->vga_gain_3 != Vga_Gain::_30dB))
        || ((configuration->vga_gain_4 != Vga_Gain::_0dB)
            && (configuration->vga_gain_4 != Vga_Gain::_5dB)
            && (configuration->vga_gain_4 != Vga_Gain::_10dB)
            && (configuration->vga_gain_4 != Vga_Gain::_15dB)
            && (configuration->vga_gain_4 != Vga_Gain::_20dB)
            && (configuration->vga_gain_4 != Vga_Gain::_25dB)
            && (configuration->vga_gain_4 != Vga_Gain::_30dB)))
    {
        return Error::UNSUPPORTED_VGA_GAIN;
    }

    if (configuration->hp_cutoff_1_Hz < 0
        || configuration->hp_cutoff_2_Hz < 0
        || configuration->hp_cutoff_3_Hz < 0
        || configuration->hp_cutoff_4_Hz < 0)
    {
        return Error::UNSUPPORTED_HP_CUTOFF;
    }

    /* check base band reset timer */
    if (device_traits.cs_register_layout == Cs_Layout_t::Version1)
    {
        if (timer_period_reg > BGT60TRXXC_MAX_BB_RESET_TIMER)
        {
            return Error::RESET_TIMER_OUT_OF_RANGE;
        }
    }
    else
    {
        if (timer_period_reg > BGT60TRXXD_MAX_BB_RESET_TIMER)
        {
            return Error::RESET_TIMER_OUT_OF_RANGE;
        }
    }

    /* remember base band settings */
    /* --------------------------- */
    auto apply = [&](Channel_Set_Settings& chnl_set) -> void {
        chnl_set.hp_gain_1 = configuration->hp_gain_1;
        chnl_set.hp_cutoff_1_reg = cutoff_to_reg(device_traits.hpf_cutoff_settings,
                                                 configuration->hp_cutoff_1_Hz);
        chnl_set.vga_gain_1 = configuration->vga_gain_1;
        chnl_set.hp_gain_2 = configuration->hp_gain_2;
        chnl_set.hp_cutoff_2_reg = cutoff_to_reg(device_traits.hpf_cutoff_settings,
                                                 configuration->hp_cutoff_2_Hz);
        chnl_set.vga_gain_2 = configuration->vga_gain_2;
        chnl_set.hp_gain_3 = configuration->hp_gain_3;
        chnl_set.hp_cutoff_3_reg = cutoff_to_reg(device_traits.hpf_cutoff_settings,
                                                 configuration->hp_cutoff_3_Hz);
        chnl_set.vga_gain_3 = configuration->vga_gain_3;
        chnl_set.hp_gain_4 = configuration->hp_gain_4;
        chnl_set.hp_cutoff_4_reg = cutoff_to_reg(device_traits.hpf_cutoff_settings,
                                                 configuration->hp_cutoff_4_Hz);
        chnl_set.vga_gain_4 = configuration->vga_gain_4;
        chnl_set.reset_period_reg = timer_period_reg;
    };

    if (m_current_mode & MODE_EASY)
    {
        /* In easy mode, settings are applied to all shape sets */
        for (auto& channel_set : m_channel_set)
            apply(channel_set);
    }
    else
    {
        apply(m_channel_set[m_currently_selected_shape]);
    }

    /* update SPI registers of BGT60TR24 chip */
    /* --------------------------------------- */
    update_spi_register_set();

    return Error::OK;
}

// ---------------------------------------------------------------------------- get_baseband_configuration
Driver::Error Driver::get_baseband_configuration(Baseband_Configuration* configuration) const
{
    auto& device_traits = Device_Traits::get(m_device_type);

    /* return current baseband settings of driver object */
    auto& cutoff_values = device_traits.hpf_cutoff_settings;
    auto& chnl_set = m_channel_set[m_currently_selected_shape];
    configuration->hp_gain_1 = chnl_set.hp_gain_1;
    configuration->hp_cutoff_1_Hz = cutoff_values[chnl_set.hp_cutoff_1_reg];
    configuration->vga_gain_1 = chnl_set.vga_gain_1;
    configuration->hp_gain_2 = chnl_set.hp_gain_2;
    configuration->hp_cutoff_2_Hz = cutoff_values[chnl_set.hp_cutoff_2_reg];
    configuration->vga_gain_2 = chnl_set.vga_gain_2;
    configuration->hp_gain_3 = chnl_set.hp_gain_3;
    configuration->hp_cutoff_3_Hz = cutoff_values[chnl_set.hp_cutoff_3_reg];
    configuration->vga_gain_3 = chnl_set.vga_gain_3;
    configuration->hp_gain_4 = chnl_set.hp_gain_4;
    configuration->hp_cutoff_4_Hz = cutoff_values[chnl_set.hp_cutoff_4_reg];
    configuration->vga_gain_4 = chnl_set.vga_gain_4;
    if (device_traits.cs_register_layout == Cs_Layout_t::Version1)
    {
        configuration->reset_period_100ps =
            (uint32_t)cycles_to_100ps(chnl_set.reset_period_reg,
                                      m_reference_clock_freq_Hz);
    }
    else
    {
        configuration->reset_period_100ps =
            (uint32_t)cycles_to_100ps(8 * chnl_set.reset_period_reg,
                                      m_reference_clock_freq_Hz);
    }

    return Error::OK;
}

// ---------------------------------------------------------------------------- set_frame_definition
Driver::Error Driver::set_frame_definition(const Frame_Definition* frame_definition)
{
    auto& device_traits = Device_Traits::get(m_device_type);

    uint32_t num_shape_groups = 0;
    uint8_t shp;
    uint32_t i;
    uint64_t num_cycles;
    Frame_Definition previous_definition;

    /* convert timings into bit field representation */
    Scaled_Timer post_delays[5];

    for (shp = 0; shp < 4; ++shp)
    {
        num_cycles =
            cycles_from_100ps(frame_definition->shapes[shp].post_delay_100ps,
                              m_reference_clock_freq_Hz);
        post_delays[shp] = Scaled_Timer(num_cycles,
                                        BGT60TRXX_MAX_SHAPE_END_DELAY_SHIFT,
                                        BGT60TRXX_MAX_SHAPE_END_DELAY);
    }

    num_cycles =
        cycles_from_100ps(frame_definition->shape_set.post_delay_100ps,
                          m_reference_clock_freq_Hz);
    post_delays[4] = Scaled_Timer(num_cycles,
                                  BGT60TRXX_MAX_SHAPE_END_DELAY_SHIFT,
                                  BGT60TRXX_MAX_SHAPE_END_DELAY);

    /* if easy mode is active, switch to normal mode */
    /* --------------------------------------------- */
    if (m_current_mode & MODE_EASY)
    {
        enable_easy_mode(false);
    }

    /* check if parameters are in valid range */
    /* -------------------------------------- */
    /* check power mode */
    for (shp = 0; shp < 4; ++shp)
    {
        if (((frame_definition->shapes[shp].following_power_mode
              != Power_Mode::Stay_Active)
             && (frame_definition->shapes[shp].following_power_mode
                 != Power_Mode::Idle)
             && (frame_definition->shapes[shp].following_power_mode
                 != Power_Mode::Deep_Sleep)
             && (frame_definition->shapes[shp].following_power_mode
                 != Power_Mode::Deep_Sleep_Continue)))
        {
            return Error::UNSUPPORTED_POWER_MODE;
        }
    }

    if (((frame_definition->shape_set.following_power_mode
          != Power_Mode::Stay_Active)
         && (frame_definition->shape_set.following_power_mode
             != Power_Mode::Idle)
         && (frame_definition->shape_set.following_power_mode
             != Power_Mode::Deep_Sleep)
         && (frame_definition->shape_set.following_power_mode
             != Power_Mode::Deep_Sleep_Continue)))
    {
        return Error::UNSUPPORTED_POWER_MODE;
    }

    /*
     * Workaround:
     * In general DEEP_SLEEP as shape end power mode is not supported. At the
     * end of a frame, DEEP_SLEEP as shape end power mode is OK, because that
     * mode is skipped and replaced by frame end power mode from CCR. Due to
     * a bug in early chip revisions. The frame end power mode is not used and
     * shape end power mode is used at the end of a frame. For this special
     * case, DEEP_SLEEP must be allowed for the last shape of a frame.
     */
    if (((frame_definition->shapes[0].following_power_mode == Power_Mode::Deep_Sleep)
         && ((frame_definition->shapes[1].num_repetitions != 0)
             || (frame_definition->shape_set.num_repetitions > 1)))
        || ((frame_definition->shapes[1].following_power_mode == Power_Mode::Deep_Sleep)
            && ((frame_definition->shapes[2].num_repetitions != 0)
                || (frame_definition->shape_set.num_repetitions > 1)))
        || ((frame_definition->shapes[2].following_power_mode == Power_Mode::Deep_Sleep)
            && ((frame_definition->shapes[3].num_repetitions != 0)
                || (frame_definition->shape_set.num_repetitions > 1)))
        || ((frame_definition->shapes[3].following_power_mode == Power_Mode::Deep_Sleep)
            && (frame_definition->shape_set.num_repetitions > 1)))
    {
        return Error::UNSUPPORTED_POWER_MODE;
    }

    /* check shape end delay */
    if ((post_delays[0].get_shift() > BGT60TRXX_MAX_SHAPE_END_DELAY_SHIFT)
        || (post_delays[1].get_shift() > BGT60TRXX_MAX_SHAPE_END_DELAY_SHIFT)
        || (post_delays[2].get_shift() > BGT60TRXX_MAX_SHAPE_END_DELAY_SHIFT)
        || (post_delays[3].get_shift() > BGT60TRXX_MAX_SHAPE_END_DELAY_SHIFT)
        || (post_delays[4].get_shift() > BGT60TRXX_MAX_SHAPE_END_DELAY_SHIFT))
    {
        return Error::POST_DELAY_OUT_OF_RANGE;
    }

    /* check number of repetitions for shapes 1 */
    if (frame_definition->shapes[0].num_repetitions == 0)
    {
        return Error::UNSUPPORTED_NUM_REPETITIONS;
    }

    num_shape_groups = 0;

    for (shp = 0; shp < 4; ++shp)
    {
        if (frame_definition->shapes[shp].num_repetitions != 0)
        {
            ++num_shape_groups;

            for (i = 1; i <= 32768; i *= 2)
            {
                if (frame_definition->shapes[shp].num_repetitions == i)
                {
                    break;
                }
            }

            if (i > 32768)
            {
                return Error::UNSUPPORTED_NUM_REPETITIONS;
            }
        }
    }

    /* check number of repetitions for shape set */
    num_shape_groups *= frame_definition->shape_set.num_repetitions;

    if (num_shape_groups > BGT60TRXX_MAX_FRAME_LENGTH)
    {
        return Error::UNSUPPORTED_NUM_REPETITIONS;
    }

    /* check if shape sequence is continuous */
    if (((frame_definition->shapes[1].num_repetitions == 0)
         && (frame_definition->shapes[2].num_repetitions != 0))
        || ((frame_definition->shapes[2].num_repetitions == 0)
            && (frame_definition->shapes[3].num_repetitions != 0)))
    {
        return Error::NONCONTINUOUS_SHAPE_SEQUENCE;
    }

    /* check number of frames */
    if (frame_definition->num_frames >= BGT60TRXX_MAX_NUM_FRAMES)
    {
        return Error::NUM_FRAMES_OUT_OF_RANGE;
    }

    /* remember previous settings for the case of an error roll back */
    get_frame_definition(&previous_definition);

    /* remember sequence settings */
    /* -------------------------- */
    for (shp = 0; shp < 4; ++shp)
    {
        m_shape[shp].num_repetitions =
            frame_definition->shapes[shp].num_repetitions;
        m_shape[shp].following_power_mode =
            frame_definition->shapes[shp].following_power_mode;
        m_shape[shp].post_delay = post_delays[shp];
    }

    m_num_set_repetitions = frame_definition->shape_set.num_repetitions;
    m_frame_end_power_mode =
        frame_definition->shape_set.following_power_mode;
    m_frame_end_delay = post_delays[4];
    m_num_frames_before_stop = frame_definition->num_frames;

    /* finally check if temperature sensing timing constraints are met */
    if (!device_traits.has_sadc && m_temperature_sensing_enabled)
    {
        Error error_code = check_temperature_sens_timing();
        if (error_code != Error::OK)
        {
            set_frame_definition(&previous_definition);
            return error_code;
        }
    }

    /* update SPI registers of BGT60TRxx chip */
    /* -------------------------------------- */
    update_spi_register_set();

    return Error::OK;
}

// ---------------------------------------------------------------------------- get_frame_definition
Driver::Error Driver::get_frame_definition(Frame_Definition* frame_definition) const
{
    uint8_t shp;
    uint64_t num_cycles;

    /* return current sequence settings from driver object */
    for (shp = 0; shp < 4; ++shp)
    {
        frame_definition->shapes[shp].num_repetitions =
            m_shape[shp].num_repetitions;
        frame_definition->shapes[shp].following_power_mode =
            m_shape[shp].following_power_mode;
        num_cycles = m_shape[shp].post_delay.get_clock_cycles();
        frame_definition->shapes[shp].post_delay_100ps =
            cycles_to_100ps(num_cycles,
                            m_reference_clock_freq_Hz);
    }

    frame_definition->shape_set.num_repetitions = m_num_set_repetitions;
    frame_definition->shape_set.following_power_mode =
        m_frame_end_power_mode;
    num_cycles = m_frame_end_delay.get_clock_cycles();
    frame_definition->shape_set.post_delay_100ps =
        cycles_to_100ps(num_cycles,
                        m_reference_clock_freq_Hz);
    frame_definition->num_frames = m_num_frames_before_stop;

    return Error::OK;
}

// ---------------------------------------------------------------------------- select_shape_to_configure
Driver::Error Driver::select_shape_to_configure(uint8_t shape, bool down_chirp)
{
    /* check if selected shape is available */
    /* ------------------------------------ */
    if (shape > 4)
    {
        return Error::SHAPE_NUMBER_OUT_OF_RANGE;
    }

    /* if easy mode is active, switch to normal mode */
    /* --------------------------------------------- */
    if (m_current_mode & MODE_EASY)
    {
        enable_easy_mode(false);
    }

    /* remember selected shape */
    /* ----------------------- */
    m_currently_selected_shape = (shape * 2)
                                 | (down_chirp != 0 ? 1 : 0);

    return Error::OK;
}

// ---------------------------------------------------------------------------- get_selected_shape
Driver::Error Driver::get_selected_shape(uint8_t* shape, bool* down_chirp) const
{
    /* return current base band settings from driver object */
    *shape = m_currently_selected_shape / 2;
    *down_chirp = m_currently_selected_shape & 1;

    return Error::OK;
}

// ---------------------------------------------------------------------------- set_chirp_timing
Driver::Error Driver::set_chirp_timing(const Chirp_Timing* timing)
{
    auto& device_traits = Device_Traits::get(m_device_type);

    uint32_t madc_init_time;

    /* remember current parameters for error rollback */
    uint16_t prev_pre_chirp_delay_reg = m_pre_chirp_delay_reg;
    uint16_t prev_post_chirp_delay_reg = m_post_chirp_delay_reg;
    uint16_t prev_pa_delay_reg = m_pa_delay_reg;
    uint8_t prev_adc_delay_reg = m_adc_delay_reg;

    /* convert given time values into target register bit field values */
    /* --------------------------------------------------------------- */
    /*
     * The resolution of the bit field value is 8 clock cycles, so the given
     * value must first be converted to clock cycles and then be divided by 8.
     * Before the division, subtract the additional clock cycles and add 4
     * for rounding to nearest bit field value.
     */
    uint64_t clock_cycles;
    uint32_t pre_chirp_delay_reg = 0;
    uint32_t post_chirp_delay_reg = 0;
    uint32_t pa_delay_reg = 0;
    uint32_t adc_delay_reg = 0;

    clock_cycles = cycles_from_100ps(timing->pre_chirp_delay_100ps,
                                     m_reference_clock_freq_Hz);
    if (clock_cycles >= 10)
    {
        clock_cycles -= 10;
        pre_chirp_delay_reg = ((uint32_t)clock_cycles + 4) / 8;
    }

    clock_cycles = cycles_from_100ps(timing->post_chirp_delay_100ps,
                                     m_reference_clock_freq_Hz);
    if (clock_cycles >= 5)
    {
        clock_cycles -= 5;
        post_chirp_delay_reg = ((uint32_t)clock_cycles + 4) / 8;
    }

    clock_cycles = cycles_from_100ps(timing->pa_delay_100ps,
                                     m_reference_clock_freq_Hz);
    pa_delay_reg = ((uint32_t)clock_cycles + 4) / 8;

    clock_cycles = cycles_from_100ps(timing->adc_delay_100ps,
                                     m_reference_clock_freq_Hz);
    if (clock_cycles >= 1)
    {
        clock_cycles -= 1;
        adc_delay_reg = ((uint32_t)clock_cycles + 4) / 8;
    }

    /* check if all parameters are in valid range */
    /* ------------------------------------------ */
    if (pre_chirp_delay_reg > BGT60TRXX_MAX_PRE_CHIRP_DELAY_100ns)
    {
        return Error::PRECHIRPDELAY_OUT_OF_RANGE;
    }

    if (post_chirp_delay_reg > BGT60TRXX_MAX_POST_CHIRP_DELAY_100ns)
    {
        return Error::POSTCHIRPDELAY_OUT_OF_RANGE;
    }

    if ((pa_delay_reg > BGT60TRXX_MAX_PA_DELAY_100ns)
        || (pa_delay_reg == 0))
    {
        return Error::PADELAY_OUT_OF_RANGE;
    }

    if (!device_traits.has_extra_startup_delays)
    {
        if (adc_delay_reg > BGT60TRXXC_MAX_ADC_DELAY_100ns)
            return Error::ADCDELAY_OUT_OF_RANGE;
    }
    else
    {
        if (adc_delay_reg > BGT60TRXXD_MAX_ADC_DELAY_100ns)
            return Error::ADCDELAY_OUT_OF_RANGE;
    }

    /* pa must not go active before the ramp starts */
    if ((8 * pa_delay_reg) < (8 * pre_chirp_delay_reg + 10))
    {
        return Error::PRECHIRP_EXCEEDS_PADELAY;
    }

    madc_init_time = (uint32_t)m_time_init0.get_clock_cycles()
                     + (uint32_t)m_time_init1.get_clock_cycles()
                     + 8 * pa_delay_reg
                     + 8 * adc_delay_reg + 1;
    if (device_traits.has_extra_startup_delays)
        madc_init_time -= (64 * m_madc_delay_reg + 1);

    if (madc_init_time < BGT60TRXX_MADC_STARTUP_CYCLES)
    {
        return Error::MADC_DELAY_OUT_OF_RANGE;
    }

    /* remember chirp timing */
    /* --------------------- */
    m_pre_chirp_delay_reg = (uint16_t)pre_chirp_delay_reg;
    m_post_chirp_delay_reg = (uint16_t)post_chirp_delay_reg;
    m_pa_delay_reg = (uint16_t)pa_delay_reg;
    m_adc_delay_reg = (uint8_t)adc_delay_reg;

    /* finally check if power sensing timing constraints are met */
    if (!device_traits.has_sadc)
    {
        if (m_power_sensing_enabled)
        {
            Error error_code = check_power_sens_timing();
            if (error_code != Error::OK)
            {
                m_pre_chirp_delay_reg = prev_pre_chirp_delay_reg;
                m_post_chirp_delay_reg = prev_post_chirp_delay_reg;
                m_pa_delay_reg = prev_pa_delay_reg;
                m_adc_delay_reg = prev_adc_delay_reg;
                return error_code;
            }
        }

        if (m_temperature_sensing_enabled)
        {
            Error error_code = check_temperature_sens_timing();
            if (error_code != Error::OK)
            {
                m_pre_chirp_delay_reg = prev_pre_chirp_delay_reg;
                m_post_chirp_delay_reg = prev_post_chirp_delay_reg;
                m_pa_delay_reg = prev_pa_delay_reg;
                m_adc_delay_reg = prev_adc_delay_reg;
                return error_code;
            }
        }
    }

    /* update SPI registers of BGT60TR24 chip */
    /* --------------------------------------- */
    update_spi_register_set();

    return Error::OK;
}

// ---------------------------------------------------------------------------- get_chirp_timing
Driver::Error Driver::get_chirp_timing(Chirp_Timing* timing) const
{
    uint64_t num_cycles;

    /*
     * One step in the register bit fields means 8 clock cycles, so multiply
     * each bit field value by 8 and then add additional clock cycles.
     */
    num_cycles = 8 * m_pre_chirp_delay_reg + 10;
    timing->pre_chirp_delay_100ps =
        (uint32_t)cycles_to_100ps(num_cycles,
                                  m_reference_clock_freq_Hz);

    num_cycles = 8 * m_post_chirp_delay_reg + 5;
    timing->post_chirp_delay_100ps =
        (uint32_t)cycles_to_100ps(num_cycles,
                                  m_reference_clock_freq_Hz);

    num_cycles = 8 * m_pa_delay_reg;
    timing->pa_delay_100ps =
        (uint32_t)cycles_to_100ps(num_cycles,
                                  m_reference_clock_freq_Hz);

    num_cycles = 8 * m_adc_delay_reg + 1;
    timing->adc_delay_100ps =
        (uint16_t)cycles_to_100ps(num_cycles,
                                  m_reference_clock_freq_Hz);

    return Error::OK;
}

// ---------------------------------------------------------------------------- set_startup_timing
Driver::Error Driver::set_startup_timing(const Startup_Timing* timing)
{
    auto& device_traits = Device_Traits::get(m_device_type);

    uint32_t madc_init_time;

    /* convert timings into bit field representation */
    uint64_t num_cycles;
    Scaled_Timer time_wake_up;
    Scaled_Timer time_init0;
    Scaled_Timer time_init1;

    num_cycles = cycles_from_100ps(timing->wake_up_time_100ps,
                                   m_reference_clock_freq_Hz);
    time_wake_up = Scaled_Timer(num_cycles,
                                BGT60TRXX_MAX_WAKEUP_COUNTER_SHIFT,
                                BGT60TRXX_MAX_WAKEUP_COUNTER);

    num_cycles = cycles_from_100ps(timing->pll_settle_time_coarse_100ps,
                                   m_reference_clock_freq_Hz);
    time_init0 = Scaled_Timer(num_cycles,
                              BGT60TRXX_MAX_PLL_INIT_COUNTER_SHIFT,
                              !device_traits.has_extra_startup_delays
                                  ? BGT60TRXXC_MAX_PLL_INIT0_COUNTER
                                  : BGT60TRXXD_MAX_PLL_INIT0_COUNTER);

    num_cycles = cycles_from_100ps(timing->pll_settle_time_fine_100ps,
                                   m_reference_clock_freq_Hz);
    time_init1 = Scaled_Timer(num_cycles,
                              BGT60TRXX_MAX_PLL_INIT_COUNTER_SHIFT,
                              BGT60TRXX_MAX_PLL_INIT1_COUNTER);

    /* check if all parameters are in valid range */
    /* ------------------------------------------ */
    if (time_wake_up.get_shift() > BGT60TRXX_MAX_WAKEUP_COUNTER_SHIFT)
    {
        return Error::WAKEUPTIME_OUT_OF_RANGE;
    }

    if ((time_init0.get_shift() > BGT60TRXX_MAX_PLL_INIT_COUNTER_SHIFT)
        || (time_init1.get_shift() > BGT60TRXX_MAX_PLL_INIT_COUNTER_SHIFT))
    {
        return Error::SETTLETIME_OUT_OF_RANGE;
    }

    if (device_traits.has_extra_startup_delays)
    {
        if (64 * m_bandgap_delay_reg > 8 * time_wake_up.get_counter() << time_wake_up.get_shift())
        {
            return Error::BANDGAP_DELAY_OUT_OF_RANGE;
        }

        if (64 * m_madc_delay_reg > 8 * time_init0.get_counter() << time_init0.get_shift())
        {
            return Error::MADC_DELAY_OUT_OF_RANGE;
        }

        if (64 * m_pll_enable_delay_reg + BGT60TRXXD_MIN_IDLE_INIT0_CYCLES > 8 * time_init0.get_counter() << time_init0.get_shift())
        {
            return Error::PLL_ENABLE_DELAY_OUT_OF_RANGE;
        }

        if (32 * m_pll_divider_delay_reg + BGT60TRXXD_MIN_IDLE_INIT1_CYCLES > 8 * time_init1.get_counter() << time_init1.get_shift())
        {
            return Error::PLL_DIVIDER_DELAY_OUT_OF_RANGE;
        }
    }

    madc_init_time = (uint32_t)time_init0.get_clock_cycles()
                     + (uint32_t)time_init1.get_clock_cycles()
                     + 8 * m_pa_delay_reg
                     + 8 * m_adc_delay_reg + 1;
    if (device_traits.has_extra_startup_delays)
        madc_init_time -= (64 * m_madc_delay_reg + 1);

    if (madc_init_time < BGT60TRXX_MADC_STARTUP_CYCLES)
    {
        return Error::MADC_DELAY_OUT_OF_RANGE;
    }

    /* remember chirp timing */
    /* --------------------- */
    m_time_wake_up = time_wake_up;
    m_time_init0 = time_init0;
    m_time_init1 = time_init1;

    /* update SPI registers of BGT60TR24 chip */
    /* --------------------------------------- */
    update_spi_register_set();

    return Error::OK;
}

// ---------------------------------------------------------------------------- get_startup_timing
Driver::Error Driver::get_startup_timing(Startup_Timing* timing) const
{
    uint64_t clock_cycles;

    /* return current wake up timing of driver object */
    clock_cycles = m_time_wake_up.get_clock_cycles();
    timing->wake_up_time_100ps =
        cycles_to_100ps(clock_cycles, m_reference_clock_freq_Hz);

    clock_cycles = m_time_init0.get_clock_cycles();
    timing->pll_settle_time_coarse_100ps =
        (uint32_t)cycles_to_100ps(clock_cycles,
                                  m_reference_clock_freq_Hz);

    clock_cycles = m_time_init1.get_clock_cycles();
    timing->pll_settle_time_fine_100ps =
        (uint32_t)cycles_to_100ps(clock_cycles,
                                  m_reference_clock_freq_Hz);

    return Error::OK;
}

// ---------------------------------------------------------------------------- set_chirp_end_delay
Driver::Error Driver::set_chirp_end_delay(uint32_t delay_100ps)
{
    auto& device_traits = Device_Traits::get(m_device_type);

    Shape_Settings* shape;

    /* convert delay into register bit field value */
    /* ------------------------------------------- */
    /*
     * The resolution of the bit field value is 8 clock cycles, so the given
     * value must first be converted to clock cycles and then be divided by 8.
     * Before the division, additional clock cycles added by the FSM must be
     * subtracted. This requires some special treatment because the number of
     * additional clock cycles is 5 for non-zero bit fields values and 2 if the
     * bit field value is zero.
     */
    uint8_t previous_value;
    uint8_t reg_value;
    uint32_t clock_cycles =
        (uint32_t)cycles_from_100ps(delay_100ps,
                                    m_reference_clock_freq_Hz);
    if (clock_cycles > 8)
    {
        /*
         * This is the generic conversion formula. Before the division 4 clock
         * cycles are added for rounding to the nearest value. Afterwards the
         * result is limited to the maximum bit field value 255.
         */
        uint32_t bitfield_value;
        clock_cycles -= 5;
        bitfield_value = (clock_cycles + 4) / 8;
        reg_value = (bitfield_value < 255) ? (uint8_t)bitfield_value : 255;
    }
    else
    {
        /*
         * The generic formula does not round 8 clock cycles up to 13 to
         * achieve a bit field value of 1, so a special case is needed to treat
         * that value.
         * Values less than 8 are rounded down to 2 clock cycles which are
         * represented by a bit field value of 2;
         */
        reg_value = (clock_cycles == 8) ? 1 : 0;
    }

    /* remember old value for the case of an error rollback */
    shape = &m_shape[m_currently_selected_shape / 2];
    if (m_currently_selected_shape & 1)
        previous_value = shape->chirp_end_delay_down_reg;
    else
        previous_value = shape->chirp_end_delay_up_reg;

    /* remember chirp end delay */
    /* ------------------------ */
    if (m_current_mode & MODE_EASY)
    {
        /* In easy mode, settings are applied to all shape sets */
        uint8_t i;

        for (i = 0; i < 4; ++i)
        {
            m_shape[i].chirp_end_delay_down_reg = reg_value;
            m_shape[i].chirp_end_delay_up_reg = reg_value;
        }
    }
    else
    {
        if (m_currently_selected_shape & 1)
            shape->chirp_end_delay_down_reg = reg_value;
        else
            shape->chirp_end_delay_up_reg = reg_value;
    }

    /* finally check if temperature sensing timing constraints are met */
    if (!device_traits.has_sadc && m_temperature_sensing_enabled)
    {
        Error error_code = check_temperature_sens_timing();
        if (error_code != Error::OK)
        {
            if (m_current_mode & MODE_EASY)
            {
                /* In easy mode, settings are applied to all shape sets */
                uint8_t i;
                for (i = 0; i < 4; ++i)
                {
                    m_shape[i].chirp_end_delay_down_reg = previous_value;
                    m_shape[i].chirp_end_delay_up_reg = previous_value;
                }
            }
            else
            {
                if (m_currently_selected_shape & 1)
                    shape->chirp_end_delay_down_reg = previous_value;
                else
                    shape->chirp_end_delay_up_reg = previous_value;
            }
            return error_code;
        }
    }

    /* update SPI registers of BGT60TR24 chip */
    /* --------------------------------------- */
    update_spi_register_set();

    return Error::OK;
}

// ---------------------------------------------------------------------------- get_chirp_end_delay
Driver::Error Driver::get_chirp_end_delay(uint32_t* delay_100ps) const
{
    /* get bit field value from driver object */
    auto& shape = m_shape[m_currently_selected_shape / 2];

    uint32_t bitfield_value = (m_currently_selected_shape & 1) ? shape.chirp_end_delay_down_reg : shape.chirp_end_delay_up_reg;

    /*
     * One step in the register bit fields means 8 clock cycles. Add 2 or 5
     * clock cycles that are added by FSM.
     */
    uint32_t clock_cycles = bitfield_value * 8;
    clock_cycles += (bitfield_value != 0) ? 5 : 2;

    *delay_100ps =
        (uint32_t)cycles_to_100ps(clock_cycles,
                                  m_reference_clock_freq_Hz);

    return Error::OK;
}

// ---------------------------------------------------------------------------- set_idle_configuration
Driver::Error Driver::set_idle_configuration(const Power_Down_Configuration* configuration)
{
    /* remember deep sleep settings */
    /* ---------------------------- */
    m_idle_settings = *configuration;

    /* update SPI registers of BGT60TR24 chip */
    /* --------------------------------------- */
    update_spi_register_set();

    return Error::OK;
}

// ---------------------------------------------------------------------------- get_idle_configuration
Driver::Error Driver::get_idle_configuration(Power_Down_Configuration* configuration) const
{
    /* return current idle settings of driver object */
    *configuration = m_idle_settings;

    return Error::OK;
}

// ---------------------------------------------------------------------------- set_deep_sleep_configuration
Driver::Error Driver::set_deep_sleep_configuration(const Power_Down_Configuration* configuration)
{
    /* remember deep sleep settings */
    /* ---------------------------- */
    m_deep_sleep_settings = *configuration;

    /* update SPI registers of BGT60TR24 chip */
    /* --------------------------------------- */
    update_spi_register_set();

    return Error::OK;
}

// ---------------------------------------------------------------------------- get_deep_sleep_configuration
Driver::Error Driver::get_deep_sleep_configuration(Power_Down_Configuration* configuration) const
{
    /* return current deep sleep settings of driver object */
    *configuration = m_deep_sleep_settings;

    return Error::OK;
}

// ---------------------------------------------------------------------------- repeat_chip_setup
Driver::Error Driver::repeat_chip_setup()
{
    auto& device_traits = Device_Traits::get(m_device_type);

    uint32_t read_back_registers[BGT60TRxxE_NUM_REGISTERS];
    uint8_t num_registers;

    /* do a chip reset to be in a clean state */
    reset(false);

    /* send all remembered register values again */
    auto configuration = get_device_configuration();
    configuration.send_to_device(m_port, false);

    /* now do a complete register read back */
    num_registers = device_traits.num_registers;
    dump_registers(read_back_registers, &num_registers);

    /* compare if all registers have the programmed value */
    for (uint8_t i = 0; i < num_registers; ++i)
    {
        /* now compare the value */
        if (configuration.is_defined(i))
        {
            uint32_t expected_value = configuration[i];
            if (((expected_value ^ read_back_registers[i]) & 0x00FFFFFF) != 0)
                return Error::CHIP_SETUP_FAILED;
        }
    }

    return Error::OK;
}

// ---------------------------------------------------------------------------- set_startup_delays
Driver::Error Driver::set_startup_delays(const Startup_Delays* delays)
{
    auto& device_traits = Device_Traits::get(m_device_type);

    uint64_t clock_cycles;
    uint8_t bandgap_delay_reg = 0;
    uint8_t madc_delay_reg = 0;
    uint8_t pll_enable_delay_reg = 0;
    uint8_t pll_divider_delay_reg = 0;
    uint32_t madc_init_time;

    /* This feature is not supported before BGT60TR13D */
    if (!device_traits.has_extra_startup_delays)
        return Error::FEATURE_NOT_SUPPORTED;

    /* convert given delay values into target register bit field values */
    /* ---------------------------------------------------------------- */
    /*
     * The resolution of the bit field value is 64 clock cycles, so the given
     * value must first be converted to clock cycles and then be divided by 64.
     * Before the division, subtract the additional clock cycles and add 32
     * for rounding to nearest bit field value.
     */
    clock_cycles = cycles_from_100ps(delays->bandgap_100ps,
                                     m_reference_clock_freq_Hz);
    if (clock_cycles >= 2)
    {
        clock_cycles -= 2;
        bandgap_delay_reg = (uint8_t)(((uint32_t)clock_cycles + 32) / 64);
    }

    clock_cycles = cycles_from_100ps(delays->madc_100ps,
                                     m_reference_clock_freq_Hz);
    if (clock_cycles >= 1)
    {
        clock_cycles -= 1;
        madc_delay_reg = (uint8_t)(((uint32_t)clock_cycles + 32) / 64);
    }

    clock_cycles = cycles_from_100ps(delays->pll_enable_100ps,
                                     m_reference_clock_freq_Hz);
    if (clock_cycles >= 2)
    {
        clock_cycles -= 2;
        pll_enable_delay_reg = (uint8_t)(((uint32_t)clock_cycles + 32) / 64);
    }

    /*
     * The resolution of the bit field value is 32 clock cycles, so the given
     * value must first be converted to clock cycles and then be divided by 32.
     * Before the division, subtract the additional clock cycles and add 16
     * for rounding to nearest bit field value.
     */
    clock_cycles = cycles_from_100ps(delays->pll_divider_100ps,
                                     m_reference_clock_freq_Hz);
    if (clock_cycles >= 1)
    {
        clock_cycles -= 1;
        pll_divider_delay_reg = (uint8_t)(((uint32_t)clock_cycles + 16) / 32);
    }

    /* check if all parameters are in valid range */
    /* ------------------------------------------ */
    if (bandgap_delay_reg > BGT60TRXXD_MAX_BANDGAP_DELAY)
    {
        return Error::BANDGAP_DELAY_OUT_OF_RANGE;
    }

    if (madc_delay_reg > BGT60TRXXD_MAX_MADC_DELAY)
    {
        return Error::MADC_DELAY_OUT_OF_RANGE;
    }

    if (pll_enable_delay_reg > BGT60TRXXD_MAX_PLL_ENABLE_DELAY)
    {
        return Error::PLL_ENABLE_DELAY_OUT_OF_RANGE;
    }

    if (pll_divider_delay_reg > BGT60TRXXD_MAX_PLL_DIVIDER_DELAY)
    {
        return Error::PLL_DIVIDER_DELAY_OUT_OF_RANGE;
    }

    if (64 * bandgap_delay_reg > 8 * m_time_wake_up.get_counter() << m_time_wake_up.get_shift())
    {
        return Error::BANDGAP_DELAY_OUT_OF_RANGE;
    }

    if (64 * madc_delay_reg > 8 * m_time_init0.get_counter() << m_time_init0.get_shift())
    {
        return Error::MADC_DELAY_OUT_OF_RANGE;
    }

    if (64 * pll_enable_delay_reg + BGT60TRXXD_MIN_IDLE_INIT0_CYCLES > 8 * m_time_init0.get_counter() << m_time_init0.get_shift())
    {
        return Error::PLL_ENABLE_DELAY_OUT_OF_RANGE;
    }

    if (32 * pll_divider_delay_reg + BGT60TRXXD_MIN_IDLE_INIT1_CYCLES > 8 * m_time_init1.get_counter() << m_time_init1.get_shift())
    {
        return Error::PLL_DIVIDER_DELAY_OUT_OF_RANGE;
    }

    madc_init_time = (uint32_t)m_time_init0.get_clock_cycles()
                     + (uint32_t)m_time_init1.get_clock_cycles()
                     + 8 * m_pa_delay_reg
                     + 8 * m_adc_delay_reg + 1
                     - (64 * madc_delay_reg + 1);

    if (madc_init_time < BGT60TRXX_MADC_STARTUP_CYCLES)
    {
        return Error::MADC_DELAY_OUT_OF_RANGE;
    }

    /* remember chirp timing */
    /* --------------------- */
    m_bandgap_delay_reg = bandgap_delay_reg;
    m_madc_delay_reg = madc_delay_reg;
    m_pll_enable_delay_reg = pll_enable_delay_reg;
    m_pll_divider_delay_reg = pll_divider_delay_reg;

    /* update SPI registers of BGT60TR24 chip */
    /* --------------------------------------- */
    update_spi_register_set();

    return Error::OK;
}

// ---------------------------------------------------------------------------- get_startup_delays
Driver::Error Driver::get_startup_delays(Startup_Delays* delays) const
{
    uint64_t num_cycles;

    /*
     * One step in the register bit fields means 64 clock cycles, so multiply
     * by 64 and then add an additional clock cycle. Note that the state
     * machine adds another additional clock cycle, if the bit field values is
     * non zero.
     */
    num_cycles = 64 * m_bandgap_delay_reg
                 + ((m_bandgap_delay_reg != 0) ? 2 : 1);
    delays->bandgap_100ps =
        (uint16_t)cycles_to_100ps(num_cycles,
                                  m_reference_clock_freq_Hz);

    num_cycles = 64 * m_madc_delay_reg
                 + ((m_madc_delay_reg != 0) ? 1 : 0);
    delays->madc_100ps =
        (uint16_t)cycles_to_100ps(num_cycles,
                                  m_reference_clock_freq_Hz);

    num_cycles = 64 * m_pll_enable_delay_reg
                 + ((m_pll_enable_delay_reg != 0) ? 2 : 1);
    delays->pll_enable_100ps =
        (uint32_t)cycles_to_100ps(num_cycles,
                                  m_reference_clock_freq_Hz);

    /*
     * One step in the register bit fields means 32 clock cycles, so multiply
     * by 32 and then add one additional clock cycle.
     */
    num_cycles = 32 * m_pll_divider_delay_reg + 1;
    delays->pll_divider_100ps =
        (uint16_t)cycles_to_100ps(num_cycles,
                                  m_reference_clock_freq_Hz);

    return Error::OK;
}

// ---------------------------------------------------------------------------- set_anti_alias_filter_settings
Driver::Error Driver::set_anti_alias_filter_settings(const Anti_Alias_Filter_Settings* settings)
{
    if (settings->frequency1_Hz < 0
        || settings->frequency2_Hz < 0
        || settings->frequency3_Hz < 0
        || settings->frequency4_Hz < 0)
    {
        return Error::UNSUPPORTED_AAF_CUTOFF;
    }

    auto& device_traits = Device_Traits::get(m_device_type);

    /* remember settings */
    /* ----------------- */
    auto apply = [&](Channel_Set_Settings& chnl_set) -> void {
        chnl_set.aaf_cutoff_1_reg = cutoff_to_reg(device_traits.aaf_cutoff_settings,
                                                  settings->frequency1_Hz);
        chnl_set.aaf_cutoff_2_reg = cutoff_to_reg(device_traits.aaf_cutoff_settings,
                                                  settings->frequency2_Hz);
        chnl_set.aaf_cutoff_3_reg = cutoff_to_reg(device_traits.aaf_cutoff_settings,
                                                  settings->frequency3_Hz);
        chnl_set.aaf_cutoff_4_reg = cutoff_to_reg(device_traits.aaf_cutoff_settings,
                                                  settings->frequency4_Hz);
    };

    if (m_current_mode & MODE_EASY)
    {
        /* In easy mode, settings are applied to all channel sets */
        for (auto& channel_set : m_channel_set)
            apply(channel_set);
    }
    else
    {
        apply(m_channel_set[m_currently_selected_shape]);
    }

    /* update SPI registers of BGT60TR24 chip */
    /* --------------------------------------- */
    update_spi_register_set();

    return Error::OK;
}

// ---------------------------------------------------------------------------- get_anti_alias_filter_settings
Driver::Error Driver::get_anti_alias_filter_settings(Anti_Alias_Filter_Settings* settings) const
{
    auto& device_traits = Device_Traits::get(m_device_type);

    /* return current phase settings of driver object */
    auto& cutoff_values = device_traits.aaf_cutoff_settings;
    auto& chnl_set = m_channel_set[m_currently_selected_shape];
    settings->frequency1_Hz = cutoff_values[chnl_set.aaf_cutoff_1_reg];
    settings->frequency2_Hz = cutoff_values[chnl_set.aaf_cutoff_2_reg];
    settings->frequency3_Hz = cutoff_values[chnl_set.aaf_cutoff_3_reg];
    settings->frequency4_Hz = cutoff_values[chnl_set.aaf_cutoff_4_reg];

    return Error::OK;
}

// ---------------------------------------------------------------------------- get_device_id
Driver::Error Driver::get_device_id(uint64_t* device_id) const
{
    auto& device_traits = Device_Traits::get(m_device_type);

    /* This feature is not supported before BGT60ATR24C */
    if (!device_traits.has_device_id)
        return Error::FEATURE_NOT_SUPPORTED;

    /*
     * Reading device ID is more than just reading some registers. The E-Fuse
     * control logic is also involved which needs a running clock.
     */
    initialize_reference_clock(m_port, get_clock_config_command());

    /*
     * Read the Efuses into SPI registers as described in chapter 10.1
     * of BGT60TR13D data sheet. In preparation the high speed compensation
     * SPI mode is configured according to the needs of the Avian port.
     * Otherwise the register values may not be read correctly.
     */
    bool needs_high_speed = m_port.get_properties().high_speed_compensation;
    std::array<HW::Spi_Command_t, 2> spi_words;
    spi_words[0] = BGT60TRxxC_SET(SFCTL, MISO_HF_READ,
                                  needs_high_speed ? 1 : 0);
    spi_words[1] = BGT60TRxxD_SET(DFT0, EFUSE_EN, 1);
    m_port.send_commands(spi_words.data(), 2);

    spi_words[1] |= BGT60TRxxD_SET(DFT0, EFUSE_SENSE, 1);
    m_port.send_commands(&spi_words[1], 1);

    /* wait until EFUSES are read */
    spi_words[0] = BGT60TRxxD_REGISTER_READ_CMD(DFT1);
    for (unsigned i = 0; i < 1000; ++i)
    {
        /* read the ready bit */
        m_port.send_commands(&spi_words[0], 1, &spi_words[1]);

        if (BGT60TRxxD_EXTRACT(DFT1, EFUSE_READY, spi_words[1]) == 1)
            break;
    }

    if (BGT60TRxxD_EXTRACT(DFT1, EFUSE_READY, spi_words[1]) != 1)
    {
        return Error::TIME_OUT;
    }

    /*
     * read the two chip ID registers from BGT60TRxxD and merge them into an
     * 48 bit word
     */
    if (device_traits.has_reordered_register_layout)
    {
        spi_words[0] = BGT60TRxxE_REGISTER_READ_CMD(DEV_ID0);
        spi_words[1] = BGT60TRxxE_REGISTER_READ_CMD(DEV_ID1);
    }
    else
    {
        spi_words[0] = BGT60TRxxD_REGISTER_READ_CMD(DEV_ID0);
        spi_words[1] = BGT60TRxxD_REGISTER_READ_CMD(DEV_ID1);
    }
    m_port.send_commands(spi_words.data(), 2, spi_words.data());

    *device_id = BGT60TRxxD_EXTRACT(DEV_ID0, DEVICE_ID, spi_words[0]);
    *device_id <<= 24;
    *device_id |= BGT60TRxxD_EXTRACT(DEV_ID1, DEVICE_ID, spi_words[1]);

    /* Turn off EFUSEs */
    spi_words[0] = BGT60TRxxD_SET(DFT0, EFUSE_EN, 0);
    m_port.send_commands(&spi_words[0], 1);

    return Error::OK;
}

// ---------------------------------------------------------------------------- set_fifo_power_mode
Driver::Error Driver::set_fifo_power_mode(Fifo_Power_Mode mode)
{
    auto& device_traits = Device_Traits::get(m_device_type);

    /* This feature is not supported before BGT60TR13D */
    if (!device_traits.has_programmable_fifo_power_mode)
        return Error::FEATURE_NOT_SUPPORTED;

    /* check if parameter is valid */
    /* --------------------------- */
    if ((mode != Fifo_Power_Mode::Always_On)
        && (mode != Fifo_Power_Mode::Deep_Sleep_Off)
        && (mode != Fifo_Power_Mode::Deep_Sleep_And_Idle_Off))
    {
        return Error::UNSUPPORTED_FIFO_POWER_MODE;
    }

    /* remember power mode */
    /* ------------------- */
    m_fifo_power_mode = mode;

    /* update SPI registers of BGT60TR24 chip */
    /* --------------------------------------- */
    update_spi_register_set();

    return Error::OK;
}

// ---------------------------------------------------------------------------- get_fifo_power_mode
Driver::Error Driver::get_fifo_power_mode(Fifo_Power_Mode* mode) const
{
    *mode = m_fifo_power_mode;
    return Error::OK;
}

// ---------------------------------------------------------------------------- set_pad_driver_mode
Driver::Error Driver::set_pad_driver_mode(Pad_Driver_Mode mode)
{
    auto& device_traits = Device_Traits::get(m_device_type);

    /* This feature is not supported before BGT60TR13D */
    if (!device_traits.has_programmable_pad_driver)
        return Error::FEATURE_NOT_SUPPORTED;

    /* check if parameter is valid */
    /* --------------------------- */
    if ((mode != Pad_Driver_Mode::Normal)
        && (mode != Pad_Driver_Mode::Strong))
    {
        return Error::UNSUPPORTED_PAD_DRIVER_MODE;
    }

    /* remember power mode */
    /* ------------------- */
    m_pad_driver_mode = mode;

    /* update SPI registers of BGT60TR24 chip */
    /* --------------------------------------- */
    update_spi_register_set();

    return Error::OK;
}

// ---------------------------------------------------------------------------- get_pad_driver_mode
Driver::Error Driver::get_pad_driver_mode(Pad_Driver_Mode* mode) const
{
    *mode = m_pad_driver_mode;
    return Error::OK;
}

// ---------------------------------------------------------------------------- set_duty_cycle_correction
Driver::Error Driver::set_duty_cycle_correction(const Duty_Cycle_Correction_Settings* settings)
{
    auto& device_traits = Device_Traits::get(m_device_type);

    /* This feature is not supported before BGT60TR13D */
    if (!device_traits.has_ref_frequency_doubler)
        return Error::FEATURE_NOT_SUPPORTED;

    /* check if parameters are valid */
    /* ----------------------------- */
    if ((settings->mode != Duty_Cycle_Correction_Mode::Only_Out)
        && (settings->mode != Duty_Cycle_Correction_Mode::In_Out)
        && (settings->mode != Duty_Cycle_Correction_Mode::SysIn_Out))
    {
        return Error::DOUBLER_MODE_NOT_SUPPORTED;
    }

    if (settings->adjust_in > 15)
    {
        return Error::DC_IN_CORRECTION_OUT_OF_RANGE;
    }

    if ((settings->adjust_out < -8) || (settings->adjust_out > 7))
    {
        return Error::DC_OUT_CORRECTION_OUT_OF_RANGE;
    }

    /* remember doubler settings */
    /* ------------------------- */
    m_dc_correction = *settings;

    /* update SPI registers of BGT60TR24 chip */
    /* -------------------------------------- */
    update_spi_register_set();

    return Error::OK;
}

// ---------------------------------------------------------------------------- get_duty_cycle_correction
Driver::Error Driver::get_duty_cycle_correction(Duty_Cycle_Correction_Settings* settings) const
{
    auto& device_traits = Device_Traits::get(m_device_type);

    /* This feature is not supported before BGT60TR13D */
    if (!device_traits.has_ref_frequency_doubler)
        return Error::FEATURE_NOT_SUPPORTED;

    *settings = m_dc_correction;

    return Error::OK;
}

// ---------------------------------------------------------------------------- set_pullup_resistor_configuration
Driver::Error Driver::set_pullup_resistor_configuration(const Pullup_Resistor_Configuration* configuration)
{
    auto& device_traits = Device_Traits::get(m_device_type);

    /* This feature is not supported before BGT60TR12E and BGT60TR11D */
    if (!device_traits.has_programmable_pullup_resistors)
        return Error::FEATURE_NOT_SUPPORTED;

    /*
     * Fields of configuration structure are boolean, so any non zero value
     * means true. Normalizing true to 1 allows to write values directly to SPI
     * registers.
     */
    Pullup_Resistor_Configuration* pu_cfg = &m_pullup_configuration;
    pu_cfg->enable_spi_cs = configuration->enable_spi_cs ? 1 : 0;
    pu_cfg->enable_spi_clk = configuration->enable_spi_clk ? 1 : 0;
    pu_cfg->enable_spi_di = configuration->enable_spi_di ? 1 : 0;
    pu_cfg->enable_spi_do = configuration->enable_spi_do ? 1 : 0;
    pu_cfg->enable_spi_dio2 = configuration->enable_spi_dio2 ? 1 : 0;
    pu_cfg->enable_spi_dio3 = configuration->enable_spi_dio3 ? 1 : 0;
    pu_cfg->enable_irq = configuration->enable_irq ? 1 : 0;

    /* update SPI registers of BGT60TRxx chip */
    /* -------------------------------------- */
    update_spi_register_set();

    return Error::OK;
}

// ---------------------------------------------------------------------------- get_pullup_resistor_configuration
Driver::Error Driver::get_pullup_resistor_configuration(Pullup_Resistor_Configuration* configuration) const
{
    auto& device_traits = Device_Traits::get(m_device_type);

    /* This feature is not supported before BGT60TR12E and BGT60TR11D */
    if (!device_traits.has_programmable_pullup_resistors)
        return Error::FEATURE_NOT_SUPPORTED;

    *configuration = m_pullup_configuration;
    return Error::OK;
}

// ---------------------------------------------------------------------------- set_power_sens_delay
Driver::Error Driver::set_power_sens_delay(uint32_t delay_100ps)
{
    auto& device_traits = Device_Traits::get(m_device_type);

    uint32_t reg_value;

    /* This feature is only available for and BGT60TR11D */
    if (device_traits.has_sadc)
        return Error::FEATURE_NOT_SUPPORTED;

    /* Convert the period into the according register value */
    reg_value = (uint32_t)cycles_from_100ps(delay_100ps,
                                            m_reference_clock_freq_Hz);
    if (reg_value >= 2)
    {
        reg_value -= 2;
        reg_value = (reg_value + 4) / 8;
    }

    /* Check if parameter is in the allowed range */
    if (reg_value > BGT60TR11D_MAX_POWER_SENS_DELAY)
        return Error::POWER_SENS_DELAY_OUT_OF_RANGE;

    /*
     * Remember the new timing and the previous value for the case of a
     * roll back due to an error.
     */
    uint8_t previous_value = m_power_sens_delay_reg;
    m_power_sens_delay_reg = (uint8_t)reg_value;

    if (m_power_sensing_enabled)
    {
        Error error_code = check_power_sens_timing();
        if (error_code != Error::OK)
        {
            m_power_sens_delay_reg = previous_value;
            return error_code;
        }
    }

    /* Remember the new timing and update registers*/
    m_power_sens_delay_reg = (uint8_t)reg_value;
    update_spi_register_set();

    return Error::OK;
}

// ---------------------------------------------------------------------------- get_power_sens_delay
Driver::Error Driver::get_power_sens_delay(uint32_t* delay_100ps) const
{
    auto& device_traits = Device_Traits::get(m_device_type);

    /* This feature is only available for and BGT60TR11D */
    if (device_traits.has_sadc)
        return Error::FEATURE_NOT_SUPPORTED;

    *delay_100ps =
        (uint32_t)cycles_to_100ps(m_power_sens_delay_reg * 8 + 2,
                                  m_reference_clock_freq_Hz);

    return Error::OK;
}

// ---------------------------------------------------------------------------- set_power_sens_enabled
Driver::Error Driver::set_power_sens_enabled(bool enabled)
{
    auto& device_traits = Device_Traits::get(m_device_type);

    /* This feature is only available for and BGT60TR11D */
    if (device_traits.has_sadc)
        return Error::FEATURE_NOT_SUPPORTED;

    if (enabled)
    {
        Error error_code = check_power_sens_timing();
        if (error_code != Error::OK)
            return error_code;
    }

    /* Remember the new setting and update registers*/
    m_power_sensing_enabled = enabled;
    update_spi_register_set();

    return Error::OK;
}

// ---------------------------------------------------------------------------- get_power_sens_enabled
Driver::Error Driver::get_power_sens_enabled(bool* enabled) const
{
    auto& device_traits = Device_Traits::get(m_device_type);

    /* This feature is only available for and BGT60TR11D */
    if (device_traits.has_sadc)
        return Error::FEATURE_NOT_SUPPORTED;

    *enabled = m_power_sensing_enabled;

    return Error::OK;
}

// ---------------------------------------------------------------------------- set_temperature_sens_enabled
Driver::Error Driver::set_temperature_sens_enabled(bool enabled)
{
    auto& device_traits = Device_Traits::get(m_device_type);

    /* This feature is only available for and BGT60TR11D */
    if (device_traits.has_sadc)
        return Error::FEATURE_NOT_SUPPORTED;

    if (enabled)
    {
        Error error_code = check_temperature_sens_timing();
        if (error_code != Error::OK)
            return error_code;

        /*
         * BGT60TR11D state machine gets stuck if rx_mask = 0 and temperature
         * sensing is enabled. Therefore all active chirps are checked for a
         * zero mask.
         */
        for (uint8_t shp = 0; shp < 4; ++shp)
        {
            if (m_shape[shp].num_repetitions == 0)
                break;

            if ((m_shape[shp].shape_type != Shape_Type::Saw_Down)
                && (m_channel_set[2 * shp].rx_mask == 0))
                return Error::TEMP_SENSING_WITH_NO_RX;

            if ((m_shape[shp].shape_type != Shape_Type::Saw_Up)
                && (m_channel_set[2 * shp + 1].rx_mask == 0))
                return Error::TEMP_SENSING_WITH_NO_RX;
        }
    }

    /* Remember the new setting and update registers*/
    m_temperature_sensing_enabled = enabled;
    update_spi_register_set();

    return Error::OK;
}

// ---------------------------------------------------------------------------- get_temperature_sens_enabled
Driver::Error Driver::get_temperature_sens_enabled(bool* enabled) const
{
    auto& device_traits = Device_Traits::get(m_device_type);

    /* This feature is only available for and BGT60TR11D */
    if (device_traits.has_sadc)
        return Error::FEATURE_NOT_SUPPORTED;

    *enabled = m_temperature_sensing_enabled;

    return Error::OK;
}

// ---------------------------------------------------------------------------- set_oscillator_configuration
Driver::Error Driver::set_oscillator_configuration(const Oscillator_Configuration* configuration)
{
    auto& device_traits = Device_Traits::get(m_device_type);

    if (!device_traits.has_internal_oscillator)
        return Error::FEATURE_NOT_SUPPORTED;

    m_oscillator_configuration = *configuration;
    update_spi_register_set();

    return Error::OK;
}

// ---------------------------------------------------------------------------- get_oscillator_configuration
Driver::Error Driver::get_oscillator_configuration(Oscillator_Configuration* configuration) const
{
    auto& device_traits = Device_Traits::get(m_device_type);

    if (!device_traits.has_internal_oscillator)
        return Error::FEATURE_NOT_SUPPORTED;

    *configuration = m_oscillator_configuration;

    return Error::OK;
}

// ---------------------------------------------------------------------------- set_lo_doubler_output_power
Driver::Error Driver::set_lo_doubler_output_power(Lo_Doubler_Output_Power power)
{
    auto& device_traits = Device_Traits::get(m_device_type);

    if (!device_traits.has_local_oscillator_frequency_doubler)
        return Error::FEATURE_NOT_SUPPORTED;

    /* remember settings */
    /* ----------------- */
    uint8_t reg_value = 0;
    switch (power)
    {
        case Lo_Doubler_Output_Power::_m10dBm16: reg_value = 0; break;
        case Lo_Doubler_Output_Power::_m2dBm66: reg_value = 1; break;
        case Lo_Doubler_Output_Power::_m0dBm49: reg_value = 2; break;
        case Lo_Doubler_Output_Power::_0dBm46: reg_value = 3; break;
        case Lo_Doubler_Output_Power::_1dBm01: reg_value = 4; break;
        case Lo_Doubler_Output_Power::_1dBm38: reg_value = 5; break;
        case Lo_Doubler_Output_Power::_1dBm64: reg_value = 6; break;
        case Lo_Doubler_Output_Power::_1dBm82: reg_value = 7; break;
    }

    if (m_current_mode & MODE_EASY)
    {
        /* In easy mode, settings are applied to all channel sets */
        for (auto& channel_set : m_channel_set)
            channel_set.lo_doubler_power_reg = reg_value;
    }
    else
    {
        auto& channel_set = m_channel_set[m_currently_selected_shape];
        channel_set.lo_doubler_power_reg = reg_value;
    }

    /* update SPI registers of Avian device chip */
    /* ----------------------------------------- */
    update_spi_register_set();

    return Error::OK;
}

// ---------------------------------------------------------------------------- get_lo_doubler_output_power
Driver::Error Driver::get_lo_doubler_output_power(Lo_Doubler_Output_Power* power) const
{
    auto& device_traits = Device_Traits::get(m_device_type);

    using Channel_Set_Layout = Device_Traits::Channel_Set_Layout;
    if ((device_traits.cs_register_layout == Channel_Set_Layout::Version1)
        || (device_traits.cs_register_layout == Channel_Set_Layout::Version2))
        return Error::FEATURE_NOT_SUPPORTED;

    /* return current settings of driver object */
    switch (m_channel_set[m_currently_selected_shape].lo_doubler_power_reg)
    {
        case 0: *power = Lo_Doubler_Output_Power::_m10dBm16; break;
        case 1: *power = Lo_Doubler_Output_Power::_m2dBm66; break;
        case 2: *power = Lo_Doubler_Output_Power::_m0dBm49; break;
        case 3: *power = Lo_Doubler_Output_Power::_0dBm46; break;
        case 4: *power = Lo_Doubler_Output_Power::_1dBm01; break;
        case 5: *power = Lo_Doubler_Output_Power::_1dBm38; break;
        case 6: *power = Lo_Doubler_Output_Power::_1dBm64; break;
        case 7: *power = Lo_Doubler_Output_Power::_1dBm82; break;
    }

    return Error::OK;
}

// ---------------------------------------------------------------------------- get_clock_config_command
HW::Spi_Command_t Driver::get_clock_config_command() const
{
    auto& device_traits = Device_Traits::get(m_device_type);

    /*
     * If the avian device does not have a frequency doubler there is no
     * configuration at all.
     */
    if (!device_traits.has_ref_frequency_doubler)
        return 0;

    /*
     * The register to be returned depends on the device type. The operator[]
     * of the register set returns only the value so the complete write command
     * is constructed by using a SET macro to set an arbitrary bit field to 0.
     * That will add just the address byte to the command.
     */
    if (device_traits.has_reordered_register_layout)
    {
        return BGT60TRxxE_SET(FD, CLK_SEL, 0)
               | m_current_configuration[BGT60TRxxE_REG_FD];
    }
    else
    {
        return BGT60TRxxD_SET(FD, CLK_SEL, 0)
               | m_current_configuration[BGT60TRxxD_REG_FD];
    }
}

// ---------------------------------------------------------------------------- get_temperature_sens_enabled
Device_Type Driver::get_device_type() const
{
    return m_device_type;
}

// ---------------------------------------------------------------------------- count_antennas
uint8_t Driver::count_antennas(uint8_t rx_mask)
{
    uint8_t num_antennas = 0;

    while (rx_mask != 0)
    {
        if (rx_mask & 0x01)
            ++num_antennas;

        rx_mask >>= 1;
    }

    return num_antennas;
}

/* ------------------------------------------------------------------------ */
}  // namespace Avian
}  // namespace Infineon

/* --- End of File -------------------------------------------------------- */
