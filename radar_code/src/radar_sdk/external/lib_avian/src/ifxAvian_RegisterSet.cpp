/**
 * \file ifxAvian_RegisterSet.cpp
 */
/* ===========================================================================
** Copyright (C) 2020-2023 Infineon Technologies AG
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
#include "ifxAvian_RegisterSet.hpp"
#include "Driver/registers_BGT60TRxxC.h"

// ---------------------------------------------------------------------------- namespaces
namespace Infineon {
namespace Avian {
namespace HW {

// ----------------------------------------------------------------------------- extract_update
RegisterSet RegisterSet::extract_update(const RegisterSet& base) const
{
    /*
     * Those register that are not defined in the base register set or that
     * have a different value are copied to the update register set.
     */
    RegisterSet update;
    for (const auto& reg : m_registers)
    {
        if (!base.is_defined(reg.first) || (base[reg.first] != reg.second))
            update.m_registers[reg.first] = reg.second;
    }
    return update;
}

// ----------------------------------------------------------------------------- apply_update
void RegisterSet::apply_update(const RegisterSet& update)
{
    for (const auto& reg : update.m_registers)
        m_registers[reg.first] = reg.second;
}

// ----------------------------------------------------------------------------- send_to_device
void RegisterSet::send_to_device(IControlPort& port, bool set_trigger_bit) const
{
    auto sequence = get_configuration_sequence(set_trigger_bit);
    port.send_commands(sequence.data(), sequence.size());
}

// ----------------------------------------------------------------------------- get_configuration_sequence
std::vector<Spi_Command_t>
RegisterSet::get_configuration_sequence(bool set_trigger_bit) const
{
    Spi_Command_t trigger_command = 0;

    std::vector<Spi_Command_t> sequence;
    for (const auto& reg : m_registers)
    {
        /*
         * The register address, the write bit and the value are combined into
         * a word that can be sent to an Avian device.
         */
        auto seq_word = (Spi_Command_t(reg.first) << 25)
                        | 0x01000000 | reg.second;

        /*
         * When a frame is triggered, the main register must be sent at
         * the end, because it contains the trigger bit.
         */
        if ((reg.first == BGT60TRxxC_REG_MAIN) && set_trigger_bit)
        {
            trigger_command = seq_word | BGT60TRxxC_SET(MAIN, FRAME_START, 1);
            continue;
        }

        sequence.push_back(seq_word);
    }

    // If there a trigger word is defined add it at the end.
    if (trigger_command)
        sequence.push_back(trigger_command);

    return sequence;
}

/* ------------------------------------------------------------------------ */
}  // namespace HW
}  // namespace Avian
}  // namespace Infineon

/* --- End of File -------------------------------------------------------- */
