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
 * @file DeviceCwTypes.h
 *
 * @defgroup gr_cat_Cw   Continuous Wave Control (ifxCw)
 *
 * @brief @copybrief gr_devicecw
 *
 * For details refer to @ref gr_devicecw
 */

#ifndef IFX_DEVICE_CW_TYPES_H
#define IFX_DEVICE_CW_TYPES_H

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include "ifxBase/Types.h"
#include <ifxFmcw/DeviceFmcwTypes.h>


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

enum ifx_Cw_Test_Signal_Generator_Mode
{
    IFX_TEST_SIGNAL_MODE_OFF,                /*!< The generator is not used.*/
    IFX_TEST_SIGNAL_MODE_BASEBAND_TEST,      /*!< The test signal is routed to the input of the
                                                  baseband filters instead of the RX mixer output signals. */
    IFX_TEST_SIGNAL_MODE_TOGGLE_TX_ENABLE,   /*!< TX1 is toggled on and off controlled by the
                                                  generated test signal. */
    IFX_TEST_SIGNAL_MODE_TOGGLE_DAC_VALUE,   /*!< The power DAC value is toggled between minimum
                                                  value and the value set trough
                                                  \ref ifx_cw_set_baseband_config controlled by the
                                                  generated test signal. */
    IFX_TEST_SIGNAL_MODE_TOGGLE_RX_SELF_TEST /*!< The test signal is routed directly into the RF
                                                 mixer of RX antenna 1. */
};

/**
 * @brief Defines the structure for the Baseband configuration.
 */
/**
 * The following fields are ignored:
 *   float end_frequency_Hz;
 *   uint32_t num_samples_per_chirp;
 */
typedef ifx_Fmcw_Sequence_Chirp_t ifx_Cw_Baseband_Config_t;

/**
 * @brief Defines the structure for the ADC configuration.
 */
typedef struct
{
    /**
     * The sample time is the time that the sample-and-hold-circuitry of the Avian
     * device's Analog-Digital-Converter (ADC) takes to sample the voltage at it's
     * input.
     *
     * \note The specified timings refer to an external reference clock frequency
     *       of 80MHz. If the clock frequency differs from that, the sample time
     *       periods are scaled accordingly.
     */
    uint32_t sample_and_hold_time_ns;

    /**
     * The Avian device's internal Analog-Digital-Converter (ADC) is capable to
     * repeat the full sample-hold-convert cycle multiple times and return the
     * average of all cycles.
     */
    uint16_t oversampling_factor;

    /**
     * The Avian device's internal Analog-Digital-Converter (ADC) has 11 bit
     * resolution and a 12 bit result register. A single conversion always produces
     * result values with unset LSB. The resolution can be increased by performing
     * additional tracking conversion. The result will be the average of all
     * conversions.
     */
    uint16_t additional_subconversions;  // tracking
    bool double_msb_time;
} ifx_Cw_Adc_Config_t;

/**
 * @brief Defines the structure for the Test Signal configuration.
 */
typedef struct
{
    enum ifx_Cw_Test_Signal_Generator_Mode mode;
    float frequency_Hz;
} ifx_Cw_Test_Signal_Generator_Config_t;

/*
==============================================================================
   4. FUNCTION PROTOTYPES
==============================================================================
*/

/**
 * @{
 */

/**
 * @}
 */

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* IFX_DEVICE_CW_TYPES_H */
