/* ===========================================================================
** Copyright (C) 2022 Infineon Technologies AG. All rights reserved.
** ===========================================================================
**
** THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
** KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
** PARTICULAR PURPOSE.
**
** The full license agreements can be found in the file license.txt.
*/

/**
 * @file AdvancedMotionSensing.h
 *
 * \brief Motion sensing algorithm including the peak-to-peak algorithm,
 *        and interference mitigation option when the latter is enabled.
 *
 * \defgroup gr_cat_MotionSensing              Motion Sensing (ifxAdvancedMotionSensing)
 */

/** @addtogroup gr_cat_MotionSensing
 * @{
 */

#ifndef IFX_MOTION_SENSING_H
#define IFX_MOTION_SENSING_H

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/
#include "ifxBase/Types.h"
#include "ifxBase/Vector.h"


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

/**
 * @brief Defines the structure for the advanced motion sensing algorithm input settings.
 *
 */
typedef struct
{
    uint16_t num_samples;                       /**< Number of samples per frame on which the advanced motion sensing algorithm is applied.
                                                        Valid range is [16 - 256]. */
    uint16_t advanced_motion_sensing_threshold; /**< Input threshold to the advanced motion sensing algorithm.
                                            Valid range is [10 - 100]. */
    bool enable_interference_mitigation;        /**< Flag indicating when true that the interference mitigation should be enabled. */
} ifx_Advanced_Motion_Sensing_Config_t;

/**
 * @brief Defines the structure for the advanced motion sensing algorithm input settings.
 *
 */
typedef struct
{
    struct
    {
        uint16_t min;
        uint16_t max;
    } num_samples;
    struct
    {
        uint16_t min;
        uint16_t max;
    } advanced_motion_sensing_threshold;
} ifx_Advanced_Motion_Sensing_Config_Limits_t;

/**
 * @brief Defines the target detection output of advanced motion sensing algorithm enum.
 */
typedef enum
{
    NO_TARGET_DETECTED = 0,
    TARGET_MOTION_DETECTED = 1,
    POTENTIAL_TARGET_DETECTED = 2,
} ifx_Target_Detection_t;

/**
 * @brief Defines the algorithm output structure.
 *
 */
typedef struct
{
    ifx_Target_Detection_t target_detection; /**< Target detection output of advanced motion sensing algorithm \ref ifx_Target_Detection_t. */
    uint16_t peak_to_peak_amplitude;         /**< Computed amplitude by the peak-to-peak algorithm. */
} ifx_Advanced_Motion_Sensing_Output_t;

/**
 * @brief Structure for an instance of the advanced motion sensing algorithm internal parameters object.
 *
 */
typedef struct ifx_Advanced_Motion_Sensing_s ifx_Advanced_Motion_Sensing_t;

/*
==============================================================================
   4. FUNCTION PROTOTYPES
==============================================================================
*/

/**
 * @brief Creates an instance of the advanced motion sensing algorithm internal parameters object.
 *
 * @param [in]   config     Motion sensing algorithm input configuration defined by \ref ifx_Advanced_Motion_Sensing_Config_t.
 * @return Newly created advanced motion sensing instance or NULL in case of failure.
 */
IFX_DLL_PUBLIC
ifx_Advanced_Motion_Sensing_t* ifx_advanced_motion_sensing_create(const ifx_Advanced_Motion_Sensing_Config_t* config);

/**
 * @brief Applies the advanced motion sensing algorithm on the given frame data.
 *
 * @param [in]  instance        An instance to the created advanced motion sensing algorithm
 *                              internal parameters object (\ref ifx_Advanced_Motion_Sensing_t).
 * @param [in]  frame           Input raw data as a vector (\ref ifx_Vector_C_t).
 * @param [out] algo_output     Algorithm final output including the target detection, and the computed
 *                              peak-to-peak amplitude \ref ifx_Advanced_Motion_Sensing_Output_t.
 */
IFX_DLL_PUBLIC
void ifx_advanced_motion_sensing_run(ifx_Advanced_Motion_Sensing_t* instance, const ifx_Vector_C_t* frame, ifx_Advanced_Motion_Sensing_Output_t* algo_output);

/**
 * @brief Destroys the advanced motion sensing algorithm instance to clear memory.
 *
 * @param [in] instance     An instance to the created advanced motion sensing algorithm
 *                          internal parameters object (\ref ifx_Advanced_Motion_Sensing_t).
 */
IFX_DLL_PUBLIC
void ifx_advanced_motion_sensing_destroy(ifx_Advanced_Motion_Sensing_t* instance);

/**
 * @brief Returns the limiting values for the advanced motion sensing algorithm input configuration.
 *
 * @param [out] limits      The limits instance to be written.
 */
IFX_DLL_PUBLIC
void ifx_advanced_motion_sensing_get_config_limits(ifx_Advanced_Motion_Sensing_Config_Limits_t* limits);

/**
 * @}
 */

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* IFX_MOTION_SENSING_H */
