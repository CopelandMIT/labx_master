/**
 * \file ifxAvian_HwTest.hpp
 *
 * This file declares some function for hardware of system comprising an
 * Avian device.
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


#ifndef IFX_AVIAN_HWTEST_H
#define IFX_AVIAN_HWTEST_H

// ---------------------------------------------------------------------------- includes
#include "ifxAvian_IPort.hpp"

// ---------------------------------------------------------------------------- namespaces
namespace Infineon {
namespace Avian {
namespace HwTest {

// ---------------------------------------------------------------------------- test_spi_connection
/**
 * \brief This function test the SPI connection between an Avian device and the
 *        SPI controller.
 *
 * This function sets up the Avian device to generate a pseudo random test
 * sequence rather than acquire real radar data. That data test data is
 * read by the SPI controller via the usual IPort interface. This function also
 * generates a pseudo random test sequence following the same generation rule
 * as the Avian device. Finally the received data is compared against the
 * locally test sequence.
 *
 * \note The SPI connection test uses the on-chip FIFO memory of the Avian
 *       device. If the on-chip memory is broken, this test fails also when the
 *       SPI connection works perfectly. The function \ref test_fifo_memory can
 *       be used to test the on-chip FIFO memory.
 *
 * \param port                  The software representation of the interface
 *                              the Avian device to be tested is connected to.
 * \param clock_config_command  An optional command word that configures the
 *                              reference clock. If this omitted no special
 *                              clock setup is done.
 *
 * \return If the received data matches the locally generated data this
 *         function returns true to indicate a passed test. Otherwise false is
 *         returned to indicate a failed test.
 */
bool test_spi_connection(HW::IPort<HW::Packed_Raw_Data_t>& port,
                         HW::Spi_Command_t clock_config_command = 0);

// ---------------------------------------------------------------------------- test_fifo_memory
/**
 * \brief This function test the FIFO memory of an Avian device.
 *
 * This function triggers an on-chip FIFO memory test routine that is
 * hard-coded into the Avian device.
 *
 * \param port                  The software representation of the interface
 *                              the Avian device to be tested is connected to.
 * \param clock_config_command  An optional command word that configures the
 *                              reference clock. If this omitted no special
 *                              clock setup is done.
 *
 * \return If the memory test succeeds, true is returned.  Otherwise false is
 *         returned.
 */
bool test_fifo_memory(HW::IControlPort& port,
                      HW::Spi_Command_t clock_config_command = 0);

/* ------------------------------------------------------------------------ */
}  // namespace HwTest
}  // namespace Avian
}  // namespace Infineon

#endif /* IFX_AVIAN_HWTEST_H */

/* --- End of File -------------------------------------------------------- */
