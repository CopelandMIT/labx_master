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
 * @file DeviceFmcwAvianConfig.h
 *
 * \brief \copybrief gr_deviceconfig
 *
 * For details refer to \ref gr_deviceconfig
 */

#ifndef IFX_DEVICE_FMCW_AVIAN_CONFIG_H
#define IFX_DEVICE_FMCW_AVIAN_CONFIG_H

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/


#ifdef __cplusplus
extern "C"
{
#endif


/*
==============================================================================
   2. DEFINITIONS
==============================================================================
*/


#define IFX_AVIAN_SAMPLE_RATE_HZ_LOWER 80e3f
#define IFX_AVIAN_SAMPLE_RATE_HZ_UPPER 2.8e6f


/*
==============================================================================
   3. TYPES
==============================================================================
*/

/**
 * @brief This enumeration type lists the available MIMO modes of an Avian sensor.
 */
enum ifx_Avian_MIMO_Mode
{
    IFX_MIMO_OFF = 0,  //!< MIMO is deactivated
    IFX_MIMO_TDM = 1   //!< time-domain multiplexing MIMO
};

// ---------------------------------------------------------------------------- Power_Mode

/**
 * @brief This enumeration type lists the available power modes of an Avian sensor, that can be configured at the
 * end of a shape or frame. These power values are used during shape end delay and frame end delay.
 */
typedef enum
{
    IFX_POWER_MODE_ACTIVE,              //!< No power saving.
    IFX_POWER_MODE_IDLE,                //!< Go to Idle Mode after last chirp.
    IFX_POWER_MODE_DEEP_SLEEP,          //!< Go to Idle Mode after last chirp.
    IFX_POWER_MODE_DEEP_SLEEP_CONTINUE  //!< Go to Deep Sleep Mode after last chirp, but keep system clock enabled.
} ifx_Avian_Power_Mode_t;

// ---------------------------------------------------------------------------- Aaf_Cutoff

/*
==============================================================================
   4. FUNCTION PROTOTYPES
==============================================================================
*/

/** @addtogroup gr_deviceconfig
 * @{
 */

/**
 * @}
 */

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* IFX_DEVICE_FMCW_AVIAN_CONFIG_H */
