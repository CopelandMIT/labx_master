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
 * @file DeInterleaver.h
 *
 * \brief \copybrief gr_complex
 *
 * For details refer to \ref gr_complex
 */

#ifndef IFX_RADAR_DEINTERLEAVER_H
#define IFX_RADAR_DEINTERLEAVER_H

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include "ifxBase/Types.h"


#ifdef __cplusplus
extern "C"
{
#endif


/*
==============================================================================
   2. DEFINITIONS
==============================================================================
*/

/**
 * @brief DeInterleaver for incoming radar data which allows all existing frame configurations to be decoded.
 *
 * New data shall be added to the de-interleaver with ifx_di_add_input_samples().
 * To check if there's sufficient input data to decode on or multiple frames, use
 * ifx_di_is_frame_complete(). Once there's enough input data to decode one or multiple frames
 * fetch them using ifx_di_get_frame(). The proper decode loop looks like
 *
 * @code{.c}
 * // add incoming samples
 *   ifx_di_add_input_samples(handle, data, length);
 *       // as long as there are complete received frames
 *       // in the de-interleaver, fetch them
 *       while (ifx_di_is_frame_complete(handle)) {
 *           ifx_di_get_frame(handle, buffer_for_frame, buffer_length);
 *           // ... do something with the received data ...
 *       }
 * @endcode
 */
typedef struct ifx_DeInterleaver_s ifx_DeInterleaver_t;

/**
 * @brief Description of the incoming radar data frames
 *
 * This structure describes the shape of the incoming radar data frames.
 * Unused shapes and/or chirps should have all their fields set to zero.
 *
 */
typedef struct
{
    struct shape_s
    {
        struct chirp_s
        {
            size_t samples_per_chirp; /**< number of samples per chirp */
            uint32_t rx_mask;         /**< mask of active rx antennas */
        } up, down;                   /**< up and down chirp format */
        uint32_t repeat;              /**< shape repeat */
    } shape[4];                       /**< format of the active shapes */
    uint32_t shape_set_repeat;        /**< how often the entire shape set is
                                           repeated within a frame*/
} ifx_DeInterleaver_Frame_Definition_t;

/*
==============================================================================
   3. TYPES
==============================================================================
*/


/**
 * @brief Create a new de-interleaver instance
 *
 * This function creates a new de-interleaver that is used to combine
 * and reshape incoming radar data.
 *
 * @return Handle to the newly created deinterleaver or NULL in case of failure
 *
 */
IFX_DLL_PUBLIC
ifx_DeInterleaver_t* ifx_di_create();

/**
 * @brief Free the de-interleaver instance and associated resources
 *
 * @param [in]  handle    Handle for the existing de-interleaver instance
 *
 */
IFX_DLL_PUBLIC
void ifx_di_destroy(ifx_DeInterleaver_t* handle);


/**
 * @brief Configure the shape of the input data
 *
 * This function configures the shape of the input data. It also clears any input
 * buffers, so it is recommended to call this function whenever starting a new acquisition
 *
 * @param [in]  handle           Handle for the existing de-interleaver instance
 * @param [in]  frame_definition Description of the shape of incoming data
 *
 */
IFX_DLL_PUBLIC
void ifx_di_set_frame_definition(ifx_DeInterleaver_t* handle, const ifx_DeInterleaver_Frame_Definition_t* frame_definition);

/**
 * @brief Add samples to the input buffer of the de-interleaver
 *
 * This function adds incoming samples to the input buffer of the de-interleaver.
 * Once the input buffer has received enough data for a complete frame, which can be
 * queried using ifx_di_is_frame_complete(), the resulting frame can be fetched using
 * ifx_di_get_frame().
 *
 * @param [in]  handle           Handle for the existing de-interleaver instance
 * @param [in]  data             Buffer containing the incoming samples
 * @param [in]  length           The number of incoming samples in the buffer
 *
 */
IFX_DLL_PUBLIC
void ifx_di_add_input_samples(ifx_DeInterleaver_t* handle, const ifx_Float_t* data, size_t length);

/**
 * @brief Retrieve how many samples there are in a frame
 *
 * This function returns the number of samples per frame for the frame definition which was previously set
 * using ifx_di_set_frame_definition()
 *
 * @param [in]  handle           Handle for the existing de-interleaver instance
 * @return                       The number of samples per frame
 *
 */
IFX_DLL_PUBLIC
size_t ifx_di_get_samples_per_frame(ifx_DeInterleaver_t* handle);

/**
 * @brief Check if sufficient data has been received for a complete frame
 *
 * This function returns if there is a sufficient numbers of samples in the buffer to de-interleave
 * a complete input frame.
 *
 * @param [in]  handle           Handle for the existing de-interleaver instance
 * @return                       True if there's at least one full frame in the input buffer.
 *
 */
IFX_DLL_PUBLIC
bool ifx_di_is_frame_complete(ifx_DeInterleaver_t* handle);


/**
 * @brief Retrieve the de-interleaved frame.
 *
 * This function retrieves a deinterleaved frame.
 *
 * The de-interleaved output data is structured as a flat array with the data
 * arranged in the following order (list nesting also indicates data nesting):
 * - samples for all upchirps structured as below
 *   - first active antenna of each active shape in this upchirp
 *     (i.e. if the first active antenna in shape 1 is antenna 1 and in
 *     shape 2 is antenna 3, the data in this block for shape 1 would be
 *     for antenna 1 and the data for shape 2 would be for antenna 3)
 *     - shape 1: the data for the first shape.
 *       - shape set repeat: The inner data is repeated according to the
 *                           to the global non-shape-specific shape_set_repeat
 *                           counter.
 *         - shape repeat: The content inside is repeated according
 *                         to the value of shape_repeat for this shape-
 *          - chirp: All the samples for the current upchirp in shape 1 for
 *                   the current shape_repeat counter, current shape_set_counter,
 *                   current active antenna
 *     - shape 2: similar to shape 1
 *     - shape 3: similar to shape 1
 *     - shape 4: similar to shape 1
 *   - second active antenna for each active shape in this upchirp
 *     (data is similarly structured to the one first active antenna)
 *   - third, fourth... till the last active antenna in this upchirp
 * - samples for all downchirps
 *   - contents storage order identical to the way it is stored for upchirps
 *
 * One side effect of this format is that for MIMO using 2 shapes and
 * shape_set_repeat as a chirp counter with shape_repeat fixed to 1
 * and the same rx antenna configuration in the first and second shape the output
 * will look the same as for a non-MIMO configuration using shape_repeat for
 * the chirp counter.
 *
 * @param [in]  handle           Handle for the existing de-interleaver instance
 * @param [in]  data             Buffer for storing the received frame.
 * @param [in]  length           The amount of space in the output buffer for receiving the decoded frame
 *
 * @note If the value of the length parameter is smaller than ifx_di_get_samples_per_frame() only
 *       the initial part of the frame will be returned. The rest of the frame will be lost.
 *
 */
IFX_DLL_PUBLIC
void ifx_di_get_frame(ifx_DeInterleaver_t* handle, ifx_Float_t* data, size_t length);


#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* IFX_RADAR_DEINTERLEAVER_H */
