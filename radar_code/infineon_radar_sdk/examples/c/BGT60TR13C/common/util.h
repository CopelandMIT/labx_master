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
 * @file util.h
 *
 * @brief This file defines the API to check if files exist and are readable.
 *
 */

#ifndef UTIL_H
#define UTIL_H

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include <stdio.h>

#include "ifxBase/Matrix.h"


#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

/*
==============================================================================
   2. DEFINITIONS
==============================================================================
*/

#ifdef _WIN32
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif

/*
==============================================================================
   3. TYPES
==============================================================================
*/

/**
 * @brief Reads matrix from file
 *
 * Read a matrix saved by \ref print_matrix_to_file_r.
 *
 * Note that matrix needs to be valid and needs to have the right dimension.
 *
 * @param [in] f        file pointer to open file
 * @param [in] matrix   matrix
 * @retval true if successful
 * @retval false if an error occurred
 */
bool get_matrix_from_file_r(FILE* f, ifx_Matrix_R_t* matrix);

/**
 * @brief Prints matrix to file
 *
 * Print matrix to the open file f. The matrix as plain text to the file, each
 * matrix on one line in row-major order.
 *
 * @param [in] f        file pointer to open file
 * @param [in] matrix   matrix that will be printed to f
 */
void print_matrix_to_file_r(FILE* f, ifx_Matrix_R_t* matrix);

/** @brief Changes to the directory name of filepath
 *
 * Given a path to a file, the function will change the current directory to
 * the directory of the file.
 *
 * Note: The function might write to filepath. However, on exit filepath will
 * be unaltered.
 *
 * @param [in]  filepath    path to file
 * @retval true if successful
 * @retval true if an error occurred
 */
bool change_to_dirname(char* filepath);

/** @brief returns pointer to character string excluding path
 *
 * Used to strip path from filename
 *
 * @param [in] pathname path to file
 * @retval  pointer to character string containing filename only
 */
char* extract_filename_from_path(char* filepath);

/** @brief Checks if file is executable
 *
 * Check if the file given by pathname is both readable and executable (o+rx).
 * On Windows, this function is identical with \ref file_readable.
 *
 * @param [in] pathname path to file
 * @retval  true    if file is readable and executable
 * @retval  false   otherwise
 */
bool file_executable(const char* pathname);

/** @brief Checks if file is readable
 *
 * Check if the file given by pathname is readable.
 *
 * @param [in] pathname path to file
 * @retval  true    if file is readable
 * @retval  false   otherwise
 */
bool file_readable(const char* pathname);

/** @brief Reads complete content of a file
 *
 * Read the content of the file given by pathname to a buffer and return it.
 * A null-byte is added to the read data.
 *
 * You have to free the returned pointer yourself after use.
 *
 * @param [in] pathname    path to file
 * @retval pointer to data if successful
 * @retval NULL otherwise
 */
void* file_slurp(const char* pathname);

/**
 * @brief Disables buffering to file handle
 *
 * @param [in] fh   file handle
 */
void disable_buffering(FILE* fh);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  /* UTIL_H */
