/**
 * @file default_settings.cpp
 *
 * This files contains the initial parameters for the BGT60TRxx driver. The
 * driver applies those settings when a new driver instance is created.
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
#include "_configuration.h"
#include "ifxAvian_DeviceTraits.hpp"
#include "ifxAvian_Driver.hpp"

// ---------------------------------------------------------------------------- namespaces
namespace Infineon {
namespace Avian {

// ---------------------------------------------------------------------------- set_default_parameters
void Driver::set_default_parameters()
{
    auto& device_traits = Device_Traits::get(m_device_type);

    /* ADC parameters */
    /* -------------- */
    m_adc_sample_rate_divider = 40; /* 2 MHz */
    m_adc_sample_time = Adc_Sample_Time::_50ns;
    m_adc_tracking = Adc_Tracking::_1_Subconversion;
    m_adc_double_msb_time = 0;
    m_adc_oversampling = Adc_Oversampling::Off;

    /* Chirp timing */
    /* -------------- */
    m_pre_chirp_delay_reg = 10;  /* 1.12 us */
    m_post_chirp_delay_reg = 14; /* 1.4 us */
    m_pa_delay_reg = 30;         /* 3 us */
    m_adc_delay_reg = 31;        /* 3.1 us */

    m_power_sens_delay_reg = 10; /* 1.025 us */
    m_power_sensing_enabled = 0;
    m_temperature_sensing_enabled = 1;

    /* Startup timing */
    /* -------------- */
    m_time_wake_up = Scaled_Timer(10000 * 8, /* 1 ms */
                                  BGT60TRXX_MAX_WAKEUP_COUNTER_SHIFT,
                                  BGT60TRXX_MAX_WAKEUP_COUNTER);
    if (!device_traits.has_extra_startup_delays)
    {
        m_time_init0 = Scaled_Timer(450 * 8, /* 45 us */
                                    BGT60TRXX_MAX_PLL_INIT_COUNTER_SHIFT,
                                    BGT60TRXXC_MAX_PLL_INIT0_COUNTER);
    }
    else
    {
        m_time_init0 = Scaled_Timer(504 * 8, /* 50.4 us */
                                    BGT60TRXX_MAX_PLL_INIT_COUNTER_SHIFT,
                                    BGT60TRXXD_MAX_PLL_INIT0_COUNTER);
    }
    m_time_init1 = Scaled_Timer(70 * 8, /* 7 us */
                                BGT60TRXX_MAX_PLL_INIT_COUNTER_SHIFT,
                                BGT60TRXX_MAX_PLL_INIT1_COUNTER);

    /* idle mode configuration */
    /* ----------------------- */
    m_idle_settings.enable_pll = 0;
    m_idle_settings.enable_vco = 0;
    m_idle_settings.enable_fdiv = 0;
    m_idle_settings.enable_baseband = 0;
    m_idle_settings.enable_rf = 0;
    m_idle_settings.enable_madc = 0;
    m_idle_settings.enable_madc_bandgap = 1;
    m_idle_settings.enable_sadc = 0;
    m_idle_settings.enable_sadc_bandgap = 0;

    /* deep sleep mode configuration */
    /* ----------------------------- */
    m_deep_sleep_settings.enable_pll = 0;
    m_deep_sleep_settings.enable_vco = 0;
    m_deep_sleep_settings.enable_fdiv = 0;
    m_deep_sleep_settings.enable_baseband = 0;
    m_deep_sleep_settings.enable_rf = 0;
    m_deep_sleep_settings.enable_madc = 0;
    m_deep_sleep_settings.enable_madc_bandgap = 0;
    m_deep_sleep_settings.enable_sadc = 0;
    m_deep_sleep_settings.enable_sadc_bandgap = 0;

    /* -------------------------------------------------------------------- */
    /* Frame Sequence Definition                                            */
    /* -------------------------------------------------------------------- */

    /* number of frames */
    /* ---------------- */
    m_num_frames_before_stop = 0; /* don't stop */

    /* shape set repetitions (and power state after last repetition) */
    /* ------------------------------------------------------------- */
    m_num_set_repetitions = 64;
    m_frame_end_power_mode = Power_Mode::Idle;
    m_frame_end_delay = Scaled_Timer(400000 * 8, /* 2ms */
                                     BGT60TRXX_MAX_SHAPE_END_DELAY_SHIFT,
                                     BGT60TRXX_MAX_SHAPE_END_DELAY);
    m_shape[0].num_repetitions = 1;
    m_shape[1].num_repetitions = 0;
    m_shape[2].num_repetitions = 0;
    m_shape[3].num_repetitions = 0;

    /* -------------------------------------------------------------------- */
    /* BGT60TRxxD Settings                                                  */
    /* -------------------------------------------------------------------- */
    if (!device_traits.has_programmable_fifo_power_mode)
        m_fifo_power_mode = Fifo_Power_Mode::Always_On;
    else
        m_fifo_power_mode = Fifo_Power_Mode::Deep_Sleep_And_Idle_Off;
    m_pad_driver_mode = Pad_Driver_Mode::Normal;

    m_bandgap_delay_reg = 6;     /* 4.825µs @ 80MHz */
    m_madc_delay_reg = 4;        /* 3.2125µs @ 80MHz */
    m_pll_enable_delay_reg = 13; /* 10.425µs @ 80MHz */
    m_pll_divider_delay_reg = 2; /* 812.5ns @ 80MHz */

    /* -------------------------------------------------------------------- */
    /* BGT60TRxxE Settings                                                  */
    /* -------------------------------------------------------------------- */
    m_pullup_configuration.enable_spi_cs = 0;
    m_pullup_configuration.enable_spi_clk = 0;
    m_pullup_configuration.enable_spi_di = 0;
    m_pullup_configuration.enable_spi_do = 1;
    m_pullup_configuration.enable_spi_dio2 = 1;
    m_pullup_configuration.enable_spi_dio3 = 1;
    m_pullup_configuration.enable_irq = 1;

    /* -------------------------------------------------------------------- */
    /* BGT120UTR24 Settings                                                 */
    /* -------------------------------------------------------------------- */
    m_oscillator_configuration.clock_source = Clock_Source::External;
    m_oscillator_configuration.amplitude = Clock_Amplitude::Reduced;
    m_oscillator_configuration.ldo_voltage = Oscillator_Ldo_Voltage::_1V44;
    m_oscillator_configuration.enable_clock_output = false;

    /* -------------------------------------------------------------------- */
    /* Shape 1                                                              */
    /* -------------------------------------------------------------------- */
    /*
     * All shapes to the same parameters, even though shape 2-4 are
     * disabled, to have them initialized to a defined value.
     */
    for (unsigned i = 0; i < 4; i++)
    {
        /* type and frequency range */
        /* ------------------------ */
        m_shape[i].shape_type = Shape_Type::Saw_Up;
        if (device_traits.pll_pre_divider == 8)
        {
            m_shape[i].lower_frequency_kHz = 58000000;
            m_shape[i].upper_frequency_kHz = 63000000;
        }
        else if (device_traits.pll_pre_divider == 16)
        {
            m_shape[i].lower_frequency_kHz = 116000000;
            m_shape[i].upper_frequency_kHz = 126000000;
        }
        else if (device_traits.pll_pre_divider == 4)
        {
            m_shape[i].lower_frequency_kHz = 24000000;
            m_shape[i].upper_frequency_kHz = 24250000;
        }

        /* shape repetitions (and power state after last repetition) */
        /* --------------------------------------------------------- */
        m_shape[i].following_power_mode = Power_Mode::Idle;
        m_shape[i].post_delay = Scaled_Timer(5000 * 8, /* 499.32us */
                                             BGT60TRXX_MAX_SHAPE_END_DELAY_SHIFT,
                                             BGT60TRXX_MAX_SHAPE_END_DELAY);

        /* chirp settings */
        /* -------------- */
        m_shape[i].num_samples_up = 64;
        m_shape[i].num_samples_down = 256;
        if (!device_traits.has_sadc)
        {
            m_shape[i].chirp_end_delay_up_reg = 3;   /* 362.5ns */
            m_shape[i].chirp_end_delay_down_reg = 3; /* 362.5ns */
        }
        else
        {
            m_shape[i].chirp_end_delay_up_reg = 0;   /* 25ns */
            m_shape[i].chirp_end_delay_down_reg = 0; /* 25ns */
        }
    }

    for (unsigned i = 0; i < 8; i++)
    {
        m_channel_set[i].rx_mask = (1 << device_traits.num_rx_antennas) - 1;
        m_channel_set[i].tx_power = 31;
        m_channel_set[i].tx_mode = Tx_Mode::Tx1_Only;

        /* base band settings */
        /* (HPF cutoff 3 was 80kHz for Avian C.) */
        m_channel_set[i].hp_gain_1 = Hp_Gain::_18dB;
        m_channel_set[i].hp_cutoff_1_reg = 3;
        m_channel_set[i].vga_gain_1 = Vga_Gain::_5dB;
        m_channel_set[i].hp_gain_2 = Hp_Gain::_18dB;
        m_channel_set[i].hp_cutoff_2_reg = 3;
        m_channel_set[i].vga_gain_2 = Vga_Gain::_5dB;
        m_channel_set[i].hp_gain_3 = Hp_Gain::_18dB;
        m_channel_set[i].hp_cutoff_3_reg = 3;
        m_channel_set[i].vga_gain_3 = Vga_Gain::_5dB;
        m_channel_set[i].hp_gain_4 = Hp_Gain::_18dB;
        m_channel_set[i].hp_cutoff_4_reg = 3;
        m_channel_set[i].vga_gain_4 = Vga_Gain::_5dB;
        if (device_traits.cs_register_layout != Device_Traits::Channel_Set_Layout::Version1)
            m_channel_set[i].reset_period_reg = 15;  /* 1.5us @ 80MHz */
        else
            m_channel_set[i].reset_period_reg = 120; /* 1.5us @ 80MHz */

        /* anti alias filter settings */
        /*
         * (AAF cutoff 0 is either 500kHz or 600kHz. That's the only setting
         *  supported by all Avian devices.)
         */
        m_channel_set[i].aaf_cutoff_1_reg = 0;
        m_channel_set[i].aaf_cutoff_2_reg = 0;
        m_channel_set[i].aaf_cutoff_3_reg = 0;
        m_channel_set[i].aaf_cutoff_4_reg = 0;

        // LO doubler output power is set to chip reset value.
        m_channel_set[i].lo_doubler_power_reg = 2;
    }
}

/* ------------------------------------------------------------------------ */
}  // namespace Avian
}  // namespace Infineon

/* --- End of File -------------------------------------------------------- */
