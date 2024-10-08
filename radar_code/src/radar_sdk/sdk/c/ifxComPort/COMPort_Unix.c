/**
 * \file COMPort_Unix.c
 *
 * \brief This file implements the API to access a serial communication port
 *        for macOS and Linux.
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

#if (defined __APPLE__) || (defined LINUX) || (defined __linux__)

#ifndef _DEFAULT_SOURCE
// _DEFAULT_SOURCE is required by cfmakeraw
// NOLINTNEXTLINE(bugprone-reserved-identifier)
#define _DEFAULT_SOURCE
#endif

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "COMPort.h"

/*
==============================================================================
   3. LOCAL TYPES
==============================================================================
*/
/**
 * \brief This struct contains all information about an open connection.
 */
struct com_s
{
    int handle;                /**< The handle to the connection. */
    int32_t timeout_period_ms; /**< The period after that reading is stopped
                                    if no more data arrives. */
};

/*
==============================================================================
   7. EXPORTED FUNCTIONS
==============================================================================
*/

#ifndef __APPLE__
static bool baudrate_to_speed(uint32_t baudrate, speed_t* speed)
{
    switch (baudrate)
    {
        case 9600:
            *speed = B9600;
            return true;
        case 19200:
            *speed = B19200;
            return true;
        case 38400:
            *speed = B38400;
            return true;
        case 57600:
            *speed = B57600;
            return true;
        case 115200:
            *speed = B115200;
            return true;
        case 230400:
            *speed = B230400;
            return true;
        case 460800:
            *speed = B460800;
            return true;
        case 500000:
            *speed = B500000;
            return true;
        case 576000:
            *speed = B576000;
            return true;
        case 921600:
            *speed = B921600;
            return true;
        case 1000000:
            *speed = B1000000;
            return true;
        case 1152000:
            *speed = B1152000;
            return true;
        case 1500000:
            *speed = B1500000;
            return true;
        case 2000000:
            *speed = B2000000;
            return true;
        case 2500000:
            *speed = B2500000;
            return true;
        case 3000000:
            *speed = B3000000;
            return true;
        case 3500000:
            *speed = B3500000;
            return true;
        case 4000000:
            *speed = B4000000;
            return true;
    }

    return false;
}
#endif  // __APPLE__

com_t* ifx_comport_open(const char* port_name, uint32_t baudrate)
{
    struct termios options;

    com_t* com_port = malloc(sizeof(com_t));
    if (com_port == NULL)
        return NULL;

    /*
     * Open the serial port read/write, with no controlling terminal, and
     * don't wait for a connection. See open(2) ("man 2 open") for details.
     */
    com_port->handle = open(port_name, O_RDWR | O_NOCTTY);

    if (com_port->handle == -1)
        goto fail;

    /*
     * Note that open() follows POSIX semantics: multiple open() calls to the
     * same file will succeed unless the TIOCEXCL ioctl is issued. This will
     * prevent additional opens except by root-owned processes.
     * See tty(4) ("man 4 tty") and ioctl(2) ("man 2 ioctl") for details.
     */
    if (ioctl(com_port->handle, TIOCEXCL) == -1)
    {
        /* Don't treat as fatal if the command is just not supported. */
        if (errno != ENOTTY)
            goto fail;
    }

    /* Get the current options and save them so we can restore the default
     * settings later.
     */
    if (tcgetattr(com_port->handle, &options) == -1)
    {
        goto fail;
    }

    /*
     * The serial port attributes such as timeouts and baud rate are set by
     * modifying the termios structure and then calling tcsetattr to  cause
     * the changes to take effect. Note that the changes will not take effect
     * without the tcsetattr() call. See tcsetattr(4) ("man 4 tcsetattr") for
     * details.
     *
     * Set raw input (non-canonical) mode, with non-blocking reads.
     * See tcsetattr(4) ("man 4 tcsetattr") and termios(4) ("man 4 termios")
     * for details.
     */
    cfmakeraw(&options);
    options.c_cc[VMIN] = 0;
    options.c_cc[VTIME] = 0;

    /* Enable local mode, because the USBD_VCOM APP does not handle the
     * virtual flow control lines
     */
    options.c_cflag |= CLOCAL;

    /** Set baudrate. If not successful, use 115200 as fallback. */
#ifdef __APPLE__
    speed_t speed = baudrate;
#else
    speed_t speed = B115200;
    baudrate_to_speed(baudrate, &speed);
#endif
    cfsetispeed(&options, speed);
    cfsetospeed(&options, speed);

    /* Cause the new options to take effect immediately. */
    if (tcsetattr(com_port->handle, TCSANOW, &options) == -1)
        goto fail;

    com_port->timeout_period_ms = 1000;
    return com_port;

fail:
    free(com_port);
    return NULL;
}

void ifx_comport_close(com_t* com_port)
{
    /* close COM port */
    close(com_port->handle);

    /* free allocated memory */
    free(com_port);
}

size_t ifx_comport_send_data(com_t* com_port, const void* data, size_t num_bytes)
{
    /* send data */
    return write(com_port->handle, data, num_bytes);
}

size_t ifx_comport_get_data(com_t* com_port,
                            void* data, size_t num_requested_bytes)
{
    size_t num_received_bytes = 0;
    char* read_buffer = (char*)data;
    struct timespec time_of_last_byte;

    /* read data */
    clock_gettime(CLOCK_MONOTONIC, &time_of_last_byte);
    while (num_received_bytes < num_requested_bytes)
    {
        ssize_t num_bytes = read(com_port->handle,
                                 read_buffer + num_received_bytes,
                                 num_requested_bytes - num_received_bytes);
        num_received_bytes += num_bytes;

        /* check for timeout */
        struct timespec current_time;
        clock_gettime(CLOCK_MONOTONIC, &current_time);
        if (num_bytes != 0)
        {
            time_of_last_byte = current_time;
        }
        else
        {
            if ((current_time.tv_sec * 1000 + current_time.tv_nsec / 1000000) > (time_of_last_byte.tv_sec * 1000
                                                                                 + time_of_last_byte.tv_nsec / 1000000
                                                                                 + com_port->timeout_period_ms))
            {
                break;
            }
        }
    }
    return num_received_bytes;
}

void ifx_comport_set_timeout(com_t* com_port, uint32_t timeout_period_ms)
{
    com_port->timeout_period_ms = timeout_period_ms;
}

/* --- Close open blocks -------------------------------------------------- */

/* End of UNIX only code */
#endif /* (defined __APPLE__) || (defined LINUX) || (defined __linux__) */

/* --- End of File -------------------------------------------------------- */
