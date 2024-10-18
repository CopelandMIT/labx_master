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
 * @file util.c
 *
 * @brief This file implements the APIs to check if files exist and are readable.
 */

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#include <windows.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif

#include "util.h"

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

void disable_buffering(FILE* fh)
{
    fflush(fh);
    setvbuf(fh, NULL, _IONBF, 0);
}

//----------------------------------------------------------------------------

bool get_matrix_from_file_r(FILE* f, ifx_Matrix_R_t* matrix)
{
    char temp[24];
    char* cur_pos = NULL;
    for (uint32_t i = 0; i < IFX_MAT_ROWS(matrix); i++)
    {
        for (uint32_t j = 0; j < IFX_MAT_COLS(matrix); j++)
        {
            do
            {
                cur_pos = fgets(temp, 24, f);
                if (cur_pos == NULL)
                    return false;
            } while ((strlen(cur_pos) <= 2) && isspace((unsigned char)*cur_pos));
            ifx_Float_t value = (ifx_Float_t)atof(temp);
            IFX_MAT_AT(matrix, i, j) = value;
        }
    }
    return true;
}

//----------------------------------------------------------------------------

void print_matrix_to_file_r(FILE* f, ifx_Matrix_R_t* matrix)
{
    ifx_Float_t cur_val;
    for (uint32_t i = 0; i < IFX_MAT_ROWS(matrix); i++)
    {
        for (uint32_t j = 0; j < IFX_MAT_COLS(matrix); j++)
        {
            cur_val = IFX_MAT_AT(matrix, i, j);
            fprintf(f, "%f\n", cur_val);
        }
        fprintf(f, "\n");
    }
}

//----------------------------------------------------------------------------
char* extract_filename_from_path(char* filepath)
{
    char* p = strrchr(filepath, PATH_SEPARATOR);

    // if the file does not have a path, called from the same directory
    if (p == NULL)
        return filepath;
    return (p + 1);
}

//----------------------------------------------------------------------------

bool file_executable(const char* pathname)
{
#ifndef _WIN32
    if (access(pathname, R_OK | X_OK) == 0)
        return true;
    else
        return false;
#else
    return file_readable(pathname);
#endif
}

//----------------------------------------------------------------------------

bool file_readable(const char* pathname)
{
#ifdef _WIN32
    if (_access(pathname, 0) == 0)
        return true;
    else
        return false;
#else
    if (access(pathname, R_OK) == 0)
        return true;
    else
        return false;
#endif
}

//----------------------------------------------------------------------------

void* file_slurp(const char* pathname)
{
    struct stat filestatus;

    if (stat(pathname, &filestatus) != 0)
        return NULL;

    FILE* fp = fopen(pathname, "rb");
    if (fp == NULL)
        return NULL;

    void* content = malloc(filestatus.st_size + (size_t)1);
    if (content == NULL)
    {
        fclose(fp);
        return NULL;
    }

    size_t ret = fread(content, 1, filestatus.st_size, fp);
    fclose(fp);

    if (ret != (size_t)filestatus.st_size)
    {
        free(content);
        return NULL;
    }

    ((char*)content)[ret] = '\0';

    return content;
}
