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

#ifndef IFX_BASE_LIST_H
#define IFX_BASE_LIST_H

#include "Types.h"


#ifdef __cplusplus
extern "C"
{
#endif


/** @addtogroup gr_cat_SDK_base
 * @{
 */

/** @defgroup gr_list List
 * @brief API for list handling
 * @{
 */


typedef struct ifxList ifx_List_t;

/**
 * @brief Creates new list.
 *
 * Create a new list.
 *
 * destructor is a pointer to a function that frees the memory for an
 * element. If destructor is a valid pointer, items of the list will
 * be freed when the corresponding pointer is removed. If destructor
 * is NULL, the memory for the entries will not be freed.
 *
 * @param   [in]    destructor  destructor
 * @return list if successful
 * @return NULL if an error occurred
 */
IFX_DLL_PUBLIC
ifx_List_t* ifx_list_create(void destructor(void*));

/**
 * @brief Destroy list.
 *
 * Destroy the list. If a destructor has been given to \ref ifx_list_create
 * then also all elements of the list will be freed.
 *
 * @param [in]  list    pointer to list
 */
IFX_DLL_PUBLIC
void ifx_list_destroy(ifx_List_t* list);

/**
 * @brief Get current size of list.
 *
 * Return the current number of elements saved in the list.
 *
 * @param [in]  list    pointer to list
 * @retval  number of elements saved in the list
 */
IFX_DLL_PUBLIC
size_t ifx_list_size(const ifx_List_t* list);

/**
 * @brief Append element to the back of the list.
 *
 * Append an element to the back of the list.
 *
 * If an allocation error occurs, the function returns false and the error
 * IFX_ERROR_MEMORY_ALLOCATION_FAILED is set.
 *
 * @param [in]  list    pointer to list
 * @param [in]  elem    element
 * @retval  true        if successful
 * @retval  false       if an allocation error occurred
 */
IFX_DLL_PUBLIC
bool ifx_list_push_back(ifx_List_t* list, void* elem);

/**
 * @brief Get element for index.
 *
 * Return the element of the list at the given index.
 *
 * If the index is not valid NULL is returned and the error
 * IFX_ERROR_ARGUMENT_OUT_OF_BOUNDS is set.
 *
 * @param [in]  list    pointer to list
 * @param [in]  index   index
 * @retval  element     if successful
 * @retval  NULL        otherwise
 */
IFX_DLL_PUBLIC
void* ifx_list_get(const ifx_List_t* list, size_t index);

/**
 * @}
 */

/**
 * @}
 */


#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* IFX_BASE_LIST_H */
