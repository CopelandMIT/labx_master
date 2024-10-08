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

#ifndef IFX_BASE_EXCEPTION_HPP
#define IFX_BASE_EXCEPTION_HPP

#include "Error.h"
#include <stdexcept>


namespace rdk {
namespace exception {

class exception : public std::exception
{
    ifx_Error_t m_error_code;

public:
    exception(ifx_Error_t error_code) :
        m_error_code(error_code)
    {}

    ifx_Error_t error_code() const
    {
        return m_error_code;
    }

    const char* what() const noexcept override
    {
        return ifx_error_to_string(m_error_code);
    }
};

template <ifx_Error_t ec>
class ExceptionBase : public exception
{
public:
    ExceptionBase() :
        exception(ec)
    {}
};

class error : public ExceptionBase<IFX_ERROR>
{};
class error_eeprom : public ExceptionBase<IFX_ERROR_EEPROM>
{};
class argument_null : public ExceptionBase<IFX_ERROR_ARGUMENT_NULL>
{};
class argument_invalid : public ExceptionBase<IFX_ERROR_ARGUMENT_INVALID>
{};
class argument_out_of_bounds : public ExceptionBase<IFX_ERROR_ARGUMENT_OUT_OF_BOUNDS>
{};
class argument_invalid_expected_real : public ExceptionBase<IFX_ERROR_ARGUMENT_INVALID_EXPECTED_REAL>
{};
class argument_invalid_expected_complex : public ExceptionBase<IFX_ERROR_ARGUMENT_INVALID_EXPECTED_COMPLEX>
{};
class index_out_of_bounds : public ExceptionBase<IFX_ERROR_INDEX_OUT_OF_BOUNDS>
{};
class dimension_mismatch : public ExceptionBase<IFX_ERROR_DIMENSION_MISMATCH>
{};
class memory_allocation_failed : public ExceptionBase<IFX_ERROR_MEMORY_ALLOCATION_FAILED>
{};
class insufficient_memory_allocated : public ExceptionBase<IFX_ERROR_INSUFFICIENT_MEMORY_ALLOCATED>
{};
class in_place_calculation_not_supported : public ExceptionBase<IFX_ERROR_IN_PLACE_CALCULATION_NOT_SUPPORTED>
{};
class matrix_singular : public ExceptionBase<IFX_ERROR_MATRIX_SINGULAR>
{};
class matrix_not_positive_definite : public ExceptionBase<IFX_ERROR_MATRIX_NOT_POSITIVE_DEFINITE>
{};
class not_supported : public ExceptionBase<IFX_ERROR_NOT_SUPPORTED>
{};
class internal : public ExceptionBase<IFX_ERROR_INTERNAL>
{};
class not_possible : public ExceptionBase<IFX_ERROR_NOT_POSSIBLE>
{};
class no_device : public ExceptionBase<IFX_ERROR_NO_DEVICE>
{};
class device_busy : public ExceptionBase<IFX_ERROR_DEVICE_BUSY>
{};
class communication_error : public ExceptionBase<IFX_ERROR_COMMUNICATION_ERROR>
{};
class num_samples_out_of_range : public ExceptionBase<IFX_ERROR_NUM_SAMPLES_OUT_OF_RANGE>
{};
class rx_antenna_combination_not_allowed : public ExceptionBase<IFX_ERROR_RX_ANTENNA_COMBINATION_NOT_ALLOWED>
{};
class if_gain_out_of_range : public ExceptionBase<IFX_ERROR_IF_GAIN_OUT_OF_RANGE>
{};
class samplerate_out_of_range : public ExceptionBase<IFX_ERROR_SAMPLERATE_OUT_OF_RANGE>
{};
class rf_out_of_range : public ExceptionBase<IFX_ERROR_RF_OUT_OF_RANGE>
{};
class tx_power_out_of_range : public ExceptionBase<IFX_ERROR_TX_POWER_OUT_OF_RANGE>
{};
class chirp_rate_out_of_range : public ExceptionBase<IFX_ERROR_CHIRP_RATE_OUT_OF_RANGE>
{};
class frame_rate_out_of_range : public ExceptionBase<IFX_ERROR_FRAME_RATE_OUT_OF_RANGE>
{};
class num_chirps_not_allowed : public ExceptionBase<IFX_ERROR_NUM_CHIRPS_NOT_ALLOWED>
{};
class frame_size_not_supported : public ExceptionBase<IFX_ERROR_FRAME_SIZE_NOT_SUPPORTED>
{};
class timeout : public ExceptionBase<IFX_ERROR_TIMEOUT>
{};
class fifo_overflow : public ExceptionBase<IFX_ERROR_FIFO_OVERFLOW>
{};
class tx_antenna_mode_not_allowed : public ExceptionBase<IFX_ERROR_TX_ANTENNA_MODE_NOT_ALLOWED>
{};
class firmware_version_not_supported : public ExceptionBase<IFX_ERROR_FIRMWARE_VERSION_NOT_SUPPORTED>
{};
class device_not_supported : public ExceptionBase<IFX_ERROR_DEVICE_NOT_SUPPORTED>
{};
class frame_acquisition_failed : public ExceptionBase<IFX_ERROR_FRAME_ACQUISITION_FAILED>
{};
class not_configured : public ExceptionBase<IFX_ERROR_NOT_CONFIGURED>
{};
class empty_directory : public ExceptionBase<IFX_ERROR_EMPTY_DIRECTORY>
{};
class opening_file : public ExceptionBase<IFX_ERROR_OPENING_FILE>
{};
class file_invalid : public ExceptionBase<IFX_ERROR_FILE_INVALID>
{};
class invalid_json : public ExceptionBase<IFX_ERROR_INVALID_JSON>
{};
class invalid_json_key : public ExceptionBase<IFX_ERROR_INVALID_JSON_KEY>
{};
class invalid_json_value : public ExceptionBase<IFX_ERROR_INVALID_JSON_VALUE>
{};
class end_of_file : public ExceptionBase<IFX_ERROR_END_OF_FILE>
{};
class missing_interface : public ExceptionBase<IFX_ERROR_MISSING_INTERFACE>
{};
class not_implemented : public ExceptionBase<IFX_ERROR_NOT_IMPLEMENTED>
{};

}  // namespace exception
}  // namespace rdk


#endif /* IFX_BASE_EXCEPTION_HPP */
