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
 * @file DeInterleaver.hpp
 *
 * @brief Deinterleaving of radar data
 *
 */

#ifndef IFX_RADAR_DEINTERLEAVER_HPP
#define IFX_RADAR_DEINTERLEAVER_HPP

#include "ifxRadar/internal/DeInterleaver.h"
#include <vector>

/*
==============================================================================
   2. LOCAL DEFINITIONS
==============================================================================
*/

class DeInterleaver
{
    std::vector<ifx_Float_t> m_input;
    ifx_DeInterleaver_Frame_Definition_t m_frame_definition {};

public:
    void set_frame_definition(const ifx_DeInterleaver_Frame_Definition_t& frame_definition);
    size_t get_samples_per_frame() const;
    void add_input_data(const ifx_Float_t* first, const ifx_Float_t* last);
    bool is_frame_complete() const;
    std::vector<ifx_Float_t> get_deinterleaved_frame();

private:
    void direction_to_antenna_set_shape_samples(std::vector<ifx_Float_t>::iterator& out, bool downwards);
    void to_direction_antenna_set_shape_samples(std::vector<ifx_Float_t>::iterator& out);
};


#endif
