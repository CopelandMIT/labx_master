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

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "Defines.h"
#include "Error.h"

/*
==============================================================================
   2. LOCAL DEFINITIONS
==============================================================================
*/

/*
==============================================================================
   3. LOCAL TYPES
==============================================================================
*/

/*
==============================================================================
   4. LOCAL DATA
==============================================================================
*/

IFX_THREAD_LOCAL ifx_Error_t last_error = IFX_OK;

/*
==============================================================================
   5. LOCAL FUNCTION PROTOTYPES
==============================================================================
*/

/*
==============================================================================
   6. LOCAL FUNCTIONS
==============================================================================
*/


/**
 * @brief Get the basename from a path
 *
 * The function searches for the last occurrence path separator (slash or
 * backslash) and returns the pointer to the string starting after the
 * separator. If no separator is found, a pointer to the beginning of
 * the string is returned.
 *
 * @param [in]  s   path
 * @retval      basename of path
 */
static const char* basename(const char* path)
{
    char* p;

    if ((p = strrchr(path, '/')) != NULL || (p = strrchr(path, '\\')) != NULL)
        return p + 1;
    else
        return path;
}

/*
==============================================================================
   7. EXPORTED FUNCTIONS
==============================================================================
*/

const char* ifx_error_to_string(ifx_Error_t error)
{
    switch (error)
    {
        case IFX_OK:
            return "ok (IFX_OK)";
        case IFX_ERROR_ARGUMENT_NULL:
            return "argument is NULL (IFX_ERROR_ARGUMENT_NULL)";
        case IFX_ERROR_ARGUMENT_INVALID:
            return "argument is invalid (IFX_ERROR_ARGUMENT_INVALID)";
        case IFX_ERROR_ARGUMENT_OUT_OF_BOUNDS:
            return "argument is out of bounds (IFX_ERROR_ARGUMENT_OUT_OF_BOUNDS)";
        case IFX_ERROR_ARGUMENT_INVALID_EXPECTED_REAL:
            return "argument is expected to be real (IFX_ERROR_ARGUMENT_INVALID_EXPECTED_REAL)";
        case IFX_ERROR_ARGUMENT_INVALID_EXPECTED_COMPLEX:
            return "argument is expected to be complex (IFX_ERROR_ARGUMENT_INVALID_EXPECTED_COMPLEX)";
        case IFX_ERROR_INDEX_OUT_OF_BOUNDS:
            return "index is out of bounds (IFX_ERROR_INDEX_OUT_OF_BOUNDS)";
        case IFX_ERROR_DIMENSION_MISMATCH:
            return "dimension mismatch (IFX_ERROR_DIMENSION_MISMATCH)";
        case IFX_ERROR_MEMORY_ALLOCATION_FAILED:
            return "memory allocation failed (IFX_ERROR_MEMORY_ALLOCATION_FAILED)";
        case IFX_ERROR_IN_PLACE_CALCULATION_NOT_SUPPORTED:
            return "inplace calculation is not supported (IFX_ERROR_IN_PLACE_CALCULATION_NOT_SUPPORTED)";
        case IFX_ERROR_MATRIX_SINGULAR:
            return "matrix is singular (IFX_ERROR_MATRIX_SINGULAR)";
        case IFX_ERROR_MATRIX_NOT_POSITIVE_DEFINITE:
            return "matrix is not positive definite (IFX_ERROR_MATRIX_NOT_POSITIVE_DEFINITE)";
        case IFX_ERROR_NOT_SUPPORTED:
            return "not supported (IFX_ERROR_NOT_SUPPORTED)";
        case IFX_ERROR_INTERNAL:
            return "internal logic error (IFX_ERROR_INTERNAL)";
        case IFX_ERROR_NOT_POSSIBLE:
            return "not possible (IFX_ERROR_NOT_POSSIBLE)";
        case IFX_ERROR_NO_DEVICE:
            return "no compatible device found (IFX_ERROR_NO_DEVICE)";
        case IFX_ERROR_DEVICE_BUSY:
            return "connected device is busy (IFX_ERROR_DEVICE_BUSY)";
        case IFX_ERROR_COMMUNICATION_ERROR:
            return "communication error between host and device (IFX_ERROR_COMMUNICATION_ERROR)";
        case IFX_ERROR_NUM_SAMPLES_OUT_OF_RANGE:
            return "device does not support requested number of samples (IFX_ERROR_NUM_SAMPLES_OUT_OF_RANGE)";
        case IFX_ERROR_RX_ANTENNA_COMBINATION_NOT_ALLOWED:
            return "device does not support enabling requested combination of RX antennas (IFX_ERROR_RX_ANTENNA_COMBINATION_NOT_ALLOWED)";
        case IFX_ERROR_IF_GAIN_OUT_OF_RANGE:
            return "device does not support requested IF gain (IFX_ERROR_IF_GAIN_OUT_OF_RANGE)";
        case IFX_ERROR_SAMPLERATE_OUT_OF_RANGE:
            return "device does not support requested sample rate (IFX_ERROR_SAMPLERATE_OUT_OF_RANGE)";
        case IFX_ERROR_RF_OUT_OF_RANGE:
            return "device does not support requested FMCW start and end frequency (IFX_ERROR_RF_OUT_OF_RANGE)";
        case IFX_ERROR_TX_POWER_OUT_OF_RANGE:
            return "device does not support the requested TX power (IFX_ERROR_TX_POWER_OUT_OF_RANGE)";
        case IFX_ERROR_CHIRP_RATE_OUT_OF_RANGE:
            return "requested chirp-to-chirp time cannot be applied (IFX_ERROR_CHIRP_RATE_OUT_OF_RANGE)";
        case IFX_ERROR_FRAME_RATE_OUT_OF_RANGE:
            return "requested frame period cannot be applied (IFX_ERROR_FRAME_RATE_OUT_OF_RANGE)";
        case IFX_ERROR_NUM_CHIRPS_NOT_ALLOWED:
            return "device does not support the requested number of chirps per frame (IFX_ERROR_NUM_CHIRPS_NOT_ALLOWED)";
        case IFX_ERROR_FRAME_SIZE_NOT_SUPPORTED:
            return "device does not support the frame size (IFX_ERROR_FRAME_SIZE_NOT_SUPPORTED)";
        case IFX_ERROR_FRAME_ACQUISITION_FAILED:
            return "a frame was dropped due to communication or buffering issues (data rate too high for underlying link or memory allocation mismatch) (IFX_ERROR_FRAME_ACQUISITION_FAILED)";
        case IFX_ERROR_TIMEOUT:
            return "device did not acquire a complete time domain data frame within the expected time (IFX_ERROR_TIMEOUT)";
        case IFX_ERROR_FIFO_OVERFLOW:
            return "device stopped acquisition of time domain data due to an internal fifo overflow (IFX_ERROR_FIFO_OVERFLOW)";
        case IFX_ERROR_TX_ANTENNA_MODE_NOT_ALLOWED:
            return "device does not support the requested mode of TX antennas (IFX_ERROR_TX_ANTENNA_MODE_NOT_ALLOWED)";
        case IFX_ERROR_FIRMWARE_VERSION_NOT_SUPPORTED:
            return "firmware version is no longer supported, please update to latest version (IFX_ERROR_FIRMWARE_VERSION_NOT_SUPPORTED)";
        case IFX_ERROR_DEVICE_NOT_SUPPORTED:
            return "device is not supported (IFX_ERROR_DEVICE_NOT_SUPPORTED)";
        case IFX_ERROR_SEQUENCER_ERROR:
            return "device sequencer error (IFX_ERROR_SEQUENCER_ERROR)";
        case IFX_ERROR_EEPROM:
            return "error accessing EEPROM; RF shield might not contain an EEPROM, EEPROM might be broken or not inizialized (IFX_ERROR_EEPROM)";
        case IFX_ERROR_NOT_CONFIGURED:
            return "device need to be configured (IFX_ERROR_NOT_CONFIGURED)";
        case IFX_ERROR_OPENING_FILE:
            return "cannot open file - it might not exist or is corrupted (IFX_ERROR_OPENING_FILE)";
        case IFX_ERROR_FILE_INVALID:
            return "file is invalid (IFX_ERROR_FILE_INVALID)";
        case IFX_ERROR_EMPTY_DIRECTORY:
            return "directory is empty (IFX_ERROR_EMPTY_DIRECTORY)";
        case IFX_ERROR_INVALID_JSON:
            return "problem parsing a json file; incorrect key used or not existing key (IFX_ERROR_INVALID_JSON_KEY)";
        case IFX_ERROR_INVALID_JSON_KEY:
            return "problem parsing a json file; incorrect JSON structure (IFX_ERROR_INVALID_JSON)";
        case IFX_ERROR_INVALID_JSON_VALUE:
            return "problem parsing a json file; incorrect value at given key (IFX_ERROR_INVALID_JSON_VALUE)";
        case IFX_ERROR_END_OF_FILE:
            return "reached the end of the file. (IFX_ERROR_END_OF_FILE)";
        /* avoid a warning from gcc */
        case IFX_ERROR_NOT_IMPLEMENTED:
            return "feature not implemented (IFX_ERROR_NOT_IMPLEMENTED)";
        default:
            break;
    }

    if (error >= IFX_ERROR_APP)
        return "unknown application related error";
    if (error >= IFX_ERROR_HOST_BASE)
        return "unknown host related error";
    if (error >= IFX_ERROR_DEV_BASE)
        return "unknown device related error";
    if (error >= IFX_ERROR_API_BASE)
        return "unknown SDK related error";

    /* if we get here, then it's an unknown error */
    return "unknown error";
}

void ifx_error_set_no_callback(ifx_Error_t error)
{
    last_error = error;
}

//----------------------------------------------------------------------------

void ifx_error_clear()
{
    last_error = IFX_OK;
}

//----------------------------------------------------------------------------

ifx_Error_t ifx_error_get()
{
    return last_error;
}

//----------------------------------------------------------------------------

ifx_Error_t ifx_error_get_and_clear()
{
    ifx_Error_t error = last_error;
    last_error = IFX_OK;
    return error;
}

//----------------------------------------------------------------------------

int ifx_error_is_set()
{
    return last_error != IFX_OK;
}

//----------------------------------------------------------------------------

IFX_DLL_PUBLIC
void (*ifx_error_callback_pointer)(const char* filename, const char* fname, int line, ifx_Error_t error) = NULL;

//----------------------------------------------------------------------------

void ifx_error_set_callback(void(callback)(const char*, const char*, int, ifx_Error_t))
{
    ifx_error_callback_pointer = callback;
}

void ifx_error_set_internal(const char* filename, const char* fname, int line, ifx_Error_t error)
{
    ifx_error_set_no_callback(error);
    if (error != IFX_OK && ifx_error_callback_pointer != NULL)
    {
        ifx_error_callback_pointer(basename(filename), fname, line, error);
    }
}
