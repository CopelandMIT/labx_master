/**
 * @file data_acquisition.cpp
 *
 * This file contains the implementation of data acquisition. Functions that
 * control processing of programmed chirp sequences and reading data from
 * BGT60TR24B are defined here.
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
#include "registers_BGT60TRxxD.h"
#include "registers_BGT60TRxxE.h"

// ---------------------------------------------------------------------------- namespaces
namespace Infineon {
namespace Avian {

// ---------------------------------------------------------------------------- reset
void Driver::reset(bool soft_reset)
{
    /* do the reset */
    if (soft_reset)
    {
        /* set trigger bit in main register */
        auto spi_word = m_current_configuration[BGT60TRxxC_REG_MAIN]
                        | BGT60TRxxC_SET(MAIN, FIFO_RESET, 1)
                        | BGT60TRxxC_SET(MAIN, FSM_RESET, 1);

        /* send configuration to chip */
        m_port.send_commands(&spi_word, 1);
    }
    else
    {
        m_port.generate_reset_sequence();
    }

    /* remember reset state */
    m_reset_state = true;
}

// ---------------------------------------------------------------------------- set_slice_size
Driver::Error Driver::set_slice_size(uint16_t num_samples)
{
    auto& device_traits = Device_Traits::get(m_device_type);

    if ((num_samples == 0) || (num_samples > device_traits.fifo_size * 2))
    {
        return Error::UNSUPPORTED_FIFO_SLICE_SIZE;
    }

    m_slice_size = num_samples;

    /* update SPI registers of BGT60TR24 chip */
    /* --------------------------------------- */
    update_spi_register_set();

    return Error::OK;
}

// ---------------------------------------------------------------------------- get_slice_size
Driver::Error Driver::get_slice_size(uint16_t* num_samples) const
{
    *num_samples = m_slice_size;
    return Error::OK;
}

// ---------------------------------------------------------------------------- notify_trigger
void Driver::notify_trigger()
{
    m_reset_state = false;
}

// ---------------------------------------------------------------------------- stop_and_reset_sequence
Driver::Error Driver::stop_and_reset_sequence()
{
    /* stop processing */
    reset(true);

    if (m_current_mode & MODE_EASY)
    {
        m_current_mode = MODE_EASY;

        /* set number of frames to 1 for manual mode */
        m_num_frames_before_stop = 1;
        update_spi_register_set();
    }

    return Error::OK;
}

// ---------------------------------------------------------------------------- get_burst_prefix
HW::Spi_Command_t Driver::get_burst_prefix() const
{
    auto& device_traits = Device_Traits::get(m_device_type);

    return 0xFF000000
           | (uint32_t(device_traits.num_registers) << 17)
           | device_traits.num_registers;
}

// ---------------------------------------------------------------------------- check_fifo_overflow
Driver::Error Driver::check_fifo_overflow(HW::IControlPort* port)
{
    auto& device_traits = Device_Traits::get(m_device_type);

    /*
     * In case of a timeout, check if a FIFO overflow has happened. If so
     * this information must be returned to the caller, because in case of
     * a "normal" timeout a later call might return data, but in case of an
     * overflow no more data will arrive.
     */
    HW::Spi_Command_t fstat_spi_word = device_traits.has_reordered_register_layout
                                           ? BGT60TRxxE_REGISTER_READ_CMD(FSTAT)
                                           : BGT60TRxxC_REGISTER_READ_CMD(FSTAT);
    port->send_commands(&fstat_spi_word, 1, &fstat_spi_word);

    return (BGT60TRxxC_EXTRACT(FSTAT, FOF_ERR, fstat_spi_word))
               ? Error::FIFO_OVERFLOW
               : Error::OK;
}

/* ------------------------------------------------------------------------ */
}  // namespace Avian
}  // namespace Infineon

/* --- End of File -------------------------------------------------------- */
