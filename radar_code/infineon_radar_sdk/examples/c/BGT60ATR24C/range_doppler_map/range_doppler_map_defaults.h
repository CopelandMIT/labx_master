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
 * @file rdm_defaults.h
 *
 * @brief This file defines the default configuration values for Range Doppler Map application.
 *
 */

#ifndef RDM_DEFAULTS_H
#define RDM_DEFAULTS_H

// Device configuration
// A Description for these parameters can be found in ifx_Avian_Config_t
// in DeviceConfig.h

#define IFX_ADC_SAMPLERATE_HZ (1000000U) /**< Sample rate of the ADC. \
                                         Valid range is [100k - 2M]Hz.*/
#define IFX_RX_MASK (1)                  /**< Receive antenna mask defines which antennas to activate. \
                                         Multiple antennas can be activated by masking.*/
#define IFX_TX_MASK (1)                  /**< tx_mode decides which TX antenna to use in case multiple \
                                         TX antennas are supported by the device  e.g. 1 => Tx1; 2 => Tx2 */
#define IFX_BGT_TX_POWER (31U)           /**< TX Power level to be used on the radar transceiver. \
                                         Valid range is [0 - 31].*/
#define IFX_IF_GAIN_DB (33)              /**< This is the amplification factor that is applied to the IF signal before sampling. \
                                         Valid range is [18 - 60]dB.*/
#define IFX_RANGE_RESOLUTION_M (0.150f)  /**< Range resolution translates into bandwidth(BW) = c / (2 * Rng_res). \
                                         This bandwidth impacts the range captured by one range FFT bin.          \
                                         Valid range is [0.025 - 1.00] meter.*/
#define IFX_MAX_RANGE_M (9.59f)          /**< Maximum range recommended to be at least 8 times the range resolution. \
                                         Valid range is [0.20 - 20] meter.*/
#define IFX_SPEED_RESOLUTION_M_S (0.08f) /**< This parameter impacts the precision in speed. \
                                         Valid range is [0.025 - 0.833]m/s.*/
#define IFX_MAX_SPEED_M_S (2.45f)        /**< This impacts chirp to chirp time parameter of chip. \
                                         Valid range is [0.25 - 25]m/s.*/
#define IFX_FRAME_RATE_HZ (10)           /**< This is the frame acquisition rate for raw data. \
                                         Valid range is [0.016 - 100]Hz.*/

#define IFX_MAXIMUM_SPEED_MPS (2.5f)     // maximum speed in m/s that can be detected

#define IFX_ALPHA_MTI_FILTER (0.3f)      // alpha parameter for MTI filter

#define IFX_SPECT_THRESHOLD (1e-6f)

#endif /* RDM_DEFAULTS_H */
