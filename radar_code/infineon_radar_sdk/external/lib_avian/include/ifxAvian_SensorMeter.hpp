/**
 * \file ifxAvian_SensorMeter.hpp
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

#ifndef IFX_AVIAN_SENSORMETER_H
#define IFX_AVIAN_SENSORMETER_H

// ---------------------------------------------------------------------------- includes
#include "ifxAvian_DeviceTraits.hpp"
#include "ifxAvian_Driver.hpp"
#include "ifxAvian_IPort.hpp"
#include "ifxAvian_Types.hpp"

// ---------------------------------------------------------------------------- namespaces
namespace Infineon {
namespace Avian {

// ---------------------------------------------------------------------------- Sensor_Meter
/*!
 */
class Sensor_Meter
{
public:
    Sensor_Meter(HW::IControlPort& port, Device_Type device_type);

    /*!
     * \brief This method measures the temperature of the Avian device.
     *
     * This method must not be called while the Avian state machine is
     * processing chirps. Use \ref get_recently_measured_temperature for
     * that use case.
     *
     * This method must also not be called in deep sleep mode. Use
     * \ref wake_up_and_measure_temperature in that case.
     *
     * \return The measured temperature in Celsius degree.
     */
    float measure_temperature();

    /*!
     * \brief This method measures the temperature of the Avian device
     *        in deep sleep mode.
     *
     * This method wakes up the Avian device from deep sleep mode and
     * measures the device temperature. Afterwards the device is put
     * back to deep sleep mode.
     *
     * This method must not be called while the Avian state machine is
     * processing chirps. Use \ref get_recently_measured_temperature for
     * that use case.
     *
     * In continuous wave mode wake up is not needed. Use
     * \ref measure_temperature in that case.
     *
     * \param[in] driver  A driver instance that maintains the configuration
     *                    to be used for device wake up.
     *
     * \return The measured temperature in Celsius degree.
     */
    float wake_up_and_measure_temperature(const Driver& driver);

    /*!
     * \brief This returns the temperature of the Avian device measured
     *        during chirp processing.
     *
     * This method only works while the Avian state machine is processing
     * chirps. Use \ref measure_temperature to measure temperature in
     * continuous wave mode or \ref wake_up_and_measure_temperature to measure
     * temperature in deep sleep mode.
     *
     * \return The measured temperature in Celsius degree.
     */
    float get_recently_measured_temperature();

    /*!
     * \brief This method measures the TX power currently emitted from the
     *        selected antenna.
     *
     * This method is meant for being called only when the Avian device is
     * operated in continuous wave mode.
     *
     * \param[in] tx_antenna  The zero based index of the TX antenna the power
     *                        should be measured at. If the index exceeds the
     *                        number of physical antennas, an exception is
     *                        thrown.
     *
     * \return The measured power in dBm is returned.
     */
    float measure_tx_power(unsigned tx_antenna);

private:
    enum class Madc_Channel
    {
        Temperature,
        Tx_Power
    };

    /*!
     * This method configures Deep Sleep and Idle mode of the Avian device to
     * prepare it for temperature measurement. The stat machine is put into
     * continuous wave mode. There is no continuous wave mode emitted, but that
     * mode is used to keep the state machine in a static rather than
     * letting the state machine do it's normal job of stepping through the
     * states for chirp processing.
     */
    void wake_up_for_temperature_measurement(const Driver& driver);

    /*!
     * \brief This function measures the MADC input voltage.
     *
     * \param[in] input_channel  The input channel to be measured.
     *
     * \return The measured voltage at MADC input in volts.
     */
    float measure_madc_voltage(Madc_Channel input_channel);

    /*!
     * \brief This function triggers an SADC measurement and return the voltage
     *        at the SADC input.
     *
     * This function configures the SADC for maximum precision and chooses the
     * given input channel. An SADC measurement is started and the SADC result is
     * polled until the busy bit is cleared. The measured value is returned.
     *
     * \param[in] channel  The SADC input channel to be measured.
     *
     * \return The measured voltage at SADC input in volts.
     */
    float measure_sadc_voltage(uint8_t channel);

    HW::IControlPort& m_port;
    const Device_Traits& m_device_traits;
};

/* ------------------------------------------------------------------------ */
}  // namespace Avian
}  // namespace Infineon

#endif /* IFX_AVIAN_SENSORMETER_H */

/* --- End of File -------------------------------------------------------- */
