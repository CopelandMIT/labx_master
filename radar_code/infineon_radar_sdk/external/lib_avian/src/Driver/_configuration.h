/**
 * @file _configuration.h
 *
 * This file is part of the BGT60TRxx driver.
 *
 * This file contains the declaration of constants needed for the driver
 * implementation. The constants are declared here to avoid "magic numbers"
 * inside the driver code.
 */
/* ===========================================================================
** Copyright (C) 2016-2023 Infineon Technologies AG
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


#ifndef SRC_IFXRADAR_BGT60TRXX_SRC__CONFIGURATION_H_
#define SRC_IFXRADAR_BGT60TRXX_SRC__CONFIGURATION_H_

/*
==============================================================================
   2. DEFINITIONS
==============================================================================
*/

/* Hardware specific constants */
#define BGT60TRXX_MAX_TX_POWER 31           /**< This is the power value \
                                                  supported by BGT60TRxx. */
#define BGT60TRXX_MAX_NUM_SAMPLES 4095      /**< BGT60TRxx support up to 4095 \
                                                  samples per chirp. */
#define BGT60TRXX_MAX_NUM_FRAMES 4095       /**< Maximum number of frames before \
                                                  BGT60TRxx stops automatically. \
                                                  */
#define BGT60TRXX_MAX_FRAME_LENGTH 4096     /**< Maximum number of shape groups \
                                                  in a frame. */
#define BGT60TRXX_MAX_ADC_RATE_DIVIDER 1023 /**< The minimum ADC sample rate \
                                                  supported by BGT60TRxx is  \
                                                  limited by this value. */
#define BGT60TRXXC_MAX_BB_RESET_TIMER 127   /**< This is the maximum register   \
                                                  value for the base band reset \
                                                  timer period. */
#define BGT60TRXXD_MAX_BB_RESET_TIMER 63    /**< This is the maximum register   \
                                                  value for the base band reset \
                                                  timer period. */
#define BGT60TRXX_MADC_STARTUP_CYCLES 660   /**< After MADC is enabled this        \
                                                  number of clock cycles is needed \
                                                  before MADC can be used */

#define BGT60TRXX_MAX_PRE_CHIRP_DELAY_100ns  511
#define BGT60TRXX_MAX_POST_CHIRP_DELAY_100ns 511
#define BGT60TRXX_MAX_PA_DELAY_100ns         511
#define BGT60TRXXC_MAX_ADC_DELAY_100ns       31
#define BGT60TRXXD_MAX_ADC_DELAY_100ns       63
#define BGT60TRXX_MAX_WAKEUP_COUNTER         255
#define BGT60TRXX_MAX_WAKEUP_COUNTER_SHIFT   15
#define BGT60TRXXC_MAX_PLL_INIT0_COUNTER     255
#define BGT60TRXXD_MAX_PLL_INIT0_COUNTER     127
#define BGT60TRXX_MAX_PLL_INIT1_COUNTER      255
#define BGT60TRXX_MAX_PLL_INIT_COUNTER_SHIFT 3
#define BGT60TRXX_MAX_SHAPE_END_DELAY        255
#define BGT60TRXX_MAX_SHAPE_END_DELAY_SHIFT  30

#define BGT60TRXXD_MAX_BANDGAP_DELAY         7
#define BGT60TRXXD_MAX_MADC_DELAY            7
#define BGT60TRXXD_MAX_PLL_ENABLE_DELAY      63
#define BGT60TRXXD_MAX_PLL_DIVIDER_DELAY     7
#define BGT60TR11D_MAX_POWER_SENS_DELAY      50
#define BGT60TR11D_MADC_IN_SWITCH_TIME_100ps 10000

#define BGT60TRXXD_MIN_IDLE_INIT0_CYCLES 3200 /**< This is the minimum period \
                                                   between and of T_PLL_ISOPD \
                                                   and T_INIT0. The value was \
                                                   reported from chip         \
                                                   verification team. */
#define BGT60TRXXD_MIN_IDLE_INIT1_CYCLES 440  /**< This is the minimum period \
                                                   between and of T_DIVEN and \
                                                   T_INIT1. The value was     \
                                                   reported from chip         \
                                                   verification team. */

/**
 * This is the basic configuration for the analog PLL register PACR1, which
 * is used at a lot of places all over the driver code.
 *
 * PACR1 = 0xEF67FD was found in the lab
 */
#define PACR1_SETTINGS                       \
    BGT60TRxxC_SET(PACR1, ANAPON, 1)         \
        | BGT60TRxxC_SET(PACR1, VANAREG, 2)  \
        | BGT60TRxxC_SET(PACR1, DIGPON, 1)   \
        | BGT60TRxxC_SET(PACR1, VDIGREG, 3)  \
        | BGT60TRxxC_SET(PACR1, BGAPEN, 1)   \
        | BGT60TRxxC_SET(PACR1, U2IEN, 1)    \
        | BGT60TRxxC_SET(PACR1, VREFSEL, 3)  \
        | BGT60TRxxC_SET(PACR1, RFILTSEL, 1) \
        | BGT60TRxxC_SET(PACR1, RLFSEL, 0)   \
        | BGT60TRxxC_SET(PACR1, SPARE, 0)    \
        | BGT60TRxxC_SET(PACR1, LOCKSEL, 3)  \
        | BGT60TRxxC_SET(PACR1, LOCKFORC, 1) \
        | BGT60TRxxC_SET(PACR1, ICPSEL, 4)   \
        | BGT60TRxxC_SET(PACR1, BIASFORC, 0) \
        | BGT60TRxxC_SET(PACR1, CPEN, 1)     \
        | BGT60TRxxC_SET(PACR1, LFEN, 1)     \
        | BGT60TRxxC_SET(PACR1, OSCCLKEN, 1)

/**
 * This is the basic configuration for the analog PLL register PACR2.
 *
 * PACR2 = 0x080074 was found in the lab
 * The DIVSET value was taken out, because this may change due to external
 * clock frequencies different from 80MHz.
 */
#define PACR2_SETTINGS                      \
    BGT60TRxxC_SET(PACR2, DIVEN, 1)         \
        | BGT60TRxxC_SET(PACR2, FSTDNEN, 2) \
        | BGT60TRxxC_SET(PACR2, FSDNTMR, 5) \
        | BGT60TRxxC_SET(PACR2, TRIVREG, 0) \
        | BGT60TRxxC_SET(PACR2, DTSEL, 2)   \
        | BGT60TRxxC_SET(PACR2, PLL_SPARE, 0)

/* --- Close open blocks -------------------------------------------------- */

#endif /* SRC_IFXRADAR_BGT60TRXX_SRC__CONFIGURATION_H_ */

/* --- End of File -------------------------------------------------------- */
