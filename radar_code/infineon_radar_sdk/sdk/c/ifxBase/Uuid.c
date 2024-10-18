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

#if _MSC_VER
/* suppress warning about unsafe function strcpy */
#pragma warning(disable : 4996)
#endif

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "Mem.h"

#include "Uuid.h"

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

/*
==============================================================================
   7. EXPORTED FUNCTIONS
==============================================================================
*/

bool ifx_uuid_from_string(const char* s, uint8_t uuid[16])
{
    const size_t length = strlen(s);
    if (length < 32)
        return false;

    uint8_t uuid_internal[16] = {0};
    // memset(uuid_internal, 0, sizeof(uuid_internal));

    int pos = 0;
    for (size_t i = 0; i < length && pos < 32; i++)
    {
        uint8_t value = 0;
        const char c = (char)tolower(s[i]);
        if (c == '-')
            continue;
        else if (c >= '0' && c <= '9')
            value = c - '0';
        else if (c >= 'a' && c <= 'f')
            value = c - 'a' + 10;
        else
            return false;

        if (pos % 2 == 0)
            uuid_internal[pos / 2] |= value << 4;
        else
            uuid_internal[pos / 2] |= value;

        pos++;
    }

    if (pos != 32)
        return false;

    memcpy(uuid, uuid_internal, sizeof(uuid_internal));
    return true;
}

void ifx_uuid_to_string(const uint8_t uuid[16], char* string)
{
    /* In its canonical textual representation, the 16 octets of a UUID are represented
     * as 32 hexadecimal (base-16) digits, displayed in 5 groups separated by hyphens,
     * in the form 8-4-4-4-12 for a total of 36 characters (32 hexadecimal characters
     * and 4 hyphens). For example:
     *    123e4567-e89b-12d3-a456-426655440000
     *    xxxxxxxx-xxxx-Mxxx-Nxxx-xxxxxxxxxxxx
     */
    sprintf(string, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
            uuid[0], uuid[1], uuid[2], uuid[3], uuid[4], uuid[5], uuid[6], uuid[7],
            uuid[8], uuid[9], uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);
}
