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

/**
 * @file time_formatter.c
 *
 * @brief This file implements the API for time formatting.
 *
 */

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
#include <windows.h>
#else
#include <sys/time.h>
#endif
#include "time_formatter.h"

/*
==============================================================================
   2. LOCAL DEFINITIONS
==============================================================================
*/

#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif

#define NUM_D_DIGITS         3
#define NUM_H_DIGITS         2
#define NUM_M_DIGITS         2
#define NUM_S_DIGITS         2
#define NUM_MS_DIGITS        3
#define NUM_ADDITIONAL_CHARS 6

#define STR(X)      #X
#define XSTR(X)     STR(X)
#define TIME_FORMAT "%0" XSTR(NUM_H_DIGITS) "d:%0" XSTR(NUM_M_DIGITS) "d:%0" XSTR(NUM_S_DIGITS) "d.%0" XSTR(NUM_MS_DIGITS) "d"

/*
==============================================================================
   3. LOCAL TYPES
==============================================================================
*/
struct ifx_Time_s
{

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
    LARGE_INTEGER freq;
    LARGE_INTEGER start;
    LARGE_INTEGER end;
#else
    struct timeval start;
    struct timeval end;
#endif
    clock_t start_clock;
    clock_t end_clock;
    char* cur_time_str;
    size_t cur_time_size;
    float cur_time_s;
};

/*
==============================================================================
   4. DATA
==============================================================================
*/

/*
==============================================================================
   5. LOCAL FUNCTION PROTOTYPES
==============================================================================
*/

static void ifx_time_init(ifx_Time_Handle_t handle);

/*
==============================================================================
  6. LOCAL FUNCTIONS
==============================================================================
*/

static void ifx_time_init(ifx_Time_Handle_t handle)
{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
    QueryPerformanceFrequency(&handle->freq);
    QueryPerformanceCounter(&handle->start);
#else
    gettimeofday(&handle->start, NULL);
#endif
}

/*
==============================================================================
   7. EXPORTED FUNCTIONS
==============================================================================
*/

ifx_Error_t ifx_time_create(ifx_Time_Handle_t* handle)
{
    *handle = malloc(sizeof(struct ifx_Time_s));

    if (*handle == NULL)
        return -1;

    ifx_time_init(*handle);

    (*handle)->cur_time_size = NUM_D_DIGITS + 2 + NUM_H_DIGITS
                               + NUM_M_DIGITS + NUM_S_DIGITS
                               + NUM_MS_DIGITS + NUM_ADDITIONAL_CHARS;
    (*handle)->cur_time_str = calloc(1, (*handle)->cur_time_size * sizeof(char));

    if ((*handle)->cur_time_str == NULL)
        return -1;

    return 0;
}

//----------------------------------------------------------------------------

int64_t ifx_time_get_ms(ifx_Time_Handle_t handle)
{
    int64_t elapsed = 0;
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
    QueryPerformanceCounter(&handle->end);
    elapsed = ((handle->end.QuadPart - handle->start.QuadPart) * 1000) / handle->freq.QuadPart;

#else
    gettimeofday(&handle->end, NULL);
    elapsed = (handle->end.tv_sec - handle->start.tv_sec) * 1000;
    elapsed += (handle->end.tv_usec - handle->start.tv_usec) / 1000;
#endif
    return elapsed;
}

//----------------------------------------------------------------------------

char* ifx_time_get_cstr(ifx_Time_Handle_t handle)
{
    int64_t time_ms = ifx_time_get_ms(handle);

    int64_t time_sec = time_ms / 1000;
    int64_t time_min = time_sec / 60;
    int64_t time_h = time_min / 60;
    int ms = (int)(time_ms % 1000);
    int s = (int)(time_sec % 60);
    int m = (int)(time_min % 60);
    int h = (int)(time_h % 24);
    int d = (int)(time_h / 24);

    if (d > 999)
        d = 0;

    if (d)
        sprintf(handle->cur_time_str, "%dd " TIME_FORMAT, d, h, m, s, ms);
    else
        sprintf(handle->cur_time_str, TIME_FORMAT, h, m, s, ms);

    return handle->cur_time_str;
}

//----------------------------------------------------------------------------

void ifx_time_destroy(ifx_Time_Handle_t handle)
{
    if (handle == NULL)
        return;

    free(handle->cur_time_str);
    free(handle);
}
