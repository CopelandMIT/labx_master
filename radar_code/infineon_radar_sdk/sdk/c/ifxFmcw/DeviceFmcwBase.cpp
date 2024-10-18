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

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include "DeviceFmcwBase.hpp"
#include "ifxBase/internal/Util.h"  // for ifx_util_popcount

// Universal
#include <universal/error_definitions.h>
#include <universal/types/DataSettingsBgtRadar.h>

#include <chrono>
#include <common/Buffer.hpp>
#include <stack>


/*
==============================================================================
   2. DEFINITIONS
==============================================================================
*/
namespace {

// Sending a lot of slices from the board to the host is not efficient.
// If possible we try to avoid slice rates higher than 20 Hz.
constexpr float slice_rate_threshold = 20.0f;  // 20 slices per second

constexpr float seconds_to_buffer = 10.0f;

}  // namespace

/*
==============================================================================
   5. LOCAL FUNCTION PROTOTYPES
==============================================================================
*/

DeviceFmcwBase::DeviceFmcwBase(ifx_Float_t max_adc_value) :
    m_max_adc_value {max_adc_value}
{
    rdk::RadarDeviceCommon::get_firmware_info(nullptr, &m_firmware_info);
}

DeviceFmcwBase::DeviceFmcwBase(ifx_Float_t max_adc_value, std::unique_ptr<BoardInstance>&& board) :
    m_max_adc_value {max_adc_value},
    m_board {std::move(board)}
{
    if (!m_board)
    {
        throw rdk::exception::no_device();
    }

    rdk::RadarDeviceCommon::get_firmware_info(m_board.get(), &m_firmware_info);

    /* Initialize data component related member variables */
    m_data_index = 0;
    m_data = m_board->getIBridge()->getIBridgeControl()->getIData();
    m_bridge_data = m_board->getIBridge()->getIBridgeData();
}

uint16_t DeviceFmcwBase::calculate_slice_size(uint32_t fifo_size) const
{
    if (m_num_samples == 0)
    {
        throw rdk::exception::num_samples_out_of_range();
    }

    // Take half the FIFO size as a hard cap to have enough buffer to
    // prevent FIFO overflows in the Avian sensor.
    // The Avian sensor will trigger an interrupt if there are at least
    // max_slice_size samples in the internal FIFO. The FW will then read the FIFO.
    // As handling the interrupt
    // takes a bit of time and in the mean time more samples are put in the
    // FIFO, max_slice_size must be smaller than the FIFO size.
    const uint32_t max_slice_size = fifo_size / 2;

    // Determine how many slices are needed for one frame
    const auto num_slices_per_frame = (m_num_samples + (max_slice_size - 1)) / max_slice_size;

    uint32_t slice_size = m_num_samples / num_slices_per_frame;

    // Compute the slice rate from the slice_size. The slice rate is the
    // number of slices that are generated per second for a given slice
    // size.
    const auto slice_rate = 1.0f * m_num_samples / slice_size / m_frame_repetition_time_s;
    if (slice_rate > slice_rate_threshold)
    {
        // Even though the full frame fits into a single slice, the
        // resulting slice rate would be too high.
        // To avoid sending many small slices, we try to send k frames
        // as one slice, to achieve a slice rate of about 20Hz.
        // However, we can not exceed the max_slice_size.
        const auto k = static_cast<uint32_t>(slice_rate / slice_rate_threshold);

        slice_size = std::min(slice_size * k, max_slice_size);
    }

    return slice_size;
}

const ifx_Firmware_Info_t* DeviceFmcwBase::get_firmware_info() const
{
    return &m_firmware_info;
}

const ifx_Radar_Sensor_Info_t* DeviceFmcwBase::get_sensor_info() const
{
    return &m_sensor_info;
}

const char* DeviceFmcwBase::get_board_uuid() const
{
    if (!m_board)
    {
        static const char invalid_uuid[] = "00000000-0000-0000-0000-000000000000";
        return invalid_uuid;
    }

    return m_board->getUuidString().c_str();
}

ifx_Fmcw_Frame_t* DeviceFmcwBase::allocate_frame()
{
    update_defaults_if_not_configured();

    auto* frame = new ifx_Fmcw_Frame_t;
    if (!frame)
    {
        throw rdk::exception::memory_allocation_failed();
    }

    frame->num_cubes = static_cast<uint32_t>(m_frame_dimensions.size());
    frame->cubes = new ifx_Mda_R_t*[frame->num_cubes];
    if (frame->cubes == nullptr)
    {
        delete frame;
        throw rdk::exception::memory_allocation_failed();
    }

    for (size_t i = 0; i < frame->num_cubes; i++)
    {
        frame->cubes[i] = ifx_mda_create_r(static_cast<uint32_t>(m_frame_dimensions[i].size()), m_frame_dimensions[i].data());
    }

    return frame;
}

ifx_Fmcw_Raw_Frame_t* DeviceFmcwBase::allocate_raw_frame()
{
    update_defaults_if_not_configured();

    auto* raw_frame = new ifx_Fmcw_Raw_Frame_t;
    if (raw_frame == nullptr)
    {
        throw rdk::exception::memory_allocation_failed();
    }
    raw_frame->num_samples = m_num_samples;
    raw_frame->samples = new uint16_t[raw_frame->num_samples];
    if (raw_frame->samples == nullptr)
    {
        delete raw_frame;
        throw rdk::exception::memory_allocation_failed();
    }

    return raw_frame;
}

void DeviceFmcwBase::start_data()
{
    m_data->start(m_data_index);
    m_bridge_data->startStreaming();
}

void DeviceFmcwBase::stop_data()
{
    m_data->stop(m_data_index);
    m_bridge_data->stopStreaming();
    m_slice.reset();
}

void DeviceFmcwBase::configure_data(uint16_t slice_size, uint16_t readout_address, uint8_t data_format)
{
    const uint16_t readouts[][2] = {
        {
            readout_address,
            slice_size,
        },
    };
    DataSettingsBgtRadar_t settings(&readouts);

    IDataProperties_t properties = {};
    properties.format = data_format;
    m_data->configure(m_data_index, &properties, &settings);

    m_data_format = data_format;
    m_frame_length = get_buffer_length(m_num_samples);

    m_bridge_data->setFrameBufferSize(get_buffer_length(slice_size));

    /* The size of the frame queue is derived from the config, allowing to hold
     * samples for a defined seconds_to_buffer time.
     */
    const auto pool_size = static_cast<uint16_t>(seconds_to_buffer / m_frame_repetition_time_s);
    m_bridge_data->setFrameQueueSize(pool_size);
}

uint32_t DeviceFmcwBase::copy_slice_data(uint8_t data_format, const uint8_t* buffer, uint32_t buffer_length, uint16_t* output)
{
    uint32_t num_samples;
    switch (data_format)
    {
        case DataFormat_Packed12:
            num_samples = buffer_length / 3 * 2;
            // unpack each 12-bit sample into a 16-bit word
            for (size_t i = 0; i < num_samples / 2; i++)
            {
                *output++ = (buffer[3 * i + 0] << 4) | (buffer[3 * i + 1] >> 4);
                *output++ = ((buffer[3 * i + 1] & 0x0f) << 8) | buffer[3 * i + 2];
            }
            break;
        case DataFormat_Raw16:
            num_samples = buffer_length / 2;
            // each sample is already stored into a 16-bit word
            {
                const auto* source = reinterpret_cast<const uint16_t*>(buffer);
                std::copy(source, source + num_samples, output);
            }
            break;
        default:
            throw rdk::exception::argument_invalid();
            break;
    }
    return num_samples;
}

void DeviceFmcwBase::get_next_frame(ifx_Fmcw_Frame_t* frame, uint16_t timeout_ms)
{
    if (frame == nullptr)
    {
        throw rdk::exception::argument_null();
    }

    if (frame->num_cubes != m_frame_dimensions.size())
    {
        throw rdk::exception::dimension_mismatch();
    }

    start_acquisition();

    SmartFmcwRawFrame raw_frame(allocate_raw_frame());
    get_next_raw_frame(raw_frame.get(), timeout_ms);

    const auto* raw_data = raw_frame->samples;
    auto** cubes = frame->cubes;
    const auto cube_offset = frame->num_cubes - 1;
    for (const auto& d : m_frame_dimensions)
    {
        const auto num_rx = d[0];
        const auto num_chirps = d[1];
        const auto num_samples_per_chirp = d[2];
        const auto* cube = *cubes++;
        const auto* shape = IFX_MDA_SHAPE(cube);
        // check if dimensions of given and expected cube are the same
        if ((IFX_MDA_DIMENSIONS(cube) != 3)
            || (shape[0] != num_rx)
            || (shape[1] != num_chirps)
            || (shape[2] != num_samples_per_chirp))
        {
            throw rdk::exception::dimension_mismatch();
        }

        // the outer loop assumes a flat (non-nested) chirp structure,
        // where all chirps have the same settings.
        // However, this is only guaranteed when using the legacy API
        const auto chirp_offset = num_rx * num_samples_per_chirp;
        const auto* cube_data = raw_data;
        for (uint32_t chirp = 0; chirp < num_chirps; chirp++)                    // slices
        {
            for (uint32_t sample = 0; sample < num_samples_per_chirp; sample++)  // rows
            {
                for (uint32_t rx = 0; rx < num_rx; rx++)                         // columns
                {
                    IFX_MDA_AT(cube, rx, chirp, sample) = static_cast<ifx_Float_t>(*cube_data++ * 2) / m_max_adc_value - 1.0f;
                }
            }
            if (m_mimo)
            {
                cube_data += cube_offset * chirp_offset;
            }
        }
        if (m_mimo)
        {
            raw_data += chirp_offset;
        }
        else
        {
            raw_data = cube_data;
        }
    }
}

void DeviceFmcwBase::get_next_raw_frame(ifx_Fmcw_Raw_Frame_t* frame, uint16_t timeout_ms)
{
    if (frame == nullptr)
    {
        throw rdk::exception::argument_null();
    }

    if (frame->num_samples != m_num_samples)
    {
        throw rdk::exception::dimension_mismatch();
    }

    start_acquisition();

    uint16_t* frame_ptr = frame->samples;
    auto remaining_bytes = m_frame_length;
    const auto expiry = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);
    while (remaining_bytes)
    {
        const auto remaining_timeout_ms = std::chrono::duration_cast<std::chrono::milliseconds>(expiry - std::chrono::steady_clock::now()).count();
        if (remaining_timeout_ms <= 0)
        {
            throw rdk::exception::timeout();
        }

        if (!m_slice)
        {
            // get next slice if no previous slice has been saved
            m_slice.reset(m_bridge_data->getFrame(static_cast<uint16_t>(remaining_timeout_ms)));
            if (!m_slice)
            {
                throw rdk::exception::timeout();
            }
        }

        const auto status = m_slice->getStatusCode();
        if (status)
        {
            m_slice.reset();
            switch (status)
            {
                case DataError_NoError:
                    break;
                case DataError_FrameDropped:
                case DataError_FramePoolDepleted:
                case DataError_FrameQueueTrimmed:
                    throw rdk::exception::frame_acquisition_failed();
                    break;
                case DataError_FrameSizeExceeded:
                    throw rdk::exception::frame_size_not_supported();
                case E_OVERFLOW:
                    throw rdk::exception::fifo_overflow();
                    break;
                default:
                    throw rdk::exception::error();
                    break;
            }
        }

        const auto slice_size = m_slice->getDataSize();
        if (remaining_bytes < slice_size)
        {
            // frame is finshed, and there is data from the next frame in the slice to keep for the next call
            copy_slice_data(m_data_format, m_slice->getData(), remaining_bytes, frame_ptr);
            m_slice->setDataOffsetAndSize(remaining_bytes, slice_size - remaining_bytes);
            return;
        }
        else
        {
            // the slice is completely used and can be released
            const auto samples = copy_slice_data(m_data_format, m_slice->getData(), slice_size, frame_ptr);
            m_slice.reset();
            frame_ptr += samples;
            remaining_bytes -= slice_size;
        }
    }
}

void DeviceFmcwBase::update_frame_settings()
{
    get_frame_dimensions();

    m_num_samples = 0;
    for (auto& frame_dimension : m_frame_dimensions)
    {
        uint32_t cube_size = 1;
        for (auto& cube_dimension : frame_dimension)
            cube_size *= cube_dimension;

        m_num_samples += cube_size;
    }
}

void DeviceFmcwBase::update_defaults_if_not_configured()
{
    if (m_num_samples)
    {
        return;
    }

    update_frame_settings();
}

double DeviceFmcwBase::get_chirp_sampling_bandwidth(const ifx_Fmcw_Sequence_Chirp_t* chirp) const
{
    const auto sampling_bandwidth = std::abs(get_chirp_sampling_range(chirp));

    return sampling_bandwidth;
}

void DeviceFmcwBase::convert_raw_data_to_float_array(uint32_t num_samples, const uint16_t* raw_data, ifx_Float_t* converted_frame)
{
    for (size_t i = 0; i < num_samples; i++)
    {
        converted_frame[i] = 2 * (static_cast<ifx_Float_t>(raw_data[i]) / m_max_adc_value) - 1.0f;
    }
}

void DeviceFmcwBase::deinterleave_raw_frame(const ifx_Fmcw_Raw_Frame_t* raw_frame, ifx_Fmcw_Raw_Frame_t* deinterleaved_frame)
{
    if (!raw_frame)
    {
        throw rdk::exception::argument_null();
    }
    const uint16_t* raw_data_ptr = raw_frame->samples;

    std::stack<const ifx_Fmcw_Sequence_Element_t*> loops_stack;
    std::stack<const ifx_Fmcw_Sequence_Element_t*> chirps_stack;
    const size_t num_cubes = m_frame_dimensions.size();
    uint32_t num_of_chirps_in_loop = 0;
    uint32_t chirp_index;
    std::vector<uint32_t> remaining_chirp_repetitions;
    remaining_chirp_repetitions.reserve(num_cubes);
    for (const auto& d : m_frame_dimensions)
    {
        remaining_chirp_repetitions.emplace_back(d[1]);
    }

    auto* sequence = get_acquisition_sequence();
    const auto* current_element = sequence;

    // At the beginning, it is needed to check if the sequence starts with the frame loop in order to skip it
    if ((current_element->type == IFX_SEQ_LOOP) && (current_element->next_element == nullptr))
    {
        current_element = current_element->loop.sub_sequence;
    }

    /* Iterate over the sequence, as long as the last linked list node is not reached. */
    while (current_element != nullptr)
    {
        switch (current_element->type)
        {
            case IFX_SEQ_CHIRP:
                {
                    const ifx_Fmcw_Sequence_Chirp_t& current_chirp = current_element->chirp;
                    uint32_t num_rx = ifx_util_popcount(current_chirp.rx_mask);
                    uint32_t num_samples_per_chirp = current_chirp.num_samples;
                    chirp_index = static_cast<uint32_t>(chirps_stack.size());
                    chirps_stack.push(current_element);
                    num_of_chirps_in_loop++;

                    // determine the offset
                    uint32_t offset = (m_frame_dimensions[chirp_index][1] - remaining_chirp_repetitions[chirp_index]) * (m_frame_dimensions[chirp_index][0] * m_frame_dimensions[chirp_index][2]);
                    if (chirp_index >= 1)
                    {
                        for (size_t i = 0; i < chirp_index; i++)
                        {
                            offset += m_frame_dimensions[i][1] * (m_frame_dimensions[i][0] * m_frame_dimensions[i][2]);
                        }
                    }

                    // de-interleave
                    for (size_t i = 0; i < num_rx; i++)
                    {
                        std::copy(raw_data_ptr, raw_data_ptr + num_samples_per_chirp, deinterleaved_frame->samples + offset + i * num_samples_per_chirp);
                        raw_data_ptr += num_samples_per_chirp;
                    }

                    // Update remaining chirp repetitions
                    remaining_chirp_repetitions[chirp_index] -= 1;

                    // In this case, the loop/ nested loops are still being processed we need to go back then to the previous chirp
                    if ((current_element->next_element == nullptr) && (remaining_chirp_repetitions[chirp_index] > 0))
                    {
                        current_element = loops_stack.top()->loop.sub_sequence;
                        for (size_t i = 0; i < num_of_chirps_in_loop; i++)
                        {
                            chirps_stack.pop();
                        }
                        num_of_chirps_in_loop = 0;
                        continue;
                    }

                    break;
                }
            case IFX_SEQ_LOOP:
                {
                    num_of_chirps_in_loop = 0;
                    loops_stack.push(current_element);
                    current_element = current_element->loop.sub_sequence;
                    continue;
                }
            default:
                break;
        }

        current_element = current_element->next_element;
        // The next element is null in case the end of the nested loop / loops, or the end of the sequence is reached
        while ((current_element == nullptr) && (!loops_stack.empty()))
        {
            current_element = loops_stack.top()->next_element;
            loops_stack.pop();
        }
    }

    ifx_fmcw_destroy_sequence(sequence);
}

/* This function traverses the sequence in order to set the frame dimensions.
 * In this function, each current_element is pushed into the stack, and depending on the type the cube dimensions are assigned
 * (when the element is a chirp), or a search for the next chirp is started (when the element is a loop).
 * When the subsequence ends (current_element == nullptr), the top of the stack which has a valid next element,
 * is popped and assigned to the current_element.
 */
void DeviceFmcwBase::get_frame_dimensions()
{
    m_frame_dimensions.clear();
    m_mimo = false;

    uint32_t num_repetitions = 1;
    std::stack<const ifx_Fmcw_Sequence_Element_t*> stack;
    auto* sequence = get_acquisition_sequence();
    const auto* current_element = sequence;

    // At the beginning, it is needed to check if the sequence starts with the frame loop in order to skip it
    if ((current_element->type == IFX_SEQ_LOOP) && (current_element->next_element == nullptr))
    {
        m_frame_repetition_time_s = current_element->loop.repetition_time_s;
        current_element = current_element->loop.sub_sequence;
    }
    else
    {
        m_frame_repetition_time_s = 0.0f;
    }

    /* Iterate over the sequence, as long as the last linked list node is not reached. */
    while (current_element != nullptr)
    {
        switch (current_element->type)
        {
            case IFX_SEQ_CHIRP:
                {
                    const ifx_Fmcw_Sequence_Chirp_t& current_chirp = current_element->chirp;
                    uint32_t num_rx = ifx_util_popcount(current_chirp.rx_mask);
                    uint32_t num_samples_per_chirp = current_chirp.num_samples;
                    m_frame_dimensions.push_back({num_rx, num_repetitions, num_samples_per_chirp});
                    if (!m_mimo)
                    {
                        m_mimo = (current_element->next_element != nullptr);
                    }
                    break;
                }
            case IFX_SEQ_LOOP:
                {
                    num_repetitions *= current_element->loop.num_repetitions;
                    stack.push(current_element);
                    current_element = current_element->loop.sub_sequence;
                    continue;
                }
            default:
                break;
        }

        current_element = current_element->next_element;
        // The next element is null in case the end of the nested loop / loops, or the end of the sequence is reached
        while ((current_element == nullptr) && (!stack.empty()))
        {
            num_repetitions = 1;
            current_element = stack.top()->next_element;
            stack.pop();
        }
    }

    ifx_fmcw_destroy_sequence(sequence);
}

uint32_t DeviceFmcwBase::get_buffer_length(uint32_t num_samples) const
{
    switch (m_data_format)
    {
        case DataFormat_Packed12:
            // Buffer size fits SPI burst size where two 12-bit samples are packed into three 8-bit words
            return num_samples * 3 / 2;
            break;
        case DataFormat_Raw16:
            // Buffer size fits SPI burst size where each 16-bit sample is stored into a 16-bit word
            return num_samples * sizeof(uint16_t);
            break;
        default:
            throw rdk::exception::argument_invalid();
            break;
    }
}

void DeviceFmcwBase::view_deinterleaved_frame(ifx_Float_t* converted_frame, ifx_Fmcw_Frame_t* deinterleaved_frame_view)
{
    for (size_t i = 0; i < m_frame_dimensions.size(); i++)
    {
        ifx_Mda_R_t* cube = deinterleaved_frame_view->cubes[i];
        cube->flags &= ~IFX_MDA_FLAG_OWNS_DATA;
        auto num_samples_per_cube = m_frame_dimensions[i][0] * m_frame_dimensions[i][1] * m_frame_dimensions[i][2];
        cube->data = converted_frame;
        converted_frame += num_samples_per_cube;
    }
}

float DeviceFmcwBase::get_sequence_duration(const ifx_Fmcw_Sequence_Element_t* sequence) const
{
    // we need double precision for the summation itself, since adding up
    // several small float numbers will run into quantization inaccuracies
    double duration = 0.0;
    while (sequence != nullptr)
    {
        duration += get_element_duration(sequence);
        sequence = sequence->next_element;
    }
    return static_cast<float>(duration);
}

float DeviceFmcwBase::get_element_duration(const ifx_Fmcw_Sequence_Element_t* element) const
{
    switch (element->type)
    {
        case IFX_SEQ_LOOP:
            {
                const ifx_Fmcw_Sequence_Loop_t& loop = element->loop;
                return (loop.num_repetitions * loop.repetition_time_s);
            }
        case IFX_SEQ_CHIRP:
            {
                const ifx_Fmcw_Sequence_Chirp_t& chirp = element->chirp;
                const float chirp_duration_s = get_chirp_duration(chirp);
                return chirp_duration_s;
            }
        case IFX_SEQ_DELAY:
            {
                const ifx_Fmcw_Sequence_Delay_t& delay = element->delay;
                return delay.time_s;
            }
        default:
            return 0;
    }
}

double DeviceFmcwBase::get_chirp_sampling_center_frequency(const ifx_Fmcw_Sequence_Chirp_t* chirp) const
{
    // FIXME: Wrong calculation! The method shall return the center frequency of the emitted frequency (i.e.,
    //      when the ADC starts sampling) which is slightly different from the configured frequency.
    //      "get_chirp_sampling_range()" could return a range (with start/end frequency which takes ADC sample start time into account).
    const auto sampling_range = get_chirp_sampling_range(chirp);
    const auto center_frequency = chirp->start_frequency_Hz + (sampling_range / 2.0);

    return center_frequency;
}
