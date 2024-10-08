/**
 * \file ifxAvian_StrataControlPort.hpp
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

#ifndef IFX_AVIAN_STRATA_CONTROL_PORT_H
#define IFX_AVIAN_STRATA_CONTROL_PORT_H

// ---------------------------------------------------------------------------- includes
#include "ifxAvian_IPort.hpp"

// strata
#include <common/EndianConversion.hpp>
#include <common/NarrowCast.hpp>
#include <components/interfaces/IProcessingRadar.hpp>
#include <components/interfaces/IRadarAvian.hpp>
#include <platform/BoardInstance.hpp>
#include <platform/exception/EConnection.hpp>
#include <vector>

// ---------------------------------------------------------------------------- namespaces
namespace Infineon {
namespace Avian {
namespace Internal {

// ------------------------------------------------------------------- StrataControlPortImpl
template <typename BASECLASS>
class StrataControlPortImpl : public BASECLASS
{
public:
    using Properties = typename HW::IControlPort::Properties;

    StrataControlPortImpl(BoardInstance* board)
    {
        constexpr uint8_t component_id = 0;
        auto* avian = board->getComponent<IRadarAvian>(component_id);
        if (avian == nullptr)
            throw EConnection("Avian device is not available on the connected board");

        m_board = board;
        m_cmd = avian->getIProtocolAvian();
        m_pins = avian->getIPinsAvian();
    }

    const Properties& get_properties() const override
    {
        /*
         * For now, we only support MCU7 with these properties.
         * Refactoring needed if more boards are supported:
         * either lookup of properties here, or better add a universal interface
         * to ISpi to query there parameters universally
         */
        static Properties s_properties =
            {
                "Avian",
                false,
                2,
            };

        return s_properties;
    }

    void send_commands(const HW::Spi_Command_t* commands, size_t num_words,
                       HW::Spi_Response_t* response = nullptr) override
    {
        std::vector<HW::Spi_Command_t> protocolCommands(commands,
                                                        commands + num_words);
        endian::swap(protocolCommands.begin(), protocolCommands.end());
        m_cmd->execute(reinterpret_cast<const IProtocolAvian::Command*>(protocolCommands.data()),
                       narrow_cast<uint32_t>(num_words), response);
    }

    void generate_reset_sequence() override
    {
        m_pins->reset();
    }

    bool read_irq_level() override
    {
        return m_pins->getIrqPin();
    }

    BoardInstance& getBoardInstance()
    {
        return *m_board;
    }

protected:
    IProtocolAvian* m_cmd;

private:
    BoardInstance* m_board;
    IPinsAvian* m_pins;
};

/* ------------------------------------------------------------------------ */
}  // namespace Internal

using StrataControlPort = Internal::StrataControlPortImpl<HW::IControlPort>;

/* ------------------------------------------------------------------------ */
}  // namespace Avian
}  // namespace Infineon

#endif /* IFX_AVIAN_STRATA_CONTROL_PORT_H */

/* --- End of File -------------------------------------------------------- */
