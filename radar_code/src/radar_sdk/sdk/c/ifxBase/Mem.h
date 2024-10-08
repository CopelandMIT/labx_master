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
 * @file Mem.h
 *
 * \brief \copybrief gr_mem
 *
 * For details refer to \ref gr_mem
 */

#ifndef IFX_BASE_MEM_H
#define IFX_BASE_MEM_H

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include "Types.h"


#ifdef __cplusplus
extern "C"
{
#endif


/*
==============================================================================
   2. DEFINITIONS
==============================================================================
*/

/// By default the data of vectors, matrices, and cubes is aligned to this boundary
#define IFX_MEMORY_ALIGNMENT 32U

#define IFX_ALIGN(x, SIZE_ALIGNMENT) (((x) + ((SIZE_ALIGNMENT)-1)) & ~((SIZE_ALIGNMENT)-1))

/// Check if pointer is aligned to SIZE_ALIGNMENT
#define IFX_IS_ALIGNED(POINTER, SIZE_ALIGNMENT) (((uintptr_t)(const void*)(POINTER)) % (SIZE_ALIGNMENT) == 0)


/*
==============================================================================
   3. TYPES
==============================================================================
*/

/*
==============================================================================
   4. FUNCTION PROTOTYPES
==============================================================================
*/

/** @addtogroup gr_cat_SDK_base
 * @{
 */

/** @defgroup gr_mem Memory
 * @brief API for memory management
 *
 * Supports memory allocation and deallocation
 * as well as aligned allocation and aligned deallocation.
 *
 * @{
 */

/**
 * @brief Allocates memory of defined size.
 *
 * @param [in]     size      Number of bytes to be allocated.
 *
 * @return Pointer to the allocated memory if successful
 *         otherwise it returns NULL.
 */
IFX_DLL_PUBLIC
void* ifx_mem_alloc(size_t size);

/**
 * @brief Allocates memory for an array of defined number of elements
 *        with specified element size and initializes each byte to zero.
 *
 * @param [in]     count               Number of elements to be allocated.
 * @param [in]     element_size        Size of one element of the array.
 *
 * @return Pointer to the allocated memory if successful
 *         otherwise it returns NULL.
 */
IFX_DLL_PUBLIC
void* ifx_mem_calloc(size_t count,
                     size_t element_size);

/**
 * @brief Allocates memory of defined size with specified alignment.
 *        The size must be a multiple of alignment.
 *
 * @param [in]     size                Number of bytes to be allocated.
 * @param [in]     alignment           The number the allocated memory is aligned to.
 *
 * @return Pointer to the allocated memory if successful
 *         otherwise it returns NULL.
 */
IFX_DLL_PUBLIC
void* ifx_mem_aligned_alloc(size_t size,
                            size_t alignment);

/**
 * @brief Deallocates the memory which has been allocated by \ref ifx_mem_alloc
 *        or \ref ifx_mem_calloc. Do not use this to deallocate memory allocated
 *        with \ref ifx_mem_aligned_alloc.
 *
 * @param [in]     mem       Pointer to the memory to be deallocated.
 */
IFX_DLL_PUBLIC
void ifx_mem_free(void* mem);

/**
 * @brief Deallocates the memory which has been allocated by \ref ifx_mem_aligned_alloc.
 *        Do not use this to deallocate memory allocated allocated with \ref ifx_mem_alloc
 *        or \ref ifx_mem_calloc.
 *
 * @param [in]     mem       Pointer to the memory to be deallocated.
 */
IFX_DLL_PUBLIC
void ifx_mem_aligned_free(void* mem);

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* IFX_BASE_MEM_H */
