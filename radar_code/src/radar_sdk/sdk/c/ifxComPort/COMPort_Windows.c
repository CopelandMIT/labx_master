/**
 * \file COMPort_Windows.c
 *
 * \brief This file implements the API to access a serial communication port
 *        for Windows.
 *
 * \see COMPort.h for details
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

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/
#include "COMPort.h"

#if defined _WIN32

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <tchar.h>

#include <windows.h>

// setupapi.h has to be included after windows.h
#include <setupapi.h>

/*
==============================================================================
   3. LOCAL TYPES
==============================================================================
*/

struct com_s
{
    HANDLE handle;
};

typedef struct
{
    char* port_list;
    uint32_t ports_available;
    size_t buffer_available;
} Port_List_t;

/*
==============================================================================
   7. EXPORTED FUNCTIONS
==============================================================================
*/

com_t* ifx_comport_open(const char* port_name, uint32_t baudrate)
{
    COMMTIMEOUTS timeouts;
    DCB com_configuration = {0};

    com_t* com_port = malloc(sizeof(com_t));
    if (com_port == NULL)
        return NULL;

/*
 * use full path because Windows won't find "COM10" and higher
 * (see knowledge base http://support.microsoft.com/kb/115831/en-us)
 *
 * To work with both Multi-Byte-Charset and Unicode Charset convert
 * port_name to TCHAR, because that type is needed by Win32 API.
 * I did not find a suitable out-of-the-box function for this conversion
 * so it's done explicitly here.
 */
#define T_BUFFER_LENGTH 32
    TCHAR full_port_name[T_BUFFER_LENGTH] = TEXT("\\\\.\\");
    unsigned idx = 0;
    while ((port_name[idx] != 0) && (idx + 5 < T_BUFFER_LENGTH))
    {
        full_port_name[4 + idx] = port_name[idx];
        ++idx;
    }
    full_port_name[4 + idx] = 0;

    /* open the COM port */
    /* ----------------- */
    com_port->handle = CreateFile(full_port_name, /* name of the COM port */
                                  /*lint -e620 we don't really have a chance to change a MSYS/MinGW header file ... */
                                  GENERIC_READ | /* access type */
                                      /*lint -e620 */
                                      GENERIC_WRITE,
                                  0,             /* shared mode */
                                  NULL,          /* security attributes */
                                  OPEN_EXISTING, /* creation disposition */
                                  0,             /* flags and attributes */
                                  0);            /* template file */

    /* if COM port could not be opened, return negative Windows error code */
    if (com_port->handle == INVALID_HANDLE_VALUE)
    {
        return NULL;
    }

    /* set timeouts */
    timeouts.ReadIntervalTimeout = 0;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.ReadTotalTimeoutConstant = 1000;
    timeouts.WriteTotalTimeoutConstant = 100;
    timeouts.WriteTotalTimeoutMultiplier = 1;
    (void)SetCommTimeouts(com_port->handle, &timeouts);

    /* configure COM Port (even though it's virtual) */
    com_configuration.DCBlength = sizeof(DCB);
    (void)GetCommState(com_port->handle, &com_configuration);
    com_configuration.BaudRate = baudrate;
    com_configuration.ByteSize = 8;
    com_configuration.StopBits = ONESTOPBIT;
    (void)SetCommState(com_port->handle, &com_configuration);

    return com_port;
}

void ifx_comport_close(com_t* com_port)
{
    if (com_port == NULL)
        return;

    /* stop all transfers are in progress */
    (void)CancelIo(com_port->handle);

    /* close COM port */
    (void)CloseHandle(com_port->handle);

    /* free memory for com_port */
    free(com_port);
}

size_t ifx_comport_send_data(com_t* com_port, const void* data, size_t num_bytes)
{
    /* send data */
    DWORD num_bytes_written;
    WriteFile(com_port->handle, data, (DWORD)num_bytes, &num_bytes_written, NULL);
    return num_bytes_written;
}

size_t ifx_comport_get_data(com_t* com_port,
                            void* data, size_t num_requested_bytes)
{
    /* read data */
    DWORD num_bytes_read = 0;

    (void)ReadFile(com_port->handle, data, (DWORD)num_requested_bytes, &num_bytes_read, NULL);

    return num_bytes_read;
}

void ifx_comport_set_timeout(com_t* com_port, uint32_t timeout_period_ms)
{
    /* set timeouts */
    COMMTIMEOUTS timeouts;
    timeouts.ReadIntervalTimeout = 0;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.ReadTotalTimeoutConstant = timeout_period_ms;
    timeouts.WriteTotalTimeoutConstant = 100;
    timeouts.WriteTotalTimeoutMultiplier = 1;
    (void)SetCommTimeouts(com_port->handle, &timeouts);
}

/* --- Close open blocks -------------------------------------------------- */

/* End of Windows only code */
#endif /* _WIN32 */

/* --- End of File -------------------------------------------------------- */
