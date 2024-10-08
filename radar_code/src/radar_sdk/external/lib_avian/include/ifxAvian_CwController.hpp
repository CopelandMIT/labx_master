/**
 * \file ifxAvian_CwController.hpp
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

#ifndef IFX_AVIAN_CWCONTROLLER_H
#define IFX_AVIAN_CWCONTROLLER_H

// ---------------------------------------------------------------------------- includes
#include "ifxAvian_Driver.hpp"
#include "ifxAvian_IPort.hpp"
#include "ifxAvian_Types.hpp"
#include <array>
#include <bitset>
#include <cstdint>
#include <map>
#include <memory>
#include <utility>
#include <vector>

// ---------------------------------------------------------------------------- namespaces
namespace Infineon {
namespace Avian {

// ---------------------------------------------------------------------------- Constant_Wave_Controller
/*!
 * This class allows to use an Avian device in continuous wave mode. All relevant
 * parameters are set through an instance of this class. This class also
 * provides an interface to acquire data received through the RX antennas.
 */
class Continuous_Wave_Controller
{
public:
    template <typename T>
    using Range = std::pair<T, T>;

    /*!
     * This type enumerates the available modes of the Avian device's test
     * signal generator.
     */
    enum class Test_Signal_Generator_Mode
    {
        Off,              /*!< The generator is not used.*/
        Test_Baseband,    /*!< The test signal is routed to the input of the
                               baseband filters instead of the RX mixer output
                               signals. */
        Toggle_Tx_Enable, /*!< TX1 is toggled on and off controlled by the
                               generated test signal. */
        Toggle_Dac_Value, /*!< The power DAC value is toggled between minimum
                               value and the value set trough
                               \ref set_continuous_wave_power controlled by the
                               generated test signal. */
        Rx_Self_Test      /*!< The test signal is routed directly into the RF
                               mixer of RX antenna 1. */
    };

    /*!
     * This type enumerates the available power detectors per antenna built
     * into the Avian device.
     */
    enum class Power_Detector
    {
        Forward,
        Reflection,
    };

    /*!
     * \brief This function initializes the continuous wave mode controller
     *        instance.
     *
     * The provided port is used to detect the type of the connected device.
     * According to the detected device type the CW mode controller is
     * initialized. If the detected device is not recognized, an exception is
     * thrown.
     *
     * After initialization the connection is still in deep sleep mode.
     *
     * \param[in]  port  This is the port the Avian device to be used is
     *                   connected to.
     */
    Continuous_Wave_Controller(HW::IControlPort& port);

    /*!
     * \brief This function initializes the continuous wave mode controller
     *        instance.
     *
     * All settings of the provided driver instance are used to initialize
     * the Constant_Wave_Controller. No device detection is executed, it is
     * assumed that the provided driver instance was created for the same
     * device type as the Constant_Wave_Controller instance will be used for.
     *
     * The purpose of this constructor is to configure expert parameters that
     * are not accessible through the interface of Constant_Wave_Controller
     * (e.g. reference clock frequency, pull up resistors, etc.)
     *
     * After initialization the connection is still in deep sleep mode.
     *
     * \param[in]  port             This is the port the Avian device to be
     *                              used is connected to.
     * \param[in]  driver_template  This is the driver instance all settings
     *                              are copied from.
     */
    Continuous_Wave_Controller(HW::IControlPort& port,
                             const Driver& driver_template);

    //! If continuous wave is enabled, the destructor disables it.
    ~Continuous_Wave_Controller();

    /**
     * \brief This method returns the device type the driver instance has been
     *        created for.
     */
    Device_Type get_device_type() const;

    /*!
     * \brief This function enables or disables the continuous wave signal.
     *
     * If enable is true, the continuous RF signal is enabled. If
     * enable is false, the Avian device is set to Deep Sleep mode, and the
     * continuous RF signal stops.
     *
     * In case the continuous wave can't be enabled due to hardware failure, an
     * exception is thrown.
     */
    void enable_continuous_wave(bool enable);

    /*!
     * This method returns true if the Avian device is emitting a continuous wave signal.
     * If the Avian device is in deep sleep mode, false is returned.
     */
    bool is_continuous_wave_enabled() const;

    /*!
     * This method returns the valid value range for the continuous wave
     * frequency. The minimum and maximum value are specified in Hz.
     */
    Range<double> get_continuous_wave_frequency_range() const;

    /*!
     * This method sets the frequency of the continuous wave in Hz. Before
     * application the specified frequency is limited to the supported RF
     * range (see \ref get_continuous_wave_frequency_range). Furthermore the
     * frequency is internally rounded to meet hardware constraints. It's
     * recommended to read back the applied frequency using
     * \ref get_continuous_wave_frequency.
     */
    void set_continuous_wave_frequency(double frequency_Hz);

    //! This method returns the frequency of the continuous wave in Hz.
    double get_continuous_wave_frequency() const;

    /*!
     * This method returns the valid value range for the DAC value that
     * controls the power of the continuous wave.
     */
    Range<unsigned> get_continuous_wave_power_range() const;

    /*!
     * This method sets the DAC value that controls the power of the constant
     * wave. Before application the specified value is limited to the supported
     * range (see \ref get_continuous_wave_power_range). It's recommended to read
     * back the applied value using \ref get_continuous_wave_power.
     */
    void set_continuous_wave_power(unsigned dac_value);

    /*!
     * This method returns the DAC value that controls the output power of
     * the continuous wave.
     */
    unsigned get_continuous_wave_power() const;

    /*!
     * This method returns the total number of physical TX antennas of the
     * used Avian device.
     */
    unsigned get_number_of_tx_antennas() const;

    /*!
     * This method enables or disables the specified TX antenna.
     *
     * \param[in] antenna  The index of the antenna to be enabled or disabled.
     *                     The value is 0 based and must be less than the value
     *                     returned by \ref get_number_of_tx_antennas. If the
     *                     value is not in the allowed range, an exception is
     *                     thrown.
     * \param[in] enable   If this is true the antenna is enabled, otherwise it's
     *                     disabled.
     */
    void enable_tx_antenna(unsigned antenna, bool enable);

    /*!
     * This method checks if the specified TX antenna is enabled.
     *
     * \param[in] antenna  The index of the antenna to be enabled or disabled.
     *                     The value is 0 based and must be less than the value
     *                     returned by \ref get_number_of_tx_antennas. If the
     *                     value is not in the allowed range, an exception is
     *                     thrown.
     * \return If the antenna is enabled, true is returned. Otherwise false is
     *         returned.
     */
    bool is_tx_antenna_enabled(unsigned antenna) const;

    /*!
     * This method sets the mode of the Avian device's internal test signal
     * generator. If the connected device does not support the specified mode,
     * an exception is thrown.
     */
    void set_test_signal_generator_mode(Test_Signal_Generator_Mode mode);

    /*!
     * This method returns the currently selected mode of the Avian device's
     * internal test signal generator.
     */
    Test_Signal_Generator_Mode get_test_signal_generator_mode() const;

    /*!
     * This method returns the valid value range for the test signal frequency.
     * The minimum and maximum value are specified in Hz.
     */
    Range<float> get_test_signal_frequency_range() const;

    /*!
     * This method sets the frequency in Hz of the test signal generated
     * internally by the Avian device. Before application the specified
     * frequency is limited to the supported range
     * (see \ref get_test_signal_frequency_range). Furthermore the frequency is
     * internally rounded to meet hardware constraints. It's recommended to
     * read back the applied frequency using \ref get_test_signal_frequency.
     */
    void set_test_signal_frequency(float frequency_Hz);

    /*!
     * This method returns the frequency in Hz of the test signal generated
     * internally by the Avian device.
     */
    float get_test_signal_frequency() const;

    /*!
     * This method returns the total number of physical RX antennas of the
     * used Avian device.
     */
    unsigned get_number_of_rx_antennas() const;

    /*!
     * This method enables or disables the specified RX antenna.
     *
     * \param[in] antenna  The index of the antenna to be enabled or disabled.
     *                     The value is 0 based and must be less than the value
     *                     returned by \ref get_number_of_rx_antennas. If the
     *                     value is not in the allowed range, an exception is
     *                     thrown.
     * \param[in] enable   If this is true the antenna is enabled, otherwise it's
     *                     disabled.
     */
    void enable_rx_antenna(unsigned antenna, bool enable);

    /*!
     * This method checks if the specified RX antenna is enabled.
     *
     * \param[in] antenna  The index of the antenna to be enabled or disabled.
     *                     The value is 0 based and must be less than the value
     *                     returned by \ref get_number_of_rx_antennas. If the
     *                     value is not in the allowed range, an exception is
     *                     thrown.
     * \return If the antenna is enabled, true is returned. Otherwise false is
     *         returned.
     */
    bool is_rx_antenna_enabled(unsigned antenna) const;

    //! This method returns the valid value range for the number of samples.
    Range<unsigned> get_number_of_samples_range() const;

    /*!
     * This method sets the number of samples that is captured each time
     * \ref capture_rx_signals is called. Before application the specified
     * value is limited to the supported range (see
     * \ref get_number_of_samples_range). It's recommended to read back the
     * applied frequency using \ref get_number_of_samples.
     */
    void set_number_of_samples(unsigned num_samples);

    /*!
     * This method returns the number of samples that is captured each time
     * \ref capture_rx_signals is called.
     */
    unsigned get_number_of_samples() const;

    /*!
     * This method returns the valid value range for the continuous wave
     * frequency. The minimum and maximum value are specified in Hz.
     */
    Range<float> get_sampling_rate_range() const;

    /*!
     * This method sets the sampling frequency in Hz that is used to capture
     * RX antenna signals whenever \ref capture_rx_signals is called. Before
     * application the specified frequency is limited to the supported range
     * (see \ref get_sampling_rate_range). Furthermore the frequency is
     * internally rounded to meet hardware constraints. It's recommended to
     * read back the applied frequency using \ref get_sampling_rate.
     */
    void set_sampling_rate(float frequency_Hz);

    /*!
     * This method returns the sampling frequency in Hz that is used to capture
     * RX antenna signals whenever \ref capture_rx_signals is called.
     */
    float get_sampling_rate() const;

    /*!
     * This method captures the signals received through the Avian device's RX
     * antennas with the Avian device's Analog-Digital-Converter (ADC).
     *
     * This method throws an exception in following cases:
     * - The port instance passed to the constructor was not of type
     *   HW::IReadPort<HW::Packed_Raw_Data_t>,
     * - Continuous wave mode is not enabled,
     * - All RX antennas are disabled,
     * - A hardware failure occurred.
     *
     * \return The captured signals are returned as a map that uses the 0 based
     *         antenna index as key. The signal of each enabled RX antenna is
     *         stored as a vector of sample values of normalized range -1...1.
     *         The length of all vectors is the currently number of samples set
     *         through \ref set_number_of_samples.
     */
    std::map<unsigned, std::vector<float>> capture_rx_signals();

    /*!
     * This method sets the gain of the Avian device's baseband high pass
     * filter.
     */
    void set_hp_gain(Hp_Gain gain);

    /*!
     * This method returns the gain of the Avian device's baseband high pass
     * filter.
     */
    Hp_Gain get_hp_gain() const;

    /*!
     * This method sets the cutoff frequency of the Avian device's baseband
     * high pass filter.
     *
     * The specified cutoff frequency is rounded to the nearest supported
     * value.
     */
    void set_hp_cutoff(int32_t cutoff_Hz);

    /*!
     * This method returns the cutoff frequency of the Avian device's baseband
     * high pass filter in Hz.
     */
    int32_t get_hp_cutoff() const;

    //! This method sets the gain of the Avian device's baseband amplifier.
    void set_vga_gain(Vga_Gain gain);

    //! This method returns the gain of the Avian device's baseband amplifier.
    Vga_Gain get_vga_gain() const;

    /*!
     * This method sets the cutoff frequency of the Avian device's baseband
     * anti alias filter.
     *
     * The specified cutoff frequency is rounded to the nearest supported
     * value.
     */
    void set_aaf_cutoff(int32_t cutoff_Hz);

    /*!
     * This method returns the cutoff frequency of the Avian device's baseband
     * anti alias filter in Hz.
     */
    int32_t get_aaf_cutoff() const;

    /*!
     * This method sets the sample time of the sample-and-hold circuitry in the
     * Avian device's Analog-Digital-Converter (ADC).
     *
     * The sample time has an impact on the possible ADC sampling rate. It is
     * recommended to read back the ADC sampling rate and the according value
     * range using \ref get_sampling_rate and \ref get_sampling_rate_range.
     */
    void set_adc_sample_time(Adc_Sample_Time sample_time);

    /*!
     * This method returns the sample time of the sample-and-hold circuitry in
     * the Avian device's Analog-Digital-Converter (ADC).
     */
    Adc_Sample_Time get_adc_sample_time() const;

    /*!
     * This method sets the tracking mode of the Avian device's
     * Analog-Digital-Converter (ADC).
     *
     * Tracking has an impact on the possible ADC sampling rate. It is
     * recommended to read back the ADC sampling rate and the according value
     * range using \ref get_sampling_rate and \ref get_sampling_rate_range.
     */
    void set_adc_tracking(Adc_Tracking tracking);

    /*!
     * This method returns the tracking mode of the Avian device's
     * Analog-Digital-Converter (ADC).
     */
    Adc_Tracking get_adc_tracking() const;

    /*!
     * This method sets the oversampling mode of the  Avian device's
     * Analog-Digital-Converter (ADC).
     *
     * Oversampling has an impact on the possible ADC sampling rate. It is
     * recommended to read back the ADC sampling rate and the according value
     * range using \ref get_sampling_rate and \ref get_sampling_rate_range.
     */
    void set_adc_oversampling(Adc_Oversampling oversampleing);

    /*!
     * This method returns the oversampling mode of the  Avian device's
     * Analog-Digital-Converter (ADC).
     */
    Adc_Oversampling get_adc_oversampling() const;

    /*!
     * This method controls if the Avian device's Analog-Digital-Converter
     * (ADC) takes twice the time to decide the value of the most significant
     * bit (MSB) of the conversion result.
     *
     * MSB decision time has an impact on the possible ADC sampling rate. It
     * is recommended to read back the ADC sampling rate and the according
     * value range using \ref get_sampling_rate and
     * \ref get_sampling_rate_range.
     */
    void set_adc_double_msb_time(bool double_time);

    /*!
     * This method returns if the Avian device's Analog-Digital-Converter
     * (ADC) takes twice the time to decide the value of the most significant
     * bit (MSB) of the conversion result.
     */
    bool get_adc_double_msb_time() const;

    /*!
     * This method measures the current temperature of the Avian device.
     *
     * This method may throw an exception in case of a hardware failure.
     *
     * \return The measured device temperature in Celsius degree is returned.
     */
    float measure_temperature();

    /*!
     * This method measures returns the power that is emitted from the
     * specified TX antenna.
     *
     * The power measurement is only performed if the continuous wave is enabled.
     *
     * This method may throw an exception in case of a hardware failure.
     *
     * \param[in] antenna   The index of the antenna to be measured. The value
     *                      is 0 based and must be less than the value returned
     *                      by \ref get_number_of_tx_antennas. If the value is
     *                      not in the allowed range, an exception is thrown.
     *
     * \param[in] detector  The power detector to be used for the measurement.
     *                      If the value is not in the allowed range, an
     *                      exception is thrown.
     *
     * \return If the continuous wave is enabled the measured TX power in dBm is
     *         returned. Otherwise -infinity is returned.
     */
    float measure_tx_power(unsigned antenna,
                           Power_Detector detector = Power_Detector::Forward);

    /**
     * \brief This method returns a const reference to the driver instance which
     * has been created.
     *
     * \return A const reference to the created driver instance.
     */
    const Driver& get_driver();

private:
    /*!
     * This internal constructor does the main construction work. It is used
     * by the public constructors.
     *
     * \param[in]  port    This is the port the Avian device to be used is
     *                     connected to.
     * \param[in]  driver  This driver instance will be used internally to
     *                     generate the register configuration.
     */
    Continuous_Wave_Controller(HW::IControlPort& port,
                             std::unique_ptr<Driver> driver);

    /*!
     * This method sends continuously trigger commands to the Avian device and
     * reads back the current power mode until the active power state is
     * reached.
     *
     * \return If the Avian device could be put to active power state true is
     *         returned. In case of an error false is returned.
     */
    bool go_to_active_state();

    HW::IControlPort& m_port;
    std::unique_ptr<Driver> m_driver;
    double m_continuous_wave_frequency;
    unsigned m_continuous_wave_power;
    Test_Signal_Generator_Mode m_test_signal_mode;
    uint32_t m_test_signal_frequency_divider;
    bool m_continuous_wave_enabled;
    std::bitset<2> m_tx_mask;
    std::bitset<4> m_rx_mask;
    uint16_t m_num_samples;
    std::array<HW::Spi_Command_t, 2> m_toggle_commands;
};

/* ------------------------------------------------------------------------ */
}  // namespace Avian
}  // namespace Infineon

#endif /* IFX_AVIAN_CWCONTROLLER_H */

/* --- End of File -------------------------------------------------------- */
