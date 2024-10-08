/**
 * \file ifxAvian_StrataPort.cpp
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
#include "ports/ifxAvian_StrataPort.hpp"

// universal
#include <common/endian/General.hpp>
#include <universal/error_definitions.h>
#include <universal/types/DataSettingsBgtRadar.h>

#include <thread>

// ---------------------------------------------------------------------------- namespaces
namespace Infineon {
namespace Avian {

// ---------------------------------------------------------------------------- StrataPort
StrataPort::StrataPort(BoardInstance* board) :
    StrataControlPortImpl(board)
{
    m_bridge_data = board->getIBridge()->getIBridgeData();
    m_data_index = 0;
    m_data = board->getIBridge()->getIBridgeControl()->getIData();

    // stop any ongoing device measurement and clear FIFO
    m_cmd->setBits(0x00, 0x00000C);
}

// ---------------------------------------------------------------------------- ~StrataPort
StrataPort::~StrataPort() = default;

// ---------------------------------------------------------------------------- start_reader
void StrataPort::start_reader(HW::Spi_Command_t burst_command,
                              size_t burst_size,
                              Data_Ready_Callback_t callback)
{
    /*
     * Before changing the reader configuration, any ongoing
     * acquisition must be stopped.
     */
    stop_reader();

    /*
     * Apply new reader configuration.
     * This will throw an exception in case configuration is invalid.
     */
    m_data_size = burst_size & 0xFFFF;
    const uint16_t address = burst_command & 0xFF;
    const uint16_t readouts[][2] =
        {
            {
                address,
                m_data_size,
            },
        };
    DataSettingsBgtRadar_t settings(&readouts);

    IDataProperties_t properties = {};
    properties.format = DataFormat_Packed12;  // receive packed data samples from device
    m_data->configure(m_data_index, &properties, &settings);

    /*
     * Turn on the callback, and allocate memory to queue
     * the for incoming data stream.
     */
    m_data_ready_callback = callback;

    /*
     * Buffer size is set to fit the SPI burst size where two 12 bit words
     * are packed into 3 bytes. The size of the frame queue is set to hold
     * twice the Avian FIFO size of 16384 samples.
     */
    const uint32_t bufferSize = m_data_size * 3 / 2;
    const uint16_t poolSize = 32768 / m_data_size;
    m_bridge_data->setFrameBufferSize(bufferSize);
    m_bridge_data->setFrameQueueSize(poolSize);
    m_bridge_data->registerListener(this);
    m_bridge_data->startStreaming();

    // Enable data transmission on the device.
    m_data->start(m_data_index);
}

// ---------------------------------------------------------------------------- stop_reader
void StrataPort::stop_reader()
{
    /*
     * First the acquisition itself is stopped to make sure no more data is
     * produced. Stopping data forwarding is the second step. The lock before
     * turning off the callback guarantees that no more callback is in progress
     * when this method returns.
     */
    m_data->stop(m_data_index);
    m_bridge_data->stopStreaming();
    m_bridge_data->registerListener(nullptr);

    std::lock_guard<std::mutex> lock(m_stop_guard);
    m_data_ready_callback = nullptr;
}

// ---------------------------------------------------------------------------- set_buffer
void StrataPort::set_buffer(HW::Packed_Raw_Data_t* buffer)
{
    m_buffer = buffer;
}

// ---------------------------------------------------------------------------- register_error_callback
void StrataPort::register_error_callback(Error_Callback_t callback)
{
    m_errorCallback = callback;
}

// ---------------------------------------------------------------------------- onNewFrame
void StrataPort::onNewFrame(IFrame* frame)
{
    const uint32_t status_code = frame->getStatusCode();

    if (status_code != 0)
    {
        /*
         * Emergency break!
         * Reasons include:
         * - connection problems
         * - FIFO overflow
         * - host not emptying received queue fast enough
         *
         * In any case, the stream is not consistent any more,
         * so it does not make sense to stream more data
         *
         * It is important to stop data acquisition before stopping
         * the streaming to avoid synchronization loss!
         */
        try
        {
            m_data->stop(m_data_index);
        }
        catch (...)
        {}
        m_bridge_data->stopStreaming();

        if (m_errorCallback)
        {
            m_errorCallback(this, status_code);
        }
        frame->release();
        return;
    }

    // handle raw data
    if (m_buffer)
    {
        std::copy(frame->getData(), frame->getData() + frame->getDataSize(),
                  m_buffer);
    }

    frame->release();

    // This lock helps the stop_reader method to wait for the end of a callback.
    std::lock_guard<std::mutex> lock(m_stop_guard);
    if (m_data_ready_callback)
        m_data_ready_callback(0);
}

/* ------------------------------------------------------------------------ */
}  // namespace Avian
}  // namespace Infineon
