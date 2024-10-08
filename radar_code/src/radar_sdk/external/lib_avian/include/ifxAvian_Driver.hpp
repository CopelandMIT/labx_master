/**
 * \file ifxAvian_Driver.hpp
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

#ifndef IFX_AVIAN_DRIVER_H
#define IFX_AVIAN_DRIVER_H

// ---------------------------------------------------------------------------- includes
#include "ifxAvian_IPort.hpp"
#include "ifxAvian_RegisterSet.hpp"
#include "ifxAvian_Types.hpp"
#include "value_conversion/ifxAvian_TimingConversion.hpp"
#include <cstdint>
#include <map>
#include <memory>

namespace Infineon {
namespace Avian {

struct Device_Traits;

}  // namespace Avian
}  // namespace Infineon

// ---------------------------------------------------------------------------- namespaces
namespace Infineon {
namespace Avian {

// ---------------------------------------------------------------------------- API Parameter types
/**
 * \brief This structure holds information about a device and its capabilities.
 *
 * A structure of this type must be provided to \ref Driver::get_device_info when
 * reading the device information.
 */
struct Device_Info
{
    const char* description;       /**< A pointer to a null terminated
                                        string holding a human
                                        readable description of the
                                        device. */
    uint32_t min_rf_frequency_kHz; /**< The minimum RF frequency the
                                        the sensor device can emit. */
    uint32_t max_rf_frequency_kHz; /**< The maximum RF frequency the
                                        sensor device can emit. */
    uint8_t num_tx_antennas;       /**< The number of RF antennas used
                                        for transmission. */
    uint8_t num_rx_antennas;       /**< The number of RF antennas used
                                        for reception. */
    uint8_t max_tx_power;          /**< The amount of RF transmission
                                        power can be controlled in the
                                        range of 0 ... max_tx_power.
                                        */
    uint8_t num_temp_sensors;      /**< The number of available
                                        temperature sensors. */
    uint8_t interleaved_rx;        /**< If this is 0, the radar data
                                        of multiple RX antennas is
                                        stored in consecutive data
                                        blocks, where each block holds
                                        data of one antenna. If this
                                        is non-zero, the radar data of
                                        multiple RX antennas is stored
                                        in one data block, where for
                                        each point in time the samples
                                        from all RX antennas are
                                        stored consecutively before
                                        the data of the next point in
                                        time follows. */
};

/**
 * \brief This structure holds all information about a single frame of radar
 *        data.
 *
 * A structure of this type must be provided to \ref Driver::get_frame_info.
 *
 * The data type of samples in sample_data depends on the ADC resolution. If
 * adc_resolution <= 8, sample_data points to an array of uint8_t. If
 * 8 < adc_resolution <= 16, sample_data points to an array of uint16_t.
 * Higher ADC resolutions are currently not supported.
 *
 * If the frame contains more than one chirp, the chirps are stored in
 * consecutive data blocks. The start of each chirp can be calculated by the
 * following formula.
 * \code
 * frame_start = &sample_data[CHIRP_NUMBER
 *                            * num_rx_antennas * num_samples_per_chirp];
 * \endcode
 *
 * The index calculation of a certain data sample captured from a certain RX
 * antenna depends on the data interleaving.
 * The following code examples demonstrate how to access a data sample.
 * \code
 * // interleaved_rx = 0
 * // --------------------------------------------------------------------
 * value = frame_start[ANTENNA_NUMBER * num_samples_per_chirp
 *                     + SAMPLE_NUMBER];
 *
 * // interleaved_rx = 1
 * // --------------------------------------------------------------------
 * value = pFrameStart[SAMPLE_NUMBER * num_rx_antennas + ANTENNA_NUMBER];
 * \endcode
 *
 * In the code examples above, ANTENNA_NUMBER does not refer to the physical
 * antenna number, but the n-th enabled antenna. If for example rx_mask = 0xA,
 * ANTENNA_NUMBER=0 refers to antenna 0010b and ANTENNA_NUMBER=1 refers to
 * antenna 1000b. Furthermore it is assumed that pDataBuffer and frame_start
 * are casted to (uint8_t*) or (uint16_t*) according to adc_resolution.
 *
 * In total the data buffer contains
 * uNumChirps * uNumRXAntennas * uNumSamplesPerChirp values.
 */
struct Frame_Info
{
    const void* sample_data;        /**< The buffer containing the
                                         radar data */
    uint32_t frame_number;          /**< The running number of the data
                                         frame. The frame counter is,
                                         reset every time
                                         \ref Driver::set_automatic_frame_trigger
                                         is called. If automatic frame
                                         trigger is not active, the frame
                                         counter may not work, and this
                                         could be 0. */
    uint32_t num_chirps;            /**< The number of chirps in this
                                         frame. */
    uint8_t num_rx_antennas;        /**< The number of RX signals that
                                         have been acquired with each
                                         chirp. */
    uint32_t num_samples_per_chirp; /**< The number of samples acquired
                                         in each chirp for each enabled
                                         RX antenna. */
    int32_t temperature_001C;       /**< The temperature during chirp
                                         acquisition. This value is
                                         always measured using the
                                         first temperature sensor. */
    uint8_t rx_mask;                /**< Each antenna is represented
                                         by a bit in this mask. If the
                                         bit is set, the according RX
                                         antenna was used to capture
                                         data in this frame. */
    uint8_t adc_resolution;         /**< The ADC resolution of the data
                                         in pDataBuffer. */
    bool interleaved_rx;            /**< If this is 0, the radar data
                                         of multiple RX antennas is
                                         stored in consecutive data
                                         blocks, where each block
                                         holds data of one antenna.
                                         If this is non-zero, the
                                         radar data of multiple RX
                                         antennas is stored in one
                                         data block, where for each
                                         point in time the samples
                                         from all RX antennas are
                                         stored consecutively before
                                         the data of the next point
                                         in time follows. */
};

// ---------------------------------------------------------------------------- Driver
/**
 * This class is a C++ version of the legacy C driver package for Avian
 * devices.
 * \note This class is just a starting point for further refactoring, so the
 *       API is not stable and will change significantly!
 */
class Driver
{
public:
    enum class Error : uint16_t
    {
        OK = 0x0000,                         /**< No error has occurred.*/
        DEVICE_DOES_NOT_EXIST = 0x0001,      /**< The device number
                                                  requested to open does
                                                  not exist. */
        BUSY = 0x0002,                       /**< The requested operation
                                                  can't be executed. A
                                                  possible reason is that
                                                  a certain test mode is
                                                  activated or the
                                                  automatic trigger is
                                                  active. */
        INCOMPATIBLE_MODE = 0x0003,          /**< The requested operation
                                                  is not supported by the
                                                  currently active mode of
                                                  operation. */
        TIME_OUT = 0x0004,                   /**< A timeout has occurred
                                                  while waiting for a data
                                                  frame to be acquired. */
        UNSUPPORTED_FRAME_INTERVAL = 0x0005, /**< The requested time
                                                  interval between two
                                                  frames is out of range.
                                                  */
        ANTENNA_DOES_NOT_EXIST = 0x0006,     /**< One or more of the
                                                  selected RX or TX
                                                  antennas is not present
                                                  on the device. */
        SENSOR_DOES_NOT_EXIST = 0x0007,      /**< The requested
                                                  temperature sensor does
                                                  not exist. */
        UNSUPPORTED_FRAME_FORMAT = 0x0008,   /**< The combination of
                                                  chirps per frame,
                                                  samples per chirp and
                                                  number of antennas is
                                                  not supported by the
                                                  driver. A possible
                                                  reason is the limit of
                                                  the driver internal data
                                                  memory. */
        FREQUENCY_OUT_OF_RANGE = 0x0009,     /**< The specified RF
                                                  frequency is not in
                                                  the supported range
                                                  of the device. */
        POWER_OUT_OF_RANGE = 0x000A,         /**< The specified
                                                  transmission power is
                                                  not in the valid range
                                                  of 0...max_tx_power (see
                                                  \ref Device_Info). */
        UNAVAILABLE_SIGNAL_PART = 0x000B,    /**< The device is not
                                                  capable to capture
                                                  the requested part of
                                                  the complex signal (see
                                                  \ref Device_Info). */
        UNSUPPORTED_DIRECTION = 0x0020,      /**< The specified FMCW ramp
                                                  direction is not
                                                  supported by the device.
                                                  */
        SAMPLERATE_OUT_OF_RANGE = 0x0050,    /**< The specified sampling
                                                  rate is out of range. */
        UNSUPPORTED_TX_MODE = 0x0100,        /**< The specified TX mode is
                                                  not supported by the
                                                  device. */
        UNSUPPORTED_HP_GAIN = 0x0101,        /**< The specified high pass
                                                  filter gain is not
                                                  defined. */
        UNSUPPORTED_VGA_GAIN = 0x0103,       /**< The specified gain
                                                  adjustment setting
                                                  is not defined. */
        RESET_TIMER_OUT_OF_RANGE = 0x0104,   /**< The specified reset
                                                  timer period is out
                                                  of range. */
        UNSUPPORTED_HP_CUTOFF = 0x0105,      /**< The specified high pass
                                                  cutoff frequency is out
                                                  of range. */
        UNSUPPORTED_AAF_CUTOFF = 0x0106,     /**< The specified low pass
                                                  cutoff frequency is out
                                                  of range. */
        CALIBRATION_FAILED = 0x010A,         /**< The calibration of phase
                                                  settings or base band
                                                  chain did not succeed.
                                                  */
        INVALID_PHASE_SETTING = 0x010B,      /**< The provided oscillator
                                                  phase setting is not
                                                  valid. It's forbidden to
                                                  disable both phase
                                                  modulators. */
        UNDEFINED_TRACKING_MODE = 0x0110,    /**< The specified ADC
                                                  tracking mode is not
                                                  supported by the device.
                                                  */
        UNDEFINED_ADC_SAMPLE_TIME = 0x0111,  /**< The specified ADC
                                                  sampling time is not
                                                  supported by the device.*/
        UNDEFINED_ADC_OVERSAMPLING = 0x0112, /**< The specified ADC
                                                  oversampling factors is
                                                  not supported by the
                                                  device. */
        NONCONTINUOUS_SHAPE_SEQUENCE = 0x0120,
        UNSUPPORTED_NUM_REPETITIONS = 0x0121,
        UNSUPPORTED_POWER_MODE = 0x0122,
        POST_DELAY_OUT_OF_RANGE = 0x0123,
        NUM_FRAMES_OUT_OF_RANGE = 0x0124,
        SHAPE_NUMBER_OUT_OF_RANGE = 0x0125,
        PRECHIRPDELAY_OUT_OF_RANGE = 0x0126,
        POSTCHIRPDELAY_OUT_OF_RANGE = 0x0127,
        PADELAY_OUT_OF_RANGE = 0x0128,
        ADCDELAY_OUT_OF_RANGE = 0x0129,
        WAKEUPTIME_OUT_OF_RANGE = 0x012A,
        SETTLETIME_OUT_OF_RANGE = 0x012B,
        UNSUPPORTED_FIFO_SLICE_SIZE = 0x012C,
        SLICES_NOT_RELEASABLE = 0x012D,
        FIFO_OVERFLOW = 0x012E,
        NO_MEMORY = 0x012F,
        CHIP_SETUP_FAILED = 0x0131,
        FEATURE_NOT_SUPPORTED = 0x0134,
        PRECHIRP_EXCEEDS_PADELAY = 0x0135,
        UNSUPPORTED_FREQUENCY = 0x0137,
        UNSUPPORTED_FIFO_POWER_MODE = 0x0140,
        UNSUPPORTED_PAD_DRIVER_MODE = 0x0141,
        BANDGAP_DELAY_OUT_OF_RANGE = 0x0142,
        MADC_DELAY_OUT_OF_RANGE = 0x0143,
        PLL_ENABLE_DELAY_OUT_OF_RANGE = 0x0144,
        PLL_DIVIDER_DELAY_OUT_OF_RANGE = 0x0145,
        DOUBLER_MODE_NOT_SUPPORTED = 0x0146,
        DC_IN_CORRECTION_OUT_OF_RANGE = 0x0147,
        DC_OUT_CORRECTION_OUT_OF_RANGE = 0x0148,
        POWER_SENS_DELAY_OUT_OF_RANGE = 0x014C,
        NO_SWITCH_TIME_MADC_POWER = 0x014D,
        NO_SWITCH_TIME_MADC_RX = 0x014E,
        NO_SWITCH_TIME_MADC_TEMP = 0x014F,
        NO_MEASURE_TIME_MADC_TEMP = 0x0150,
        TEMP_SENSING_WITH_NO_RX = 0x0151
    };

    /**
     * This method checks the device type of the connection Avian device and
     * creates a suitable driver instance.
     *
     * \param[in] port  The port the Avian device is connected to.
     *
     * \return If the type of the connected Avian device is supported, the
     *         created driver instance is returned. If the type is not
     *         supported, nullptr is returned.
     */
    static std::unique_ptr<Driver> create_driver(HW::IControlPort& port);

    /**
     * The constructor initializes the Driver instance for the specified Avian
     * device type.
     *
     * It is expected that the specified device type is the type of the Avian
     * device connected to the specified port. The type is not checked. If the
     * wrong device type has been specified, the Driver instance will not work
     * properly.
     *
     * \param[in] port         The port the Avian device is connected to.
     * \param[in] device_type  The type of the Avian device the driver is
     *                         initialized for.
     */
    Driver(HW::IControlPort& port, Device_Type device_type);

    /*
     * Instead of a copy constructor this constructor copies all parameter from
     * the source instance. The port must be still provided and is not taken
     * from the source.
     *
     * \param[in] port    The port the Avian device is connected to.
     * \param[in] source  The instance all parameters are copied from.
     */
    Driver(HW::IControlPort& port, const Driver& source);

    /*
     * The copy construction copies all parameters from the source instance and
     * uses the same port instance as the source.
     */
    Driver(const Driver& source) = default;

    ~Driver() = default;

    /*
     * The assignment operator takes all parameters from the source instance
     * but keeps the port.
     */
    const Driver& operator=(const Driver& source);

    /**
     * \brief This function returns information about a device and its
     *        capabilities.
     *
     * This function returns device information through a structure of type
     * \ref Device_Info. The structure must be provided by the user and may be
     * uninitialized.
     *
     * \param[out] device_info  A pointer to a location, where the device
     *                          information will be stored.
     *
     * \return An error code indicating if the function succeeded. The following
     *         error codes can occur:
     *         - \ref Error::OK     if the pDeviceInfo has been filled
     *                              successfully
     */
    Error get_device_info(Device_Info* device_info) const;

    /**
     * \brief This function returns the current temperature of the sensor device.
     *
     * This function writes the current temperature value to an integer variable.
     * The variable must be provided by the user and may be uninitialized.
     *
     * The unit of the temperature value is 0.001 degree Celsius.
     *
     * A radar sensor device may contain more than one temperature sensors, so the
     * number of the sensor to be used for measuring must be specified. The number
     * of available temperature sensors is part of the \ref Device_Info structure
     * returned by \ref get_device_info. The number of the first temperature
     * sensor is 0.
     *
     * A call to this function not necessarily triggers a temperature measurement.
     * The function may return the last known temperature of the specified sensor.
     *
     * \param[out] temperature_001C  A pointer to a location, where the
     *                               temperature value will be stored.
     *
     * \return An error code indicating if the function succeeded. The following
     *         error codes can occur:
     *         - \ref Error::OK                        if a valid temperature
     *                                                 could be returned
     *         - \ref Error::SENSOR_DOES_NOT_EXIST     if the specified
     *                                                 temperature sensor does not
     *                                                 exist
     */
    Error get_temperature(int32_t* temperature_001C) const;

    /**
     * \brief This function returns the RF transmission power of the specified TX
     *        antenna.
     *
     * This function writes the current RF transmission power value to an integer
     * variable. The variable must be provided by the user and may be
     * uninitialized.
     *
     * The unit of the RF transmission power value is 0.001 dBm.
     *
     * Typically a radar sensor device contains a power sensor for each
     * transmission antenna. The number of transmission antennas is part of the
     * \ref Device_Info structure returned by \ref get_device_info. The
     * number of the first antenna (and TX power sensor) is 0.
     *
     * A call to this function not necessarily triggers a power measurement. The
     * function may return the last known transmission power of the specified TX
     * antenna.
     *
     * \param[in]  tx_antenna       The number of the TX antenna whose transmission
     *                              power is requested.
     * \param[out] tx_power_001dBm  A pointer to a location, where the
     *                              transmission power value will be stored.
     *
     * \return An error code indicating if the function succeeded. The following
     *         error codes can occur:
     *         - \ref Error::OK                        if a valid power value
     *                                                 could be returned
     *         - \ref Error::DEVICE_DOES_NOT_EXIST     if the specified
     *                                                 transmission power sensor
     *                                                 does not exist
     */
    Error get_tx_power(uint8_t tx_antenna, int32_t* tx_power_001dBm);

    /**
     * \brief This function returns the duration for a single chirp in current
     *        configuration.
     *
     * This function writes the duration, a chirp with current configuration will
     * take, to an integer variable. The variable must be provided by the user and
     * may be uninitialized.
     *
     * The unit of chirp duration value is nanoseconds.
     *
     * While automatic trigger is active, reading the chirp duration may fail.
     *
     * \param[out] chirp_duration_ns  A pointer to a location, where the chirp
     *                                duration value will be stored.
     *
     * \return An error code indicating if the function succeeded. The following
     *         error codes can occur:
     *         - \ref Error::OK       if a valid chirp duration value could be
     *                                returned
     *         - \ref Error::BUSY     if automatic trigger is active
     */
    Error get_chirp_duration(uint32_t* chirp_duration_ns) const;

    /**
     * \brief This function modifies the current radar data frame format.
     *
     * The function changes the radar data format according to the provided
     * structure of type \ref Frame_Format.
     *
     * A value check is applied to each field of configuration. If any value or
     * the combination of the values is out of range, the frame format is not
     * changed and an error is returned.
     *
     * The configuration can not be changed while the automatic trigger is active.
     *
     * \param[in] frame_format   A pointer to the new frame configuration to be
     *                           applied.
     *
     * \return An error code indicating if the function succeeded. The following
     *         error codes can occur:
     *         - \ref Error::OK                           if the frame
     *                                                    configuration could be
     *                                                    applied
     *         - \ref Error::BUSY                         if the automatic trigger
     *                                                    is active and the device
     *                                                    is busy with acquiring
     *                                                    radar data
     *         - \ref Error::ANTENNA_DOES_NOT_EXIST       if one ore more of the
     *                                                    antennas requested by
     *                                                    rx_mask does not exist
     *         - \ref Error::UNSUPPORTED_FRAME_FORMAT     if a radar data frame
     *                                                    with the specified
     *                                                    configuration would
     *                                                    exceed the internal
     *                                                    memory
     *         - \ref Error::UNAVAILABLE_SIGNAL_PART      if the selected signal
     *                                                    part can't be captured
     *                                                    by the device.
     */
    Error set_frame_format(const Frame_Format* frame_format);

    /**
     * \brief This function returns the current frame configuration.
     *
     * This function returns the current frame configuration through a structure of
     * type\ref Frame_Format. The structure must be provided by the user and may be
     * uninitialized.
     *
     * While automatic trigger is active, reading the current frame format may
     * fail.
     *
     * \param[out] frame_format   A pointer to a location, where the current frame
     *                             configuration will be stored.
     *
     * \return An error code indicating if the function succeeded. The following
     *          error codes can occur:
     *         - \ref Error::OK       if pConfiguration has been filled
     *                                successfully
     *         - \ref Error::BUSY     if automatic trigger is active
     */
    Error get_frame_format(Frame_Format* frame_format) const;

    /**
     * \brief This function selects the reference clock frequency
     *
     * This function selects which reference clock frequency the driver should be
     * configured for. If this function is not called, driver instances will be
     * configured for 80MHz oscillators.
     *
     * \note Changing the reference clock affects a lot of parameters. It's
     *       recommended either to reset all parameters, or query the all current
     *       parameters after this function is called.
     */
    Error set_reference_clock_frequency(Reference_Clock_Frequency frequency);
    Error get_reference_clock_frequency(Reference_Clock_Frequency* frequency) const;

    /**
     * \brief This function modifies the RF parameters in FMCW radar mode.
     *
     * The function changes the RF parameters for FMCW radar mode according to the
     * provided structure of type \ref Fmcw_Configuration. After calling this
     * function, the device will be in FMCW radar mode.
     *
     * A value check is applied to each field of configuration. If any value or
     * the combination of the values is out of range, the FMCW parameters are not
     * changed and an error is returned.
     *
     * The configuration can not be changed while the automatic trigger is active.
     *
     * \param[in] configuration  A pointer to the new FMCW parameters to be
     *                           applied.
     *
     * \return An error code indicating if the function succeeded. The following
     *         error codes can occur:
     *         - \ref Error::OK                         if the FMCW parameters
     *                                                  could be applied
     *         - \ref Error::BUSY                       if the automatic trigger
     *                                                  is active and the device
     *                                                  is busy with acquiring
     *                                                  radar data
     *         - \ref Error::FREQUENCY_OUT_OF_RANGE     if the upper and/or lower
     *                                                  FMCW frequency is not in
     *                                                  the radar sensor's RF
     *                                                  range
     *         - \ref Error::POWER_OUT_OF_RANGE         if the specified TX power
     *                                                  value exceeds the maximum
     *                                                  value
     *         - \ref Error::UNSUPPORTED_DIRECTION      if the device does not
     *                                                  support the requested
     *                                                  chirp direction
     */
    Error set_fmcw_configuration(const Fmcw_Configuration* configuration);

    /**
     * \brief This function returns the current RF parameters in FMCW radar mode.
     *
     * This function returns the current RF parameters in FMCW mode through a
     * structure of type \ref Fmcw_Configuration. The structure must be provided by
     * the user and may be uninitialized.
     *
     * While automatic trigger is active, reading the current frame format may
     * fail.
     *
     * \param[out] configuration  A pointer to a location, where the current FMCW
     *                            configuration will be stored.
     *
     * \return An error code indicating if the function succeeded. The following
     *         error codes can occur:
     *         - \ref Error::OK                 if pConfiguration has been filled
     *                                          successfully
     *         - \ref Error::BUSY               if automatic trigger is active
     *         - \ref Error::INCOMPATIBLE_MODE  if the device is currently not
     *                                          in FMCW mode
     */
    Error get_fmcw_configuration(Fmcw_Configuration* configuration) const;

    /**
     * \brief This function returns the FMCW ramp speed in current
     *        configuration.
     *
     * This function writes the RF frequency change per second to an integer
     * variable. The variable must be provided by the user and may be
     * uninitialized.
     *
     * The unit of chirp duration value is MHz/s.
     *
     * While automatic trigger is active, reading the chirp duration may fail.
     *
     * \param[out] bandwidth_per_second_MHz_s  A pointer to a location, where the
     *                                         FMCW ramp speed value will be
     *                                         stored.
     *
     * \return An error code indicating if the function succeeded. The following
     *         error codes can occur:
     *         - \ref Error::OK       if a valid FMCW ramp speed value could be
     *                                returned
     *         - \ref Error::BUSY     if automatic trigger is active
     */
    Error get_bandwidth_per_second(uint32_t* bandwidth_per_second_MHz_s) const;

    /**
     * \brief This function modifies the ADC sampling rate.
     *
     * The function changes the sampling rate of the BGT6x devices built in
     * Analog/Digital Converter.
     *
     * The sampling rate is specified in Hz. The value affects the chirp duration.
     *
     * A value check is applied to the specified sampling rate. If the value is
     * out of range, the ADC sampling rate is not changed and an error is
     * returned.
     *
     * If the specified sampling rate is 0 the sampling rate will be set to the
     * highest possible value.
     *
     * For more detailed ADC configuration, the function
     * \ref set_adc_configuration can be used, but that function is only for
     * expert use.
     *
     * \note Internally the sample rate is rounded, to a value that can divided
     *       from the reference oscillator. It is recommended to read the sample
     *       rate back by calling \ref get_adc_samplerate.
     *
     * The sampling rate can not be changed while the automatic trigger is active.
     *
     * \param[in] samplerate_Hz  A pointer to the new ADC sampling rate to be
     *                           applied.
     *
     * \return An error code indicating if the function succeeded. The following
     *         error codes can occur:
     *         - \ref Error::OK                          if the ADC parameters
     *                                                   could be applied
     *         - \ref Error::BUSY                        if the automatic trigger
     *                                                   is active and the device
     *                                                   is busy with acquiring
     *                                                   radar data
     *         - \ref Error::SAMPLERATE_OUT_OF_RANGE     if the specified sampling
     *                                                   rate is not supported by
     *                                                   the ADC
     */
    Error set_adc_samplerate(uint32_t samplerate_Hz);

    /**
     * \brief This function returns the current ADC sampling rate.
     *
     * This function returns the current sampling rate of the BGT6x devices built
     * in Analog/Digital Converter. The value is returned through a variable that
     * must be provided by the user and may be uninitialized.
     *
     * While automatic trigger is active, reading the current frame format may
     * fail.
     *
     * \param[out] samplerate_Hz  A pointer to a location, where the current ADC
     *                            sampling rate will be stored.
     *
     * \return An error code indicating if the function succeeded. The following
     *         error codes can occur:
     *         - \ref Error::OK       if pConfiguration has been filled
     *                                successfully
     *         - \ref Error::BUSY     if automatic trigger is active
     */
    Error get_adc_samplerate(uint32_t* samplerate_Hz) const;

    /**
     * \brief This function changes the ADC configuration.
     *
     * The function changes the parameters of the BGT6x devices built in
     * Analog/Digital Converter. It is only for expert use. For changing the
     * sampling rate use \ref set_adc_samplerate.
     *
     * The sampling rate is specified in Hz. The value affects the chirp duration.
     *
     * A value check is applied to the specified parameters. If any of the values
     * is out of range, the ADC configuration is not changed and an error is
     * returned.
     *
     * If the specified sampling rate is 0 the sampling rate will be set to the
     * highest possible value.
     *
     * \note Internally the sample rate is rounded, to a value that can divided
     *       from the reference oscillator. It is recommended to read the sample
     *       rate back by calling \ref get_adc_samplerate.
     *
     * The sampling rate can not be changed while the automatic trigger is active.
     *
     * \param[in] configuration  A pointer to the new ADC parameters to be applied.
     *
     * \return An error code indicating if the function succeeded. The following
     *         error codes can occur:
     *         - \ref Error::OK                            if the ADC parameters
     *                                                     could be applied
     *         - \ref Error::BUSY                          if the automatic
     *                                                     trigger is active and
     *                                                     the device is busy with
     *                                                     acquiring radar data
     *         - \ref Error::SAMPLERATE_OUT_OF_RANGE       if the specified
     *                                                     sampling rate is not
     *                                                     supported by the ADC
     *         - \ref Error::UNDEFINED_TRACKING_MODE       if the specified ADC
     *                                                     tracking mode is not
     *                                                     supported by the ADC
     *         - \ref Error::UNDEFINED_ADC_SAMPLE_TIME     if the specified ADC
     *                                                     sampling time is not
     *                                                     supported by the ADC
     */
    Error set_adc_configuration(const Adc_Configuration* configuration);

    /**
     * \brief This function returns the current ADC configuration.
     *
     * This function returns the current parameters of the BGT6x devices built in
     * Analog/Digital Converter through a structure of type \ref Adc_Configuration.
     * The structure must be provided by the user and may be uninitialized.
     *
     * This function is only for expert use. To query just the sampling rate use
     * \ref get_adc_samplerate.
     *
     * While automatic trigger is active, reading the current frame format may
     * fail.
     *
     * \param[out] configuration  A pointer to a location, where the current ADC
     *                            parameters will be stored.
     *
     * \return An error code indicating if the function succeeded. The following
     *         error codes can occur:
     *         - \ref Error::OK       if pConfiguration has been filled
     *                                successfully
     *         - \ref Error::BUSY     if automatic trigger is active
     */
    Error get_adc_configuration(Adc_Configuration* configuration) const;

    /**
     * \brief This function changes the TX antenna mode.
     *
     * The function chooses which TX antennas to use during radar data frame
     * acquisition.
     *
     * If the specified TX antenna mode is undefined or not supported, the current
     * TX antenna mode will not be changed and an error is returned.
     *
     * The configuration can not be changed while the automatic trigger is active.
     *
     * \param[in] tx_mode  A pointer to the new TX antenna mode to be applied.
     *
     * \return An error code indicating if the function succeeded. The following
     *         error codes can occur:
     *         - \ref Error::OK                    if the ADC parameters could be
     *                                             applied
     *         - \ref Error::BUSY                  if the automatic trigger is
     *                                             active and the device is busy
     *                                             with acquiring radar data
     *         - \ref Error::UNSUPPORTED_TX_MODE   if the specified TX antenna
     *                                             mode is not defined
     */
    Error set_tx_mode(Tx_Mode tx_mode);

    /**
     * \brief This function returns the current TX antenna mode.
     *
     * This function returns which TX antennas are used during radar data frame
     * acquisition through an enumeration variable of type \ref Tx_Mode. The variable
     * must be provided by the user and may be uninitialized.
     *
     * While automatic trigger is active, reading the current frame format may
     * fail.
     *
     * \param[out] tx_mode  A pointer to a location, where the current TX antenna
     *                      mode will be stored.
     *
     * \return An error code indicating if the function succeeded. The following
     *         error codes can occur:
     *         - \ref Error::OK       if pConfiguration has been filled
     *                                successfully
     *         - \ref Error::BUSY     if automatic trigger is active
     */
    Error get_tx_mode(Tx_Mode* tx_mode) const;

    /**
     * \brief This function modifies the base band settings of the BGT6x device.
     *
     * The function changes the base band setting of all four RX channels
     * according to the provided structure of type \ref Baseband_Configuration.
     *
     * A value check is applied to each field of pConfiguration. If any gain
     * value is out of range, the base band parameters are not changed and an
     * error is returned. The high pass cutoff frequencies are rounded to the
     * nearest supported value.
     *
     * The configuration can not be changed while the automatic trigger is active.
     *
     * \param[in] configuration  A pointer to the new base band settings to be
     *                           applied.
     *
     * \return An error code indicating if the function succeeded. The following
     *         error codes can occur:
     *         - \ref Error::OK                      if the ADC parameters could
     *                                               be applied
     *         - \ref Error::BUSY                    if the automatic trigger is
     *                                               active and the device is busy
     *                                               with acquiring radar data
     *         - \ref Error::UNSUPPORTED_HP_GAIN     if any of the four high pass
     *                                               filter gain values is not
     *                                               defined.
     *         - \ref Error::UNSUPPORTED_VGA_GAIN    if any of the four gain
     *                                               adjustment values is not
     *                                               defined.
     */
    Error set_baseband_configuration(const Baseband_Configuration* configuration);

    /**
     * \brief This function returns the current base band settings of the
     *        BGT6x device.
     *
     * This function returns the current analog base band settings through a
     * structure of type \ref Baseband_Configuration. The variable must be provided
     * by the user and may be uninitialized.
     *
     * While automatic trigger is active, reading the current frame format may
     * fail.
     *
     * \param[out] configuration  A pointer to a location, where the current base
     *                            band settings will be stored.
     *
     * \return An error code indicating if the function succeeded. The following
     *         error codes can occur:
     *         - \ref Error::OK       if pConfiguration has been filled
     *                                successfully
     *         - \ref Error::BUSY     if automatic trigger is active
     */
    Error get_baseband_configuration(Baseband_Configuration* configuration) const;

    Error set_frame_definition(const Frame_Definition* frame_definition);
    Error get_frame_definition(Frame_Definition* frame_definition) const;

    Error select_shape_to_configure(uint8_t shape, bool down_chirp);
    Error get_selected_shape(uint8_t* shape, bool* down_chirp) const;

    Error set_chirp_timing(const Chirp_Timing* timing);
    Error get_chirp_timing(Chirp_Timing* timing) const;

    Error set_startup_timing(const Startup_Timing* timing);
    Error get_startup_timing(Startup_Timing* timing) const;

    Error set_chirp_end_delay(uint32_t delay_100ps);
    Error get_chirp_end_delay(uint32_t* delay_100ps) const;

    Error set_idle_configuration(const Power_Down_Configuration* configuration);
    Error get_idle_configuration(Power_Down_Configuration* configuration) const;

    Error set_deep_sleep_configuration(const Power_Down_Configuration* configuration);
    Error get_deep_sleep_configuration(Power_Down_Configuration* configuration) const;

    Error set_startup_delays(const Startup_Delays* delays);
    Error get_startup_delays(Startup_Delays* delays) const;

    Error set_anti_alias_filter_settings(const Anti_Alias_Filter_Settings* settings);
    Error get_anti_alias_filter_settings(Anti_Alias_Filter_Settings* settings) const;

    Error set_fifo_power_mode(Fifo_Power_Mode mode);
    Error get_fifo_power_mode(Fifo_Power_Mode* mode) const;

    Error set_pad_driver_mode(Pad_Driver_Mode mode);
    Error get_pad_driver_mode(Pad_Driver_Mode* mode) const;

    Error set_duty_cycle_correction(const Duty_Cycle_Correction_Settings* settings);
    Error get_duty_cycle_correction(Duty_Cycle_Correction_Settings* settings) const;

    Error set_pullup_resistor_configuration(const Pullup_Resistor_Configuration* configuration);
    Error get_pullup_resistor_configuration(Pullup_Resistor_Configuration* configuration) const;

    Error set_power_sens_delay(uint32_t delay_100ps);
    Error get_power_sens_delay(uint32_t* delay_100ps) const;

    Error set_power_sens_enabled(bool enabled);
    Error get_power_sens_enabled(bool* enabled) const;

    Error set_temperature_sens_enabled(bool enabled);
    Error get_temperature_sens_enabled(bool* enabled) const;

    Error set_oscillator_configuration(const Oscillator_Configuration* configuration);
    Error get_oscillator_configuration(Oscillator_Configuration* configuration) const;

    Error set_lo_doubler_output_power(Lo_Doubler_Output_Power power);
    Error get_lo_doubler_output_power(Lo_Doubler_Output_Power* power) const;

    /**
     * \brief This function returns the register set derived from the current
     *        driver parameters.
     */
    HW::RegisterSet get_device_configuration() const;

    /**
     * This method notifies the driver that the Avian device was triggered by
     * the application. Calling this method is only required if temperature or
     * TX power measurements between the trigger and the next reset.
     */
    void notify_trigger();

    Error set_slice_size(uint16_t num_samples);
    Error get_slice_size(uint16_t* num_samples) const;
    Error stop_and_reset_sequence();
    HW::Spi_Command_t get_burst_prefix() const;

    Error check_fifo_overflow(HW::IControlPort* port);

    Error enable_easy_mode(bool enable);
    Error is_in_easy_mode(bool* enable) const;
    void set_easy_mode_buffer_size(uint16_t num_samples);

    /**
     * \brief This function returns the minimum frame interval possible in current
     *        configuration.
     *
     * This function writes the minimum frame interval currently possible to an
     * integer variable. The variable must be provided by the user and may be
     * uninitialized.
     *
     * The unit of frame interval value is micro seconds.
     *
     * Passing a value to \ref set_automatic_frame_trigger that is smaller
     * than the returned one will result in an error.
     *
     * While automatic trigger is active, reading the minimum frame interval may
     * fail.
     *
     * \param[out] min_frame_interval_us  A pointer to a location, where the chirp
     *                                    duration value will be stored.
     *
     * \return An error code indicating if the function succeeded. The following
     *         error codes can occur:
     *         - \ref Error::OK       if a valid minimum frame interval value
     *                                could be returned
     *         - \ref Error::BUSY     if automatic trigger is active
     */
    Error get_min_frame_interval(uint32_t* min_frame_interval_us) const;

    /**
     * \brief This function starts or stops automatic radar frame trigger.
     *
     * This function starts the automatic frame trigger if uFrameInterval is
     * non-zero. If frame_interval_us is zero, the automatic trigger is stopped.
     *
     * While the automatic frame trigger active, an internal timer triggers the
     * acquisition of radar data frames with the given interval and stores the
     * acquired data in an internal FIFO memory.
     *
     * While the automatic trigger is active an internal frame counter is
     * increased each time the trigger interval elapses. The sensor may not
     * trigger a frame in case the internal FIFO memory has runs out of capacity.
     * The frame counter is increased anyway, so the user has the chance to detect
     * this data loss. The frame counter is reset each time this function is
     * called.
     *
     * When this function is called while the frame automatic trigger is active,
     * the trigger timer is stopped and restarted with the new frame interval.
     *
     * \param[in] frame_interval_us  The time interval between two consecutive
     *                               frames in micro seconds.
     *
     * \return An error code indicating if the function succeeded. The following
     *         error codes can occur:
     *         - \ref Error::OK                             if the a frame of
     *                                                      radar data could be
     *                                                      returned
     *         - \ref Error::UNSUPPORTED_FRAME_INTERVAL     if the internal timer
     *                                                      can't be configured to
     *                                                      the given frame
     *                                                      interval
     *         - \ref Error::BUSY                           if the device is not
     *                                                      ready for data
     *                                                      acquisition (maybe
     *                                                      it's in test mode)
     */
    Error set_automatic_frame_trigger(uint32_t frame_interval_us);

    bool is_automatic_frame_trigger_running() const;

    void get_frame_info(Frame_Info* frame_info) const;

    Error get_device_id(uint64_t* device_id) const;

    /**
     * \brief This function reads the current registers of the BGT6x chip.
     *
     * This function reads the current register values from the BGT6x chip
     * and writes them into the provided array pData. The capacity of pData must
     * be passed through pNumRegisters. This function returns the number of dumped
     * registers through pNumRegisters. Array elements beyond that number are not
     * modified by this function. Each array element contains the register number
     * in the upper 8 bit and the register value in the lower 24 bit.
     *
     * The register read out can not be performed while the automatic trigger is
     * active or while antenna test mode is active.
     *
     * \param[out]     register_data  A pointer to a location, where the current
     *                                register values will be stored.
     * \param[in, out] num_registers  A pointer to a variable that is initialized
     *                                with the capacity of register_data. The
     *                                variable is overwritten with the number of
     *                                dumped registers.
     *
     * \return An error code indicating if the function succeeded. The following
     *         error codes can occur:
     *         - \ref Error::OK       if requested calibration succeeded
     *         - \ref Error::BUSY     if the automatic trigger is active and the
     *                                device is busy with acquiring radar data or
     *                                antenna test mode is active
     */
    Error dump_registers(uint32_t* register_data, uint8_t* num_registers) const;
    Error repeat_chip_setup();

    Error set_register_modification(uint8_t register_address, uint32_t and_mask, uint32_t or_mask);
    Error get_register_modification(uint8_t register_address, uint32_t* and_mask, uint32_t* or_mask) const;
    Error clear_all_register_modifications();

    /**
     * \brief This method returns the device type the driver instance has been
     *        created for.
     */
    Device_Type get_device_type() const;

    /**
     * \brief This method returns the SPI command that configures the reference
     *        clock of the Avian device.
     *
     * This method may return 0 for Avian C devices that don't have any clock
     * configuration bit fields.
     *
     * It is recommended not to not send the returned command to the Avian
     * device directly, but use the utility function
     * \ref initialize_reference_clock, because that function will also take
     * care for to do the oscillator startup sequence if needed.
     */
    HW::Spi_Command_t get_clock_config_command() const;

private:
    using Scaled_Timer = Value_Conversion::Scaled_Timer;

    /*
     * \brief This type holds all bit fields of registers PLLx_0 to PLLx_6.
     */
    struct PLL_Bitfield_Set
    {
        uint32_t fsu;
        uint32_t rsu;
        uint32_t rtu;
        uint32_t tedu;
        uint32_t apu;
        uint32_t fsd;
        uint32_t rsd;
        uint32_t rtd;
        uint32_t tedd;
        uint32_t apd;
    };

    struct Shape_Settings
    {
        Shape_Type shape_type;
        uint32_t lower_frequency_kHz;
        uint32_t upper_frequency_kHz;
        uint16_t num_samples_up;
        uint8_t chirp_end_delay_up_reg;
        uint16_t num_samples_down;
        uint8_t chirp_end_delay_down_reg;
        uint16_t num_repetitions;
        Power_Mode following_power_mode;
        Scaled_Timer post_delay;
    };

    struct Channel_Set_Settings
    {
        uint8_t rx_mask;
        uint8_t num_rx_antennas;
        uint8_t tx_power;
        Tx_Mode tx_mode;
        Hp_Gain hp_gain_1;
        uint8_t hp_cutoff_1_reg;
        Vga_Gain vga_gain_1;
        uint8_t aaf_cutoff_1_reg;
        Hp_Gain hp_gain_2;
        uint8_t hp_cutoff_2_reg;
        Vga_Gain vga_gain_2;
        uint8_t aaf_cutoff_2_reg;
        Hp_Gain hp_gain_3;
        uint8_t hp_cutoff_3_reg;
        Vga_Gain vga_gain_3;
        uint8_t aaf_cutoff_3_reg;
        Hp_Gain hp_gain_4;
        uint8_t hp_cutoff_4_reg;
        Vga_Gain vga_gain_4;
        uint8_t aaf_cutoff_4_reg;
        uint32_t reset_period_reg;
        uint8_t lo_doubler_power_reg;
    };

    HW::IControlPort& m_port;
    Device_Type m_device_type;
    uint8_t m_pll_div_set;
    uint32_t m_reference_clock_freq_Hz;
    bool m_enable_frequency_doubler;

    uint16_t m_adc_sample_rate_divider;
    Adc_Sample_Time m_adc_sample_time;
    Adc_Tracking m_adc_tracking;
    bool m_adc_double_msb_time;
    Adc_Oversampling m_adc_oversampling;
    uint16_t m_pre_chirp_delay_reg;
    uint16_t m_post_chirp_delay_reg;
    uint16_t m_pa_delay_reg;
    uint8_t m_adc_delay_reg;
    Scaled_Timer m_time_wake_up;
    Scaled_Timer m_time_init0;
    Scaled_Timer m_time_init1;
    Power_Down_Configuration m_idle_settings;
    Power_Down_Configuration m_deep_sleep_settings;

    uint8_t m_currently_selected_shape;
    Shape_Settings m_shape[4];
    Channel_Set_Settings m_channel_set[8];
    uint16_t m_num_set_repetitions;
    Power_Mode m_frame_end_power_mode;
    Scaled_Timer m_frame_end_delay;
    uint16_t m_num_frames_before_stop;

    Fifo_Power_Mode m_fifo_power_mode;
    Pad_Driver_Mode m_pad_driver_mode;

    uint8_t m_bandgap_delay_reg;
    uint8_t m_madc_delay_reg;
    uint8_t m_pll_enable_delay_reg;
    uint8_t m_pll_divider_delay_reg;
    Duty_Cycle_Correction_Settings m_dc_correction;
    Pullup_Resistor_Configuration m_pullup_configuration;
    Oscillator_Configuration m_oscillator_configuration;

    uint8_t m_power_sens_delay_reg;
    bool m_power_sensing_enabled;
    bool m_temperature_sensing_enabled;

    uint16_t m_slice_size;
    uint16_t m_easy_mode_buffer_size;

    bool m_reset_state;

    enum Mode_e
    {
        MODE_NORMAL = 0x00,
        MODE_EASY = 0x02,
        MODE_EASY_TRIGGER_RUNNING = 0x03
    };
    Mode_e m_current_mode;

    HW::RegisterSet m_current_configuration;

    int32_t m_tx_power[8][2];

    struct Register_Modification
    {
        uint32_t and_mask;
        uint32_t or_mask;
    };
    std::map<uint8_t, Register_Modification> m_reg_modifications;

    /**
     * \brief This function calculates the total supported frequency range.
     *
     * This function calculates the maximum programmable frequency range according
     * to reference oscillator frequency and PACR2:DIV_SET bit field.
     */
    void get_frequency_range(uint32_t* min_freq_kHz, uint32_t* max_freq_kHz) const;

    /**
     * \brief This function calculates the time the MADC needs to acquire a single
     *        value
     *
     * \param[in] config  The MADC parameters that have an impact on acquisition
     *                    time.
     *
     * \return The total time for sampling and conversion is returned in clock
     *         cycles.
     */
    static uint16_t get_madc_acquisition_cycles(const Adc_Configuration* config);

    /**
     * \brief This function checks all timing constraints related to power sensing
     *
     * \return If timing constraints are met 0 is returned, otherwise an error
     *         code is returned.
     */
    Error check_power_sens_timing();

    /**
     * \brief This function checks all timing constraints related to temperature
     *        sensing
     *
     * \return If timing constraints are met 0 is returned, otherwise an error
     *         code is returned.
     */
    Error check_temperature_sens_timing();

    /**
     * \brief This function sets the default configuration of the drive.
     *
     * Whenever a new driver instance is created, this function is called to
     * set the parameters.
     */
    void set_default_parameters();

    /**
     * \brief This function counts the set bits in a given bit mask
     */
    static uint8_t count_antennas(uint8_t rx_mask);

    /**
     * \brief This function updates the internally cached SPI configuration.
     *
     * This function sets up a complete SPI programming sequence taking all
     * current parameters into account. The resulting programming sequence is
     * compared to the sequence currently programmed to the BGT60TRxx device.
     * Register changes are scheduled to be sent to the chip.
     */
    void update_spi_register_set();

    /**
     * \brief This function reset the BGT60TR24 chip.
     *
     * If \par soft_reset is 0, this function generates a hardware reset condition
     * for the BGT60TR24 chip. The chip will loose all configuration settings and
     * need full reprogramming afterwards.
     *
     * If \par soft_reset is non-zero, this function just triggers an FSM and FIFO
     * reset in the BGT60TRxx. All configuration settings are kept.
     *
     * In both cases the software FIFO pointers of the driver are reset.
     */
    void reset(bool soft_reset);

    /**
     * \brief This function calculates bit fields for BGT60TRxx
     *
     * The function calculates all bit fields of SPI registers PLLx_0 ... PLLx_6
     * from the provided shape settings. It also takes the global chirp timings of
     * the provided driver instance into account.
     *
     * \param[in] shape_settings  The shape settings to be converted to bit fields.
     * \param[out] bitfields      The calculated bit-fields are returned trough this
     *                            pointer.
     */
    void setup_pll_bitfields(const Shape_Settings* shape_settings,
                             PLL_Bitfield_Set* bitfields);

    /**
     * \brief This function prepares SPI programming of the MAIN register.
     *
     * The parameter settings of the provided driver instance are compiled into an
     * SPI sequence to program the MAIN register. That sequence is then cached in
     * the provided driver instance to be sent at a later time.
     */
    void program_registers_main();

    /**
     * \brief This function prepares SPI programming of the MADC register.
     *
     * The parameter settings of the provided driver instance are compiled into an
     * SPI sequence to program the ADC0 register. That sequence is then cached in
     * the provided driver instance to be sent at a later time.
     */
    void program_registers_madc();

    /**
     * \brief This function prepares SPI programming of the SADC register.
     *
     * The parameter settings of the provided driver instance are compiled into an
     * SPI sequence to program the SADC_CTRL register. That sequence is then
     * cached in the provided driver instance to be sent at a later time.
     *
     * Shape settings are taken into account to select the accuracy of the SADC.
     */
    void program_registers_sadc();

    /**
     * \brief This function prepares SPI programming of the SFCTL register.
     *
     * The parameter settings of the provided driver instance are compiled into an
     * SPI sequence to program the SFCTL register. That sequence is then cached in
     * the provided driver instance to be sent at a later time.
     */
    void program_registers_fifo();

    /**
     * \brief This function prepares SPI programming of the chirp control
     *        registers.
     *
     * The parameter settings of the provided driver instance are compiled into an
     * SPI sequence to program chirp control registers CCR0...CCR3. That sequence
     * is then cached in the  provided driver instance to be sent at a later time.
     */
    void program_registers_ccr();

    /**
     * \brief This function prepares SPI programming of the frame definition
     *        registers
     *
     * The parameter settings of the provided driver instance are compiled into an
     * SPI sequence to program shape register PLLx_7 and channel set register
     * CSCx. That sequence is then cached in the  provided driver instance to be
     * sent at a later time.
     */
    void program_registers_frame();

    /**
     * \brief This function prepares SPI programming of the shape registers.
     *
     * The parameter settings of the provided driver instance are compiled into an
     * SPI sequence to program shape registers PLLx_0...PLLx_6. That sequence is
     * then cached in the  provided driver instance to be sent at a later time.
     *
     * Shape registers are programmed for both, up chirp and down chirp.
     *
     * \param[in] shape_index  The index of the shape to be programmed. This value
     *                         is used to setup register addresses in the SPI
     *                         sequence. This value must be in range 0...3.
     */
    void program_registers_shape(uint8_t shape_index);

    /**
     * \brief This function prepares SPI programming of the channel set registers.
     *
     * The parameter settings of the provided driver instance are compiled into an
     * SPI sequence to program channel set registers CSCx_U_0...CSCx_U_2 or
     * CSCx_D_0...CSCx_D_2. That sequence is then cached in the  provided driver
     * instance to be sent at a later time.
     *
     * Channel set registers are programmed for either up chirp or down chirp,
     * according to the target_index. If shape_index is even the up chirp is
     * programmed, if shape_index is odd, the down chirp is programmed. Bits 1&2
     * of shape_index must contain the shape number.
     *
     * The SPI sequence is written to the provided buffer.
     *
     * \param[in] source_index  The index of the channel set parameters in the
     *                          settings array of the driver state.
     * \param[in] target_index  The index of the channel set to be programmed.
     *                          This value is used to setup register addresses in
     *                          the SPI sequence. This value must be in the
     *                          range 0...7.
     */
    void program_registers_channel_set(uint8_t source_index, uint8_t target_index);

    /**
     * \brief This function prepares SPI programming of the idle or deep sleep
     *        registers.
     *
     * The parameter settings of the provided driver instance are compiled into an
     * SPI sequence to program idle registers CSP_I_0...CSP_I_2 and CSCI or deep sleep
     * registers CSP_D_0...CSP_D_2 and CSCDS. That sequence is then cached in the
     * provided driver instance to be sent at a later time.
     *
     * The SPI sequence is written to the provided buffer.
     *
     * \param[in] deep_sleep  If this is 0, programming sequence for idle mode is
     *                        generated. If this is 1, the sequence is generated
     *                        for the deep sleep mode.
     */
    void program_registers_power(bool deep_sleep);

    /**
     * \brief This function prepares SPI programming of the FD register for
     *        frequency double configuration.
     *
     * The parameter settings of the provided driver instance are compiled into an
     * SPI sequence to program the RFT0 register. That sequence is then cached in
     * the provided driver instance to be sent at a later time.
     */
    void program_registers_clock();

    /**
     * \brief This function applies easy mode settings.
     *
     * For easy mode number of chirps per frame, chirp direction and tx_mode must
     * be taken into account to setup the proper shape sequence, so for this
     * case the work is put into a separate function.
     */
    Error setup_easy_mode_frame(const Frame_Format* frame_format,
                                const Fmcw_Configuration* fmcw_settings,
                                Tx_Mode tx_mode);
};

/* ------------------------------------------------------------------------ */
}  // namespace Avian
}  // namespace Infineon

#endif /* IFX_AVIAN_DRIVER_H */

/* --- End of File -------------------------------------------------------- */
