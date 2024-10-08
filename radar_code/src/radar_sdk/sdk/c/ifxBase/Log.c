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

#include "Log.h"

/*
==============================================================================
   2. DEFINITIONS
==============================================================================
*/

#define IFX_LOG_TAG_WARN  "WARNING"
#define IFX_LOG_TAG_ERROR "ERROR"
#define IFX_LOG_TAG_DEBUG "DEBUG"
#define IFX_LOG_TAG_INFO  "INFO"

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

/*
==============================================================================
   5. LOCAL FUNCTION PROTOTYPES
==============================================================================
*/

static char* get_severity_tag(ifx_Log_Severity_t severity);

/*
==============================================================================
   6. LOCAL FUNCTIONS
==============================================================================
*/

static char* get_severity_tag(ifx_Log_Severity_t severity)
{
    switch (severity)
    {
        case IFX_LOG_WARNING:
            return IFX_LOG_TAG_WARN;

        case IFX_LOG_ERROR:
            return IFX_LOG_TAG_ERROR;

        case IFX_LOG_DEBUG:
            return IFX_LOG_TAG_DEBUG;

        case IFX_LOG_INFO:
            return IFX_LOG_TAG_INFO;

        default:
            return IFX_LOG_TAG_ERROR;
    }
}

/*
==============================================================================
   7. EXPORTED FUNCTIONS
==============================================================================
*/

void ifx_log(FILE* f, ifx_Log_Severity_t severity, const char* msg, ...)
{
    va_list argl;
    fprintf(f, "%s: ", get_severity_tag(severity));
    va_start(argl, msg);
    vfprintf(f, msg, argl);
    va_end(argl);
    fprintf(f, "\n");
}
