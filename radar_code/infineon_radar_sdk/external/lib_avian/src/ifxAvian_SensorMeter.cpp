/**
 * \file ifxAvian_SensorMeter.cpp
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

// ---------------------------------------------------------------------------- includes
#include "ifxAvian_SensorMeter.hpp"
#include "Driver/registers_BGT60TR11D.h"
#include "Driver/registers_BGT60TRxxC.h"
#include "Driver/registers_BGT60TRxxD.h"
#include "Driver/registers_BGT60TRxxE.h"
#include "ifxAvian_DeviceTraits.hpp"
#include "ifxAvian_Utilities.hpp"
#include <array>
#include <chrono>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <thread>

// ---------------------------------------------------------------------------- constants
static constexpr float s_adc_ref_voltage = 1.21f;  // This is the MADC and SADC reference voltage in  volts.

/* Constants for conversion from voltage to physical unit */
static constexpr float s_temp_cff_sadc_A = 0.78984f;       // Temperature sensor offset in micro volts.
static constexpr float s_temp_cff_sadc_B = 0.00286f;       // Temperature sensor slope in micro volts per Kelvin.

static constexpr float s_temp_cff_madc_A = -0.168896f;     // Temperature sensor offset in micro volts.
static constexpr float s_temp_cff_madc_B = 0.001513f;      // Temperature sensor slope in micro volts per Kelvin.

static constexpr float s_power_cff_sadc_A1 = 0.09972f;     // TX power sensor slope in volts^-1.
static constexpr float s_power_cff_sadc_y0 = -0.00836f;    // TX power sensor offset in volts.
static constexpr float s_power_cff_sadc_t1 = 8.82773f;     // TX power sensor scaling factor t1, in dBm.

static constexpr float s_power_cff_madc_A1 = 0.1297927f;   // TX power sensor slope in volts^-1.
static constexpr float s_power_cff_madc_y0 = -0.0272855f;  // TX power sensor offset in volts.
static constexpr float s_power_cff_madc_t1 = 10.81336f;    // TX power sensor scaling factor t1, in dBm.

static constexpr uint8_t s_sadc_channel_temperature = 0;

// ---------------------------------------------------------------------------- namespaces
namespace Infineon {
namespace Avian {

// ---------------------------------------------------------------------------- Sensor_Meter
Sensor_Meter::Sensor_Meter(HW::IControlPort& port, Device_Type device_type) :
    m_port(port),
    m_device_traits(Device_Traits::get(device_type))
{}

// ---------------------------------------------------------------------------- measure_temperature
float Sensor_Meter::measure_temperature()
{
    float voltage = (!m_device_traits.has_sadc)
                        ? measure_madc_voltage(Madc_Channel::Temperature)
                        : measure_sadc_voltage(s_sadc_channel_temperature);

    /*
     * BGT60TR11D has different conversion coefficients than other devices due
     * to differential MADC measurement.
     */
    if (m_device_traits.has_sadc)
        return (voltage - s_temp_cff_sadc_A) / s_temp_cff_sadc_B;
    else
        return (voltage - s_temp_cff_madc_A) / s_temp_cff_madc_B;
}

// ---------------------------------------------------------------------------- wake_up_and_measure_temperature
float Sensor_Meter::wake_up_and_measure_temperature(const Driver& driver)
{
    wake_up_for_temperature_measurement(driver);
    float temperature = measure_temperature();
    m_port.generate_reset_sequence();

    return temperature;
}

// ---------------------------------------------------------------------------- get_recently_measured_temperature
float Sensor_Meter::get_recently_measured_temperature()
{
    if (m_device_traits.has_sadc)
    {
        /*
         * The most recently measured SADC value is read from the
         * SADC_RESULT register.
         */
        HW::Spi_Command_t spi_word;
        spi_word = m_device_traits.has_reordered_register_layout
                       ? BGT60TRxxE_REGISTER_READ_CMD(SADC_RESULT)
                       : BGT60TRxxC_REGISTER_READ_CMD(SADC_RESULT);
        m_port.send_commands(&spi_word, 1, &spi_word);
        auto result = BGT60TRxxC_EXTRACT(SADC_RESULT, SADC_RESULT, spi_word);

        /*
         * The register value is converted to voltage and the voltage is
         * converted to Celsius degree.
         */
        float voltage = result * s_adc_ref_voltage / 1023.f;
        return (voltage - s_temp_cff_sadc_A) / s_temp_cff_sadc_B;
    }
    else
    {
        /*
         * The most recently measured temperature value is read from the
         * SENSOR_RESULT register.
         */
        HW::Spi_Command_t spi_word;
        spi_word = BGT60TR11D_REGISTER_READ_CMD(SENSOR_RESULT);
        m_port.send_commands(&spi_word, 1, &spi_word);
        auto result = BGT60TR11D_EXTRACT(SENSOR_RESULT, TEMP_RESULT, spi_word);

        /*
         * The register value is converted to voltage and the voltage is
         * converted to Celsius degree.
         */
        float voltage = s_adc_ref_voltage * (result * 2.f / 4095.f - 1.f);
        return (voltage - s_temp_cff_madc_A) / s_temp_cff_madc_B;
    }
}

// ---------------------------------------------------------------------------- measure_tx_power
float Sensor_Meter::measure_tx_power(unsigned tx_antenna)
{
    // Check number of TX antenna
    if (tx_antenna >= m_device_traits.num_tx_antennas)
        throw std::runtime_error("Selected TX antenna does not exist.");

    if (!m_device_traits.has_sadc)
    {
        /*
         * Devices without SADC use MADC to measure the TX power. The MADC
         * input is differential, so the sensor voltage can be captured with
         * a single measurement.
         */
        float voltage = measure_madc_voltage(Madc_Channel::Tx_Power);
        if (voltage > s_power_cff_madc_y0)
        {
            // The conversion formula is taken from the data sheet.
            return s_power_cff_madc_t1 * logf((voltage - s_power_cff_madc_y0) / s_power_cff_madc_A1);
        }
    }
    else
    {
        /*
         * The power sensor has differential output, but the SADC just has a
         * single ended input. Therefore both outputs are measured separately
         * with the SADC.
         * According to the antenna number different SADC input channels must
         * be selected.
         */
        auto sadc_channels = m_device_traits.sadc_power_channels[tx_antenna];
        float voltage = measure_sadc_voltage(sadc_channels.first)
                        - measure_sadc_voltage(sadc_channels.second);

        // The conversion formula is taken from the data sheet.
        if (voltage > s_power_cff_sadc_y0)
            return s_power_cff_sadc_t1 * logf((voltage - s_power_cff_sadc_y0) / s_power_cff_sadc_A1);
    }

    /*
     * It seems that the measured value is too low so that the conversion
     * formula can't be applied.
     */
    return -std::numeric_limits<float>::infinity();
}

// ---------------------------------------------------------------------------- measure_madc_voltage
float Sensor_Meter::measure_madc_voltage(Madc_Channel input_channel)
{
    std::array<HW::Spi_Command_t, 22> spi_words;

    /*
     * To trigger the MADC, the register ADC0 must be overwritten. The current
     * value of that register must be preserved because it contains the ADC
     * configuration. There current value is read here and used later when the
     * ADC measurement is triggered.
     */
    uint32_t adc_config;
    spi_words[0] = BGT60TRxxC_REGISTER_READ_CMD(ADC0);
    m_port.send_commands(spi_words.data(), 1, &adc_config);
    adc_config &= 0x00FFFFFF;

    /*
     * ADC1 register is rewritten according to the selected sensor.
     * (When the MADC measurement is triggered directly like in this case, the
     * bit field ADC1::TR_PSSTART does not have any effect, so it is not
     * programmed here.)
     */
    if (input_channel == Madc_Channel::Temperature)
        spi_words[0] = BGT60TR11D_SET(ADC1, SENSOR_SEL, 2);
    else
        spi_words[0] = BGT60TR11D_SET(ADC1, SENSOR_SEL, 1);

    /*
     * After switching the ADC input it takes ~1µs until the MADC input voltage
     * is stable. Assuming the maximum SPI rate of 50MHz, this switching time
     * takes the time of 50 SPI bits. Inserting two dummy read commands inserts
     * enough delay to assure a stable input voltage before triggering the MADC.
     */
    spi_words[1] = BGT60TR11D_REGISTER_READ_CMD(CHIP_ID);
    spi_words[2] = BGT60TR11D_REGISTER_READ_CMD(CHIP_ID);

    // ADC measurement is started.
    spi_words[3] = BGT60TR11D_SET(ADC0, TRIG_MADC, 1) | adc_config;

    /*
     * There is no way to check if MADC measurement has finished. With all
     * MADC parameters set to the slowest option the MADC conversion takes
     * 840 clock cycles. At a reference clock frequency of 76.8MHz the maximum
     * measurement time is 10.9µs. Assuming further the maximum SPI rate of
     * 50MHz, a measurement takes the time of 547 SPI bits, or 17 SPI words at
     * maximum. Therefore the result register is read 18 times. The last read
     * will definitely return the measured result.
     */
    for (uint8_t i = 0; i < 18; ++i)
        spi_words[4 + i] = BGT60TR11D_REGISTER_READ_CMD(SENSOR_RESULT);

    // The whole sequence is transferred and the result is read back.
    m_port.send_commands(spi_words.data(), spi_words.size(), spi_words.data());

    // The last word read back contains the measurement result.
    auto result = (input_channel == Madc_Channel::Tx_Power)
                      ? BGT60TR11D_EXTRACT(SENSOR_RESULT, POWER_RESULT,
                                           spi_words.back())
                      : BGT60TR11D_EXTRACT(SENSOR_RESULT, TEMP_RESULT,
                                           spi_words.back());

    return s_adc_ref_voltage * (result * 2.f / 4095.f - 1.f);
}

// ---------------------------------------------------------------------------- measure_sadc_voltage
float Sensor_Meter::measure_sadc_voltage(uint8_t channel)
{
    /*
     * The SDAC is configured for maximum precision (32 times oversampling ->
     * effective accuracy of 10 bit) and triggered.
     */
    HW::Spi_Command_t command_word;
    command_word = BGT60TRxxC_SET(SADC_CTRL, SD_EN, 1)
                   | BGT60TRxxC_SET(SADC_CTRL, OVERS_CFG, 3)
                   | BGT60TRxxC_SET(SADC_CTRL, SESP, 1)
                   | BGT60TRxxC_SET(SADC_CTRL, LVGAIN, 0)
                   | BGT60TRxxC_SET(SADC_CTRL, DSCAL, 0)
                   | BGT60TRxxC_SET(SADC_CTRL, TC_TRIM, 4)
                   | BGT60TRxxC_SET(SADC_CTRL, SADC_CHSEL, channel)
                   | BGT60TRxxC_SET(SADC_CTRL, START_SADC, 1);
    if (m_device_traits.has_explicit_sadc_bg_div_control)
        command_word |= BGT60TRxxD_SET(SADC_CTRL, SADC_CLK_DIV, 3);
    m_port.send_commands(&command_word, 1);

    // wait while SADC is busy
    command_word = m_device_traits.has_reordered_register_layout
                       ? BGT60TRxxE_REGISTER_READ_CMD(SADC_RESULT)
                       : BGT60TRxxC_REGISTER_READ_CMD(SADC_RESULT);

    HW::Spi_Response_t sadc_status = 0;
    for (unsigned i = 0; i < 1000; ++i)
    {
        /* read the busy bit */
        m_port.send_commands(&command_word, 1, &sadc_status);

        if (BGT60TRxxC_EXTRACT(SADC_RESULT, SADC_BUSY, sadc_status) == 0)
            break;
    }
    if (BGT60TRxxC_EXTRACT(SADC_RESULT, SADC_BUSY, sadc_status) != 0)
        throw std::runtime_error("SADC measurement failed.");

    return BGT60TRxxC_EXTRACT(SADC_RESULT, SADC_RESULT, sadc_status)
           * s_adc_ref_voltage / 1023.f;
}

// ---------------------------------------------------------------------------- wake_up_for_temperature_measurement
void Sensor_Meter::wake_up_for_temperature_measurement(const Driver& driver)
{
    std::array<HW::Spi_Command_t, 8> config_words;

    // All circuitry is disabled in Deep Sleep Mode.
    config_words[0] = BGT60TRxxC_SET(CSP_D_0, TX1_EN, 0);
    config_words[1] = BGT60TRxxC_SET(CSCDS, REPC, 0)
                      | BGT60TRxxC_SET(CSCDS, CS_EN, 0)
                      | BGT60TRxxC_SET(CSCDS, ABB_ISOPD, 1)
                      | BGT60TRxxC_SET(CSCDS, RF_ISOPD, 1)
                      | BGT60TRxxC_SET(CSCDS, BG_EN, 1)
                      | BGT60TRxxC_SET(CSCDS, MADC_ISOPD, 1)
                      | BGT60TRxxC_SET(CSCDS, BG_TMRF_EN, 0)
                      | BGT60TRxxC_SET(CSCDS, PLL_ISOPD, 1);
    if (m_device_traits.has_sadc)
        config_words[1] |= BGT60TRxxC_SET(CSCDS, SADC_ISOPD, 1);

    /*
     * Idle mode is configured for temperature measurement. The analog and
     * RF circuitry is enabled, but PLL and VCO are kept disabled. At this
     * time also the MADC and SADC is kept disabled, because the startup
     * calibration may not work best when it runs while the band gap startup
     * has not finished.
     */
    config_words[2] = BGT60TRxxC_SET(CSP_I_0, TX1_EN, 0);
    config_words[3] = BGT60TRxxC_SET(CSP_I_1, TX1_DAC, 0)
                      | BGT60TRxxC_SET(CSP_I_1, TX2_DAC, 0);

    if (m_device_traits.cs_register_layout == Device_Traits::Channel_Set_Layout::Version1)
    {
        config_words[3] |= BGT60TRxxC_SET(CSP_I_1, MADC_EN, 0)
                           | BGT60TRxxC_SET(CSP_I_1, TEMP_MEAS_EN, 1)
                           | BGT60TRxxC_SET(CSP_I_1, BB_RSTCNT, 0)
                           | BGT60TRxxC_SET(CSP_I_1, BBCH_SEL, 0);
    }
    else
    {
        config_words[2] |= BGT60TRxxD_SET(CSP_I_0, TEMP_MEAS_EN, 1);

        config_words[3] |= BGT60TRxxD_SET(CSP_I_1, BB_RSTCNT, 0)
                           | BGT60TRxxD_SET(CSP_I_1, MADC_BBCH1_EN,
                                            (!m_device_traits.has_sadc) ? 1 : 0)
                           | BGT60TRxxD_SET(CSP_I_1, MADC_BBCH2_EN, 0)
                           | BGT60TRxxD_SET(CSP_I_1, MADC_BBCH3_EN, 0)
                           | BGT60TRxxD_SET(CSP_I_1, MADC_BBCH4_EN, 0);
    }

    config_words[4] = BGT60TRxxC_SET(CSCI, REPC, 0)
                      | BGT60TRxxC_SET(CSCI, CS_EN, 0)
                      | BGT60TRxxC_SET(CSCI, ABB_ISOPD, 0)
                      | BGT60TRxxC_SET(CSCI, RF_ISOPD, 0)
                      | BGT60TRxxC_SET(CSCI, BG_EN, 1)
                      | BGT60TRxxC_SET(CSCI, MADC_ISOPD, 1)
                      | BGT60TRxxC_SET(CSCI, BG_TMRF_EN, 1)
                      | BGT60TRxxC_SET(CSCI, PLL_ISOPD, 1);
    if (m_device_traits.has_sadc)
        config_words[4] |= BGT60TRxxC_SET(CSCI, SADC_ISOPD, 1);

    if (m_device_traits.has_extra_startup_delays)
    {
        config_words[4] |= BGT60TRxxD_SET(CSCI, TR_PLL_ISOPD, 0)
                           | BGT60TRxxD_SET(CSCI, TR_MADCEN, 0)
                           | BGT60TRxxD_SET(CSCI, TR_BGEN, 0);
    }

    /*
     * The Avian device is put into Idle mode. The CW bit is set, to avoid
     * that the state machine advances through the steps of chirp operation.
     * The VCO will not be enabled and no RF will be emitted, so actually
     * there is no continuous wave.
     */
    config_words[5] = BGT60TRxxC_SET(PACR1, OSCCLKEN, 1);
    config_words[6] = driver.get_device_configuration()[BGT60TRxxC_REG_MAIN]
                      | BGT60TRxxC_SET(MAIN, CW_MODE, 1)
                      | BGT60TRxxC_SET(MAIN, FRAME_START, 1);

    /*
     * Below the wakeup of the band gap and the SADC are monitored by reading
     * certain bit fields. In preparation the high speed compensation SPI mode
     * is configured according to the needs of the Avian port. Otherwise the
     * startup can fail, because register values are not read correctly.
     */
    bool needs_high_speed = m_port.get_properties().high_speed_compensation;
    config_words[7] = BGT60TRxxC_SET(SFCTL, MISO_HF_READ,
                                     needs_high_speed ? 1 : 0);

    initialize_reference_clock(m_port, driver.get_clock_config_command());
    m_port.send_commands(config_words.data(), config_words.size());

    /*
     * After the transition from Deep Sleep mode to Idle mode the band gap
     * needs some time to startup. The easiest way to wait for the band gap
     * is to poll the status 0 register.
     */
    HW::Spi_Command_t read_cmd = m_device_traits.has_reordered_register_layout
                                     ? BGT60TRxxE_REGISTER_READ_CMD(STAT0)
                                     : BGT60TRxxC_REGISTER_READ_CMD(STAT0);
    HW::Spi_Response_t status_word;

    for (unsigned i = 0; i < 1000; ++i)
    {
        /* read the band gap status bit */
        m_port.send_commands(&read_cmd, 1, &status_word);

        if (BGT60TRxxC_EXTRACT(STAT0, MADC_BGUP, status_word) == 1)
            break;
    }

    if (BGT60TRxxC_EXTRACT(STAT0, MADC_BGUP, status_word) == 0)
        throw std::runtime_error("Band gap of Avian Device did not start up");

    /*
     * Now that the band gap is up and running, either the SADC or MADC or the
     * MADC is enabled.
     */
    HW::Spi_Command_t enable_cmd;
    enable_cmd = BGT60TRxxC_SET(CSCI, ABB_ISOPD, 0)
                 | BGT60TRxxC_SET(CSCI, RF_ISOPD, 0)
                 | BGT60TRxxC_SET(CSCI, BG_EN, 1)
                 | BGT60TRxxC_SET(CSCI, BG_TMRF_EN, 1)
                 | BGT60TRxxC_SET(CSCI, PLL_ISOPD, 1);
    if (m_device_traits.has_sadc)
    {
        enable_cmd |= BGT60TRxxC_SET(CSCI, SADC_ISOPD, 0)
                      | BGT60TRxxC_SET(CSCI, MADC_ISOPD, 1);
    }
    else
    {
        enable_cmd |= BGT60TRxxC_SET(CSCI, MADC_ISOPD, 0);
    }

    if (m_device_traits.has_extra_startup_delays)
    {
        enable_cmd |= BGT60TRxxD_SET(CSCI, TR_PLL_ISOPD, 0)
                      | BGT60TRxxD_SET(CSCI, TR_MADCEN, 0)
                      | BGT60TRxxD_SET(CSCI, TR_BGEN, 0);
    }

    m_port.send_commands(&enable_cmd, 1);

    // Wait for startup and calibration of the MADC or SADC
    if (m_device_traits.has_sadc)
    {
        /*
         * When the SADC is up and running the according in the status register
         * is set, so the easiest way to wait is to poll and check that bit.
         */
        read_cmd = m_device_traits.has_reordered_register_layout
                       ? BGT60TRxxE_REGISTER_READ_CMD(STAT0)
                       : BGT60TRxxC_REGISTER_READ_CMD(STAT0);

        for (unsigned i = 0; i < 1000; ++i)
        {
            m_port.send_commands(&read_cmd, 1, &status_word);
            if (BGT60TRxxC_EXTRACT(STAT0, SADC_RDY, status_word) == 1)
                break;
        }
        if (BGT60TRxxC_EXTRACT(STAT0, SADC_RDY, status_word) == 0)
            throw std::runtime_error("SADC of Avian Device did not start up");
    }
    else
    {
        /*
         * According to data sheet MADC startup takes 660 + 16801 cycles
         * with slowest settings. At a reference clock frequency of 76.8MHz
         * the maximum startup time is 227.35µs.
         */
        std::this_thread::sleep_for(std::chrono::microseconds(228));
    }
}

/* ------------------------------------------------------------------------ */
}  // namespace Avian
}  // namespace Infineon

/* --- End of File -------------------------------------------------------- */
