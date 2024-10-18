/* ===========================================================================
** Copyright (C) 2022 Infineon Technologies AG
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
 * @file DeviceFmcwTypes.h
 *
 * @defgroup gr_cat_Fmcw      FMCW Device Control (ifxFmcw)
 *
 * @brief @copybrief gr_devicefmcw
 *
 * For details refer to @ref gr_devicefmcw
 */

#ifndef IFX_DEVICE_FMCW_TYPES_H
#define IFX_DEVICE_FMCW_TYPES_H

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include "ifxBase/Error.h"
#include "ifxBase/Mda.h"
#include "ifxBase/Types.h"
#include "ifxRadarDeviceCommon/RadarDeviceCommon.h"


#ifdef __cplusplus
extern "C"
{
#endif


/*
==============================================================================
   2. DEFINITIONS
==============================================================================
*/

/*
==============================================================================
   3. TYPES
==============================================================================
*/

// ---------------------------------------------------------------------------- ifx_Fmcw_Raw_Frame_t
/**
 * @brief This structure holds a complete frame of raw interleaved radar data,
 * consisting of num_samples samples.
 */

typedef struct ifx_Fmcw_Raw_Frame_s
{
    uint32_t num_samples;
    uint16_t* samples;
} ifx_Fmcw_Raw_Frame_t;

// ---------------------------------------------------------------------------- ifx_Fmcw_Frame_t
/**
 * @brief This structure holds a complete frame of radar data.
 *
 * For each chirp in the acquisition sequence the frame structure contains a
 * cube of real data. The number of dimensions of each cube depends on the
 * number of nested loops in the acquisition sequence that surround the chirp
 * the cube belongs to.
 * The first (outermost) dimensions corresponds to the different RX antennas.
 * The second dimensions corresponds to the outermost loop in the sequence
 * (not considering the frame loop), or with other words the surrounding loop
 * with longest repetition time.
 * The following dimensions up to the next to last dimension correspond to the
 * repetitions of the nested loops, with descending repetition times. So the
 * next to last dimension corresponds to the innermost loop with shortest
 * repetition time. Finally the last (innermost) dimension corresponds to the
 * samples of a chirps.
 */

typedef struct ifx_Fmcw_Frame_s
{
    uint32_t num_cubes;
    ifx_Mda_R_t** cubes;
} ifx_Fmcw_Frame_t;

// ---------------------------------------------------------------------------- ifx_Fmcw_Element_Type
/**
 * @brief Lists all building blocks a frame sequence can be built from.
 */
typedef enum
{
    IFX_SEQ_LOOP,
    IFX_SEQ_CHIRP,
    IFX_SEQ_DELAY,
} ifx_Fmcw_Element_Type;

// Forward declaration needed by ifx_Fmcw_Sequence_Loop below.
typedef struct ifx_Fmcw_Sequence_Element ifx_Fmcw_Sequence_Element_t;

// ---------------------------------------------------------------------------- ifx_Fmcw_Sequence_Loop_t
/**
 * @brief A loop in an acquisition sequence
 *
 * A loop repeats a sub-sequence at a fixed rate for a given number of times.
 */
typedef struct ifx_Fmcw_Sequence_Loop
{
    /**
     * @brief The first element of the looped sub-sequence.
     *
     * This must not be NULL, because an empty loop does not make sense.
     */
    ifx_Fmcw_Sequence_Element_t* sub_sequence;

    /**
     * @brief The number of loop repetitions.
     *
     * The loop is executed this number of times. A value of 1 means the
     * sub-sequence is executed only once (so actually not repeated). This
     * value may be 0 to indicate an infinite number of executions. The latter
     * makes only sense if the loop is the last element in the sequence.
     */
    uint32_t num_repetitions;

    /**
     * @brief The reciprocal repetition rate.
     *
     * This is the time period in seconds that elapses between the beginnings
     * of two consecutive repetitions of the looped sub-sequence.
     * A value of 0 means "as fast as possible". In that case, the duration of
     * the looped sub-sequence defines the repetition rate.
     */
    float repetition_time_s;

} ifx_Fmcw_Sequence_Loop_t;

// ---------------------------------------------------------------------------- ifx_Fmcw_Sequence_Chirp_t
/**
 * @brief A chirp in an acquisition sequence
 *
 * This structure represents a chirp in the acquisition sequence and holds all
 * chirp parameters.
 */
typedef struct ifx_Fmcw_Sequence_Chirp
{
    /**
     * @brief Start frequency (in Hz)
     *
     * Start frequency of FMCW chirp. See also
     * @ref sct_radarsdk_introduction_parametersexplained.
     */
    double start_frequency_Hz;

    /**
     * @brief End frequency (in Hz)
     *
     * End frequency of the FMCW chirp. See also
     * @ref sct_radarsdk_introduction_parametersexplained.
     */
    double end_frequency_Hz;

    /**
     * @brief ADC sampling rate (in Hz)
     *
     * Sampling rate of the ADC used to acquire the samples during a chirp. The
     * duration of a single chirp depends on the number of samples and the
     * sampling rate.
     */
    float sample_rate_Hz;

    /**
     * @brief Number of samples per chirp
     *
     * This is the number of samples acquired during each chirp of a frame. The
     * duration of a single chirp depends on the number of samples and the
     * sampling rate.
     */
    uint32_t num_samples;

    /**
     * @brief Bit mask of activated RX antennas
     *
     * Bit mask where each bit represents one RX antenna of the radar device.
     * If a bit is set the according RX antenna is enabled during the chirps
     * and the signal received through that antenna is captured. The least
     * significant bit corresponds to antenna RX1.
     */
    uint32_t rx_mask;

    /**
     * @brief Bit mask of activated TX antennas
     *
     * Bit mask where each bit represents one TX antenna. The least significant
     * bit corresponds to antenna TX1. It is also possible to disable all TX
     * antennas by setting tx_mask to 0.
     */
    uint32_t tx_mask;

    /**
     * @brief TX power level
     *
     * This value controls the transmitted power (allowed values in the range
     * [0,31]). Higher values correspond to higher TX powers.
     */
    uint32_t tx_power_level;

    /**
     * @brief Cutoff frequency of the low pass (anti-aliasing) filter (in Hz)
     *
     *
     * @note It is not possible to set all values exactly. Supported values can
     *       be queried using @ref ifx_fmcw_get_sensor_information
     */
    int32_t lp_cutoff_Hz;

    /**
     * @brief Cutoff frequency of the high pass filter (in Hz)
     *
     * The high pass filter is used in order to remove the DC-offset at the
     * output of the RX mixer and also suppress the reflected signal from close
     * in unwanted targets (radome, e.g.).
     *
     *
     * @note It is not possible to set all values exactly. Supported values can
     *       be queried using @ref ifx_fmcw_get_sensor_information
     */
    int32_t hp_cutoff_Hz;

    /**
     * @brief IF Gain (in dB)
     *
     * Amplification factor that is applied to the IF signal coming from the RF
     * mixer before it is fed into the ADC.
     *
     * Internally, if there are multiple stages, separate values are computed from
     * if_gain_dB. Usually the first-stage gain is chosen as high as possible
     * possible to reduce noise.
     *
     * @note It is not possible to set all values of if_gain_dB exactly.
     *       Available values can be queried using
     *       @ref ifx_fmcw_get_sensor_information. The configured value will be
     *       rounded to the nearest available value. The value actually set for
     *       if_gain_dB can be read using @ref ifx_fmcw_get_acquisition_sequence.
     */
    int8_t if_gain_dB;

} ifx_Fmcw_Sequence_Chirp_t;

// ---------------------------------------------------------------------------- ifx_Fmcw_Sequence_Delay_t
/**
 * @brief A delay in an acquisition sequence
 *
 * This structure represents a delay in the acquisition sequence. It must not
 * occur at the beginning or the end of a sequence or sub-sequence. It can only
 * be used to add a delay between loop elements.
 */
typedef struct ifx_Fmcw_Sequence_Delay
{
    /**
     * @brief The delay period.
     *
     * This is the time period in seconds inserted between the previous and
     * the following loop element.
     */
    float time_s;

} ifx_Fmcw_Sequence_Delay_t;

// ---------------------------------------------------------------------------- ifx_Fmcw_Sequence_Element_t
/**
 * @brief An element of an acquisition sequence.
 *
 * This structure is the base of all acquisition sequence elements. An
 * application must never use this type directly, but build acquisition
 * sequences from  @ref ifx_Fmcw_Sequence_Loop_t,
 * @ref ifx_Fmcw_Sequence_Chirp_t and @ref ifx_Fmcw_Sequence_Delay_t.
 *
 * Sequence elements are concatenated to an acquisition sequence as a single
 * linked list.
 */
struct ifx_Fmcw_Sequence_Element
{
    /**
     * @brief The type of this sequence element.
     */
    ifx_Fmcw_Element_Type type;

    /**
     * @brief The following sequence element.
     *
     * For the last element in the sequence, this is NULL.
     */
    ifx_Fmcw_Sequence_Element_t* next_element;

    union
    {
        ifx_Fmcw_Sequence_Loop_t loop;
        ifx_Fmcw_Sequence_Chirp_t chirp;
        ifx_Fmcw_Sequence_Delay_t delay;
    };
};

// ---------------------------------------------------------------------------- ifx_Fmcw_Simple_Sequence_Config_t
/**
 * @brief An acquisition sequence with either a single chirp or a sequence of
 *        similar chirps with TDM MIMO configuration.
 *
 * This is a convenience type that represents a frame, that consists of a
 * single chirp. It contains all parameters of `ifx_Avian_Config_t`.
 *
 * If the flag tdm_mimo is set, the single chirp is replicated to one chirp per
 * TX antenna. All these chirps have same parameters except the TX mask.
 */
typedef struct ifx_Fmcw_Simple_Sequence_Config
{
    float frame_repetition_time_s;
    float chirp_repetition_time_s;
    uint32_t num_chirps;
    bool tdm_mimo;

    /**
     * @brief The chirp parameters.
     */
    ifx_Fmcw_Sequence_Chirp_t chirp;

} ifx_Fmcw_Simple_Sequence_Config_t;

/*
==============================================================================
   4. FUNCTION PROTOTYPES
==============================================================================
*/

/**
 * @addtogroup gr_cat_Fmcw
 * @{
 */

/**
 * @brief API for Radar device configuration
 * @{
 */

/**
 * @brief Initializes a single shape acquisition sequence.
 *
 * This function initializes a single shape configuration structure, setting
 * each field to a defined value and linking the loops and chirp members
 * properly.
 *
 * @param[out] config  The single shape configuration structure to initialize.
 */
IFX_DLL_PUBLIC
ifx_Fmcw_Sequence_Element_t* ifx_fmcw_create_simple_sequence(ifx_Fmcw_Simple_Sequence_Config_t* config);

/**
 * @brief destroy sequence.
 *
 * This function destroys a sequence structure.
 *
 * @param[out] sequence  The sequence structure to destroy.
 */
IFX_DLL_PUBLIC
void ifx_fmcw_destroy_sequence(ifx_Fmcw_Sequence_Element_t* sequence);

/**
 * @brief create a sequence element.
 *
 * This function allocate memory for a sequence element and sets the ifx_Fmcw_Element_Type.
 *
 * @param[out] type  The type of element to be created.
 */
IFX_DLL_PUBLIC
ifx_Fmcw_Sequence_Element_t* ifx_fmcw_create_sequence_element(ifx_Fmcw_Element_Type type);

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* IFX_DEVICE_FMCW_TYPES_H */
