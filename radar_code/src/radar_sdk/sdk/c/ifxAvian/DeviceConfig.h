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
 * @file DeviceConfig.h
 *
 * \brief \copybrief gr_deviceconfig
 *
 * For details refer to \ref gr_deviceconfig
 */

#ifndef IFX_RADAR_DEVICE_CONFIG_H
#define IFX_RADAR_DEVICE_CONFIG_H

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include "ifxBase/Types.h"
#include "ifxFmcw/avian/DeviceFmcwAvianConfig.h"
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

#define IF_GAIN_DB_LOWER ((uint32_t)18)
#define IF_GAIN_DB_UPPER ((uint32_t)60)

#define TX_POWER_LEVEL_LOWER ((uint32_t)0)
#define TX_POWER_LEVEL_UPPER ((uint32_t)31)

/*
==============================================================================
   3. TYPES
==============================================================================
*/

/**
 * @brief Defines the structure for acquisition of time domain data related settings.
 *
 * When a connection to sensor device is established, the device is configured according to the
 * parameters of this struct.
 *
 * The power modes (shape end power mode and frame end power mode) are
 * configured to minimize the power consumption. Depending on how much time is
 * left at the end of a shape and at the end of the frame, the lowest power
 * modes possible are chosen.
 */
typedef struct
{
    /** @brief ADC sampling rate (in Hz)
     *
     * Sampling rate of the ADC used to acquire the samples during a chirp. The
     * duration of a single chirp depends on the number of samples and the
     * sampling rate.
     */
    uint32_t sample_rate_Hz;

    /** @brief Bitmask of activated RX antennas
     *
     * Bitmask where each bit represents one RX antenna of the radar device. If
     * a bit is set the according RX antenna is enabled during the chirps and
     * the signal received through that antenna is captured. The least
     * significant bit corresponds to antenna RX1.
     */
    uint32_t rx_mask;

    /** @brief Bitmask of activated TX antennas
     *
     * Bitmask where each bit represents one TX antenna. The least significant
     * bit corresponds to antenna TX1. It is also possible to disable all TX
     * antennas by setting tx_mask to 0.
     */
    uint32_t tx_mask;

    /** @brief TX power level
     *
     * This value controls the transmitted power (allowed values in the range
     * [0,31]). Higher values correspond to higher TX powers.
     */
    uint32_t tx_power_level;

    /** @brief IF Gain (in dB)
     *
     * Amplification factor that is applied to the IF signal coming from the RF
     * mixer before it is fed into the ADC (allowed values in the range
     * [18,60]).
     *
     * Internally, the values for HP gain (possible values: 18dB, 30dB) and
     * VGA gain (possible values: 0dB, 5dB, 10dB, ..., 30dB) are computed from
     * if_gain_dB. HP gain is chosen as high and VGA gain is chosen as low as
     * possible to reduce noise. HP gain is 18dB if_gain_dB is less than 30,
     * otherwise HP gain is 30dB. VGA gain is chosen such that the sum of
     * HP gain and VGA gain is as close to if_gain_dB as possible.
     *
     * Note that it is not possible to set all values of if_gain_dB exactly.
     * The configured value and actually set value of if_gain_dB might differ
     * by up to 2dB. The value actually set for if_gain_dB can be read using
     * \ref ifx_avian_get_config.
     */
    uint32_t if_gain_dB;

    /** @brief Start frequency (in Hz)
     *
     * Start frequency of FMCW chirp. See also \ref sct_radarsdk_introduction_parametersexplained.
     */
    uint64_t start_frequency_Hz;

    /** @brief End frequency (in Hz)
     *
     * End frequency of the FMCW chirp. See also \ref sct_radarsdk_introduction_parametersexplained.
     */
    uint64_t end_frequency_Hz;

    /** @brief Number of samples per chirp
     *
     * This is the number of samples acquired during each chirp of a frame. The
     * duration of a single chirp depends on the number of samples and the
     * sampling rate.
     */
    uint32_t num_samples_per_chirp;

    /** @brief Number of chirps per frame
     *
     * This is the number of chirps in a frame.
     */
    uint32_t num_chirps_per_frame;

    /** @brief Chirp repetition time (in seconds)
     *
     * This is the time period that elapses between the beginnings of two
     * consecutive chirps in a frame.
     *
     * The chirp repetition time is also commonly referred to as pulse
     * repetition time or chirp-to-chirp-time)
     */
    float chirp_repetition_time_s;

    /** @brief Frame repetition time (in seconds)
     *
     * This is the time period that elapses between the beginnings of two
     * consecutive frames. The reciprocal of the frame repetition time is the
     * frame rate.
     *
     * The frame repetition time is also commonly referred to as frame time or
     * frame period.
     */
    float frame_repetition_time_s;

    /** @brief Cutoff frequency of the high pass filter (in Hz)
     *
     * The high pass filter is used in order to remove the DC-offset at the
     * output of the RX mixer and also suppress the reflected signal from close
     * in unwanted targets (radome, e.g.).
     *
     * @note Different sensors support different cutoff frequencies. The frequency
     * provided by the user will be rounded to the closest supported cutoff frequency.
     * You can read the cutoff frequency that was actually set using \ref ifx_avian_get_config.
     */
    uint32_t hp_cutoff_Hz;

    /** @brief Cutoff frequency of the anti-aliasing filter (in Hz)
     */
    uint32_t aaf_cutoff_Hz;

    /** @brief Mode of MIMO
     *
     * Mode of MIMO (multiple-input and multiple-output).
     *
     * @note If \ref mimo_mode is \ref IFX_MIMO_TDM the value of tx_mask is ignored.
     */
    enum ifx_Avian_MIMO_Mode mimo_mode;
} ifx_Avian_Config_t;

/*
==============================================================================
   4. FUNCTION PROTOTYPES
==============================================================================
*/

/** @addtogroup gr_cat_Avian
 * @{
 */

/** @defgroup gr_deviceconfig Device Configuration
 * @brief API for Radar device configuration
 * @{
 */

/**
 * @brief Calculates the center frequency in use from given device configuration
 *
 * @param [in]     config    Device configuration structure
 * @return Center frequency (in Hz) used by device configuration
 */
IFX_DLL_PUBLIC
float ifx_devconf_get_center_frequency(const ifx_Avian_Config_t* config);

/**
 * @brief Calculates the bandwidth in use from given device configuration
 *
 * @param [in]     config    Device configuration structure
 * @return Bandwidth (in Hz) used by device configuration
 */
IFX_DLL_PUBLIC
float ifx_devconf_get_bandwidth(const ifx_Avian_Config_t* config);

/**
 * @brief Calculates the Chirp time in use from given device configuration
 *
 * The chirp time is the time where sampling is performed to get sampler per chirp. Initial chirp delays
 * like PLL setup delay and ADC sampling delay are not used for chirp time calculations.
 *
 * @param [in]     config    Device configuration structure
 * @return Chirp time (in seconds) used by device configuration
 */
IFX_DLL_PUBLIC
float ifx_devconf_get_chirp_time(const ifx_Avian_Config_t* config);

/**
 * @brief Counts the number of receive antennas based on the given device configuration
 *
 * If MIMO is active this function will return virtual receive antenna count.
 *
 * @param [in]     config    Device configuration structure
 * @return Number of receive antennas activated by device configuration
 */
IFX_DLL_PUBLIC
uint8_t ifx_devconf_count_rx_antennas(const ifx_Avian_Config_t* config);

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* IFX_RADAR_DEVICE_CONFIG_H */
