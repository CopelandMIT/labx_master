/**
 * \file ifxAvian_RegisterSet.hpp
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

#ifndef IFX_AVIAN_REGISTER_SET_H
#define IFX_AVIAN_REGISTER_SET_H

// ---------------------------------------------------------------------------- includes
#include "ifxAvian_IPort.hpp"
#include <cstdint>
#include <map>
#include <vector>

// ---------------------------------------------------------------------------- namespaces
namespace Infineon {
namespace Avian {
namespace HW {

// ---------------------------------------------------------------------------- RegisterSet
/**
 * This class represents a register configuration of an Avian device. A
 * register set may contain a full configuration or just a few registers.
 * It allows to read and modify registers and to check if a register is defined
 * at all. It's possible to extract the difference between two register sets
 * and a register set can be converted into a set of SPI write commands that
 * programs a register set into an Avian device.
 */
class RegisterSet
{
public:
    /**
     * This operator returns the value of the specified register. If the
     * specified register is not defined in this register set, an exception
     * is thrown.
     *
     * \param[in] address  The address of the register to read.
     *
     * \return The value of the specified register.
     */
    inline uint32_t operator[](uint8_t address) const;

    /**
     * This method sets the value of a register. If that register was already
     * defined, the value is overwritten. Otherwise the register is added to
     * the set.
     *
     * \param[in] address  The address of the register the new value should be
     *                     written to.
     * \param[in] value    The new value that will be written to the register.
     */
    inline void set(uint8_t address, uint32_t value);

    /**
     * This method extracts address and value from an Avian SPI register write
     * command word and sets the value of the addressed register accordingly.
     * If that register was already defined, the value is overwritten.
     * Otherwise the register is added to the set.
     *
     * \param[in] command_word  The SPI command word containing address and
     *                          value of the register to be set.
     */
    inline void set(Spi_Command_t command_word);

    /**
     * This method checks if the specified register is defined in a register
     * set.
     *
     * \param[in] address  The address of the register to be checked.
     *
     * \return This method returns true if the register set contains a value
     *         for the specified register. If the register value is not defined
     *         false is returned.
     */
    inline bool is_defined(uint8_t address) const;

    /**
     * This method removes the value of the specified register from the
     * register set, so the register will not be defined in the register set
     * any longer.
     *
     * \param[in] address  The address of the register to be removed.
     */
    inline void remove(uint8_t address);

    /**
     * This method compares a register set to another one and returns those
     * registers that have a different value or are not defined in the "base"
     * register set.
     *
     * \param[in] base  The register set this register should be compared to.
     *
     * \return A register set containing all changes from base to register set.
     */
    RegisterSet extract_update(const RegisterSet& base) const;

    /**
     * This method overwrites all registers that are defined in the provided
     * register set "update". This method can also be seen as a merge of two
     * register sets.
     *
     * \param[in] update  The register changes that should be applied to this
     *                    register set.
     */
    void apply_update(const RegisterSet& update);

    /**
     * This method sends all defined register value to an Avian device.
     *
     * See \ref get_configuration_sequence for more information about the
     * SPI command sequence that is sent to the Avian device.
     *
     * \param[in] port             The port where the Avian device to be
     *                             updated is connected to.
     * \param[in] set_trigger_bit  If this is true, the FRAME_START bit is
     *                             also set. See also
     *                             \ref get_configuration_sequence
     *
     */
    void send_to_device(IControlPort& port, bool set_trigger_bit) const;

    /**
     * This method generates a sequence of SPI write command words, that can
     * be sent to an Avian device to transfer all registers value from this
     * register set to the device.
     *
     * If set_trigger_bit is true, the FRAME_START bit in the MAIN register
     * is set and the command word to program the MAIN register is moved to the
     * end of the sequence. If the register set does not contain a value for
     * the MAIN register, set_trigger_bit is ignored. The FRAME_START bit in
     * this register set is not changed.
     *
     * \param[in] set_trigger_bit  See description above.
     *
     * \return The SPI command sequence to update an Avian device.
     */
    std::vector<Spi_Command_t>
    get_configuration_sequence(bool set_trigger_bit) const;

private:
    std::map<uint8_t, uint32_t> m_registers;
};

// ---------------------------------------------------------------------------- RegisterSet::operator[]
inline uint32_t RegisterSet::operator[](uint8_t address) const
{
    return m_registers.at(address);
}

// ---------------------------------------------------------------------------- RegisterSet::set
inline void RegisterSet::set(uint8_t address, uint32_t value)
{
    m_registers[address] = value & 0x00FFFFFF;
}

// ---------------------------------------------------------------------------- RegisterSet::set
inline void RegisterSet::set(Spi_Command_t word)
{
    m_registers[word >> 25] = word & 0x00FFFFFF;
}

// ---------------------------------------------------------------------------- RegisterSet::is_defined
inline bool RegisterSet::is_defined(uint8_t address) const
{
    return m_registers.find(address) != m_registers.end();
}

// ---------------------------------------------------------------------------- RegisterSet::remove
inline void RegisterSet::remove(uint8_t address)
{
    m_registers.erase(address);
}

/* ------------------------------------------------------------------------ */
}  // namespace HW
}  // namespace Avian
}  // namespace Infineon

#endif /* IFX_AVIAN_IPORT_H */

/* --- End of File -------------------------------------------------------- */
