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


/*
==============================================================================
   2. LOCAL DEFINITIONS
==============================================================================
*/

#if (_MSC_VER && !__INTEL_COMPILER) || ((_WIN32 || _WIN64) && __GNUC__)
#include <malloc.h>
#define ALIGNED_MALLOC(size, align, mem) mem = _aligned_malloc((size), (alignment))
#define ALIGNED_FREE(mem)   \
    do                      \
    {                       \
        _aligned_free(mem); \
        (mem) = NULL;       \
    } while (0)
#else
// posix_memalign requires _POSIX_C_SOURCE >= 200112L. See manpage of posix_memalign for more information.
// This define needs to be set before including stdlib.h.
#ifndef _POSIX_C_SOURCE
// NOLINTNEXTLINE(bugprone-reserved-identifier)
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdlib.h>

#define ALIGNED_MALLOC(size, align, mem)                      \
    do                                                        \
    {                                                         \
        if (posix_memalign(&(mem), (alignment), (size)) != 0) \
        {                                                     \
            (mem) = NULL;                                     \
        }                                                     \
    } while (0)
#define ALIGNED_FREE(mem) \
    do                    \
    {                     \
        free(mem);        \
        (mem) = NULL;     \
    } while (0)
#endif

// include only here to avoid warning about posix_memalign
#include "Mem.h"

/*
==============================================================================
   7. EXPORTED FUNCTIONS
==============================================================================
*/

void* ifx_mem_alloc(size_t size)
{
    void* mem = malloc(size);
    return mem;
}

//----------------------------------------------------------------------------

void* ifx_mem_calloc(size_t count,
                     size_t element_size)
{
    void* mem = calloc(count, element_size);
    return mem;
}

//----------------------------------------------------------------------------

void* ifx_mem_aligned_alloc(size_t size,
                            size_t alignment)
{

    void* mem = 0;
    ALIGNED_MALLOC(size, alignment, mem);
    return mem;
}

//----------------------------------------------------------------------------

void ifx_mem_free(void* mem)
{
    free(mem);
}

//----------------------------------------------------------------------------

void ifx_mem_aligned_free(void* mem)
{
    ALIGNED_FREE(mem);
}
