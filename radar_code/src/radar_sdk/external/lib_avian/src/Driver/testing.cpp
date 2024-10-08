/**
    @file testing.cpp

    This file is part of the BGT60TRxx driver.

    This file contains functions to configure the testing functionality of the
    BGT60TRxx chip.
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
#include "ifxAvian_DeviceTraits.hpp"
#include "ifxAvian_Driver.hpp"
#include "registers_BGT60TRxxC.h"
#include "registers_BGT60TRxxD.h"
#include "registers_BGT60TRxxE.h"

// ---------------------------------------------------------------------------- namespaces
namespace Infineon {
namespace Avian {

// ---------------------------------------------------------------------------- dump_registers
Driver::Error Driver::dump_registers(uint32_t* register_data, uint8_t* num_registers) const
{
    auto& device_traits = Device_Traits::get(m_device_type);

    HW::Spi_Command_t spi_words[BGT60TRxxE_NUM_REGISTERS];
    uint8_t i;

    /* setup SPI words for read out */
    uint8_t num_read_regs = device_traits.num_registers;
    for (i = 0; i < num_read_regs; ++i)
    {
        /*
         *  setup read register command:
         *  - address in upper bits 25...31,
         *  - R/W in bit 24 (0 = read)
         *  - bits 0...23 are don't care, send 0
         */
        spi_words[i] = i << 25;
    }

    /* send words to chip and read registers back */
    m_port.send_commands(spi_words, num_read_regs, spi_words);

    /* copy data back to buffer */
    *num_registers = (*num_registers > num_read_regs) ? num_read_regs : *num_registers;

    for (i = 0; i < *num_registers; ++i)
    {
        register_data[i] = spi_words[i] & 0x00FFFFFF;
        register_data[i] |= i << 24;
    }

    return Error::OK;
}

/* ------------------------------------------------------------------------ */
}  // namespace Avian
}  // namespace Infineon

/* --- End of File -------------------------------------------------------- */
