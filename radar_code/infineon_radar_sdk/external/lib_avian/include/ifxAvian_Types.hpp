/**
 * \file ifxAvian_Types.hpp
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

#ifndef IFX_AVIAN_TYPES_H
#define IFX_AVIAN_TYPES_H

// ---------------------------------------------------------------------------- includes
#include <cstdint>

// ---------------------------------------------------------------------------- namespaces
namespace Infineon {
namespace Avian {

// ---------------------------------------------------------------------------- Device_Type
//! This enumeration type lists all Avian device types supported by libAvian.
enum class Device_Type
{
    BGT60TR13C,
    BGT60ATR24C,
    BGT60UTR13D,
    BGT60TR12E,
    BGT60UTR11AIP,
    BGT120UTR13E,
    BGT24LTR24,
    BGT120UTR24,
    BGT60ATR24E,
    BGT24LTR13E,
    Unknown
};

// ---------------------------------------------------------------------------- Shape_Type
//! This enumeration type lists the FMCW shape types.
enum class Shape_Type
{
    Saw_Up,    //!< A sawtooth shape with a single up chirp.
    Saw_Down,  //!< A sawtooth shape with a single down chirp.
    Tri_Up,    //!< A triangle shape with an up chirp followed by a down chirp.
    Tri_Down   //!< A triangle shape with a down chirp followed by an up chirp.
};

// ---------------------------------------------------------------------------- Tx_Mode
//! This enumeration type lists all available TX antenna configurations.
enum class Tx_Mode
{
    Tx1_Only,    //!< Use TX 1 for all chirps.
    Tx2_Only,    //!< Use TX 2 for all chirps.
    Alternating, /*!< Both TX antennas are used alternating. The first
                      chirp of a frame and all odd chirps use TX 1, while all
                      even chirps use TX 2. */
    Off          //!< No RF is transmitted during chirps.
};

// ---------------------------------------------------------------------------- Hp_Gain
/*!
 * This enumeration type lists the available gain settings of the Avian
 * device's integrated baseband high pass filters.
 */
enum class Hp_Gain
{
    _18dB,  //!< The gain of the high pass filter is +18dB.
    _30dB   //!< The gain of the high pass filter is +30dB.
};

// ---------------------------------------------------------------------------- Vga_Gain
/*!
 * This enumeration type lists the available gain settings of the Avian
 * device's integrated baseband amplifier (VGA).
 */
enum class Vga_Gain
{
    _0dB,   //!< The gain of the VGA is 0dB.
    _5dB,   //!< The gain of the VGA is +5dB.
    _10dB,  //!< The gain of the VGA is +10dB.
    _15dB,  //!< The gain of the VGA is +15dB.
    _20dB,  //!< The gain of the VGA is +20dB.
    _25dB,  //!< The gain of the VGA is +25dB.
    _30dB   //!< The gain of the VGA is +30dB.
};

// ---------------------------------------------------------------------------- Adc_Sample_Time
/*!
 * This enumeration type lists the available ADC sample time settings.
 *
 * The sample time is the time that the sample-and-hold-circuitry of the Avian
 * device's Analog-Digital-Converter (ADC) takes to sample the voltage at it's
 * input.
 *
 * \note The specified timings refer to an external reference clock frequency
 *       of 80MHz. If the clock frequency differs from that, the sample time
 *       periods are scaled accordingly.
 */
enum class Adc_Sample_Time
{
    _50ns,   //!< The voltage is sampled for 50ns.
    _100ns,  //!< The voltage is sampled for 100ns.
    _200ns,  //!< The voltage is sampled for 200ns.
    _400ns   //!< The voltage is sampled for 400ns.
};

// ---------------------------------------------------------------------------- Adc_Tracking
/*!
 * This enumeration type lists the available ADC tracking modes.
 *
 * The Avian device's internal Analog-Digital-Converter (ADC) has 11 bit
 * resolution and a 12 bit result register. A single conversion always produces
 * result values with unset LSB. The resolution can be increased by performing
 * additional tracking conversion. The result will be the average of all
 * conversions.
 */
enum class Adc_Tracking
{
    None,               //!< Each sampled voltage value is converted once.
    _1_Subconversion,   //!< Each sampled voltage value is converted two times.
    _3_Subconversions,  //!< Each sampled voltage value is converted four times.
    _7_Subconversions   //!< Each sampled voltage value is converted eight times.
};

// ---------------------------------------------------------------------------- Adc_Oversampling
/*!
 * This enumeration type lists the available ADC oversampling modes.
 *
 * The Avian device's internal Analog-Digital-Converter (ADC) is capable to
 * repeat the full sample-hold-convert cycle multiple times and return the
 * average of all cycles.
 */
enum class Adc_Oversampling
{
    Off,  //!< No oversampling */
    _2x,  //!< Oversampling factor 2
    _4x,  //!< Oversampling factor 4
    _8x   //!< Oversampling factor 8
};

// ---------------------------------------------------------------------------- Power_Mode
/*!
 * This enumeration type lists the power modes that can be configured at the
 * end of a shape or frame.
 */
enum class Power_Mode
{
    Stay_Active,        //!< No power saving.
    Idle,               //!< Go to Idle Mode after last chirp.
    Deep_Sleep,         //!< Go to Deep Sleep Mode after last chirp.
    Deep_Sleep_Continue /*!< Go to Deep Sleep Mode after last chirp, but keep
                             system clock enabled. */
};

// ---------------------------------------------------------------------------- Fifo_Power_Mode
/*!
 * This enumeration lists the power saving modes of an Avian device's built in
 * FIFO memory.
 */
enum class Fifo_Power_Mode
{
    Always_On,              /*!< FIFO is always enabled. */
    Deep_Sleep_Off,         /*!< FIFO is disabled when empty in Deep Sleep
                                 mode. */
    Deep_Sleep_And_Idle_Off /*!< FIFO is disabled when empty in Deep Sleep
                                 and Idle mode. */
};

// ---------------------------------------------------------------------------- Pad_Driver_Mode
//! This enumeration lists the driver modes of the Avian device's pads.
enum class Pad_Driver_Mode
{
    Normal,
    Strong
};

// ---------------------------------------------------------------------------- Reference_Clock_Frequency
/*!
 * This enumeration lists the reference clock frequencies support by Avian
 * devices.
 */
enum class Reference_Clock_Frequency
{
    _80MHz,    //!< 80MHz
    _76_8MHz,  //!< 76.8MHz
    _40MHz,    //!< 40MHz
    _38_4MHz   //!< 38.4MHz
};

// ---------------------------------------------------------------------------- Reference_Clock_Frequency
/*!
 * This enumeration lists the duty cycle correction options of the reference
 * clock doubler.
 *
 * Avian devices have two clock frequency doublers (if at all), one for the
 * system clock one for the PLL clock. Duty cycle correction is always applied
 * to the output of both doublers. For doubler input signals duty cycle
 * correction is optional.
 */
enum class Duty_Cycle_Correction_Mode
{
    Only_Out,  //!< Only output signals are corrected.
    In_Out,    //!< Input signals to both doublers are corrected.
    SysIn_Out  /*!< Only input to system clock doubler is corrected. The input
                    signal to the PLL clock doubler is not corrected. */
};

// ---------------------------------------------------------------------------- Clock_Source
/*!
 * This enumeration is used to select between internal and external clock
 * oscillator.
 */
enum class Clock_Source : bool
{
    Internal,
    External
};

// ---------------------------------------------------------------------------- Oscillator_Ldo_Voltage
/*!
 * This enumeration lists the possible output voltages of the clock
 * oscillator's power supply.
 */
enum class Oscillator_Ldo_Voltage
{
    _1V44,
    _1V5,
    _1V55,
    _1V6
};

// ---------------------------------------------------------------------------- Clock_Amplitude
/*!
 * This enumeration lists the amplification modes of the oscillator output.
 */
enum class Clock_Amplitude : bool
{
    Reduced,
    Increased
};

// ---------------------------------------------------------------------------- Lo_Doubler_Output_Power
/*!
 * This enumeration lists the amplification modes of local oscillator frequency
 * doubler's output power.
 */
enum class Lo_Doubler_Output_Power
{
    _m10dBm16,  //!< -10.16dBm
    _m2dBm66,   //!<  -2.66dBm
    _m0dBm49,   //!<  -0.49dBm
    _0dBm46,    //!<   0.46dBm
    _1dBm01,    //!<   1.01dBm
    _1dBm38,    //!<   1.38dBm
    _1dBm64,    //!<   1.64dBm
    _1dBm82     //!<   1.82dBm
};

// ---------------------------------------------------------------------------- Frame_Format
/**
 * \brief This structure holds the parameters that define the structure of a
 *        radar data frame.
 *
 * A structure of this type must be provided to \ref Driver::set_frame_format
 * and \ref Driver::get_frame_format.
 */
struct Frame_Format
{
    uint32_t num_samples_per_chirp; /**< The number of samples captured
                                         during each chirp. This value
                                         affects the duration of a
                                         single chirp. Must not be 0. */
    uint32_t num_chirps_per_frame;  /**< The number of chirps that are
                                         processed consecutively in a
                                         frame. Must not be 0.*/
    uint8_t rx_mask;                /**< Each available RX antenna is
                                         represented by a bit in this
                                         mask. If a bit is set, the IF
                                         signal received through the
                                         according RX antenna is
                                         captured during chirp
                                         processing. */
};

// ---------------------------------------------------------------------------- Fmcw_Configuration
/**
 * \brief This structure holds the parameters to configure the RF behavior in
 *        FMCW radar mode.
 *
 * A structure of this type must be provided to \ref Driver::set_fmcw_configuration
 * and \ref Driver::get_fmcw_configuration.
 */
struct Fmcw_Configuration
{
    uint32_t lower_frequency_kHz; /**< The lower frequency of an FMCW chirp.
                                       The value is specified in kHz. */
    uint32_t upper_frequency_kHz; /**< The upper frequency of an FMCW chirp.
                                       The value is specified in kHz. */
    Shape_Type shape_type;        /**< The direction of the frequency ramp
                                       during FMCW operation. */
    uint8_t tx_power;             /**< The transmission power the emitted
                                       FMCW chirps. The value is expected to
                                       be in the range of 0...max_tx_power
                                       (see \ref Device_Info). */
};

// ---------------------------------------------------------------------------- Adc_Configuration
/**
 * \brief This structure holds the full configuration set for the BGT6x devices
 *        built in ADC.
 *
 * A structure of this type must be provided to \ref Driver::set_adc_configuration
 * and \ref Driver::get_adc_configuration.
 */
struct Adc_Configuration
{
    uint32_t samplerate_Hz;        /**< The sampling rate. */
    Adc_Sample_Time sample_time;   /**< The duration of the sampling
                                        phase (charging internal
                                        capacitor)*/
    Adc_Tracking tracking;         /**< The tracking mode (Number of sub
                                        conversions) */
    uint8_t double_msb_time;       /**< If this is non-zero, the MSB
                                        decision takes the double time
                                        during conversion phase. */
    Adc_Oversampling oversampling; /**< The oversampling factor */
};

// ---------------------------------------------------------------------------- Baseband_Configuration
/**
 * \brief This structure holds the parameters of the four base band channels of
          BGT6x device.
 *
 * A structure of this type must be provided to
 * \ref Driver::set_baseband_configuration  and
 * \ref Driver::get_baseband_configuration.
 */
struct Baseband_Configuration
{
    Hp_Gain hp_gain_1;           /**< The gain of the high pass filter in
                                      channel 1. */
    int32_t hp_cutoff_1_Hz;      /**< The cutoff frequency of the high pass
                                      filter in channel 1. */
    Vga_Gain vga_gain_1;         /**< The gain of the VGA in channel 1. */
    Hp_Gain hp_gain_2;           /**< The gain of the high pass filter in
                                      channel 2. */
    int32_t hp_cutoff_2_Hz;      /**< The cutoff frequency of the high pass
                                      filter in channel 2. */
    Vga_Gain vga_gain_2;         /**< The gain of the VGA in channel 2. */
    Hp_Gain hp_gain_3;           /**< The gain of the high pass filter in
                                      channel 3. */
    int32_t hp_cutoff_3_Hz;      /**< The cutoff frequency of the high pass
                                      filter in channel 3. */
    Vga_Gain vga_gain_3;         /**< The gain of the VGA in channel 3. */
    Hp_Gain hp_gain_4;           /**< The gain of the high pass filter in
                                      channel 4. */
    int32_t hp_cutoff_4_Hz;      /**< The cutoff frequency of the high pass
                                      filter in channel 4. */
    Vga_Gain vga_gain_4;         /**< The gain of the VGA in channel 4. */
    uint32_t reset_period_100ps; /**< The period of the reset timer in
                                      0.1ns. */
};

// ---------------------------------------------------------------------------- Shape_Group
struct Shape_Group
{
    uint16_t num_repetitions;
    Power_Mode following_power_mode;
    uint64_t post_delay_100ps;
};

// ---------------------------------------------------------------------------- Frame_Definition
struct Frame_Definition
{
    Shape_Group shapes[4];
    Shape_Group shape_set;
    uint16_t num_frames;
};

// ---------------------------------------------------------------------------- Chirp_Timing
struct Chirp_Timing
{
    uint32_t pre_chirp_delay_100ps;
    uint32_t post_chirp_delay_100ps;
    uint32_t pa_delay_100ps;
    uint16_t adc_delay_100ps;
};

// ---------------------------------------------------------------------------- Startup_Timing
struct Startup_Timing
{
    uint64_t wake_up_time_100ps;
    uint32_t pll_settle_time_coarse_100ps;
    uint32_t pll_settle_time_fine_100ps;
};

// ---------------------------------------------------------------------------- Power_Down_Configuration
struct Power_Down_Configuration
{
    bool enable_pll;
    bool enable_vco;
    bool enable_fdiv;
    bool enable_baseband;
    bool enable_rf;
    bool enable_madc;
    bool enable_madc_bandgap;
    bool enable_sadc;
    bool enable_sadc_bandgap;
};

// ---------------------------------------------------------------------------- Startup_Delays
struct Startup_Delays
{
    uint16_t bandgap_100ps;
    uint16_t madc_100ps;
    uint32_t pll_enable_100ps;
    uint16_t pll_divider_100ps;
};

// ---------------------------------------------------------------------------- Anti_Alias_Filter_Settings
struct Anti_Alias_Filter_Settings
{
    int32_t frequency1_Hz;
    int32_t frequency2_Hz;
    int32_t frequency3_Hz;
    int32_t frequency4_Hz;
};

// ---------------------------------------------------------------------------- Duty_Cycle_Correction_Settings
struct Duty_Cycle_Correction_Settings
{
    Duty_Cycle_Correction_Mode mode;
    bool invert_input;
    uint8_t adjust_in;
    int8_t adjust_out;
};

// ---------------------------------------------------------------------------- Pullup_Resistor_Configuration
struct Pullup_Resistor_Configuration
{
    bool enable_spi_cs;
    bool enable_spi_clk;
    bool enable_spi_di;
    bool enable_spi_do;
    bool enable_spi_dio2;
    bool enable_spi_dio3;
    bool enable_irq;
};

// ---------------------------------------------------------------------------- Oscillator_Configuration
struct Oscillator_Configuration
{
    Clock_Source clock_source;
    Oscillator_Ldo_Voltage ldo_voltage;
    Clock_Amplitude amplitude;
    bool enable_clock_output;
};

/* ------------------------------------------------------------------------ */
}  // namespace Avian
}  // namespace Infineon

#endif /* IFX_AVIAN_TYPES_H */

/* --- End of File -------------------------------------------------------- */
