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
 * @file DeInterleaver.cpp
 *
 * @brief Raw data application main program source file.
 *
 */

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#define NOMINMAX  // for MSVC
#include "DeInterleaver.hpp"
#include "ifxBase/Exception.hpp"
#include "ifxBase/internal/Util.h"
#include <algorithm>
#include <cstring>

static size_t get_samples_per_chirp(const ifx_DeInterleaver_Frame_Definition_t::shape_s::chirp_s& chirp)
{
    return chirp.samples_per_chirp * ifx_util_popcount(chirp.rx_mask);
}

static size_t get_samples_per_shape(const ifx_DeInterleaver_Frame_Definition_t::shape_s& shape)
{
    return get_samples_per_chirp(shape.up)
           + get_samples_per_chirp(shape.down);
}

void DeInterleaver::set_frame_definition(const ifx_DeInterleaver_Frame_Definition_t& frame_definition)
{
    m_frame_definition = frame_definition;
    m_input.clear();
    m_input.reserve(get_samples_per_frame() * 2);
}

size_t DeInterleaver::get_samples_per_frame() const
{
    size_t shape_set_size = 0;

    for (const auto& shape : m_frame_definition.shape)
    {
        const auto samples_per_shape = get_samples_per_shape(shape);

        shape_set_size += samples_per_shape * shape.repeat;
    }

    shape_set_size *= m_frame_definition.shape_set_repeat;

    return shape_set_size;
}

void DeInterleaver::add_input_data(const ifx_Float_t* first, const ifx_Float_t* last)
{
    m_input.insert(m_input.end(),
                   first,
                   last);
}

void DeInterleaver::direction_to_antenna_set_shape_samples(std::vector<ifx_Float_t>::iterator& out, bool downwards)
{
    struct
    {
        size_t active_antennas;
        size_t size_chirp;
        size_t shape_offset;
    } indexing[4];
    size_t size_shape_set = 0;
    for (size_t i_shape = 0; i_shape < 4; i_shape++)
    {
        const auto& shape = m_frame_definition.shape[i_shape];
        const auto& cur_chirp = downwards ? shape.down : shape.up;
        const auto& alt_chirp = downwards ? shape.up : shape.down;

        const size_t ant_cur_chirp = ifx_util_popcount(cur_chirp.rx_mask);
        const size_t size_cur_chirp = cur_chirp.samples_per_chirp * ant_cur_chirp;

        const size_t ant_alt_chirp = ifx_util_popcount(alt_chirp.rx_mask);
        const size_t size_alt_chirp = alt_chirp.samples_per_chirp * ant_alt_chirp;

        size_t size_chirp = size_cur_chirp + size_alt_chirp;
        size_t size_shape = size_chirp * shape.repeat;

        indexing[i_shape].active_antennas = ant_cur_chirp;
        indexing[i_shape].size_chirp = size_chirp;
        indexing[i_shape].shape_offset = size_shape_set;

        // if we are in a down-chirp, adjust the shape offset by the size of the preceding up-chirp
        if (downwards)
            indexing[i_shape].shape_offset += size_alt_chirp;

        size_shape_set += size_shape;
    }

    for (size_t i_ant = 0; i_ant < 32; i_ant++)
    {
        for (size_t i_shape = 0; i_shape < 4; i_shape++)
        {
            for (size_t i_set = 0; i_set < m_frame_definition.shape_set_repeat; i_set++)
            {
                const auto& shape = m_frame_definition.shape[i_shape];
                const auto& chirp = downwards ? shape.down : shape.up;

                const size_t antennas = indexing[i_shape].active_antennas;
                if (i_ant >= antennas)
                    break;  // early escape if the antenna isn't active

                for (size_t i_chirp = 0; i_chirp < shape.repeat; i_chirp++)
                {
                    const size_t base =
                        i_ant
                        + size_shape_set * i_set
                        + indexing[i_shape].shape_offset
                        + indexing[i_shape].size_chirp * i_chirp;

                    for (size_t i_sample = 0; i_sample < chirp.samples_per_chirp; i_sample++)
                    {
                        *out++ = m_input[base + i_sample * antennas];
                    }
                }
            }
        }
    }
}

void DeInterleaver::to_direction_antenna_set_shape_samples(std::vector<ifx_Float_t>::iterator& out)
{
    direction_to_antenna_set_shape_samples(out, false);
    direction_to_antenna_set_shape_samples(out, true);
}

bool DeInterleaver::is_frame_complete() const
{
    return m_input.size() >= get_samples_per_frame();
}

std::vector<ifx_Float_t> DeInterleaver::get_deinterleaved_frame()
{
    if (!is_frame_complete())
        throw rdk::exception::dimension_mismatch();

    const auto samples_per_frame = get_samples_per_frame();

    std::vector<ifx_Float_t> output;
    output.resize(samples_per_frame);

    auto it_out = output.begin();
    to_direction_antenna_set_shape_samples(it_out);

    m_input.erase(m_input.begin(), m_input.begin() + samples_per_frame);

    return output;
}

/* C-compatibility defines and implementation */

struct ifx_DeInterleaver_s : public DeInterleaver
{
    // This struct is just for C compatibility
};

ifx_DeInterleaver_t* ifx_di_create()
{
    try
    {
        return static_cast<ifx_DeInterleaver_s*>(new DeInterleaver());
    }
    catch (const rdk::exception::exception& e)
    {
        ifx_error_set(e.error_code());
        return nullptr;
    }
}

void ifx_di_destroy(ifx_DeInterleaver_t* handle)
{
    // a destructor must not throw exceptions, so no need to wrap the delete
    delete handle;
}

void ifx_di_set_frame_definition(ifx_DeInterleaver_t* handle, const ifx_DeInterleaver_Frame_Definition_t* frame_definition)
{
    try
    {
        handle->set_frame_definition(*frame_definition);
    }
    catch (const rdk::exception::exception& e)
    {
        ifx_error_set(e.error_code());
    }
}

void ifx_di_add_input_samples(ifx_DeInterleaver_t* handle, const ifx_Float_t* data, size_t length)
{
    try
    {
        handle->add_input_data(data, data + length);
    }
    catch (const rdk::exception::exception& e)
    {
        ifx_error_set(e.error_code());
    }
}

size_t ifx_di_get_samples_per_frame(ifx_DeInterleaver_t* handle)
{
    try
    {
        return handle->get_samples_per_frame();
    }
    catch (const rdk::exception::exception& e)
    {
        ifx_error_set(e.error_code());
        return 0;
    }
}

bool ifx_di_is_frame_complete(ifx_DeInterleaver_t* handle)
{
    try
    {
        return handle->is_frame_complete();
    }
    catch (const rdk::exception::exception& e)
    {
        ifx_error_set(e.error_code());
        return false;
    }
}

void ifx_di_get_frame(ifx_DeInterleaver_t* handle, ifx_Float_t* data, size_t length)
{
    try
    {
        const auto frame = handle->get_deinterleaved_frame();
        const auto sample_count = std::min(frame.size(), length);
        std::copy(frame.begin(), frame.begin() + sample_count,
                  data);
    }
    catch (const rdk::exception::exception& e)
    {
        ifx_error_set(e.error_code());
    }
}
