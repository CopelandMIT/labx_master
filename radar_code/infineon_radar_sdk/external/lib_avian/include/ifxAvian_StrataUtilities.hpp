/**
 * \file ifxAvian_StrataUtilities.hpp
 *
 * This file declares auxiliary functions that support usage of Avian devices
 * and require Strata connection.
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

#ifndef IFX_AVIAN_STRATA_UTILITIES_H
#define IFX_AVIAN_STRATA_UTILITIES_H

// ---------------------------------------------------------------------------- includes
#include "ifxAvian_Driver.hpp"
#include "ifxAvian_Types.hpp"
#include "platform/BoardInstance.hpp"
#include <platform/NamedMemory.hpp>

#include <map>
#include <string>
// ---------------------------------------------------------------------------- namespaces
namespace Infineon {
namespace Avian {

// ---------------------------------------------------------------------------- detect_reference_clock
/**
 * \brief This function tries to detect the reference clock frequency.

 * This function tries to detect if the reference clock frequency of the Avian
 * device is 40Mhz or 80MHz. Internally it configures the Avian device to route
 * a signal derived from the system clock to the IRQ pin and uses a special
 * firmware feature to capture timestamps of IRQ events.
 *
 * If frequency detection succeeds the provided driver instance is configured
 * for the detected reference frequency.
 *
 * If something goes wrong during frequency detection, an exception is thrown.
 *
 * \return If firmware does not support frequency measurement, false is
 *         returned, otherwise true is returned.
 */
bool detect_reference_clock(BoardInstance& board, Driver& driver);
/**
 * \brief Return the register map for the given device.
 *
 * The register map contains name, address and the current value for all registers.
 *
 * \param device_type The device type
 * \return A map with register infos (name, address, value)
 */
std::map<std::string, NamedMemory<uint16_t, uint32_t>::Address> get_register_map(Device_Type device_type);


/* ------------------------------------------------------------------------ */
}  // namespace Avian
}  // namespace Infineon

#endif /* IFX_AVIAN_STRATA_UTILITIES_H */

/* --- End of File -------------------------------------------------------- */
