/**
 * \file COMPort.h
 *
 * \brief This file defines the API to access a serial communication port.
 *
 * This interface is an abstraction of a serial communication port. The
 * interface provides basic operations open, close, read and write.
 * Additionally it allows to get a list of available ports.
 *
 * Even though the name is 'COMPort', the actual communication may use a
 * different type of connection (COM, USB, SPI, etc.). The implementation is
 * platform specific and will look differently for each combination of
 * platform and connection type.
 */

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

#ifndef COMPORT_H_INCLUDED
#define COMPORT_H_INCLUDED

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/
#include "ifxBase/Types.h"

/* Enable C linkage if header is included in C++ files */
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#define IFX_COMPORT_BAUDRATE_DEFAULT 115200

typedef struct com_s com_t;

/*
==============================================================================
   5. FUNCTION PROTOTYPES AND INLINE FUNCTIONS
==============================================================================
*/
/**
 * \brief This function returns a list of available COM ports.
 *
 * The port list is compiled into a zero terminated string, where all
 * available ports are separated by a semicolon (';'). The string buffer to
 * hold the port list must be provided by the caller. If the buffer is not
 * sufficient to hold the names of all available ports, it is filled with as
 * much complete port names as possible. Port names will not be truncated.
 *
 * The function will return the number of available COM ports regardless of
 * the number of port names that have been written to the buffer. The caller
 * can count the number of names in the string buffer and compare with the
 * returned number to check if it is sensible to call this function again with
 * a bigger string buffer.
 *
 * \param[in]  vid			USB vendor id.
 * \param[in]  pid			USB product id.
 * \param[out] port_list    A pointer to the string buffer where the port
 *                          names will be written to.
 * \param[in]  buffer_size  The size of the string buffer in bytes.
 *
 * \return The function returns the number of available COM ports.
 */
uint32_t ifx_comport_get_list(uint16_t vid, uint16_t pid, char* port_list, size_t buffer_size);

/**
 * \brief This functions opens the specified COM port.
 *
 * The name of the COM port must be provided as zero terminated string. It is
 * recommended to call \ref ifx_comport_get_list to find the available COM ports
 * and pass one of the returned port names to the open function.
 *
 * \param[in] port_name  A zero terminated string containing the name of the
 *                       COM port
 * \param[in] baudrate	 The used baudrate.
 *
 * \return If the function succeeds, the opened COM port object is returned.
 *         If the COM port could not be opened, NULL is returned.
 */
com_t* ifx_comport_open(const char* port_name, uint32_t baudrate);

/**
 * \brief This function closes the specified COM port.
 *
 * The function expects a handle to an opened COM port that have been returned
 * by \ref ifx_comport_open.
 *
 * \param[in] com_port  The COM port that should be closed.
 */
void ifx_comport_close(com_t* com_port);

/**
 * \brief This function sends data through an open COM port.
 *
 * The function transmits each byte from the provided buffer through the
 * specified COM port and returns after the last byte has been sent.
 *
 * The function expects a handle to an opened COM port that have been returned
 * by \ref ifx_comport_open.
 *
 * \param[in] com_port     The open COM port.
 * \param[in] data         A pointer to the buffer containing the data to be
 *                         sent.
 * \param[in] num_bytes    The number of bytes in pBuffer.
 */
size_t ifx_comport_send_data(com_t* com_port, const void* data, size_t num_bytes);

/**
 * \brief This function reads data from an open COM port.
 *
 * The function tries to read the specified number of bytes from an open COM
 * port and stores them in the provided buffer. The function waits for the
 * data, but returns after an internally defined timeout period and returns
 * the number of received bytes.
 *
 * The function expects a handle to an opened COM port that have been returned
 * by \ref ifx_comport_open.
 *
 * \param[in]  com_port             The open COM port.
 * \param[out] data                 A pointer to the buffer where the received
 *                                  data will be stored.
 * \param[in]  num_requested_bytes  The number of bytes that should be read
 *                                  from the COM port.
 *
 * \return The function returns the number of bytes that actually have been
 *         received. This number may be smaller than uNumRequestedBytes.
 */
size_t ifx_comport_get_data(com_t* com_port, void* data, size_t num_requested_bytes);

/**
 * \brief This function changes the timeout period of an open COM port
 *
 * This function changes the timeout period that will be used when data is read
 * through \ref ifx_comport_get_data. That function will wait until timeout period is
 * elapsed before returning, if not enough data is received.
 *
 * The function expects a handle to an open COM port that have been returned
 * by \ref ifx_comport_open.
 *
 * \param[in]  com_port             The open COM port.
 * \param[in]  timeout_period_ms    The new timeout period in milli seconds.
 */
void ifx_comport_set_timeout(com_t* com_port, uint32_t timeout_period_ms);

/* --- Close open blocks -------------------------------------------------- */

/* Disable C linkage for C++ files */
#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

/* End of include guard */
#endif /* COMPORT_H_INCLUDED */

/* --- End of File -------------------------------------------------------- */
