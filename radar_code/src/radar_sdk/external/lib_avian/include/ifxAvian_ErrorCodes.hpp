/**
 * \file ifxAvian_ErrorCodes.hpp
 *
 * This file contains functions that handle error codes from Avian driver.
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

#ifndef IFX_AVIAN_ERRORCODES_H
#define IFX_AVIAN_ERRORCODES_H

// ---------------------------------------------------------------------------- includes
#include "ifxAvian_Driver.hpp"
#include <stdexcept>
#include <stdint.h>
#include <string>
#include <string_view>

// ---------------------------------------------------------------------------- namespaces
namespace Infineon {
namespace Avian {

/**
 * This exception type represents an error that occurred while using
 * the Avian driver. An exception instance must be initialized with an
 * error code that was returned by any function of the Avian driver.
 * When the function \ref check_error is used, an exception of this
 * type is thrown in case of an error.
 */
class DriverError : public std::runtime_error
{
public:
    explicit DriverError(Driver::Error error_code);

private:
    std::string make_description(Driver::Error error_code);
};

/**
 * This function returns a human readable error message that explains
 * the provided error code.
 *
 * \param [in] error_code  An error code returned by any function of
 *                         the Avian driver.
 */
std::string_view get_error_code_description(Driver::Error error_code);

/**
 * This function checks an error code that has been returned by a
 * function of the Avian driver and throws an exception of type
 * \ref DriverError in case of an error.
 */
void check_error(Driver::Error error_code);

/* ------------------------------------------------------------------------ */
}  // namespace Avian
}  // namespace Infineon

#endif /* IFX_AVIAN_ERRORCODES_H */

/* --- End of File -------------------------------------------------------- */
