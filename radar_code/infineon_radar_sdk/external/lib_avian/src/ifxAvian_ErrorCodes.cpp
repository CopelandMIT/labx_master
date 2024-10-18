/**
 * \file ifxAvian_ErrorCodes.cpp
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

// ---------------------------------------------------------------------------- includes
#include "ifxAvian_ErrorCodes.hpp"
#include <iomanip>
#include <sstream>

// ---------------------------------------------------------------------------- namespaces
namespace Infineon {
namespace Avian {

// ----------------------------------------------------------------------------- Driver::Error::DriverError
DriverError::DriverError(Driver::Error error_code) :
    std::runtime_error(make_description(error_code))
{}

// ----------------------------------------------------------------------------- Driver::Error::make_description
std::string DriverError::make_description(Driver::Error error_code)
{
    std::stringstream sErrorMsg;

    sErrorMsg << Infineon::Avian::get_error_code_description(error_code);

    sErrorMsg << std::setfill('0') << std::hex;
    sErrorMsg << " (Error Code 0x" << std::setw(4) << unsigned(error_code) << ")";

    return sErrorMsg.str();
}

// ---------------------------------------------------------------------------- get_error_code_description
std::string_view get_error_code_description(Driver::Error error_code)
{
    switch (error_code)
    {
        case Driver::Error::OK:
            return "No Error.";

        case Driver::Error::BUSY:
            return "The device is busy. Maybe test mode or automatic trigger is active.";

        case Driver::Error::INCOMPATIBLE_MODE:
            return "The requested operation is not supported by the currently active mode of operation.";

        case Driver::Error::TIME_OUT:
            return "A timeout has occurred while waiting for a data frame to be acquired.";

        case Driver::Error::UNSUPPORTED_FRAME_INTERVAL:
            return "The requested time interval is out of range.";

        case Driver::Error::ANTENNA_DOES_NOT_EXIST:
            return "One or more of the selected antennas is not present on the device.";

        case Driver::Error::SENSOR_DOES_NOT_EXIST:
            return "The requested temperature sensor does not exist.";

        case Driver::Error::UNSUPPORTED_FRAME_FORMAT:
            return "The specified frame format is not supported.";

        case Driver::Error::FREQUENCY_OUT_OF_RANGE:
            return "The specified RF frequency is out of range.";

        case Driver::Error::POWER_OUT_OF_RANGE:
            return "The specified TX power is out of range.";

        case Driver::Error::UNAVAILABLE_SIGNAL_PART:
            return "The device is not capable to capture the requested part of the complex signal.";

        case Driver::Error::UNSUPPORTED_DIRECTION:
            return "The specified FMCW ramp direction is not supported by the device.";

        case Driver::Error::SAMPLERATE_OUT_OF_RANGE:
            return "The specified sampling rate is out of range.";

        case Driver::Error::UNSUPPORTED_TX_MODE:
            return "The specified TX mode is not supported by the device.";

        case Driver::Error::UNSUPPORTED_HP_GAIN:
            return "The specified high pass filter gain is not supported by the device.";

        case Driver::Error::UNSUPPORTED_VGA_GAIN:
            return "The specified gain adjustment setting is not supported by the device.";

        case Driver::Error::RESET_TIMER_OUT_OF_RANGE:
            return "The specified reset timer period is out of range.";

        case Driver::Error::UNSUPPORTED_HP_CUTOFF:
            return "The specified high pass cutoff is out of range.";

        case Driver::Error::UNSUPPORTED_AAF_CUTOFF:
            return "The specified low pass cutoff is out of range.";

        case Driver::Error::CALIBRATION_FAILED:
            return "The calibration of phase settings or base band chain did not succeed.";

        case Driver::Error::INVALID_PHASE_SETTING:
            return "The provided oscillator phase setting is not valid. It's forbidden to disable both phase modulators.";

        case Driver::Error::UNDEFINED_TRACKING_MODE:
            return "The specified ADC tracking mode is not supported by the device.";

        case Driver::Error::UNDEFINED_ADC_SAMPLE_TIME:
            return "The specified ADC sampling time is not supported by the device.";

        case Driver::Error::UNDEFINED_ADC_OVERSAMPLING:
            return "The specified ADC oversampling factors is not supported by the device.";

        case Driver::Error::NONCONTINUOUS_SHAPE_SEQUENCE:
            return "The specified shape sequence is not supported. There must not be a gap between used shapes.";

        case Driver::Error::UNSUPPORTED_NUM_REPETITIONS:
            return "One or more specified number of repetition is not supported. Only powers of two are allowed. Total numbers of shape groups must not exceed 4096.";

        case Driver::Error::UNSUPPORTED_POWER_MODE:
            return "One or more of the specified power modes is not supported.";

        case Driver::Error::POST_DELAY_OUT_OF_RANGE:
            return "One or more of the specified post shape / post shape set delays is not supported.";

        case Driver::Error::NUM_FRAMES_OUT_OF_RANGE:
            return "The specified number of frames is out of range.";

        case Driver::Error::SHAPE_NUMBER_OUT_OF_RANGE:
            return "The requested shape does not exist.";

        case Driver::Error::PRECHIRPDELAY_OUT_OF_RANGE:
            return "The specified pre-chirp delay is out of range.";

        case Driver::Error::POSTCHIRPDELAY_OUT_OF_RANGE:
            return "The specified post-chirp delay is out of range.";

        case Driver::Error::PADELAY_OUT_OF_RANGE:
            return "The specified PA delay is out of range.";

        case Driver::Error::ADCDELAY_OUT_OF_RANGE:
            return "The specified ADC delay is out of range.";

        case Driver::Error::WAKEUPTIME_OUT_OF_RANGE:
            return "The specified wake up time is out of range.";

        case Driver::Error::SETTLETIME_OUT_OF_RANGE:
            return "The specified PLL settle time is out of range.";

        case Driver::Error::UNSUPPORTED_FIFO_SLICE_SIZE:
            return "The specified FIFO slice size is not supported.";

        case Driver::Error::SLICES_NOT_RELEASABLE:
            return "The FIFO slice can't be released. It has not been used.";

        case Driver::Error::FIFO_OVERFLOW:
            return "A FIFO overflow has occurred. A reset is needed.";

        case Driver::Error::NO_MEMORY:
            return "No memory buffer has been provided to store the radar data.";

        case Driver::Error::CHIP_SETUP_FAILED:
            return "The chip could not be programmed.";

        case Driver::Error::FEATURE_NOT_SUPPORTED:
            return "The requested feature is not supported by the connected device.";

        case Driver::Error::PRECHIRP_EXCEEDS_PADELAY:
            return "The PA Delay is shorter than the pre-chirp delay.";

        case Driver::Error::UNSUPPORTED_FREQUENCY:
            return "The selected reference clock frequency is not supported by the device.";

        case Driver::Error::UNSUPPORTED_FIFO_POWER_MODE:
            return "The specified FIFO power mode is not supported.";

        case Driver::Error::UNSUPPORTED_PAD_DRIVER_MODE:
            return "The specified pad driver mode is not supported.";

        case Driver::Error::BANDGAP_DELAY_OUT_OF_RANGE:
            return "The specified band gap startup delay is out of range.";

        case Driver::Error::MADC_DELAY_OUT_OF_RANGE:
            return "The specified MADC startup delay is out of range.";

        case Driver::Error::PLL_ENABLE_DELAY_OUT_OF_RANGE:
            return "The specified PLL startup delay is out of range.";

        case Driver::Error::PLL_DIVIDER_DELAY_OUT_OF_RANGE:
            return "The specified PLL divider startup delay is out of range.";

        case Driver::Error::DOUBLER_MODE_NOT_SUPPORTED:
            return "The specified clock doubler mode is not supported.";

        case Driver::Error::DC_IN_CORRECTION_OUT_OF_RANGE:
            return "The specified input duty cycle correction is out of range.";

        case Driver::Error::DC_OUT_CORRECTION_OUT_OF_RANGE:
            return "The specified output duty cycle correction is out of range.";

        case Driver::Error::POWER_SENS_DELAY_OUT_OF_RANGE:
            return "The specified power sensing delay is out of range.";

        case Driver::Error::NO_SWITCH_TIME_MADC_POWER:
            return "The period between chirp start and power measurement is "
                   "not long enough to allow switching the MADC input to power"
                   " sensor channel.";

        case Driver::Error::NO_SWITCH_TIME_MADC_RX:
            return "The period between power measurement and chirp acquisition"
                   " is not long enough to switch MADC input to the RX channel.";

        case Driver::Error::NO_SWITCH_TIME_MADC_TEMP:
            return "The period between end of chirp acquisition and temperature"
                   " measurement is not long enough to switch MADC input to "
                   "temperature sensor channel.";

        case Driver::Error::NO_MEASURE_TIME_MADC_TEMP:
            return "The chirp end delay is not long enough for temperature"
                   " measurement.";

        case Driver::Error::TEMP_SENSING_WITH_NO_RX:
            return "When temperature sensing is enabled, chirps with all RX "
                   "channels disabled are not allowed.";

        default:
            return "Unknown Error Code";
    }
}

// ---------------------------------------------------------------------------- checkError
void check_error(Driver::Error error_code)
{
    if (error_code != Driver::Error::OK)
        throw DriverError(error_code);
}

/* ------------------------------------------------------------------------ */
}  // namespace Avian
}  // namespace Infineon

/* --- End of File -------------------------------------------------------- */
