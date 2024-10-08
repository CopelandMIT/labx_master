/**
 * @file easy_mode.cpp
 *
 * This file contains the implementation of easy mode functionality. In easy
 * mode only one shape and channel set is available (as it was with BGT60TR24B).
 * Internally all shapes are configured to the easy mode settings.
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
#include "registers_BGT60TR11D.h"
#include "registers_BGT60TRxxC.h"

// ---------------------------------------------------------------------------- namespaces
namespace Infineon {
namespace Avian {

// ---------------------------------------------------------------------------- setup_easy_mode_frame
Driver::Error Driver::setup_easy_mode_frame(const Frame_Format* frame_format,
                                            const Fmcw_Configuration* fmcw_settings,
                                            Tx_Mode tx_mode)
{
    uint16_t shape_repetitions[4] = {0, 0, 0, 0};
    uint16_t num_chirps_per_frame = (uint16_t)frame_format->num_chirps_per_frame;
    uint16_t bit_position;
    uint8_t i;

    /* check if parameter combination is valid */
    /* --------------------------------------- */
    if ((tx_mode == Tx_Mode::Alternating)
        && ((fmcw_settings->shape_type == Shape_Type::Saw_Up)
            || (fmcw_settings->shape_type == Shape_Type::Saw_Down))
        && (num_chirps_per_frame != 2))
    {
        /* Alternating TX mode can easily be applied with triangle ramps in
         * a single shape. In sawtooth mode, two shapes are needed to setup
         * this pattern, but then the number of repetitions must per shape
         * must be 1 (=> total number of chirps is 2).
         */
        return Error::UNSUPPORTED_FRAME_FORMAT;
    }

    if ((tx_mode == Tx_Mode::Alternating)
        || (fmcw_settings->shape_type == Shape_Type::Tri_Up)
        || (fmcw_settings->shape_type == Shape_Type::Tri_Down))
    {
        if ((num_chirps_per_frame & 0x01) != 0)
        {
            /* In alternating TX mode, are in triangle mode, number of chirps
             * must be even.
             */
            return Error::UNSUPPORTED_FRAME_FORMAT;
        }
        else
        {
            /* in triangle mode two chirps are one shape so continue with
             * number of shapes instead of number of chirps
             */
            num_chirps_per_frame /= 2;
        }
    }

    /* check if specified number of chirps/shapes can be applied */
    /* (A shape repetition must be a power of two, but up to four shapes can
     * be combined.)
     */
    bit_position = 1;
    for (i = 0; i < 4; ++i)
    {
        if (num_chirps_per_frame == 0)
        {
            break;
        }
        /* find next set bit */
        while ((num_chirps_per_frame & bit_position) == 0)
        {
            bit_position <<= 1;
        }
        shape_repetitions[i] = bit_position;
        num_chirps_per_frame -= bit_position;
    }
    if ((bit_position > 32768) || (num_chirps_per_frame != 0))
    {
        /* specified number was to big, or can't be represented with 4 set
         * bits */
        return Error::UNSUPPORTED_FRAME_FORMAT;
    }

    /* apply shape and channel set parameters to all shapes */
    /* ---------------------------------------------------- */

    /* apply to shape one and channel set 1 first */
    m_shape[0].shape_type = fmcw_settings->shape_type;
    m_shape[0].lower_frequency_kHz = fmcw_settings->lower_frequency_kHz;
    m_shape[0].upper_frequency_kHz = fmcw_settings->upper_frequency_kHz;
    m_shape[0].num_samples_down = (uint16_t)frame_format->num_samples_per_chirp;
    m_shape[0].num_samples_up = (uint16_t)frame_format->num_samples_per_chirp;
    m_shape[0].post_delay = Scaled_Timer();

    m_channel_set[0].tx_power = fmcw_settings->tx_power;
    m_channel_set[0].rx_mask = frame_format->rx_mask;
    m_channel_set[0].num_rx_antennas =
        count_antennas(frame_format->rx_mask);

    /*
     * copy settings to all other shapes
     * (this copies also settings that are not touched here, e.g. base band
     * settings)
     */
    for (i = 1; i < 4; ++i)
    {
        m_shape[i] = m_shape[0];
    }
    for (i = 1; i < 8; ++i)
    {
        m_channel_set[i] = m_channel_set[0];
    }

    /* setup frame structure and parameters that are not equal */
    /* ------------------------------------------------------- */
    /* set TX mode */
    for (i = 0; i < 4; ++i)
    {
        if (tx_mode == Tx_Mode::Alternating)
        {
            if (fmcw_settings->shape_type == Shape_Type::Tri_Down)
            {
                m_channel_set[2 * i].tx_mode = Tx_Mode::Tx2_Only;
                m_channel_set[2 * i + 1].tx_mode = Tx_Mode::Tx1_Only;
            }
            else
            {
                m_channel_set[2 * i].tx_mode = Tx_Mode::Tx1_Only;
                m_channel_set[2 * i + 1].tx_mode = Tx_Mode::Tx2_Only;
            }
        }
        else
        {
            m_channel_set[2 * i].tx_mode = tx_mode;
            m_channel_set[2 * i + 1].tx_mode = tx_mode;
        }
    }

    /* set shape sequence */
    if ((tx_mode == Tx_Mode::Alternating)
        && ((fmcw_settings->shape_type == Shape_Type::Saw_Up)
            || (fmcw_settings->shape_type == Shape_Type::Saw_Down)))
    {
        /* In this special case, use shape 1 once and shape 2 once */
        m_shape[0].num_repetitions = 1;
        m_shape[0].following_power_mode = Power_Mode::Stay_Active;

        m_shape[1].num_repetitions = 1;
        m_shape[1].following_power_mode = Power_Mode::Idle;

        m_shape[2].num_repetitions = 0;
        m_shape[3].num_repetitions = 0;

        /* loop above has set all up chirps to TX1 and all down chirps to TX2,
         * but for this special case, up chirp of shape 2 must be set to TX1
         */
        m_channel_set[2].tx_mode = Tx_Mode::Tx2_Only;
    }
    else
    {
        for (i = 0; i < 4; ++i)
        {
            m_shape[i].num_repetitions = shape_repetitions[i];
        }
        for (i = 0; i < 3; ++i)
        {
            m_shape[i].following_power_mode =
                (shape_repetitions[i + 1] != 0) ? Power_Mode::Stay_Active : Power_Mode::Idle;
        }
        m_shape[3].following_power_mode =
            Power_Mode::Idle;
    }

    m_num_set_repetitions = 1;
    m_frame_end_power_mode = Power_Mode::Idle;

    return Error::OK;
}

// ---------------------------------------------------------------------------- set_easy_mode_buffer_size
void Driver::set_easy_mode_buffer_size(uint16_t num_samples)
{
    m_easy_mode_buffer_size = num_samples;
}

// ---------------------------------------------------------------------------- enable_easy_mode
Driver::Error Driver::enable_easy_mode(bool enable)
{
    if (enable)
    {
        Frame_Format frame_format;
        Fmcw_Configuration fmcw_settings;
        Tx_Mode tx_mode;
        size_t frame_size;
        uint8_t i;

        m_currently_selected_shape = 0;

        /* gather easy parameters */
        /* ---------------------- */
        frame_format.num_chirps_per_frame = 0;
        frame_format.num_samples_per_chirp = m_shape[0].num_samples_up;
        frame_format.rx_mask = m_channel_set[0].rx_mask;

        fmcw_settings.shape_type = m_shape[0].shape_type;
        fmcw_settings.lower_frequency_kHz = m_shape[0].lower_frequency_kHz;
        fmcw_settings.upper_frequency_kHz = m_shape[0].upper_frequency_kHz;
        fmcw_settings.tx_power = m_channel_set[0].tx_power;

        /* count current number of chirps in shape_set */
        for (i = 0; i < 4; ++i)
        {
            frame_format.num_chirps_per_frame += m_shape[i].num_repetitions;
        }
        if ((fmcw_settings.shape_type == Shape_Type::Tri_Up)
            || (fmcw_settings.shape_type == Shape_Type::Tri_Down))
        {
            frame_format.num_chirps_per_frame *= 2;
        }

        /* identify alternating TX mode, that may have been set before */
        tx_mode = m_channel_set[0].tx_mode;
        if (((fmcw_settings.shape_type == Shape_Type::Tri_Up)
             || (fmcw_settings.shape_type == Shape_Type::Tri_Down))
            && (m_channel_set[0].tx_mode == Tx_Mode::Tx1_Only)
            && (m_channel_set[1].tx_mode == Tx_Mode::Tx2_Only))
        {
            tx_mode = Tx_Mode::Alternating;
        }
        else if (((fmcw_settings.shape_type == Shape_Type::Saw_Up)
                  || (fmcw_settings.shape_type == Shape_Type::Saw_Down))
                 && (m_channel_set[0].tx_mode == Tx_Mode::Tx1_Only)
                 && (m_channel_set[2].tx_mode == Tx_Mode::Tx2_Only)
                 && (m_shape[0].num_repetitions == 1)
                 && (m_shape[1].num_repetitions == 1))
        {
            tx_mode = Tx_Mode::Alternating;
        }

        /*
         * if frame size is too big to keep two frames in software FIFO,
         * set number of chirps to one, to be on the safe side
         */
        frame_size = m_shape[0].num_samples_up
                     * m_channel_set[0].num_rx_antennas
                     * frame_format.num_chirps_per_frame;

        if ((frame_size + 2) * 2 > m_easy_mode_buffer_size)
        {
            frame_format.num_chirps_per_frame = 1;
            frame_size = m_shape[0].num_samples_up
                         * m_channel_set[0].num_rx_antennas
                         * frame_format.num_chirps_per_frame;
        }

        /* apply all settings */
        /* ------------------ */
        setup_easy_mode_frame(&frame_format, &fmcw_settings, tx_mode);
        m_slice_size = (uint16_t)frame_size;

        /* set number of frames to 1 for manual mode */
        m_num_frames_before_stop = 1;

        m_current_mode = MODE_EASY;

        update_spi_register_set();
    }
    else if (m_current_mode & MODE_EASY)
    {
        m_current_mode = MODE_NORMAL;
    }

    /* reset and force reprogramming with next trigger */
    reset(false);

    return Error::OK;
}

// ---------------------------------------------------------------------------- is_in_easy_mode
Driver::Error Driver::is_in_easy_mode(bool* enable) const
{
    *enable = (m_current_mode & MODE_EASY) ? 1 : 0;

    return Error::OK;
}

// ---------------------------------------------------------------------------- get_frame_info
void Driver::get_frame_info(Frame_Info* frame_info) const
{
    /* fill fields of frame info */
    frame_info->num_samples_per_chirp = m_shape[0].num_samples_up;
    frame_info->adc_resolution = 12;
    frame_info->interleaved_rx = 1;
    get_temperature(&frame_info->temperature_001C);

    frame_info->rx_mask = m_channel_set[0].rx_mask;
    frame_info->num_rx_antennas = m_channel_set[0].num_rx_antennas;
    frame_info->num_chirps = m_slice_size
                             / (frame_info->num_rx_antennas
                                * frame_info->num_samples_per_chirp);
}

// ---------------------------------------------------------------------------- set_automatic_frame_trigger
Driver::Error Driver::set_automatic_frame_trigger(uint32_t frame_interval_us)
{
    /* this function is only available in easy mode */
    if ((m_current_mode & MODE_EASY) == 0)
    {
        return Error::INCOMPATIBLE_MODE;
    }

    if (frame_interval_us != 0)
    {
        uint32_t min_interval_us;
        uint64_t clock_cycles;
        get_min_frame_interval(&min_interval_us);

        if (frame_interval_us < min_interval_us)
        {
            return Error::UNSUPPORTED_FRAME_INTERVAL;
        }

        /*
         * The minimum frame interval is also the time needed to process a
         * single frame. To get the required frame end delay the minimum frame
         * end delay can be subtracted from the specified frame interval.
         * Afterwards the time is converted to clock cycles.
         */
        clock_cycles = frame_interval_us - min_interval_us;
        clock_cycles *= m_reference_clock_freq_Hz;
        clock_cycles = clock_cycles / 1000000;
        m_frame_end_delay = Scaled_Timer(clock_cycles,
                                         BGT60TRXX_MAX_SHAPE_END_DELAY_SHIFT,
                                         BGT60TRXX_MAX_SHAPE_END_DELAY);

        /* set number of frames to 0 for automatic trigger mode */
        m_num_frames_before_stop = 0;

        /* update SPI registers of BGT60TR24 chip */
        /* --------------------------------------- */
        update_spi_register_set();

        /* trigger sequence */
        m_current_mode = MODE_EASY_TRIGGER_RUNNING;
    }

    return Error::OK;
}

// ---------------------------------------------------------------------------- is_automatic_frame_trigger_running
bool Driver::is_automatic_frame_trigger_running() const
{
    return m_current_mode == MODE_EASY_TRIGGER_RUNNING;
}

/* ------------------------------------------------------------------------ */
}  // namespace Avian
}  // namespace Infineon

/* --- End of File -------------------------------------------------------- */
