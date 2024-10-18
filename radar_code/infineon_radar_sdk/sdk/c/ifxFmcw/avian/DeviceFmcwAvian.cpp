/* ===========================================================================
** Copyright (C) 2021 - 2022 Infineon Technologies AG
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

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include "DeviceFmcwAvian.hpp"
#include "ifxBase/Exception.hpp"


// libAvian
#include "ifxAvian_DeviceTraits.hpp"
#include "ifxAvian_ParameterExtractor.hpp"
#include "ifxAvian_StrataUtilities.hpp"
#include "ifxAvian_TimingModel.hpp"
#include "ifxAvian_Types.hpp"
#include "ports/ifxAvian_DummyPort.hpp"
#include "ports/ifxAvian_StrataControlPort.hpp"


// Strata
#include <platform/exception/EConnection.hpp>
#include <platform/exception/EProtocolFunction.hpp>
#include <platform/NamedMemory.hpp>
#include <universal/error_definitions.h>

#include <cmath>  // for std::round
#include <numeric>


/*
==============================================================================
   2. LOCAL DEFINITIONS
==============================================================================
*/

using namespace Infineon::Avian;

namespace {

// Amplification factor that is applied to the IF signal before sampling; Valid range: [18-60]dB
constexpr int8_t IFX_IF_GAIN_DB_LOWER_LIMIT = 18;
constexpr int8_t IFX_IF_GAIN_DB_UPPER_LIMIT = 60;

constexpr float MIN_ADC_SAMPLING_RATE_HZ = 78.201e3f;
constexpr float MAX_ADC_SAMPLING_RATE_HZ = 4e6f;

constexpr float MAX_ADC_VALUE = 4095.0f;              // 2**12 - 1

constexpr uint32_t MAX_NUM_SAMPLES_PER_CHIRP = 4095;  // 2**12 - 1

constexpr uint8_t data_format = DataFormat_Packed12;  // default data format

}  // namespace

/*
==============================================================================
   3. LOCAL TYPES
==============================================================================
*/

/*
==============================================================================
   4. FUNCTION PROTOTYPES
==============================================================================
*/

/*
==============================================================================
   5. LOCAL FUNCTION PROTOTYPES
==============================================================================
*/
using namespace Infineon;

namespace {

inline ifx_Error_t translate_error_code(Avian::Driver::Error error_code)
{
    using Avian::Driver;

    switch (error_code)
    {
        case Driver::Error::OK:
            return IFX_OK;

        case Driver::Error::BUSY:
            return IFX_ERROR_DEVICE_BUSY;

        case Driver::Error::UNSUPPORTED_FRAME_FORMAT:
            /* ep_radar_base_set_frame_format could return this. If this happens the only reason could
             * be an invalid number of samples, because all other parameters are known not to be
             * critical.
             */
            return IFX_ERROR_NUM_SAMPLES_OUT_OF_RANGE;

        case Driver::Error::ANTENNA_DOES_NOT_EXIST:
            /* ep_radar_base_set_frame_format returns this code to indicate a bad RX mask. */
            return IFX_ERROR_RX_ANTENNA_COMBINATION_NOT_ALLOWED;

        case Driver::Error::UNSUPPORTED_TX_MODE:
            /* ep_radar_bgt6x_set_tx_mode returns this code if specified TX mode is not
             * supported by the device.*/
            return IFX_ERROR_TX_ANTENNA_MODE_NOT_ALLOWED;

        case Driver::Error::SAMPLERATE_OUT_OF_RANGE:
            /* ep_radar_bgt6x_set_adc_samplerate can return this code. */
            return IFX_ERROR_SAMPLERATE_OUT_OF_RANGE;

        case Driver::Error::FREQUENCY_OUT_OF_RANGE:
            /* ep_radar_fmcw_set_fmcw_configuration can return this code. */
            return IFX_ERROR_RF_OUT_OF_RANGE;

        case Driver::Error::POWER_OUT_OF_RANGE:
            /* ep_radar_fmcw_set_fmcw_configuration can return this code. */
            return IFX_ERROR_TX_POWER_OUT_OF_RANGE;

        case Driver::Error::UNSUPPORTED_NUM_REPETITIONS:
            /* The number of chirps per frame is set as number of set repetitions to
             * ep_radar_bgt60trxx_set_frame_definition.
             */
            return IFX_ERROR_NUM_CHIRPS_NOT_ALLOWED;

        case Driver::Error::UNSUPPORTED_FIFO_SLICE_SIZE:
            /* ep_radar_bgt60trxx_set_data_slice_size returns this if the slice size is not a multiple
             * of 4.
             */
            return IFX_ERROR_FRAME_SIZE_NOT_SUPPORTED;

        case Driver::Error::NUM_FRAMES_OUT_OF_RANGE:
            /* returns this if number of frames too high.
             */
            return IFX_ERROR_NUM_FRAMES_OUT_OF_RANGE;

        case Driver::Error::SENSOR_DOES_NOT_EXIST:
            /* The requested sensor does not exist, hence not possible.*/
            return IFX_ERROR_NOT_POSSIBLE;

        case Driver::Error::INCOMPATIBLE_MODE:
            /* The requested operation is not supported by the currently active mode of operation*/
            return IFX_ERROR_NOT_SUPPORTED;

        default:
            /* If none of the above error code is returned this can have different reasons:
             * 1) Endpoint 0 or comm lib itself returned some errors indicating invalid protocol
             *    messages.
             * 2) It's known from BGT60TRxx driver code, that the received error code cannot be sent by
             *    the function calls used above. If this happens this must be a "misunderstanding"
             *    between host and radar device, so it is also a communication error.
             * 3) It's known from BGT60TRxx driver code, that the received error code can be sent by
             *    any of the function calls used above, but the sent parameters are known to be good,
             *    so the error should not occur. Again this is considered to be a communication error.
             *    Those known error codes are:
             *    - from ep_radar_bgt6x_set_baseband_configuration
             *        EP_RADAR_ERR_UNSUPPORTED_HP_GAIN
             *        EP_RADAR_ERR_UNSUPPORTED_HP_CUTOFF
             *        EP_RADAR_ERR_UNSUPPORTED_VGA_GAIN
             *        EP_RADAR_ERR_RESET_TIMER_OUT_OF_RANGE
             *    - from ep_radar_bgt6x_set_adc_samplerate
             *        EP_RADAR_ERR_UNDEFINED_TRACKING_MODE
             *        EP_RADAR_ERR_UNDEFINED_ADC_SAMPLE_TIME
             *    - from ep_radar_fmcw_set_fmcw_configuration
             *        EP_RADAR_ERR_UNSUPPORTED_DIRECTION
             *    - from ep_radar_bgt60trxx_set_frame_definition
             *        EP_RADAR_ERR_UNSUPPORTED_POWER_MODE
             *        EP_RADAR_ERR_NONCONTINUOUS_SHAPE_SEQUENCE
             *        EP_RADAR_ERR_NUM_FRAMES_OUT_OF_RANGE
             *        EP_RADAR_ERR_POST_DELAY_OUT_OF_RANGE (post delay can go up to 7h!)
             *    - from ep_radar_bgt60trxx_trigger_sequence
             *        EP_RADAR_ERR_INCOMPATIBLE_MODE
             *        EP_RADAR_ERR_NO_MEMORY
             */
            return IFX_ERROR_INTERNAL;
    }
}

void check_libavian_return(Avian::Driver::Error error_code)
{
    if (error_code == Avian::Driver::Error::OK)
    {
        return;
    }

    throw rdk::exception::error::exception(translate_error_code(error_code));
}

Tx_Mode tx_mask_to_tx_mode(const uint32_t tx_mask)
{
    switch (tx_mask)
    {
        case 0:
            // disable all TX antennas
            return Avian::Tx_Mode::Off;
        case 1:
            // enable first TX antenna
            return Avian::Tx_Mode::Tx1_Only;
        case 2:
            // enable second TX antenna
            return Avian::Tx_Mode::Tx2_Only;
        case 3:
            // enable both TX antenna (TX1 and TX2)'
            // Despite the name "Alternating" means both TX antennas are activated
            return Avian::Tx_Mode::Alternating;
            break;
        default:
            throw rdk::exception::tx_antenna_mode_not_allowed();
    }
}

uint32_t tx_mode_to_tx_mask(const Avian::Tx_Mode tx_mode)
{
    switch (tx_mode)
    {
        case Avian::Tx_Mode::Tx1_Only:
            return 1;
        case Avian::Tx_Mode::Tx2_Only:
            return 2;
        case Avian::Tx_Mode::Alternating:
            return 3;
        case Avian::Tx_Mode::Off:
        default:
            return 0;
    }
}

}  // namespace

DeviceFmcwAvian::DeviceFmcwAvian(std::unique_ptr<BoardInstance>&& board) :
    DeviceFmcwBase(MAX_ADC_VALUE, std::move(board))
{
    // Checks internally that we are really connected to a board with an Avian sensor
    m_port = std::make_unique<StrataControlPort>(m_board.get());

    m_driver = Driver::create_driver(*m_port);
    if (!m_driver)
    {
        throw rdk::exception::device_not_supported();
    }

    detect_reference_clock();

    /*
     * Temperature measurement during acquisition is not allowed for BGT60UTR11AIP.
     * Even though under some circumstances it is possible, but there are some
     * cases that do not work. For simplicity the measurement is disabled for
     * all cases.
     */
    if (m_driver->get_device_type() == Device_Type::BGT60UTR11AIP)
    {
        m_driver->set_temperature_sens_enabled(false);
    }

    DeviceFmcwAvian::initialize_sensor_info();
    DeviceFmcwAvian::generate_register_list();
}

DeviceFmcwAvian::DeviceFmcwAvian(ifx_Radar_Sensor_t device_type, float reference_clock) :
    DeviceFmcwBase(MAX_ADC_VALUE)
{
    m_port = std::make_unique<DummyPort>();
    m_driver = std::make_unique<Driver>(*m_port, static_cast<Device_Type>(device_type));
    if (reference_clock != 80e6f)
    {
        set_reference_clock(reference_clock);
    }

    /*
     * Temperature measurement during acquisition is not allowed for BGT60UTR11AIP.
     * Even though under some circumstances it is possible, but there are some
     * cases that do not work. For simplicity the measurement is disabled for
     * all cases.
     */
    if (m_driver->get_device_type() == Device_Type::BGT60UTR11AIP)
    {
        m_driver->set_temperature_sens_enabled(false);
    }

    DeviceFmcwAvian::initialize_sensor_info();
    DeviceFmcwAvian::generate_register_list();
}

DeviceFmcwAvian::DeviceFmcwAvian(const DeviceFmcwAvian& other) :
    DeviceFmcwBase(MAX_ADC_VALUE)  // NOLINT(readability-redundant-member-init)
{
    m_port = std::make_unique<DummyPort>();
    m_driver = std::make_unique<Driver>(*m_port, *other.m_driver);

    DeviceFmcwAvian::initialize_sensor_info();
    DeviceFmcwAvian::generate_register_list();
}

DeviceFmcwAvian::~DeviceFmcwAvian()
{
    try
    {
        DeviceFmcwAvian::stop_acquisition();
    }
    catch (...)
    {
        // It might happen that stop_acquisition throws an exception in case
        // the device is no longer present.
        //
        // As a destructor must not throw exceptions, we ignore the exception here.
        //
        // Anyhow, if the device is longer present, it is also not necessary to
        // stop the acquisition.
    }
}

void DeviceFmcwAvian::stop_acquisition()
{
    if (!m_data_started)
    {
        return;
    }

    m_data_started = false;

    // check if dummy device
    if (!m_board)
    {
        return;
    }

    /*
     * The acquisition is stopped to make sure no more data is
     * produced. Stopping data forwarding is the second step.
     */
    stop_data();

    // Stop and Reset sequence
    const auto rc = m_driver->stop_and_reset_sequence();
    check_libavian_return(rc);
}

void DeviceFmcwAvian::initialize_sensor_info()
{
    Device_Info avian_device_info;
    const auto rc = m_driver->get_device_info(&avian_device_info);
    check_libavian_return(rc);

    m_sensor_info.description = avian_device_info.description;
    m_sensor_info.min_rf_frequency_Hz = 1e3 * static_cast<double>(avian_device_info.min_rf_frequency_kHz);
    m_sensor_info.max_rf_frequency_Hz = 1e3 * static_cast<double>(avian_device_info.max_rf_frequency_kHz);
    m_sensor_info.max_num_samples_per_chirp = MAX_NUM_SAMPLES_PER_CHIRP;
    m_sensor_info.min_adc_sampling_rate = MIN_ADC_SAMPLING_RATE_HZ;
    m_sensor_info.max_adc_sampling_rate = MAX_ADC_SAMPLING_RATE_HZ;
    m_sensor_info.num_tx_antennas = avian_device_info.num_tx_antennas;
    m_sensor_info.num_rx_antennas = avian_device_info.num_rx_antennas;
    m_sensor_info.max_tx_power = avian_device_info.max_tx_power;
    m_sensor_info.adc_resolution_bits = 12;
    m_sensor_info.device_id = 0;

    auto sensor_type = m_driver->get_device_type();
    const auto& device_traits = Avian::Device_Traits::get(sensor_type);

    m_sensor_info.hp_cutoff_list = device_traits.hpf_cutoff_settings.data();
    m_sensor_info.lp_cutoff_list = device_traits.aaf_cutoff_settings.data();

    for (const auto& hp_gain : device_traits.hp_gain_settings)
    {
        if (hp_gain == -1)
        {
            break;
        }
        for (const auto& vga_gain : device_traits.vga_gain_settings)
        {
            if (vga_gain == -1)
            {
                break;
            }
            m_if_gain_list.emplace_back(hp_gain + vga_gain);
        }
    }
    std::sort(m_if_gain_list.begin(), m_if_gain_list.end());
    m_if_gain_list.push_back(-1);
    m_sensor_info.if_gain_list = m_if_gain_list.data();
}

ifx_Radar_Sensor_t DeviceFmcwAvian::get_sensor_type() const
{
    return static_cast<ifx_Radar_Sensor_t>(m_driver->get_device_type());
}

float DeviceFmcwAvian::get_temperature()
{
    // Reading temperature from a BGT60UTR11AIP is not supported:
    // With BGT60UTR11AIP you can measure the temperature when the chip is off,
    // in CW mode, or when the chip is operating with a triangular shape.
    // During saw tooth shapes temperature measurement is not supported.
    // To avoid any problems we set the error IFX_ERROR_NOT_SUPPORTED
    // when reading the temperature with a BGT60UTR11AIP.
    if (m_driver->get_device_type() == Avian::Device_Type::BGT60UTR11AIP)
    {
        throw rdk::exception::not_supported();
    }

    // Do not read the temperature from the radar sensor too often as it
    // decreases performance (negative impact on data rate), might cause
    // problems as fetching temperature takes some time, and it is not
    // required (the temperature hardly changes within 100ms).
    //
    // If the cached temperature value is not yet expired, we simply use the
    // cached value instead of reading a new value from the radar sensor.
    auto now = std::chrono::steady_clock::now();
    if (now < m_temperature_expiration_time)
    {
        // Returning a cached value is not an error, so don't throw an
        // exception here but merely return from this method.
        return m_temperature_value;
    }

    // temperature in units of 0.001 degree Celsius
    int32_t temp;
    const auto rc = m_driver->get_temperature(&temp);
    check_libavian_return(rc);

    // Save the current temperature value and get a new temperature value
    // from the radar sensor earliest in 100ms again.
    m_temperature_value = temp * 1e-3f;
    m_temperature_expiration_time = now + std::chrono::milliseconds(100);
    return m_temperature_value;
}

void DeviceFmcwAvian::start_acquisition()
{
    if (m_data_started)
    {
        /*
         * Before changing the configuration, any ongoing acquisition must be stopped.
         * However, we simply return anyways if the acquisition is already started.
         */
        return;
    }

    // check if dummy device
    if (!m_board)
    {
        throw rdk::exception::not_supported();
    }

    update_defaults_if_not_configured();

    /* get the device FIFO size in samples
     * The unit used by Device_Traits corresponds to pairs of samples,
     * therefore the number of samples is obtained by multiplying by two.
     */
    const auto device_type = m_driver->get_device_type();
    const auto& device_traits = Avian::Device_Traits::get(device_type);
    const uint32_t fifo_count = static_cast<uint32_t>(device_traits.fifo_size) * 2;

    const auto slice_size = calculate_slice_size(fifo_count);
    const auto rc = m_driver->set_slice_size(slice_size);
    check_libavian_return(rc);

    const uint16_t readout_address = m_driver->get_burst_prefix() & 0xFF;
    configure_data(slice_size, readout_address, data_format);
    start_data();

    // Data reading is active now, but the Avian device must be triggered, too.
    m_driver->get_device_configuration().send_to_device(*m_port, true);
    m_driver->notify_trigger();

    m_data_started = true;
}

void DeviceFmcwAvian::set_acquisition_sequence(const ifx_Fmcw_Sequence_Element_t* sequence)
{
    using namespace Avian;

    if (sequence == nullptr)
    {
        throw rdk::exception::argument_null();
    }

    /*
     * A local copy of the driver allows to change parameters and drop them in
     * case of an error. Only when no exception is thrown and no error occurs,
     * the local driver with the new parameters is swapped with the old one.
     */
    auto local_driver = std::make_unique<Driver>(*m_driver);

    /*
     * The frame definition structure will be filled during parsing of the
     * acquisition sequence tree.
     */
    Frame_Definition frame_definition =
        {
            {{0, Power_Mode::Stay_Active, 0},
             {0, Power_Mode::Stay_Active, 0},
             {0, Power_Mode::Stay_Active, 0},
             {0, Power_Mode::Stay_Active, 0}},
            {0, Power_Mode::Stay_Active, 0},
            0};

    /*
     * It is expected that the top level sequence element is the frame loop.
     * The frame loop must not be empty, because an empty loop does not make any
     * sense. The frame loop must not have a following element, because the end
     * of the frame loop is the end of acquisition.
     */
    if ((sequence->type != IFX_SEQ_LOOP) || (sequence->next_element != nullptr)
        || (sequence->loop.sub_sequence == nullptr))
    {
        throw rdk::exception::argument_invalid();
    }
    frame_definition.num_frames = sequence->loop.num_repetitions;
    const auto frame_repetition_time = sequence->loop.repetition_time_s;
    sequence = sequence->loop.sub_sequence;

    /*
     * The Avian state machine allows two loop levels within a frame, the loop
     * over the shape set and a loop over each shape. The shape loop is more
     * flexible, because it allows numbers of repetition that are non powers of
     * two and way longer repetition times. On the other hand the shape set
     * loop cannot have a following element, because all shapes are inside the
     * shape set.
     * The top level loop within a frame is mapped to the shape set loop if
     * possible. If the provided sequence uses nested loops, this is anyhow the
     * right choice. If the provided sequence just uses a simple loop over
     * chirps, mapping that to the shape set loop is preferable due to the
     * flexibility of parameters.
     * If the first element within the frame cannot be mapped to the shape set
     * loop, a dummy loop is inserted to meet the constraints of the Avian
     * state machine.
     */
    float shape_set_repetition_time;

    if ((sequence->type == IFX_SEQ_LOOP) && (sequence->next_element == nullptr)
        && (sequence->loop.sub_sequence != nullptr))
    {
        frame_definition.shape_set.num_repetitions = sequence->loop.num_repetitions;
        shape_set_repetition_time = sequence->loop.repetition_time_s;
        sequence = sequence->loop.sub_sequence;
    }
    else
    {
        frame_definition.shape_set.num_repetitions = 1;
        shape_set_repetition_time = 0;
    }

    /*
     * Below the looped sequence is evaluated whenever there is chirp element,
     * these helper lambda function applies the chirp parameters to the driver.
     * The RF range is not set, because that's a shape setting that may apply
     * to two chirps.
     */
    float current_sampling_rate = 0;
    auto setup_chirp = [&](uint8_t shape, bool down,
                           const ifx_Fmcw_Sequence_Chirp_t& chirp) {
        auto rc = local_driver->select_shape_to_configure(shape, down);
        check_libavian_return(rc);

        // Acquisition without RX antennas does not make any sense.
        if (chirp.rx_mask == 0)
            throw rdk::exception::rx_antenna_combination_not_allowed();

        Frame_Format frame_format =
            {
                /* .num_samples_per_chirp = */ chirp.num_samples,
                /* .num_chirps_per_frame = */ 1,
                /* .rx_mask = */ static_cast<uint8_t>(chirp.rx_mask)};
        rc = local_driver->set_frame_format(&frame_format);
        check_libavian_return(rc);

        rc = local_driver->set_tx_mode(tx_mask_to_tx_mode(chirp.tx_mask));
        check_libavian_return(rc);

        /*
         * IF gain is split into HP gain and VGA gain. HP Gain is chosen as
         * high as possible and VGA gain as low as possible to minimize noise.
         */
        const auto& if_gain_dB = chirp.if_gain_dB;
        if ((if_gain_dB < IFX_IF_GAIN_DB_LOWER_LIMIT) || (if_gain_dB > IFX_IF_GAIN_DB_UPPER_LIMIT))
            throw rdk::exception::error::exception(IFX_ERROR_IF_GAIN_OUT_OF_RANGE);

        const auto hp_gain = (if_gain_dB >= 30) ? Hp_Gain::_30dB
                                                : Hp_Gain::_18dB;

        // VGA gain can be 0dB, 5dB, 10dB, ..., 30dB.
        // Choose vga_gain such that hp_gain + vga_gain is as close to if_gain_dB
        // as possible.
        const int gain_left_dB = if_gain_dB - ((hp_gain == Hp_Gain::_30dB) ? 30 : 18);
        const div_t div = std::div(gain_left_dB, 5);                     // 5 because of the steps of 5dB
        const auto vga_gain = (div.rem >= 3)
                                  ? static_cast<Vga_Gain>(div.quot + 1)  // round up (hp_gain+vga_gain > if_gain_dB)
                                  : static_cast<Vga_Gain>(div.quot);     // round down (hp_gain+vga_gain <= if_gain_dB)

        Baseband_Configuration baseband_config =
            {
                /* .hp_gain_1 = */ hp_gain,
                /* .hp_cutoff_1_Hz = */ chirp.hp_cutoff_Hz,
                /* .vga_gain_1 = */ vga_gain,
                /* .hp_gain_2 = */ hp_gain,
                /* .hp_cutoff_2_Hz = */ chirp.hp_cutoff_Hz,
                /* .vga_gain_2 = */ vga_gain,
                /* .hp_gain_3 = */ hp_gain,
                /* .hp_cutoff_3_Hz = */ chirp.hp_cutoff_Hz,
                /* .vga_gain_3 = */ vga_gain,
                /* .hp_gain_4 = */ hp_gain,
                /* .hp_cutoff_4_Hz = */ chirp.hp_cutoff_Hz,
                /* .vga_gain_4 = */ vga_gain,
                /* .reset_period_100ps = */ 15875  // 1.5875e-06
            };
        rc = local_driver->set_baseband_configuration(&baseband_config);
        check_libavian_return(rc);

        Anti_Alias_Filter_Settings aaf =
            {
                /* .frequency1_Hz = */ chirp.lp_cutoff_Hz,
                /* .frequency2_Hz = */ chirp.lp_cutoff_Hz,
                /* .frequency3_Hz = */ chirp.lp_cutoff_Hz,
                /* .frequency4_Hz = */ chirp.lp_cutoff_Hz};
        rc = local_driver->set_anti_alias_filter_settings(&aaf);
        check_libavian_return(rc);

        /*
         * The sampling rate is a global parameter, so all chirps must use the
         * same sampling rate. The sampling rate is only set, if it has not
         * been set before, otherwise it's checked, if the current chirp uses
         * the same sampling rate as the one already set.
         */
        if (current_sampling_rate == 0)
        {
            current_sampling_rate = chirp.sample_rate_Hz;

            /*
             * First it's tried to apply the Avian driver's default parameters.
             * If that does not work, the tracking conversions are disabled,
             * which allows higher sampling rates.
             */
            Adc_Configuration adc_configuration;
            adc_configuration.samplerate_Hz = static_cast<uint32_t>(chirp.sample_rate_Hz);
            adc_configuration.sample_time = Adc_Sample_Time::_50ns;
            adc_configuration.tracking = Adc_Tracking::_1_Subconversion;
            adc_configuration.double_msb_time = 0;
            adc_configuration.oversampling = Adc_Oversampling::Off;
            rc = local_driver->set_adc_configuration(&adc_configuration);

            if (rc != Driver::Error::OK)
            {
                adc_configuration.tracking = Adc_Tracking::None;
                rc = local_driver->set_adc_configuration(&adc_configuration);
                check_libavian_return(rc);
            }
        }
        else
        {
            if (current_sampling_rate != chirp.sample_rate_Hz)
                throw rdk::exception::error::exception(IFX_ERROR_SAMPLERATE_OUT_OF_RANGE);
        }
    };

    /*
     * This helper lambda function maps one or two consecutive chirps from a
     * sequence to a shape of the Avian state machine. The second chirp is only
     * considered if both chirps are part of a triangular shape with same RF
     * range. A delay in between the chirps is allowed and mapped to the chirp
     * end delay. The first element following the last considered chirp is
     * returned, so the caller can pick up parsing the sequence where this
     * function stopped.
     */
    auto setup_shape = [&](uint8_t shape, const ifx_Fmcw_Sequence_Element_t* element) {
        // A chirp is expected at the beginning of the provided sequence.
        if ((element == nullptr) || (element->type != IFX_SEQ_CHIRP))
            throw rdk::exception::argument_invalid();
        const auto* first_chirp = &element->chirp;
        auto* potential_2nd_chirp_element = element->next_element;

        /*
         * If the first chirp is followed by a delay that delay is tentatively
         * considered to be a delay in between two chirps of a triangular
         * shape. At this point it's not clear if the shape is triangular at
         * all, this mus be confirmed during further sequence analysis.
         */
        float in_between_delay_s = 0.0f;
        if ((potential_2nd_chirp_element != nullptr)
            && (potential_2nd_chirp_element->type == IFX_SEQ_DELAY))
        {
            in_between_delay_s = potential_2nd_chirp_element->delay.time_s;
            potential_2nd_chirp_element = potential_2nd_chirp_element->next_element;
        }

        /*
         * If there is another chirp in the sequence, it could be the second
         * chirp of a triangular shape. If so, it must have same RF range but
         * negative slope, and same TX power as the first chirp, because these
         * parameters are common to both chirps of a shape in libAvian.
         */
        ifx_Fmcw_Sequence_Chirp_t* second_chirp = nullptr;
        if ((potential_2nd_chirp_element != nullptr)
            && (potential_2nd_chirp_element->type == IFX_SEQ_CHIRP))
        {
            auto& chirp = potential_2nd_chirp_element->chirp;
            if ((first_chirp->start_frequency_Hz == chirp.end_frequency_Hz)
                && (first_chirp->end_frequency_Hz == chirp.start_frequency_Hz)
                && (first_chirp->tx_power_level == chirp.tx_power_level))
            {
                second_chirp = &chirp;
            }
        }

        /*
         * At this point, it's clear if the shape is saw tooth or triangle. For
         * a saw tooth shape with a single chirp an "in between delay" does not
         * make any sense, so it is reset in this case.
         */
        if (!second_chirp)
            in_between_delay_s = 0.0f;

        auto rc = local_driver->select_shape_to_configure(shape, false);
        check_libavian_return(rc);

        // The FMCW configuration of the shape is set.
        double start_frequency_Hz;
        double end_frequency_Hz;
        if ((first_chirp->start_frequency_Hz == 0.0)
            || (first_chirp->end_frequency_Hz == 0.0))
        {
            const auto range_center = (m_sensor_info.min_rf_frequency_Hz
                                       + m_sensor_info.max_rf_frequency_Hz)
                                      / 2.0;
            const auto chirp_range = first_chirp->end_frequency_Hz
                                     - first_chirp->start_frequency_Hz;

            start_frequency_Hz = range_center - (chirp_range / 2.0);
            end_frequency_Hz = range_center + (chirp_range / 2.0);
        }
        else
        {
            start_frequency_Hz = first_chirp->start_frequency_Hz;
            end_frequency_Hz = first_chirp->end_frequency_Hz;
        }

        const auto start_frequency_kHz = static_cast<uint32_t>(round(start_frequency_Hz / 1000.0));
        const auto end_frequency_kHz = static_cast<uint32_t>(round(end_frequency_Hz / 1000.0));
        const auto [lower_frequency_kHz, upper_frequency_kHz] = std::minmax(start_frequency_kHz,
                                                                            end_frequency_kHz);

        bool first_chirp_down = start_frequency_kHz > end_frequency_kHz;
        auto shape_type = second_chirp ? (first_chirp_down ? Shape_Type::Tri_Down : Shape_Type::Tri_Up)
                                       : (first_chirp_down ? Shape_Type::Saw_Down : Shape_Type::Saw_Up);
        Fmcw_Configuration fmcw_config =
            {
                /* .lower_frequency_kHz = */ lower_frequency_kHz,
                /* .upper_frequency_kHz = */ upper_frequency_kHz,
                /* .shape_type = */ shape_type,
                /* .tx_power = */ static_cast<uint8_t>(first_chirp->tx_power_level)};
        rc = local_driver->set_fmcw_configuration(&fmcw_config);
        check_libavian_return(rc);

        /*
         * After the common shape parameters have been set, the remaining
         * per-chirp parameters are set. The delay between the chirps is mapped
         * to the post chirp delay of the first chirp.
         */
        setup_chirp(shape, first_chirp_down, *first_chirp);

        auto delay_100ps = static_cast<uint32_t>(std::round(in_between_delay_s / 100e-12f));
        rc = local_driver->set_chirp_end_delay(delay_100ps);
        check_libavian_return(rc);

        if (second_chirp)
        {
            setup_chirp(shape, !first_chirp_down, *second_chirp);
            rc = local_driver->set_chirp_end_delay(0);
            check_libavian_return(rc);
        }

        /*
         * The element following the last chirp of the shape is returned, to
         * let the caller continue analyzing the sequence.
         */
        return second_chirp ? potential_2nd_chirp_element->next_element
                            : element->next_element;
    };

    /*
     * This helper lambda function configures a given period of time as the
     * shape end or frame end delay. It takes also the wake up times from IDLE
     * and DEEP SLEEP state into account and choses the power mode accordingly.
     */
    auto set_shape_end_delay = [&local_driver](double time_s,
                                               Shape_Group& shape_group) {
        /*
         * To chose the best power saving mode, the wake up times of the
         * Avian state machine must be considered. As a default no power
         * saving mode was set in during the initialization of
         * frame_definition at the top of the function, because without
         * power saving mode, there is no additional wakeup time.
         */
        Startup_Timing startup_timing;
        auto rc = local_driver->get_startup_timing(&startup_timing);
        check_libavian_return(rc);

        float time_after_idle =
            (startup_timing.pll_settle_time_coarse_100ps
             + startup_timing.pll_settle_time_fine_100ps)
            * 100e-12f;
        float time_after_deep_sleep =
            startup_timing.wake_up_time_100ps * 100e-12f;

        /*
         * If the delay is long enough to wake up from IDLE state again,
         * the IDLE state is chosen. The delay must be decreased by the
         * INIT time (called PLL settle time by libAvian) because the Avian
         * state machine applies that delay additionally after the shape end
         * delay.
         */
        if (time_s >= time_after_idle)
        {
            shape_group.following_power_mode = Power_Mode::Idle;
            time_s -= time_after_idle;
        }

        /*
         * If the delay is long enough to also wake up from DEEP SLEEP
         * state, the DEEP SLEEP state is chosen. The delay must be
         * decreased by the wake up time because the Avian state machine
         * applies that delay additionally after the shape end delay and
         * before the INIT time.
         * (Full DEEP SLEEP between shapes is not allowed, so DEEP SLEEP
         * with continue is chosen, which keeps the system clock enabled.)
         */
        if (time_s >= time_after_deep_sleep)
        {
            shape_group.following_power_mode = Power_Mode::Deep_Sleep_Continue;
            time_s -= time_after_deep_sleep;
        }

        shape_group.post_delay_100ps = static_cast<uint64_t>(time_s / 100e-12);
    };

    /*
     * The sub sequence of the frame loop (or the shape set loop, if present)
     * is analyzed. The chirps in that sub sequence will be mapped to the
     * (up to) four shapes of the Avian state machine with (up to) two chirp
     * per shape. Delays between the chirps are allowed, and each shape may be
     * surrounded by a loop.
     */
    // A sub sequence must not start with a delay.
    if (sequence->type == IFX_SEQ_DELAY)
        throw rdk::exception::argument_invalid();

    uint8_t next_shape_index = 0;
    while (sequence != nullptr)
    {
        if (sequence->type == IFX_SEQ_CHIRP)
        {
            /*
             * If a chirp is found in the sequence, the next shape is obviously
             * not looped, so the number of repetitions is set to 1. The lambda
             * function setup_shape does the work of picking the relevant
             * chirps from the sequence and returning the next sequence element
             * to consider.
             * If all four of the Avian state machine's shapes are already
             * used, no more chirps can't be handled.
             */
            if (next_shape_index >= 4)
                throw rdk::exception::argument_invalid();

            frame_definition.shapes[next_shape_index].num_repetitions = 1;
            sequence = setup_shape(next_shape_index++, sequence);
        }
        else if (sequence->type == IFX_SEQ_LOOP)
        {
            /*
             * If a loop is found in the sequence, it is considered to be the
             * loop of a shape and the loop's sub sequence defines the chirps
             * of the shape to be looped.
             * If all four of the Avian state machine's shapes are already
             * used, no more loops can't be handled.
             */
            if (next_shape_index >= 4)
                throw rdk::exception::argument_invalid();

            /*
             * The number of repetition is directly taken from the loop
             * element, the chirps in the loop's sub sequence are handled by
             * the lambda function setup_shape.
             * The latter only picks one or two chirps from the beginning of a
             * sequence and returns the rest of sequence. The Avian state
             * machine does not allow to put more than two chirps into a shape
             * loop, so the remainder must be empty. If it's not, the provided
             * sequence cannot be executed by the Avian state machine.
             */
            const auto& loop = sequence->loop;
            frame_definition.shapes[next_shape_index].num_repetitions = loop.num_repetitions;
            auto* remainder = setup_shape(next_shape_index, loop.sub_sequence);
            if (remainder != nullptr)
                throw rdk::exception::argument_invalid();

            /*
             * To achieve the desired loop repetition time, the chirp end delay
             * of the shapes' last chirp is used to set an additional delay.
             * The Avian timing model can precisely predict a loop repetition
             * time from a configuration, but there is no straight forward way
             * for the opposite direction.
             * When the chirp parameters have been set above, the chirp end
             * delay has bee set to 0, so the timing model can tell the
             * preliminary loop repetition time without any additional delay.
             * The frame definition hasn't been set before, so it's done now,
             * otherwise the timing model can't take it into account.
             */
            auto rc = local_driver->set_frame_definition(&frame_definition);
            check_libavian_return(rc);
            TimingModel::StateSequence timing_model(*local_driver);
            auto num_cycles = timing_model.getChirpToChirpTime(next_shape_index);
            auto prelim_rep_time = timing_model.toSeconds(num_cycles);

            /*
             * The additional delay to stretch the loop repetition time is
             * just of the difference of the preliminary repetition time that
             * considers the working time of the shape only and the total
             * desired repetition time.
             * If the preliminary repetition time is already longer than the
             * desired one, it's not possible to configure the specified
             * sequence, because negative delays can't be set.
             */
            auto additional_delay = loop.repetition_time_s - prelim_rep_time;
            if (additional_delay < 0.f)
                throw rdk::exception::argument_invalid();

            auto delay_100ps = round(additional_delay / 100e-12f);
            local_driver->set_chirp_end_delay(static_cast<uint32_t>(delay_100ps));

            ++next_shape_index;
            sequence = sequence->next_element;
        }
        else if (sequence->type == IFX_SEQ_DELAY)
        {
            // A delay at the end of a sequence is not allowed.
            if (sequence->next_element == nullptr)
                throw rdk::exception::argument_invalid();

            /*
             * The delay is applied as the shape end delay of the previous
             * shape. It's guaranteed that there is always a previous shape,
             * because a sequence must not begin with a delay. When this branch
             * is executed, a chirp or a loop have been found before and been
             * mapped to a shape of the state machine.
             */
            auto& last_shape = frame_definition.shapes[next_shape_index - 1];
            set_shape_end_delay(sequence->delay.time_s, last_shape);

            sequence = sequence->next_element;
        }
        else
        {
            // Unknown element type.
            throw rdk::exception::argument_invalid();
        }
    }

    /*
     * If there was a loop element in the sequence that represent the shape
     * set loop (shape_set_repetition_time > 0), an additional delay is set as
     * the shape end delay of the last shape to stretch the total shape set
     * repetition time.
     * First the current state of frame_definition is sent to the local driver,
     * so that the afterwards created timing model can return the preliminary
     * shape set repetition time, which results from all parameters set above.
     * It's known that the shape end delay of the last shape is so far set to
     * zero, so the additional delay to stretch the loop repetition time is
     * simply the difference of the desired repetition time and the preliminary
     * repetition time.
     * Actually the Avian state machine adds an additional clock cycle delay,
     * even when the shape end delay is set to zero. Therefore the preliminary
     * repetition time has to be corrected by one clock cycle.
     */
    if (shape_set_repetition_time > 0.0f)
    {
        auto rc = local_driver->set_frame_definition(&frame_definition);
        check_libavian_return(rc);
        TimingModel::StateSequence timing_model(*local_driver);
        auto num_cycles = timing_model.getSetToSetTime() - 1;
        auto prelim_rep_time = timing_model.toSeconds(num_cycles);

        auto additional_delay = shape_set_repetition_time - prelim_rep_time;
        if (additional_delay < 0.0f)
            throw rdk::exception::chirp_rate_out_of_range();

        auto& last_shape = frame_definition.shapes[next_shape_index - 1];
        set_shape_end_delay(additional_delay, last_shape);
    }

    /*
     * To achieve the desired frame repetition time as it was set in the outer
     * most loop element in the sequence, an additional delay is set as frame
     * end delay to stretch the total frame repetition time.
     * First the current state of frame_definition is sent to the local driver,
     * so that the afterwards created timing model can return the preliminary
     * frame repetition time, which results from all parameters set above.
     * It's known that the frame end delay is so far set to zero, so the
     * additional delay to stretch the loop repetition time is simply the
     * difference of the desired repetition time and the preliminary repetition
     * time.
     * Actually the Avian state machine adds an additional clock cycle delay,
     * even when the frame end delay is set to zero. Therefore the preliminary
     * repetition time has to be corrected by one clock cycle.
     */
    auto rc = local_driver->set_frame_definition(&frame_definition);
    check_libavian_return(rc);
    TimingModel::StateSequence timing_model(*local_driver);
    auto num_cycles = timing_model.getFrameDuration() - 1;
    auto prelim_rep_time = timing_model.toSeconds(num_cycles);

    auto additional_delay = frame_repetition_time - prelim_rep_time;
    if (additional_delay < 0.0f)
        throw rdk::exception::frame_rate_out_of_range();

    set_shape_end_delay(additional_delay, frame_definition.shape_set);

    /*
     * Now that all repetition times have been set up properly, the frame
     * definition has reached its final content. It is sent to the local driver
     * instance to finalize the Avian configuration.
     */
    rc = local_driver->set_frame_definition(&frame_definition);
    check_libavian_return(rc);

    /*
     * Finally the parameters of the new acquisition sequence are applied.
     * Before the configuration of the local driver is made active, any ongoing
     * acquisition has to be stopped.
     */
    stop_acquisition();
    std::swap(m_driver, local_driver);
    generate_register_list();

    /*
     * The base class needs information about the frame structure for data
     * fetching during acquisition.
     */
    update_frame_settings();
}

ifx_Fmcw_Sequence_Element_t* DeviceFmcwAvian::get_acquisition_sequence() const
{
    /*
     * The timing model knows best about chirp, set and frame repetition rates,
     * so a temporary instance is created, to provide the repetition times for
     * the created loop elements.
     */
    auto timing_model = create_timing_model();

    /*
     * At some points the acquisition sequence may contain optional delays. At
     * these points the Avian driver never reports 0 delay time, because the
     * device's state machine always inserts a few clock cycles. To check if
     * there is a significant delay threshold will be used below. Those
     * thresholds are expressed in terms of clock cycles, so the Avian device's
     * reference clock is needed.
     */
    Avian::Reference_Clock_Frequency ref_frequency;
    auto rc = m_driver->get_reference_clock_frequency(&ref_frequency);
    check_libavian_return(rc);
    float cycle_time = 1 / 80.0e6f;
    if ((ref_frequency == Avian::Reference_Clock_Frequency::_38_4MHz)
        || (ref_frequency == Avian::Reference_Clock_Frequency::_76_8MHz))
    {
        cycle_time = 1 / 76.8e6f;
    }

    // The frame definition contains the number of repetitions for all loops.
    Avian::Frame_Definition frame_definition;
    rc = m_driver->get_frame_definition(&frame_definition);
    check_libavian_return(rc);

    /*
     * As a starting point the frame loop element is created. At this point the
     * sub sequence stays empty. The elements of the sub sequence are appended
     * below. For appending the pointer shape_append_ptr is used. It is
     * initialized here, and updated whenever an element is appended.
     */
    auto* frame_loop = ifx_fmcw_create_sequence_element(IFX_SEQ_LOOP);
    frame_loop->loop.num_repetitions = frame_definition.num_frames;
    auto frame_ticks = timing_model->getFrameDuration();
    auto frame_time = timing_model->toSeconds(frame_ticks);
    frame_loop->loop.repetition_time_s = static_cast<float>(frame_time);

    auto* shape_append_ptr = &frame_loop->loop.sub_sequence;

    /*
     * With Avian devices a single frame is always a repetition of shape sets,
     * but the number of repetitions may be 1. A loop element representing the
     * frame loop is only inserted, if the sub sequence is executed more than
     * once.
     */
    bool set_loop_present = frame_definition.shape_set.num_repetitions > 1;
    if (set_loop_present)
    {
        auto* set_loop = ifx_fmcw_create_sequence_element(IFX_SEQ_LOOP);
        set_loop->loop.num_repetitions = frame_definition.shape_set.num_repetitions;
        auto set_ticks = timing_model->getSetToSetTime();
        auto set_time = timing_model->toSeconds(set_ticks);
        set_loop->loop.repetition_time_s = static_cast<float>(set_time);

        /*
         * The set loop is now the only element of the frame loop's sub
         * sequence. The shapes of the shape set are append as part of the
         * set loop's sub sequence.
         */
        *shape_append_ptr = set_loop;
        shape_append_ptr = &set_loop->loop.sub_sequence;
    }

    /*
     * Now those shapes are added to the sequence which are really used
     * (num_repetitions != 0). The first shape is always used, because the
     * Avian driver does not allow num_repetitions = 0 for the first shape.
     * The Avian state machine does not allow gaps in the shape sequence. If a
     * shape is not used, all following shapes aren't used either.
     */
    for (uint8_t shp = 0; shp < 4; ++shp)
    {
        const auto& shape = frame_definition.shapes[shp];

        if (shape.num_repetitions == 0)
            break;

        /*
         * Each shape may contain either one chirp ("saw tooth") or two
         * chirps ("triangle"), and may start either with an up chirp or
         * a down chirp. The shape type is part of tho FMCW configuration.
         * The FMCW configuration is common for up and down chirps, so it
         * does not which one is selected here.
         */
        rc = m_driver->select_shape_to_configure(shp, false);
        check_libavian_return(rc);

        Avian::Fmcw_Configuration fmcw_configuration;
        rc = m_driver->get_fmcw_configuration(&fmcw_configuration);
        check_libavian_return(rc);

        /*
         * This lambda function creates a chirp element for the currently
         * selected shape and chirp. It retrieves all relevant chirp parameters
         * from the Avian driver and fills the fields of the chirp element.
         * The FMCW configuration queried above is captured by the lambda and
         * also taken into account.
         * The chirp element structure does not allow different base band
         * settings for different RX antennas. Therefore only the settings for
         * RX1 are taken into account and it is assumed that the other RX
         * channels use the same settings.
         */
        auto create_chirp = [&](bool down_chirp) {
            auto* chirp = ifx_fmcw_create_sequence_element(IFX_SEQ_CHIRP);

            chirp->chirp.start_frequency_Hz = 1e3 * static_cast<double>(fmcw_configuration.lower_frequency_kHz);
            chirp->chirp.end_frequency_Hz = 1e3 * static_cast<double>(fmcw_configuration.upper_frequency_kHz);
            chirp->chirp.tx_power_level = fmcw_configuration.tx_power;
            if (down_chirp)
                std::swap(chirp->chirp.start_frequency_Hz, chirp->chirp.end_frequency_Hz);

            Avian::Adc_Configuration adc_configuration;
            rc = m_driver->get_adc_configuration(&adc_configuration);
            check_libavian_return(rc);
            chirp->chirp.sample_rate_Hz = static_cast<float>(adc_configuration.samplerate_Hz);

            Avian::Frame_Format frame_format;
            rc = m_driver->get_frame_format(&frame_format);
            check_libavian_return(rc);
            chirp->chirp.num_samples = frame_format.num_samples_per_chirp;
            chirp->chirp.rx_mask = frame_format.rx_mask;

            Avian::Tx_Mode tx_mode;
            rc = m_driver->get_tx_mode(&tx_mode);
            check_libavian_return(rc);
            chirp->chirp.tx_mask = tx_mode_to_tx_mask(tx_mode);

            Avian::Baseband_Configuration baseband_configuration;
            rc = m_driver->get_baseband_configuration(&baseband_configuration);
            check_libavian_return(rc);

            const Avian::Hp_Gain hp_gain = baseband_configuration.hp_gain_1;
            const uint32_t hpg = (hp_gain == Avian::Hp_Gain::_30dB) ? 30 : 18;
            const Avian::Vga_Gain vga_gain = baseband_configuration.vga_gain_1;
            chirp->chirp.if_gain_dB = 5 * static_cast<uint32_t>(vga_gain) + hpg;
            chirp->chirp.hp_cutoff_Hz = baseband_configuration.hp_cutoff_1_Hz;

            Avian::Anti_Alias_Filter_Settings anti_alias_filter_settings;
            rc = m_driver->get_anti_alias_filter_settings(&anti_alias_filter_settings);
            check_libavian_return(rc);
            chirp->chirp.lp_cutoff_Hz = anti_alias_filter_settings.frequency1_Hz;

            return chirp;
        };

        /*
         * The shape's first chirp is created. An append pointer is prepared in
         * case there is another chirp following.
         */
        bool first_down = (fmcw_configuration.shape_type == Avian::Shape_Type::Saw_Down)
                          || (fmcw_configuration.shape_type == Avian::Shape_Type::Tri_Down);
        rc = m_driver->select_shape_to_configure(shp, first_down);
        check_libavian_return(rc);
        auto* first_chirp = create_chirp(first_down);
        auto* chirp_append_ptr = &first_chirp->next_element;

        /*
         * If the current shape is a triangle the second chirp is appended to
         * the first chirp. If needed, an optional delay is inserted.
         */
        if ((fmcw_configuration.shape_type == Avian::Shape_Type::Tri_Up)
            || (fmcw_configuration.shape_type == Avian::Shape_Type::Tri_Down))
        {
            /*
             * The chirp end delay is calculated, taking also additional delays
             * due to power saving mode into account. The chirp end delay is
             * programmed in steps of 8 clock cycles. If the delay is shorter
             * than 8 clock cycles, it's just the few additional clock cycles
             * due to FSM implementation. In that case the delay is ignored.
             */
            uint32_t chirp_end_delay_100ps;
            rc = m_driver->get_chirp_end_delay(&chirp_end_delay_100ps);
            check_libavian_return(rc);
            float chirp_end_delay = chirp_end_delay_100ps * 100e-12f;
            if (chirp_end_delay >= 8 * cycle_time)
            {
                auto* delay = ifx_fmcw_create_sequence_element(IFX_SEQ_DELAY);
                delay->delay.time_s = chirp_end_delay;

                *chirp_append_ptr = delay;
                chirp_append_ptr = &delay->next_element;
            }

            rc = m_driver->select_shape_to_configure(shp, !first_down);
            check_libavian_return(rc);
            auto* second_chirp = create_chirp(!first_down);

            *chirp_append_ptr = second_chirp;
            chirp_append_ptr = &second_chirp->next_element;
        }

        /*
         * Now the loop element representing the shape repetitions is appended
         * and the chirp sequence created above is set inserted as sub
         * sequence. If the number of repetitions is just 1, that loop element
         * is useless. In this case it is omitted and the sub sequence is
         * inserted directly.
         * On the other hand, the shape loop is added if the shape set loop
         * element was omitted, because there should be at least one loop
         * element with num_elements = 1. Even though it is technically not
         * necessary, most users think of a frame as repeated chirps and they
         * might expect a loop element.
         */
        if ((shape.num_repetitions > 1) || !set_loop_present)
        {
            auto* shape_loop = ifx_fmcw_create_sequence_element(IFX_SEQ_LOOP);
            shape_loop->loop.sub_sequence = first_chirp;
            shape_loop->loop.num_repetitions = shape.num_repetitions;
            auto shape_ticks = timing_model->getChirpToChirpTime(shp);
            auto shape_time = timing_model->toSeconds(shape_ticks);
            shape_loop->loop.repetition_time_s = static_cast<float>(shape_time);

            *shape_append_ptr = shape_loop;
            shape_append_ptr = &shape_loop->next_element;
        }
        else
        {
            *shape_append_ptr = first_chirp;
            shape_append_ptr = chirp_append_ptr;
        }

        /*
         * The shape end delay is inserted as a delay element into the
         * sequence. This happens only if there is another shape following,
         * because a sequence must not end with a delay.
         * If the Avian device goes into a power saving mode during the shape
         * end delay, the wake up times from IDLE and DEEP SLEEP modes are
         * also taken into account.
         */
        if ((shp < 3) && (frame_definition.shapes[shp + 1].num_repetitions > 0))
        {
            Avian::Startup_Timing startup_timing;
            rc = m_driver->get_startup_timing(&startup_timing);
            check_libavian_return(rc);

            float post_delay = shape.post_delay_100ps * 100e-12f;

            /*
             * After both, IDLE and DEEP SLEEP mode the INIT0 and INIT1 phase
             * are applied.
             */
            if (shape.following_power_mode != Avian::Power_Mode::Stay_Active)
            {
                post_delay += startup_timing.pll_settle_time_coarse_100ps * 100e-12f;
                post_delay += startup_timing.pll_settle_time_fine_100ps * 100e-12f;

                // After DEEP SLEEP mode additionally the wake up phase is applied.
                if (shape.following_power_mode != Avian::Power_Mode::Idle)
                    post_delay += startup_timing.wake_up_time_100ps * 100e-12f;
            }

            /*
             * If the shape end delay timer is used, it adds always 3 extra
             * clock cycles. If the shape end delay is shorter than that, the
             * timer is not used, and so no extra delay element is inserted.
             */
            if (post_delay >= 3 * cycle_time)
            {
                auto* delay = ifx_fmcw_create_sequence_element(IFX_SEQ_DELAY);
                delay->delay.time_s = post_delay;

                *shape_append_ptr = delay;
                shape_append_ptr = &delay->next_element;
            }
        }
    }
    return frame_loop;
}

/*
==============================================================================
   DeviceControlAvian.cpp
==============================================================================
*/

void DeviceFmcwAvian::generate_register_list()
{
    auto avian_registers = m_driver->get_device_configuration().get_configuration_sequence(false);

    m_register_map.clear();
    for (auto spi_command : avian_registers)
    {
        const uint16_t address = spi_command >> 25;
        const uint32_t value = spi_command & 0x00FFFFFF;
        m_register_map.insert(std::make_pair(address, value));
    }
}

std::map<uint16_t, uint32_t>& DeviceFmcwAvian::get_register_list()
{
    return m_register_map;
}

std::map<uint16_t, uint32_t> DeviceFmcwAvian::import_register_list(const char* filename)
{
    // Each entry in the retrieved  register map alwayas refers to layout index 0.
    // Therefore, provide one dummy layout with no bitfields in it (an empty map).
    auto device_type = m_driver->get_device_type();
    std::vector<NamedMemory<uint16_t, uint32_t>::Layout> dummy_layout {{"dummy_layout", {}}};
    auto named_memory = std::make_unique<NamedMemory<uint16_t, uint32_t>>(nullptr, get_register_map(device_type), dummy_layout);

    std::map<uint16_t, uint32_t> register_list;
    named_memory->loadConfig(filename, register_list);

    return register_list;
}

void DeviceFmcwAvian::export_register_list(const char* filename, const std::map<uint16_t, uint32_t>& register_list)
{
    // Each entry in the retrieved register map alwayas refers to layout index 0.
    // Therefore, provide one dummy layout with with no bitfields in it (an empty map).
    auto device_type = m_driver->get_device_type();
    std::vector<NamedMemory<uint16_t, uint32_t>::Layout> dummy_layout {{"dummy_layout", {}}};
    auto named_memory = std::make_unique<NamedMemory<uint16_t, uint32_t>>(nullptr, get_register_map(device_type), dummy_layout);

    named_memory->saveConfig(filename, register_list);
}

void DeviceFmcwAvian::load_register_file(const char* filename)
{
    const auto register_list = import_register_list(filename);
    apply_register_list(register_list);
}

void DeviceFmcwAvian::save_register_file(const char* filename)
{
    auto& register_list = get_register_list();
    export_register_list(filename, register_list);
}

void DeviceFmcwAvian::apply_register_list(const std::map<uint16_t, uint32_t>& register_list)
{
    // mapping from generic interface map with uint16_t address to Avian Driver specific map with uint8_t address
    std::map<uint8_t, uint32_t> register_map;
    for (const auto& entry : register_list)
    {
        register_map.insert(std::make_pair(static_cast<uint8_t>(entry.first), entry.second));
    }

    // The subsequent code can be greatly simplified once lib_avian allows to
    // directly import register lists.

    const auto sensor_type = m_driver->get_device_type();
    const auto& device_traits = Avian::Device_Traits::get(sensor_type);
    const auto extractor = Avian::Parameter_Extractor(register_map, sensor_type);

    // create a copy of the current state
    // we first set the driver and only after the importing was successful, we
    // update m_driver by driver.
    auto driver = std::make_unique<Avian::Driver>(*m_driver);

    Avian::Driver::Error rc;

    // Global parameters

    const auto reference_clock = extractor.get_reference_clock();
    rc = driver->set_reference_clock_frequency(reference_clock);
    check_libavian_return(rc);

    const auto slice_size = extractor.get_slice_size();
    rc = driver->set_slice_size(slice_size);
    check_libavian_return(rc);

    const auto frame_definition = extractor.get_frame_definition();
    rc = driver->set_frame_definition(&frame_definition);
    check_libavian_return(rc);

    const auto adc_configuration = extractor.get_adc_configuration();
    rc = driver->set_adc_configuration(&adc_configuration);
    check_libavian_return(rc);

    const auto chirp_timing = extractor.get_chirp_timing();
    rc = driver->set_chirp_timing(&chirp_timing);
    check_libavian_return(rc);

    const auto startup_timing = extractor.get_startup_timing();
    rc = driver->set_startup_timing(&startup_timing);
    check_libavian_return(rc);

    const auto idle_configuration = extractor.get_idle_configuration();
    rc = driver->set_idle_configuration(&idle_configuration);
    check_libavian_return(rc);

    const auto deep_sleep_configuration = extractor.get_deep_sleep_configuration();
    rc = driver->set_deep_sleep_configuration(&deep_sleep_configuration);
    check_libavian_return(rc);

    // Global parameters introduced with 'generation D'

    if (device_traits.has_extra_startup_delays)
    {
        const auto startup_delays = extractor.get_startup_delays();
        rc = driver->set_startup_delays(&startup_delays);
        check_libavian_return(rc);
    }

    if (device_traits.has_ref_frequency_doubler)
    {
        const auto duty_cycle_correction = extractor.get_duty_cycle_correction();
        rc = driver->set_duty_cycle_correction(&duty_cycle_correction);
        check_libavian_return(rc);
    }

    if (device_traits.has_programmable_fifo_power_mode)
    {
        const auto fifo_power_mode = extractor.get_fifo_power_mode();
        rc = driver->set_fifo_power_mode(fifo_power_mode);
        check_libavian_return(rc);
    }

    if (device_traits.has_programmable_pad_driver)
    {
        const auto pad_driver_mode = extractor.get_pad_driver_mode();
        rc = driver->set_pad_driver_mode(pad_driver_mode);
        check_libavian_return(rc);
    }

    // Global parameters introduced with 'generation E'

    if (device_traits.has_programmable_pullup_resistors)
    {
        const auto pullup_resistor_configuration = extractor.get_pullup_resistor_configuration();
        rc = driver->set_pullup_resistor_configuration(&pullup_resistor_configuration);
        check_libavian_return(rc);
    }

    // Global parameters introduced with BGT60UTR11AIP

    if (!device_traits.has_sadc)  // only possible if SADC is *not* available
    {
        const auto power_sens_delay = extractor.get_power_sens_delay();
        rc = driver->set_power_sens_delay(power_sens_delay);
        check_libavian_return(rc);

        const auto power_sens_enabled = extractor.get_power_sens_enabled();
        rc = driver->set_power_sens_enabled(power_sens_enabled);
        check_libavian_return(rc);

        const auto temperature_sens_enabled = extractor.get_temperature_sens_enabled();
        rc = driver->set_temperature_sens_enabled(temperature_sens_enabled);
        check_libavian_return(rc);
    }

    // Chirp parameters

    for (const uint8_t shape : {0, 1, 2, 3})   // for all 4 shapes
    {
        for (const bool down : {true, false})  // and up- and down-chirps
        {
            rc = driver->select_shape_to_configure(shape, down);
            check_libavian_return(rc);

            if (frame_definition.shapes[shape].num_repetitions == 0)
            {
                // If this shape has no repetitions the shape is disabled.
                // So, we also need to disable the current shape.
                Avian::Frame_Format frame_format {0, 0, 0};
                driver->set_frame_format(&frame_format);
                break;
            }

            const auto fmcw_configuration = extractor.get_fmcw_configuration(shape);
            rc = driver->set_fmcw_configuration(&fmcw_configuration);
            check_libavian_return(rc);

            // Continue if and only if:
            //  - imported shape type is Saw_Up and down is false (means: up-chirp)
            //  - imported shape type is Saw_Down and down is true (means: down-chirp)
            //  - imported shape type is Tri_Up or Tri_Down (has both up- and down chirps)
            if ((fmcw_configuration.shape_type == Avian::Shape_Type::Saw_Down && !down)
                || (fmcw_configuration.shape_type == Avian::Shape_Type::Saw_Up && down))
                continue;

            const auto tx_mode = extractor.get_tx_mode(shape, down);
            rc = driver->set_tx_mode(tx_mode);
            check_libavian_return(rc);

            const auto frame_format = extractor.get_frame_format(shape, down);
            rc = driver->set_frame_format(&frame_format);
            check_libavian_return(rc);

            const auto baseband_configuration = extractor.get_baseband_configuration(shape, down);
            rc = driver->set_baseband_configuration(&baseband_configuration);
            check_libavian_return(rc);

            const auto chirp_end_delay = extractor.get_chirp_end_delay(shape, down);
            rc = driver->set_chirp_end_delay(chirp_end_delay);
            check_libavian_return(rc);

            if (device_traits.cs_register_layout != Avian::Device_Traits::Channel_Set_Layout::Version1)
            {
                const auto anti_alias_filter_settings = extractor.get_anti_alias_filter_settings(shape, down);
                rc = driver->set_anti_alias_filter_settings(&anti_alias_filter_settings);
                check_libavian_return(rc);
            }
        }
    }


    /*
     * As a first step of initialization the device configuration provided by the user is copied into
     * the handle. Some of the parameters are needed during fetching of time domain data.
     */
    stop_acquisition();
    m_driver = std::move(driver);

    m_num_samples = 0;
    update_defaults_if_not_configured();
    generate_register_list();
}

uint32_t DeviceFmcwAvian::export_register_list_legacy(bool set_trigger_bit, uint32_t* register_list)
{
    auto registers = m_driver->get_device_configuration().get_configuration_sequence(set_trigger_bit);
    if (register_list != nullptr)
    {
        std::copy(registers.begin(), registers.end(), register_list);
    }
    return static_cast<uint32_t>(registers.size());
}

// ---------------------------------------------------------------------------
std::unique_ptr<TimingModel::StateSequence> DeviceFmcwAvian::create_timing_model() const
{
    using StateSequence = TimingModel::StateSequence;
    using RegisterSet = HW::RegisterSet;

    const auto device_type = m_driver->get_device_type();
    if (device_type == Device_Type::Unknown)
    {
        return nullptr;
    }

    RegisterSet avian_registers;
    for (const auto& entry : m_register_map)
        avian_registers.set(static_cast<uint8_t>(entry.first), entry.second);

    return std::make_unique<StateSequence>(avian_registers, device_type);
}

float DeviceFmcwAvian::get_chirp_duration(const ifx_Fmcw_Sequence_Chirp_t& chirp) const
{
    /*
     * Duration of chirp element and minimum chirp repetition time
     * is almost the same. The only difference is the latter includes
     * also chirp end delay. The easiest way to get the chirp duration
     * is to start from the minimum repetition time and subtract the
     * end delay.
     */
    auto min_repetition_time = get_minimum_chirp_repetition_time(chirp.num_samples, chirp.sample_rate_Hz);

    /*
     * "Minimum repetition time" means that minimum chirp end delay is
     * configured. A local driver instance is used to find out that
     * minimum chirp end delay without touching the main driver
     * instance.
     */
    Avian::Driver local_driver(*m_driver);
    local_driver.set_chirp_end_delay(0);

    uint32_t min_chirp_end_delay_100ps;
    local_driver.get_chirp_end_delay(&min_chirp_end_delay_100ps);
    auto min_chirp_end_delay = min_chirp_end_delay_100ps * 100.0e-12f;

    /*
     * The Avian state machine always applies chirp end delay for both,
     * up-chirp and down-chirp (even for sawtooth shapes), so the min
     * delay is subtracted twice.
     */
    return min_repetition_time - 2 * min_chirp_end_delay;
}

void DeviceFmcwAvian::set_reference_clock(float reference_clock)
{
    Reference_Clock_Frequency parameter;
    if (reference_clock == 38.4e6f)
    {
        parameter = Reference_Clock_Frequency::_38_4MHz;
    }
    else if (reference_clock == 40e6f)
    {
        parameter = Reference_Clock_Frequency::_40MHz;
    }
    else if (reference_clock == 76.8e6f)
    {
        parameter = Reference_Clock_Frequency::_76_8MHz;
    }
    else if (reference_clock == 80e6f)
    {
        parameter = Reference_Clock_Frequency::_80MHz;
    }
    else
    {
        throw rdk::exception::argument_invalid();
    }

    const auto rc = m_driver->set_reference_clock_frequency(parameter);
    check_libavian_return(rc);
}

void DeviceFmcwAvian::detect_reference_clock()
{
    try
    {
        if (!Avian::detect_reference_clock(*m_board, *m_driver))
        {
            IFX_LOG_WARNING("FW does not support detection of reference clock, continuing with default setting");
            return;
        }
    }
    catch (const EException& e)
    {
        (void)e;
        IFX_LOG_WARNING("Could not generate measurement signal for oscillator frequency: %s", e.what());
    }
}

float DeviceFmcwAvian::get_minimum_chirp_repetition_time(uint32_t num_samples, float sample_rate_Hz) const
{
    /*
     * The Avian timing model knows best about all internal timings of the
     * Avian state machine, so it's best to use it to get minimum chirp
     * repetition time.
     * There is no direct way to ask the timing model for minimum repetition
     * time but the timing can be created for a configuration with minimum
     * delays between chirps.
     * A local driver instance is needed to setup such a configuration, because
     * the main driver must not be changed (and cannot because this a const
     * function).
     */

    /*
     * The local driver instance is created from main driver instance, copying
     * all parameters currently set.
     */
    Avian::Driver local_driver(*m_driver);

    // Sampling rate is applied according to provided argument.
    local_driver.set_adc_samplerate(static_cast<uint32_t>(sample_rate_Hz));

    /*
     * It's unclear if the main driver is currently configured for
     * up-chirp, down chirp or triangle shape. To have defined
     * conditions, the local driver is configured for an up-chirp.
     */
    local_driver.select_shape_to_configure(0, false);

    Avian::Fmcw_Configuration fmcw_configuration;
    local_driver.get_fmcw_configuration(&fmcw_configuration);
    fmcw_configuration.shape_type = Avian::Shape_Type::Saw_Up;
    local_driver.set_fmcw_configuration(&fmcw_configuration);

    // Number of samples is applied according to provided argument.
    Avian::Frame_Format frame_format;
    local_driver.get_frame_format(&frame_format);
    frame_format.num_samples_per_chirp = num_samples;
    local_driver.set_frame_format(&frame_format);

    /*
     * For minimum repetition time, the chirp end delay is set to minimum for
     * both, up-chirp and down-chirp, because the Avian state machine always
     * applies both of them.
     */
    local_driver.set_chirp_end_delay(0);
    local_driver.select_shape_to_configure(0, true);
    local_driver.set_chirp_end_delay(0);

    /*
     * Now, with all settings made, the timing model can tell the chirp
     * repetition time.
     */
    Avian::TimingModel::StateSequence timing_model(local_driver);
    return float(timing_model.toSeconds(timing_model.getChirpToChirpTime(0)));
}

double DeviceFmcwAvian::get_chirp_sampling_range(const ifx_Fmcw_Sequence_Chirp_t* chirp) const
{
    Avian::Chirp_Timing chirp_timing;
    auto rc = m_driver->get_chirp_timing(&chirp_timing);
    check_libavian_return(rc);

    /*
     * The chirp parameters specify the RF range, where TX amplifiers are enabled
     * ("emitting range"). After Avian state machine has turned on TX, it first
     * applies the ADC delay, before sampling starts. TX is automatically disabled
     * when ADC has finished. To get the "sampling range" the part of the ramp
     * during the ADC delay must be ignored. The RF ramp is linear so the sampling
     * delay can simply be scaled by the ratio of plain sampling time, and TX
     * active time.
     */
    const auto emitting_range = chirp->end_frequency_Hz - chirp->start_frequency_Hz;
    const auto sampling_time = static_cast<double>(chirp->num_samples) / chirp->sample_rate_Hz;
    const auto adc_delay = chirp_timing.adc_delay_100ps * 100e-12;
    const auto sampling_range = emitting_range * sampling_time / (sampling_time + adc_delay);

    return sampling_range;
}
