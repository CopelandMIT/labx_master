/**
    @file spi_programming.cpp

    This file is part of the BGT60TRxx driver.

    This file contains functions to setup the SPI programming sequence for
    normal radar operation. Special programming for test modes is not
    included.
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
#include "registers_BGT120TR24E.h"
#include "registers_BGT60TR11D.h"
#include "registers_BGT60TRxxC.h"
#include "registers_BGT60TRxxD.h"
#include "registers_BGT60TRxxE.h"
#include "value_conversion/ifxAvian_RfConversion.hpp"
#include <vector>

// ---------------------------------------------------------------------------- namespaces
namespace Infineon {
namespace Avian {

using RF_Converter = Value_Conversion::RF_Converter;
using Cs_Layout_t = Device_Traits::Channel_Set_Layout;

// ---------------------------------------------------------------------------- get_device_configuration
HW::RegisterSet Driver::get_device_configuration() const
{
    // The register modifications are applied
    HW::RegisterSet cfg_to_be_sent = m_current_configuration;
    for (const auto& modification : m_reg_modifications)
    {
        auto value = 0;
        if (cfg_to_be_sent.is_defined(modification.first))
            value = cfg_to_be_sent[modification.first];

        cfg_to_be_sent.set(modification.first,
                           (value & modification.second.and_mask)
                               | modification.second.or_mask);
    }
    return cfg_to_be_sent;
}

// ---------------------------------------------------------------------------- program_registers_main
void Driver::program_registers_main()
{
    auto& device_traits = Device_Traits::get(m_device_type);

    uint32_t spi_word;
    spi_word = BGT60TRxxC_SET(MAIN, FRAME_START, 0) /* don't start */
               | BGT60TRxxC_SET(MAIN, SW_RESET, 0)  /* no reset */
               | BGT60TRxxC_SET(MAIN, FSM_RESET, 0)
               | BGT60TRxxC_SET(MAIN, FIFO_RESET, 0)
               | BGT60TRxxC_SET(MAIN, CW_MODE, 0)
               | BGT60TRxxC_SET(MAIN, BG_CLK_DIV, 3)
               | BGT60TRxxC_SET(MAIN, LDO_LOAD_STRENGTH, 0)
               | BGT60TRxxC_SET(MAIN, LDO_MODE, 0);
    if (device_traits.wu_register_type == Device_Traits::Wu_Register_Type::None)
    {
        spi_word |= BGT60TRxxC_SET(MAIN, TWKUP,
                                   m_time_wake_up.get_counter())
                    | BGT60TRxxC_SET(MAIN, TWKUP_MUL,
                                     m_time_wake_up.get_shift());
    }

    if (device_traits.has_programmable_pullup_resistors)
    {
        Pullup_Resistor_Configuration* pu_cfg = &m_pullup_configuration;
        spi_word |=
            BGT60TRxxE_SET(MAIN, SPI_BROADCAST_MODE, 0)
            | BGT60TRxxE_SET(MAIN, PU_EN_SPICSN, pu_cfg->enable_spi_cs)
            | BGT60TRxxE_SET(MAIN, PU_EN_SPICLK, pu_cfg->enable_spi_clk)
            | BGT60TRxxE_SET(MAIN, PU_EN_SPIDI, pu_cfg->enable_spi_di)
            | BGT60TRxxE_SET(MAIN, PU_EN_SPIDO, pu_cfg->enable_spi_do)
            | BGT60TRxxE_SET(MAIN, PU_EN_SPIDIO2, pu_cfg->enable_spi_dio2)
            | BGT60TRxxE_SET(MAIN, PU_EN_SPIDIO3, pu_cfg->enable_spi_dio3)
            | BGT60TRxxE_SET(MAIN, PU_EN_IRQ, pu_cfg->enable_irq);
    }

    if (device_traits.has_explicit_sadc_bg_div_control)
        spi_word |= BGT60TRxxD_SET(MAIN, SADC_BG_CLK_DIV, 2); /* 40MHz BG clock */
    else
        spi_word |= BGT60TRxxC_SET(MAIN, SADC_CLK_DIV, 3);    /* 20MHz SADC clock */
    m_current_configuration.set(spi_word);

    if (device_traits.wu_register_type != Device_Traits::Wu_Register_Type::None)
    {
        spi_word = BGT60TRxxE_SET(WU, TR_TWKUP,
                                  m_time_wake_up.get_counter())
                   | BGT60TRxxE_SET(WU, TR_TWKUP_MUL,
                                    m_time_wake_up.get_shift());
        m_current_configuration.set(spi_word);
    }
}

// ---------------------------------------------------------------------------- program_registers_madc
void Driver::program_registers_madc()
{
    auto& device_traits = Device_Traits::get(m_device_type);

    uint32_t dscal;
    uint32_t spi_word;

    /* check if startup time is sufficient for a calibration */
    uint32_t madc_startup_cycles =
        BGT60TRXX_MADC_STARTUP_CYCLES
        + 1569
        + ((m_adc_double_msb_time != 0) ? 896 : 0);
    switch (m_adc_sample_time)
    {
        case Adc_Sample_Time::_50ns: madc_startup_cycles += 1792; break;
        case Adc_Sample_Time::_100ns: madc_startup_cycles += 1792 * 2; break;
        case Adc_Sample_Time::_200ns: madc_startup_cycles += 1792 * 4; break;
        case Adc_Sample_Time::_400ns: madc_startup_cycles += 1792 * 8; break;
    }

    uint64_t init_cycles = m_time_init0.get_clock_cycles()
                           + m_time_init1.get_clock_cycles();

    if (device_traits.has_extra_startup_delays)
        init_cycles -= 64 * m_madc_delay_reg + 1;

    dscal = (init_cycles < madc_startup_cycles) ? 1 : 0;

    /* compile parameters into an SPI configuration word */
    spi_word =
        BGT60TRxxC_SET(ADC0, STC, m_adc_sample_time)
        | BGT60TRxxC_SET(ADC0, TRACK_CFG, m_adc_tracking)
        | BGT60TRxxC_SET(ADC0, ADC_OVERS_CFG,
                         m_adc_oversampling)
        | BGT60TRxxC_SET(ADC0, BG_TC_TRIM, 4)
        | BGT60TRxxC_SET(ADC0, BG_CHOP_EN, 0) /* no band gap chopping */
        | BGT60TRxxC_SET(ADC0, DSCAL, dscal)
        | BGT60TRxxC_SET(ADC0, MSB_CTRL,
                         m_adc_double_msb_time ? 1 : 0)
        | BGT60TRxxC_SET(ADC0, TRIG_MADC, 0) /*no manual test trigger*/
        | BGT60TRxxC_SET(ADC0, ADC_DIV, m_adc_sample_rate_divider);
    m_current_configuration.set(spi_word);

    if (!device_traits.has_sadc)
    {
        spi_word = BGT60TR11D_SET(ADC1, TR_PSSTART,
                                  m_power_sens_delay_reg);
        m_current_configuration.set(spi_word);
    }
}

// ---------------------------------------------------------------------------- program_registers_sadc
void Driver::program_registers_sadc()
{
    auto& device_traits = Device_Traits::get(m_device_type);

    uint32_t spi_word;
    uint32_t chirp_cycles = 0xFFFFFFFF;
    uint8_t i;

    /* find shortest chirp in configuration in cycles @80MHz */
    for (i = 0; i < 4; ++i)
    {
        /* skip, if shape is disabled */
        if (m_shape[i].num_repetitions == 0)
        {
            continue;
        }

        if ((m_shape[i].shape_type != Shape_Type::Saw_Down)
            && (chirp_cycles > m_shape[i].num_samples_up))
        {
            chirp_cycles = m_shape[i].num_samples_up;
        }

        if ((m_shape[i].shape_type != Shape_Type::Saw_Up)
            && (chirp_cycles > m_shape[i].num_samples_down))
        {
            chirp_cycles = m_shape[i].num_samples_down;
        }
    }
    chirp_cycles *= m_adc_sample_rate_divider;

    /* compile SADC parameters into an SPI configuration word */
    spi_word = BGT60TRxxC_SET(SADC_CTRL, SADC_CHSEL, 0)
               | BGT60TRxxC_SET(SADC_CTRL, TC_TRIM, 4)
               | BGT60TRxxC_SET(SADC_CTRL, SD_EN, 1)
               | BGT60TRxxC_SET(SADC_CTRL, LVGAIN, 0)
               | BGT60TRxxC_SET(SADC_CTRL, DSCAL, 0);

    /*
     * With the beginning of a chirp, the SADC is also triggered. One SADC
     * conversion must not take longer than a chirp, because some circuitry is
     * disabled at the end of the chirp and the SADC measurement would be
     * invalid otherwise.
     *
     * The following if-else-if chain tries to find the SADC configuration
     * with best accuracy possible with the programmed chirp length. Note
     * that SADC at 20MHz so the number of clock cycles is multiplied by 4
     * to meet the reference frequency of 80MHz.
     */
    if (chirp_cycles >= 1440 * 4)
    {
        // 32x oversampling with early sample spread --> 1440 clock cycles
        spi_word |= BGT60TRxxC_SET(SADC_CTRL, OVERS_CFG, 3)
                    | BGT60TRxxC_SET(SADC_CTRL, SESP, 1);
    }
    else if (chirp_cycles >= 928 * 4)
    {
        // 32x oversampling without early sample spread --> 928 clock cycles
        spi_word |= BGT60TRxxC_SET(SADC_CTRL, OVERS_CFG, 3)
                    | BGT60TRxxC_SET(SADC_CTRL, SESP, 0);
    }
    else if (chirp_cycles >= 180 * 4)
    {
        // 4x oversampling with early sample spread --> 180 clock cycles
        spi_word |= BGT60TRxxC_SET(SADC_CTRL, OVERS_CFG, 2)
                    | BGT60TRxxC_SET(SADC_CTRL, SESP, 1);
    }
    else if (chirp_cycles >= 116 * 4)
    {
        // 4x oversampling without early sample spread --> 116 clock cycles
        spi_word |= BGT60TRxxC_SET(SADC_CTRL, OVERS_CFG, 2)
                    | BGT60TRxxC_SET(SADC_CTRL, SESP, 0);
    }
    else if (chirp_cycles >= 90 * 4)
    {
        // 2x oversampling with early sample spread --> 90 clock cycles
        spi_word |= BGT60TRxxC_SET(SADC_CTRL, OVERS_CFG, 1)
                    | BGT60TRxxC_SET(SADC_CTRL, SESP, 1);
    }
    else if (chirp_cycles >= 58 * 4)
    {
        // 2x oversampling without early sample spread --> 58 clock cycles
        spi_word |= BGT60TRxxC_SET(SADC_CTRL, OVERS_CFG, 1)
                    | BGT60TRxxC_SET(SADC_CTRL, SESP, 0);
    }
    else if (chirp_cycles >= 45 * 4)
    {
        // no oversampling with early sample spread --> 45 clock cycles
        spi_word |= BGT60TRxxC_SET(SADC_CTRL, OVERS_CFG, 0)
                    | BGT60TRxxC_SET(SADC_CTRL, SESP, 1);
    }
    else
    {
        // no oversampling without early sample spread --> 29 clock cycles
        spi_word |= BGT60TRxxC_SET(SADC_CTRL, OVERS_CFG, 0)
                    | BGT60TRxxC_SET(SADC_CTRL, SESP, 0);
    }

    if (device_traits.has_explicit_sadc_bg_div_control)
        spi_word |= BGT60TRxxD_SET(SADC_CTRL, SADC_CLK_DIV, 3);
    m_current_configuration.set(spi_word);
}

// ---------------------------------------------------------------------------- program_registers_fifo
void Driver::program_registers_fifo()
{
    auto& device_traits = Device_Traits::get(m_device_type);

    auto properties = m_port.get_properties();

    uint32_t spi_word =
        BGT60TRxxC_SET(SFCTL, FIFO_CREF, (m_slice_size / 2) - 1)
        | BGT60TRxxC_SET(SFCTL, MISO_HF_READ,
                         properties.high_speed_compensation ? 1 : 0)
        | BGT60TRxxC_SET(SFCTL, LFSR_EN, 0)
        | BGT60TRxxC_SET(SFCTL, PREFIX_EN, 0)
        | BGT60TRxxC_SET(SFCTL, QSPI_WT, properties.quad_spi_wait_cycles - 1);

    if (device_traits.has_programmable_fifo_power_mode)
        spi_word |= BGT60TRxxD_SET(SFCTL, FIFO_PD_MODE, m_fifo_power_mode);
    else
        spi_word |= BGT60TRxxC_SET(SFCTL, FIFO_LP_MODE, 1);

    if (device_traits.has_programmable_pad_driver)
        spi_word |= BGT60TRxxD_SET(SFCTL, PAD_MODE, m_pad_driver_mode);

    m_current_configuration.set(spi_word);
}

// ---------------------------------------------------------------------------- program_registers_ccr
void Driver::program_registers_ccr()
{
    auto& device_traits = Device_Traits::get(m_device_type);

    uint32_t cont_mode;
    uint32_t pd_mode;
    uint32_t frame_length;
    uint32_t spi_words[4];
    uint8_t i;

    /* count number of active shape groups and calculate frame length */
    frame_length = 0;
    for (i = 0; i < 4; ++i)
    {
        if (m_shape[i].num_repetitions != 0)
            ++frame_length;
    }
    frame_length *= m_num_set_repetitions;

    /* configure shape set */
    switch (m_frame_end_power_mode)
    {
        case Power_Mode::Stay_Active:
            cont_mode = 1;
            pd_mode = 0;
            break;

        case Power_Mode::Deep_Sleep:
            cont_mode = 0;
            pd_mode = 2;
            break;

        case Power_Mode::Deep_Sleep_Continue:
            cont_mode = 1;
            pd_mode = 2;
            break;

        case Power_Mode::Idle:
        default:
            cont_mode = 1;
            pd_mode = 1;
            break;
    }

    spi_words[0] = BGT60TRxxC_SET(CCR0, CONT_MODE, cont_mode)
                   | BGT60TRxxC_SET(CCR0, REPT, 15);
    spi_words[1] = BGT60TRxxC_SET(CCR1, PD_MODE, pd_mode)
                   | BGT60TRxxC_SET(CCR1, TFED,
                                    m_frame_end_delay.get_counter())
                   | BGT60TRxxC_SET(CCR1, TFED_MUL,
                                    m_frame_end_delay.get_shift());
    spi_words[2] = BGT60TRxxC_SET(CCR2, MAX_FRAME_CNT,
                                  m_num_frames_before_stop)
                   | BGT60TRxxC_SET(CCR2, FRAME_LEN, frame_length - 1);

    /* configure chirp timing */
    spi_words[0] |= BGT60TRxxC_SET(CCR0, TMREND,
                                   m_post_chirp_delay_reg);
    spi_words[1] |= BGT60TRxxC_SET(CCR1, TMRSTRT,
                                   m_pre_chirp_delay_reg);
    spi_words[3] = BGT60TRxxC_SET(CCR3, T_PAEN,
                                  m_pa_delay_reg);
    if (!device_traits.has_extra_startup_delays)
        spi_words[3] |= BGT60TRxxC_SET(CCR3, T_SSTRT, m_adc_delay_reg);
    else
        spi_words[3] |= BGT60TRxxD_SET(CCR3, TR_SSTART, m_adc_delay_reg);

    /* configure startup timing */
    spi_words[0] |= BGT60TRxxC_SET(CCR0, TR_INIT1,
                                   m_time_init1.get_counter())
                    | BGT60TRxxC_SET(CCR0, TR_MUL1,
                                     m_time_init1.get_shift());

    spi_words[3] |= BGT60TRxxC_SET(CCR3, TR_MUL0,
                                   m_time_init0.get_shift());
    if (!device_traits.has_extra_startup_delays)
    {
        spi_words[3] |= BGT60TRxxC_SET(CCR3, TR_INIT0,
                                       m_time_init0.get_counter());
    }
    else
    {
        spi_words[3] |= BGT60TRxxD_SET(CCR3, TR_INIT0,
                                       m_time_init0.get_counter());
    }
    for (auto spi_word : spi_words)
        m_current_configuration.set(spi_word);
}

// ---------------------------------------------------------------------------- program_registers_frame
void Driver::program_registers_frame()
{
    auto& device_traits = Device_Traits::get(m_device_type);

    uint32_t address_offset_pll = (BGT60TRxxC_REG_PLL2_7
                                   - BGT60TRxxC_REG_PLL1_7)
                                  << 25;
    uint32_t address_offset_csc = (BGT60TRxxC_REG_CS2
                                   - BGT60TRxxC_REG_CS1)
                                  << 25;
    uint8_t i;

    for (i = 0; i < 4; ++i)
    {
        uint32_t spi_words[2];
        uint32_t reps;
        uint32_t cont_mode;
        uint32_t pd_mode;

        /* do brute force logarithm dualis */
        for (reps = 15; reps > 0; --reps)
        {
            if (m_shape[i].num_repetitions == (1 << reps))
                break;
        }

        switch (m_shape[i].following_power_mode)
        {
            case Power_Mode::Stay_Active:
                cont_mode = 1;
                pd_mode = 0;
                break;

            case Power_Mode::Deep_Sleep:
                cont_mode = 0;
                pd_mode = 2;
                break;

            case Power_Mode::Deep_Sleep_Continue:
                cont_mode = 1;
                pd_mode = 2;
                break;

            case Power_Mode::Idle:
            default:
                cont_mode = 1;
                pd_mode = 1;
                break;
        }

        spi_words[0] =
            (BGT60TRxxC_SET(PLL1_7, REPS, reps)
             | BGT60TRxxC_SET(PLL1_7, SH_EN,
                              (m_shape[i].num_repetitions != 0) ? 1 : 0)
             | BGT60TRxxC_SET(PLL1_7, CONT_MODE, cont_mode)
             | BGT60TRxxC_SET(PLL1_7, PD_MODE, pd_mode)
             | BGT60TRxxC_SET(PLL1_7, T_SED,
                              m_shape[i].post_delay.get_counter())
             | BGT60TRxxC_SET(PLL1_7, T_SED_MUL,
                              m_shape[i].post_delay.get_shift()))
            + i * address_offset_pll;

        spi_words[1] =
            (BGT60TRxxC_SET(CS1, REPC, reps)
             | BGT60TRxxC_SET(CS1, CS_EN,
                              (m_shape[i].num_repetitions != 0) ? 1 : 0)
             | BGT60TRxxC_SET(CS1, ABB_ISOPD, 0) /* enable all */
             | BGT60TRxxC_SET(CS1, RF_ISOPD, 0)  /* circuitry in */
             | BGT60TRxxC_SET(CS1, BG_EN, 1)     /* channel sets */
             | BGT60TRxxC_SET(CS1, MADC_ISOPD, 0)
             | BGT60TRxxC_SET(CS1, BG_TMRF_EN, 1)
             | BGT60TRxxC_SET(CS1, PLL_ISOPD, 0));
        if (device_traits.has_sadc)
            spi_words[1] |= BGT60TRxxC_SET(CS1, SADC_ISOPD, 0);
        spi_words[1] += i * address_offset_csc;

        for (auto spi_word : spi_words)
            m_current_configuration.set(spi_word);
    }
}

// ---------------------------------------------------------------------------- setup_pll_bitfields
void Driver::setup_pll_bitfields(const Shape_Settings* shape_settings,
                                 PLL_Bitfield_Set* bitfields)
{
    auto& device_traits = Device_Traits::get(m_device_type);

    int32_t lower_freq;
    int32_t upper_freq;
    int32_t up_chirp_length_cycles;
    int32_t down_chirp_length_cycles;
    int32_t freq_increment_up;
    int32_t freq_increment_down;
    int32_t additional_cycles;

    /*
     * The frequency range specified by the user should apply to the period
     * while PA is active. Taking the PA delay and the pre-chirp delay into
     * account the total frequency range programmed to FSU/FSD, RTU/RTD and
     * RSU/RSD is extended.
     */

    /* convert specified frequency range in PLL register settings */
    RF_Converter converter(m_reference_clock_freq_Hz, m_pll_div_set,
                           device_traits.pll_pre_divider);
    lower_freq = converter.freq_to_pll(shape_settings->lower_frequency_kHz);
    upper_freq = converter.freq_to_pll(shape_settings->upper_frequency_kHz);

    /* calculate up chirp length in clock cycles */
    /* (take adc delay into account) */
    up_chirp_length_cycles = m_adc_sample_rate_divider
                                 * shape_settings->num_samples_up
                             + 8 * m_adc_delay_reg + 1;
    down_chirp_length_cycles = m_adc_sample_rate_divider
                                   * shape_settings->num_samples_down
                               + 8 * m_adc_delay_reg + 1;

    /* calculate frequency increment per cycle (round to nearest) */
    freq_increment_up = (upper_freq - lower_freq);
    freq_increment_up *= 2;
    freq_increment_up /= up_chirp_length_cycles;
    freq_increment_up += 1;
    freq_increment_up /= 2;

    freq_increment_down = (lower_freq - upper_freq);
    freq_increment_down *= 2;
    freq_increment_down /= down_chirp_length_cycles;
    freq_increment_down -= 1;
    freq_increment_down /= 2;

    /*
     * adjust ramp start frequency by taking pa delay and pre-chirp delay
     * into account
     */
    additional_cycles = (8 * m_pa_delay_reg)
                        - (8 * m_pre_chirp_delay_reg + 10);
    if (additional_cycles > 0)
    {
        lower_freq -= additional_cycles * freq_increment_up;
        upper_freq -= additional_cycles * freq_increment_down;
        up_chirp_length_cycles += additional_cycles;
        down_chirp_length_cycles += additional_cycles;
    }

    /*
     * Assign calculated values to bit fields
     * Note: The FSM of BGT60TRxx always starts with up chirp settings. If
     * shape type requests first chirp of a shape to be the down chirp,
     * parameters must be swapped and up chirp registers must take the down
     * chirp values.
     */
    if ((shape_settings->shape_type == Shape_Type::Saw_Up)
        || (shape_settings->shape_type == Shape_Type::Tri_Up))
    {
        bitfields->fsu = lower_freq;
        bitfields->rsu = freq_increment_up;
        bitfields->rtu = (up_chirp_length_cycles / 8)
                         + ((up_chirp_length_cycles & 0x7) ? 1 : 0);
        bitfields->tedu = shape_settings->chirp_end_delay_up_reg;
        bitfields->apu = shape_settings->num_samples_up;
    }
    else
    {
        bitfields->fsu = upper_freq;
        bitfields->rsu = freq_increment_down;
        bitfields->rtu = (down_chirp_length_cycles / 8)
                         + ((down_chirp_length_cycles & 0x7) ? 1 : 0);
        bitfields->tedu = shape_settings->chirp_end_delay_down_reg;
        bitfields->apu = shape_settings->num_samples_down;
    }

    if (shape_settings->shape_type == Shape_Type::Tri_Up)
    {
        bitfields->fsd = upper_freq;
        bitfields->rsd = freq_increment_down;
        bitfields->rtd = (down_chirp_length_cycles / 8)
                         + ((down_chirp_length_cycles & 0x7) ? 1 : 0);
        bitfields->tedd = shape_settings->chirp_end_delay_down_reg;
        bitfields->apd = shape_settings->num_samples_down;
    }
    else if (shape_settings->shape_type == Shape_Type::Tri_Down)
    {
        bitfields->fsd = lower_freq;
        bitfields->rsd = freq_increment_up;
        bitfields->rtd = (up_chirp_length_cycles / 8)
                         + ((up_chirp_length_cycles & 0x7) ? 1 : 0);
        bitfields->tedd = shape_settings->chirp_end_delay_up_reg;
        bitfields->apd = shape_settings->num_samples_up;
    }
    else
    {
        /*
         * if Shape_Type::Saw_Up or Shape_Type::Saw_Down, disable
         * second part of the shape (aka down chirp)
         */
        bitfields->fsd = 0;
        bitfields->rsd = 0;
        bitfields->rtd = 0;
        bitfields->tedd = 0;
        bitfields->apd = 0;
    }
}

// ---------------------------------------------------------------------------- program_registers_shape
void Driver::program_registers_shape(uint8_t shape_index)
{
    PLL_Bitfield_Set bitfields;
    uint32_t spi_words[7];
    uint32_t i;

    uint32_t address_offset =
        shape_index * (BGT60TRxxC_REG_PLL2_0 - BGT60TRxxC_REG_PLL1_0);
    Shape_Settings& shape = m_shape[shape_index];

    setup_pll_bitfields(&shape, &bitfields);
    if (((shape.shape_type == Shape_Type::Tri_Up)
         && (m_channel_set[2 * shape_index + 1].rx_mask == 0))
        || ((shape.shape_type == Shape_Type::Tri_Down)
            && (m_channel_set[2 * shape_index].rx_mask == 0)))
    {
        bitfields.apd = 0;
    }

    if ((((shape.shape_type == Shape_Type::Tri_Up)
          || (shape.shape_type == Shape_Type::Saw_Up))
         && (m_channel_set[2 * shape_index].rx_mask == 0))
        || (((shape.shape_type == Shape_Type::Tri_Down)
             || (shape.shape_type == Shape_Type::Saw_Down))
            && (m_channel_set[2 * shape_index + 1].rx_mask == 0)))
    {
        bitfields.apu = 0;
    }

    /* program sequence */
    spi_words[0] = BGT60TRxxC_SET(PLL1_0, FSU, bitfields.fsu);
    spi_words[1] = BGT60TRxxC_SET(PLL1_1, RSU, bitfields.rsu);
    spi_words[2] = BGT60TRxxC_SET(PLL1_2, RTU, bitfields.rtu)
                   | BGT60TRxxC_SET(PLL1_2, T_EDU, bitfields.tedu);
    spi_words[3] = BGT60TRxxC_SET(PLL1_3, APU, bitfields.apu)
                   | BGT60TRxxC_SET(PLL1_3, APD, bitfields.apd);
    spi_words[4] = BGT60TRxxC_SET(PLL1_4, FSD, bitfields.fsd);
    spi_words[5] = BGT60TRxxC_SET(PLL1_5, RSD, bitfields.rsd);
    spi_words[6] = BGT60TRxxC_SET(PLL1_6, RTD, bitfields.rtd)
                   | BGT60TRxxC_SET(PLL1_6, T_EDD, bitfields.tedd);
    for (i = 0; i < 7; ++i)
    {
        spi_words[i] += address_offset << 25;
    }

    for (auto spi_word : spi_words)
        m_current_configuration.set(spi_word);
}

// ---------------------------------------------------------------------------- program_registers_channel_set
void Driver::program_registers_channel_set(uint8_t source_index,
                                           uint8_t target_index)
{
    auto& device_traits = Device_Traits::get(m_device_type);

    const Channel_Set_Settings* channel_set = &m_channel_set[source_index];
    uint32_t spi_words[3];
    uint32_t hp_gain_mask = 0;
    uint32_t i;

    uint32_t address_offset =
        (target_index / 2) * (BGT60TRxxC_REG_CS2_U_0 - BGT60TRxxC_REG_CS1_U_0)
        + (target_index & 1) * (BGT60TRxxC_REG_CS1_D_0 - BGT60TRxxC_REG_CS1_U_0);

    uint8_t tx1_enable =
        ((channel_set->tx_mode == Tx_Mode::Tx1_Only)
         || (channel_set->tx_mode == Tx_Mode::Alternating))
            ? 1
            : 0;
    uint8_t tx2_enable =
        ((channel_set->tx_mode == Tx_Mode::Tx2_Only)
         || (channel_set->tx_mode == Tx_Mode::Alternating))
            ? 1
            : 0;

    /* compile channel set settings into SPI configuration words */
    /* --------------------------------------------------------- */
    spi_words[0] = BGT60TRxxC_SET(CS1_U_0, VCO_EN, 1)
                   | BGT60TRxxC_SET(CS1_U_0, FDIV_EN, 1);
    spi_words[2] = 0;

    if (device_traits.cs_register_layout == Cs_Layout_t::Version1)
    {
        spi_words[0] |= BGT60TRxxC_SET(CS1_U_0, TEST_DIV_EN, 0)
                        | BGT60TRxxC_SET(CS1_U_0, BBCHGLOB_EN, 1);

        spi_words[1] = BGT60TRxxC_SET(CS1_U_1, MADC_EN, 1)
                       | BGT60TRxxC_SET(CS1_U_1, TEMP_MEAS_EN, 1)
                       | BGT60TRxxC_SET(CS1_U_1, BB_RSTCNT,
                                        channel_set->reset_period_reg)
                       | BGT60TRxxC_SET(CS1_U_1, BBCH_SEL, channel_set->rx_mask);
    }
    else
    {
        spi_words[1] =
            BGT60TRxxD_SET(CS1_U_1, BB_RSTCNT,
                           channel_set->reset_period_reg)
            | BGT60TRxxD_SET(CS1_U_1, MADC_BBCH1_EN, (channel_set->rx_mask & 1) ? 1 : 0)
            | BGT60TRxxD_SET(CS1_U_1, MADC_BBCH2_EN, (channel_set->rx_mask & 2) ? 1 : 0)
            | BGT60TRxxD_SET(CS1_U_1, MADC_BBCH3_EN, (channel_set->rx_mask & 4) ? 1 : 0)
            | BGT60TRxxD_SET(CS1_U_1, MADC_BBCH4_EN, (channel_set->rx_mask & 8) ? 1 : 0);

        if (!device_traits.has_sadc)
        {
            spi_words[0] |= BGT60TRxxD_SET(CS1_U_0, TEMP_MEAS_EN,
                                           m_temperature_sensing_enabled ? 1 : 0)
                            | BGT60TRxxD_SET(CS1_U_0, PD1_EN,
                                             m_power_sensing_enabled ? 1 : 0);
        }
        else
        {
            spi_words[0] |= BGT60TRxxD_SET(CS1_U_0, TEMP_MEAS_EN, 1);
        }

        if (device_traits.has_local_oscillator_frequency_doubler)
        {
            spi_words[0] |= BGT120TR24E_SET(CS1_U_0, LO_DBL_CTRL,
                                            channel_set->lo_doubler_power_reg);
        }
    }

    /* LO_DISTx_EN is different for different devices */
    if ((device_traits.num_tx_antennas == 1)
        && (device_traits.num_rx_antennas == 3))
    {
        if (channel_set->rx_mask & 0x05)
            spi_words[0] |= BGT60TRxxC_SET(CS1_U_0, LO_DIST2_EN, 1);

        if ((channel_set->rx_mask & 0x02) || tx1_enable)
            spi_words[0] |= BGT60TRxxC_SET(CS1_U_0, LO_DIST1_EN, 1);
    }
    else
    {
        if (channel_set->rx_mask != 0)
            spi_words[0] |= BGT60TRxxC_SET(CS1_U_0, LO_DIST2_EN, 1);

        if (tx1_enable || tx2_enable)
            spi_words[0] |= BGT60TRxxC_SET(CS1_U_0, LO_DIST1_EN, 1);
    }

    /* TX antenna 1 */
    spi_words[0] |= BGT60TRxxC_SET(CS1_U_0, TX1_EN, tx1_enable)
                    | BGT60TRxxC_SET(CS1_U_0, PD1_EN, 0);
    spi_words[1] |= BGT60TRxxC_SET(CS1_U_1, TX1_DAC,
                                   tx1_enable ? channel_set->tx_power : 0);

    /* TX antenna 2 */
    if (device_traits.num_tx_antennas >= 2)
    {
        spi_words[0] |= BGT60TRxxC_SET(CS1_U_0, TX2_EN, tx2_enable)
                        | BGT60TRxxC_SET(CS1_U_0, PD2_EN, 0);
        spi_words[1] |= BGT60TRxxC_SET(CS1_U_1, TX2_DAC,
                                       tx2_enable ? channel_set->tx_power : 0);
    }

    /* RX antenna 1 */
    spi_words[0] |= BGT60TRxxC_SET(CS1_U_0, RX1LOBUF_EN,
                                   (channel_set->rx_mask & 1) ? 1 : 0)
                    | BGT60TRxxC_SET(CS1_U_0, RX1MIX_EN,
                                     (channel_set->rx_mask & 1) ? 1 : 0);
    if (device_traits.cs_register_layout == Cs_Layout_t::Version1)
    {
        spi_words[2] |= BGT60TRxxC_SET(CS1_U_2, HPF_SEL1,
                                       channel_set->hp_cutoff_1_reg)
                        | BGT60TRxxC_SET(CS1_U_2, VGA_GAIN1,
                                         channel_set->vga_gain_1);
        hp_gain_mask = (channel_set->hp_gain_1 == Hp_Gain::_18dB) ? 1 : 0;
    }
    else
    {
        spi_words[1] |=
            BGT60TRxxD_SET(CS1_U_1, HP1_GAIN,
                           (channel_set->hp_gain_1 == Hp_Gain::_18dB) ? 1 : 0);
        spi_words[2] |= BGT60TRxxD_SET(CS1_U_2, HPF_SEL1,
                                       channel_set->hp_cutoff_1_reg)
                        | BGT60TRxxD_SET(CS1_U_2, VGA_GAIN1,
                                         channel_set->vga_gain_1);

        if (device_traits.cs_register_layout == Cs_Layout_t::Version2)
        {
            spi_words[0] |= BGT60TRxxD_SET(CS1_U_0, ABB1_AAF_CTRL,
                                           channel_set->aaf_cutoff_1_reg);
        }
        else
        {
            spi_words[0] |= BGT120TR24E_SET(CS1_U_0, ABB_AAF_CTRL,
                                            channel_set->aaf_cutoff_1_reg);
        }
    }

    /* RX antenna 2 */
    if (device_traits.num_rx_antennas >= 2)
    {
        spi_words[0] |= BGT60TRxxC_SET(CS1_U_0, RX2LOBUF_EN,
                                       (channel_set->rx_mask & 2) ? 1 : 0)
                        | BGT60TRxxC_SET(CS1_U_0, RX2MIX_EN,
                                         (channel_set->rx_mask & 2) ? 1 : 0);
        if (device_traits.cs_register_layout == Cs_Layout_t::Version1)
        {
            spi_words[2] |= BGT60TRxxC_SET(CS1_U_2, HPF_SEL2,
                                           channel_set->hp_cutoff_2_reg)
                            | BGT60TRxxC_SET(CS1_U_2, VGA_GAIN2,
                                             channel_set->vga_gain_2);
            hp_gain_mask |= (channel_set->hp_gain_2 == Hp_Gain::_18dB) ? 2 : 0;
        }
        else
        {
            spi_words[1] |=
                BGT60TRxxD_SET(CS1_U_1, HP2_GAIN,
                               (channel_set->hp_gain_2 == Hp_Gain::_18dB) ? 1 : 0);
            spi_words[2] |= BGT60TRxxD_SET(CS1_U_2, HPF_SEL2,
                                           channel_set->hp_cutoff_2_reg)
                            | BGT60TRxxD_SET(CS1_U_2, VGA_GAIN2,
                                             channel_set->vga_gain_2);

            if (device_traits.cs_register_layout == Cs_Layout_t::Version2)
            {
                spi_words[0] |= BGT60TRxxD_SET(CS1_U_0, ABB2_AAF_CTRL,
                                               channel_set->aaf_cutoff_2_reg);
            }
        }
    }

    /* RX antenna 3 */
    if (device_traits.num_rx_antennas >= 3)
    {
        spi_words[0] |= BGT60TRxxC_SET(CS1_U_0, RX3LOBUF_EN,
                                       (channel_set->rx_mask & 4) ? 1 : 0)
                        | BGT60TRxxC_SET(CS1_U_0, RX3MIX_EN,
                                         (channel_set->rx_mask & 4) ? 1 : 0);
        if (device_traits.cs_register_layout == Cs_Layout_t::Version1)
        {
            spi_words[2] |= BGT60TRxxC_SET(CS1_U_2, HPF_SEL3,
                                           channel_set->hp_cutoff_3_reg)
                            | BGT60TRxxC_SET(CS1_U_2, VGA_GAIN3,
                                             channel_set->vga_gain_3);
            hp_gain_mask |= (channel_set->hp_gain_3 == Hp_Gain::_18dB) ? 4 : 0;
        }
        else
        {
            spi_words[1] |=
                BGT60TRxxD_SET(CS1_U_1, HP3_GAIN,
                               (channel_set->hp_gain_3 == Hp_Gain::_18dB) ? 1 : 0);
            spi_words[2] |= BGT60TRxxD_SET(CS1_U_2, HPF_SEL3,
                                           channel_set->hp_cutoff_3_reg)
                            | BGT60TRxxD_SET(CS1_U_2, VGA_GAIN3,
                                             channel_set->vga_gain_3);

            if (device_traits.cs_register_layout == Cs_Layout_t::Version2)
            {
                spi_words[0] |= BGT60TRxxD_SET(CS1_U_0, ABB3_AAF_CTRL,
                                               channel_set->aaf_cutoff_3_reg);
            }
        }
    }

    /* RX antenna 4 */
    if (device_traits.num_rx_antennas >= 4)
    {
        spi_words[0] |= BGT60TRxxC_SET(CS1_U_0, RX4LOBUF_EN,
                                       (channel_set->rx_mask & 8) ? 1 : 0)
                        | BGT60TRxxC_SET(CS1_U_0, RX4MIX_EN,
                                         (channel_set->rx_mask & 8) ? 1 : 0);
        if (device_traits.cs_register_layout == Cs_Layout_t::Version1)
        {
            spi_words[2] |= BGT60TRxxC_SET(CS1_U_2, HPF_SEL4,
                                           channel_set->hp_cutoff_4_reg)
                            | BGT60TRxxC_SET(CS1_U_2, VGA_GAIN4,
                                             channel_set->vga_gain_4);
            hp_gain_mask |= (channel_set->hp_gain_4 == Hp_Gain::_18dB) ? 8 : 0;
        }
        else
        {
            spi_words[1] |=
                BGT60TRxxD_SET(CS1_U_1, HP4_GAIN,
                               (channel_set->hp_gain_4 == Hp_Gain::_18dB) ? 1 : 0);
            spi_words[2] |= BGT60TRxxD_SET(CS1_U_2, HPF_SEL4,
                                           channel_set->hp_cutoff_4_reg)
                            | BGT60TRxxD_SET(CS1_U_2, VGA_GAIN4,
                                             channel_set->vga_gain_4);

            if (device_traits.cs_register_layout == Cs_Layout_t::Version2)
            {
                spi_words[0] |= BGT60TRxxD_SET(CS1_U_0, ABB4_AAF_CTRL,
                                               channel_set->aaf_cutoff_4_reg);
            }
        }
    }

    if (device_traits.cs_register_layout == Cs_Layout_t::Version1)
        spi_words[2] |= BGT60TRxxC_SET(CS1_U_2, HP_GAIN, hp_gain_mask);

    /* adjust register addresses */
    for (i = 0; i < 3; ++i)
    {
        spi_words[i] += address_offset << 25;
    }

    for (auto spi_word : spi_words)
        m_current_configuration.set(spi_word);
}

// ---------------------------------------------------------------------------- program_registers_power
void Driver::program_registers_power(bool deep_sleep)
{
    auto& device_traits = Device_Traits::get(m_device_type);

    const Power_Down_Configuration* power_settings = deep_sleep ? &m_deep_sleep_settings : &m_idle_settings;
    uint32_t spi_words[4];

    /* compile parameters into an SPI configuration word,
     * turn off all RF, base band and ADC
     */
    spi_words[0] =
        BGT60TRxxC_SET(CSP_I_0, TX1_EN, 0)
        | BGT60TRxxC_SET(CSP_I_0, PD1_EN, 0)
        | BGT60TRxxC_SET(CSP_I_0, TX2_EN, 0)
        | BGT60TRxxC_SET(CSP_I_0, PD2_EN, 0)
        | BGT60TRxxC_SET(CSP_I_0, VCO_EN, power_settings->enable_vco ? 1 : 0)
        | BGT60TRxxC_SET(CSP_I_0, FDIV_EN, power_settings->enable_fdiv ? 1 : 0)
        | BGT60TRxxC_SET(CSP_I_0, RX1LOBUF_EN, 0)
        | BGT60TRxxC_SET(CSP_I_0, RX1MIX_EN, 0)
        | BGT60TRxxC_SET(CSP_I_0, RX2LOBUF_EN, 0)
        | BGT60TRxxC_SET(CSP_I_0, RX2MIX_EN, 0)
        | BGT60TRxxC_SET(CSP_I_0, RX3LOBUF_EN, 0)
        | BGT60TRxxC_SET(CSP_I_0, RX3MIX_EN, 0)
        | BGT60TRxxC_SET(CSP_I_0, RX4LOBUF_EN, 0)
        | BGT60TRxxC_SET(CSP_I_0, RX4MIX_EN, 0)
        | BGT60TRxxC_SET(CSP_I_0, LO_DIST1_EN, 0)
        | BGT60TRxxC_SET(CSP_I_0, LO_DIST2_EN, 0);

    spi_words[1] =
        BGT60TRxxC_SET(CSP_I_1, TX1_DAC, 0)
        | BGT60TRxxC_SET(CSP_I_1, TX2_DAC, 0)
        | BGT60TRxxC_SET(CSP_I_1, BBCH_SEL, 0);

    if (device_traits.cs_register_layout == Cs_Layout_t::Version1)
    {
        spi_words[0] |= BGT60TRxxC_SET(CSP_I_0, TEST_DIV_EN, 0)
                        | BGT60TRxxC_SET(CSP_I_0, BBCHGLOB_EN, 0);

        spi_words[1] |= BGT60TRxxC_SET(CSP_I_1, MADC_EN, 0)
                        | BGT60TRxxC_SET(CSP_I_1, TEMP_MEAS_EN, 0)
                        | BGT60TRxxC_SET(CSP_I_1, BB_RSTCNT, 0);

        spi_words[2] = BGT60TRxxC_SET(CSP_I_2, HPF_SEL1, 0)
                       | BGT60TRxxC_SET(CSP_I_2, VGA_GAIN1, 0)
                       | BGT60TRxxC_SET(CSP_I_2, HPF_SEL2, 0)
                       | BGT60TRxxC_SET(CSP_I_2, VGA_GAIN2, 0)
                       | BGT60TRxxC_SET(CSP_I_2, HPF_SEL3, 0)
                       | BGT60TRxxC_SET(CSP_I_2, VGA_GAIN3, 0)
                       | BGT60TRxxC_SET(CSP_I_2, HPF_SEL4, 0)
                       | BGT60TRxxC_SET(CSP_I_2, VGA_GAIN4, 0)
                       | BGT60TRxxC_SET(CSP_I_2, HP_GAIN, 0);
    }
    else
    {
        if (device_traits.cs_register_layout == Cs_Layout_t::Version2)
        {
            spi_words[0] |= BGT60TRxxD_SET(CSP_I_0, TEMP_MEAS_EN, 0)
                            | BGT60TRxxD_SET(CSP_I_0, ABB1_AAF_CTRL, 0)
                            | BGT60TRxxD_SET(CSP_I_0, ABB2_AAF_CTRL, 0)
                            | BGT60TRxxD_SET(CSP_I_0, ABB3_AAF_CTRL, 0)
                            | BGT60TRxxD_SET(CSP_I_0, ABB4_AAF_CTRL, 0);
        }
        else
        {
            spi_words[0] |= BGT120TR24E_SET(CSP_I_0, TEMP_MEAS_EN, 0)
                            | BGT120TR24E_SET(CSP_I_0, ABB_AAF_CTRL, 0);
        }

        spi_words[1] |= BGT60TRxxD_SET(CSP_I_1, BB_RSTCNT, 0)
                        | BGT60TRxxD_SET(CSP_I_1, HP1_GAIN, 0)
                        | BGT60TRxxD_SET(CSP_I_1, HP2_GAIN, 0)
                        | BGT60TRxxD_SET(CSP_I_1, HP3_GAIN, 0)
                        | BGT60TRxxD_SET(CSP_I_1, HP4_GAIN, 0);
        spi_words[2] = BGT60TRxxD_SET(CSP_I_2, HPF_SEL1, 0)
                       | BGT60TRxxD_SET(CSP_I_2, VGA_GAIN1, 0)
                       | BGT60TRxxD_SET(CSP_I_2, HPF_SEL2, 0)
                       | BGT60TRxxD_SET(CSP_I_2, VGA_GAIN2, 0)
                       | BGT60TRxxD_SET(CSP_I_2, HPF_SEL3, 0)
                       | BGT60TRxxD_SET(CSP_I_2, VGA_GAIN3, 0)
                       | BGT60TRxxD_SET(CSP_I_2, HPF_SEL4, 0)
                       | BGT60TRxxD_SET(CSP_I_2, VGA_GAIN4, 0);
    }

    spi_words[3] =
        BGT60TRxxC_SET(CSCI, REPC, 0)
        | BGT60TRxxC_SET(CSCI, CS_EN, 0)
        | BGT60TRxxC_SET(CSCI, ABB_ISOPD,
                         power_settings->enable_baseband ? 0 : 1)
        | BGT60TRxxC_SET(CSCI, RF_ISOPD, power_settings->enable_rf ? 0 : 1)
        | BGT60TRxxC_SET(CSCI, BG_EN,
                         power_settings->enable_madc_bandgap ? 1 : 0)
        | BGT60TRxxC_SET(CSCI, MADC_ISOPD,
                         power_settings->enable_madc ? 0 : 1)
        | BGT60TRxxC_SET(CSCI, BG_TMRF_EN,
                         power_settings->enable_sadc_bandgap ? 1 : 0)
        | BGT60TRxxC_SET(CSCI, PLL_ISOPD, power_settings->enable_pll ? 0 : 1);
    if (device_traits.has_sadc)
        spi_words[3] |= BGT60TRxxC_SET(CSCI, SADC_ISOPD,
                                       power_settings->enable_sadc ? 0 : 1);

    if (device_traits.has_extra_startup_delays && !deep_sleep)
    {
        spi_words[3] |= BGT60TRxxD_SET(CSCI, TR_PLL_ISOPD,
                                       m_pll_enable_delay_reg)
                        | BGT60TRxxD_SET(CSCI, TR_MADCEN, m_madc_delay_reg)
                        | BGT60TRxxD_SET(CSCI, TR_BGEN, m_bandgap_delay_reg);
    }

    /* adjust register addresses for deep sleep mode */
    if (deep_sleep)
    {
        uint8_t i;
        uint32_t address_offset = BGT60TRxxC_REG_CSP_D_0 - BGT60TRxxC_REG_CSP_I_0;
        for (i = 0; i < 4; ++i)
        {
            spi_words[i] += address_offset << 25;
        }
    }

    for (auto spi_word : spi_words)
        m_current_configuration.set(spi_word);
}

// ---------------------------------------------------------------------------- program_registers_clock
void Driver::program_registers_clock()
{
    auto& device_traits = Device_Traits::get(m_device_type);

    uint32_t spi_word;

    /* This feature was first supported by BGT60TR13D (ID=6) */
    if (!device_traits.has_ref_frequency_doubler)
        return;

    /* enable test signal generator */
    /*
     * Note that FD:DC_IN is set twice. This is because DC_IN contains actually
     * two bit fields. The highest bit of DC_IN is an invert flag. It seems
     * that this was not clear at the time when the register definition header
     * was generated.
     */
    uint32_t clk_sel = 0;
    if (m_enable_frequency_doubler)
    {
        switch (m_dc_correction.mode)
        {
            case Duty_Cycle_Correction_Mode::Only_Out: clk_sel = 1; break;
            case Duty_Cycle_Correction_Mode::In_Out: clk_sel = 2; break;
            case Duty_Cycle_Correction_Mode::SysIn_Out: clk_sel = 3; break;
        }
    }

    spi_word = BGT60TRxxD_SET(FD, CLK_SEL, clk_sel)
               | BGT60TRxxD_SET(FD, DC_IN, m_dc_correction.adjust_in)
               | BGT60TRxxD_SET(FD, DC_IN,
                                m_dc_correction.invert_input ? 0x10 : 0)
               | BGT60TRxxD_SET(FD, DC_OUT, m_dc_correction.adjust_out + 8)
               | BGT60TRxxD_SET(FD, IRQ_FD_SEL, 0);

    if (device_traits.has_internal_oscillator)
    {
        const auto& osc_config = m_oscillator_configuration;
        spi_word |= BGT120TR24E_SET(CLK, XOSC_CLK_EXT_EN,
                                    osc_config.enable_clock_output ? 1 : 0);

        if (osc_config.clock_source == Clock_Source::Internal)
        {
            /*
             * Internal Clock
             * The final register value to be programmed at the end of the
             * startup sequence is set here. A dedicated startup routine is
             * responsible for setting the according bits one after the other
             * with the right delay.
             */
            spi_word |= BGT120TR24E_SET(CLK, XOSC_CLK_SEL, 0)
                        | BGT120TR24E_SET(CLK, XOSC_HP_MODE, 1)
                        | BGT120TR24E_SET(CLK, XOSC_FILT_EN, 1)
                        | BGT120TR24E_SET(CLK, XOSC_BUF_EN, 1)
                        | BGT120TR24E_SET(CLK, XOSC_CORE_EN, 1)
                        | BGT120TR24E_SET(CLK, XOSC_LDO_EN, 1)
                        | BGT120TR24E_SET(CLK, XOSC_BG_RFILT_SEL, 1)
                        | BGT120TR24E_SET(CLK, XOSC_BG_EN, 1);

            if (osc_config.amplitude == Clock_Amplitude::Increased)
                spi_word |= BGT120TR24E_SET(CLK, XOSC_AMP_SEL, 1);

            switch (osc_config.ldo_voltage)
            {
                case Oscillator_Ldo_Voltage::_1V44:
                    spi_word |= BGT120TR24E_SET(CLK, XOSC_LDO_VPROG, 0);
                    break;

                case Oscillator_Ldo_Voltage::_1V5:
                    spi_word |= BGT120TR24E_SET(CLK, XOSC_LDO_VPROG, 1);
                    break;

                case Oscillator_Ldo_Voltage::_1V55:
                    spi_word |= BGT120TR24E_SET(CLK, XOSC_LDO_VPROG, 2);
                    break;

                case Oscillator_Ldo_Voltage::_1V6:
                    spi_word |= BGT120TR24E_SET(CLK, XOSC_LDO_VPROG, 3);
                    break;
            }
        }
        else
        {
            // External Clock -> Internal Oscillator is kept disabled.
            spi_word |= BGT120TR24E_SET(CLK, XOSC_CLK_SEL, 1);
        }
    }

    if (device_traits.has_reordered_register_layout)
    {
        spi_word &= 0x00FFFFFF;
        spi_word |= BGT60TRxxE_SET(FD, IRQ_FD_SEL, 0);
    }
    m_current_configuration.set(spi_word);
}

// ---------------------------------------------------------------------------- update_spi_register_set
void Driver::update_spi_register_set()
{
    auto& device_traits = Device_Traits::get(m_device_type);

    uint32_t spi_words[3];
    spi_words[0] = PACR1_SETTINGS;
    spi_words[1] = PACR2_SETTINGS
                   | BGT60TRxxC_SET(PACR2, DIVSET, m_pll_div_set);
    if (device_traits.has_extra_startup_delays)
        spi_words[1] |= BGT60TRxxD_SET(PACR2, TR_DIVEN, m_pll_divider_delay_reg);

    spi_words[2] = BGT60TRxxC_SET(RFT1, TX1_SPARE, 0);
    for (auto spi_word : spi_words)
        m_current_configuration.set(spi_word);

    program_registers_main();
    program_registers_fifo();
    program_registers_madc();
    if (device_traits.has_sadc)
        program_registers_sadc();
    program_registers_power(false /* idle */);
    program_registers_power(true /* deep sleep */);
    program_registers_ccr();
    program_registers_frame();
    program_registers_clock();

    for (uint8_t i = 0; i < 4; ++i)
    {
        const Shape_Settings* current_shape = &m_shape[i];
        uint8_t channel_set_idx;

        if (current_shape->num_repetitions == 0)
        {
            continue;
        }

        program_registers_shape(i);

        /* for certain shape types up and down channel sets must be swapped
         * because BGT60TRxx assumes that the first chirp is always up
         */
        channel_set_idx =
            ((current_shape->shape_type == Shape_Type::Saw_Down)
             || (current_shape->shape_type == Shape_Type::Tri_Down))
                ? 2 * i + 1
                : 2 * i;

        /* update channel set settings (up) */
        program_registers_channel_set(channel_set_idx, 2 * i);

        /* update channel set settings (down) */
        if (current_shape->shape_type == Shape_Type::Tri_Down)
        {
            program_registers_channel_set(2 * i, 2 * i + 1);
        }
        else if (current_shape->shape_type == Shape_Type::Tri_Up)
        {
            program_registers_channel_set(2 * i + 1, 2 * i + 1);
        }
    }

    /*
     * The following registers are just written to their defaults to allow
     * overwriting with register modification masks. If they are not programmed
     * through schedule_spi_register_update modification masks can't be applied.
     */
    m_current_configuration.set(BGT60TRxxC_SET(SDFT0, TST_DIG_I, 0));
}

// ---------------------------------------------------------------------------- set_register_modification
Driver::Error Driver::set_register_modification(uint8_t register_address,
                                                uint32_t and_mask, uint32_t or_mask)
{
    m_reg_modifications[register_address].and_mask = and_mask | 0xFF000000;
    m_reg_modifications[register_address].or_mask = or_mask & 0x00FFFFFF;
    return Error::OK;
}

// ---------------------------------------------------------------------------- get_register_modification
Driver::Error Driver::get_register_modification(uint8_t register_address,
                                                uint32_t* and_mask,
                                                uint32_t* or_mask) const
{
    if (m_reg_modifications.count(register_address))
    {
        *and_mask = m_reg_modifications.at(register_address).and_mask;
        *or_mask = m_reg_modifications.at(register_address).or_mask;
    }
    else
    {
        *and_mask = 0xFFFFFFFF;
        *or_mask = 0x00000000;
    }
    return Error::OK;
}

// ---------------------------------------------------------------------------- clear_all_register_modifications
Driver::Error Driver::clear_all_register_modifications()
{
    m_reg_modifications.clear();
    return Error::OK;
}

/* ------------------------------------------------------------------------ */
}  // namespace Avian
}  // namespace Infineon

/* --- End of File -------------------------------------------------------- */
