/**
 * \file ifxAvian_StrataPort.hpp
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

#ifndef IFX_AVIAN_STRATA_PORT_H
#define IFX_AVIAN_STRATA_PORT_H

// ---------------------------------------------------------------------------- includes
#include "ports/ifxAvian_StrataControlPort.hpp"

#include <memory>
#include <mutex>

// ---------------------------------------------------------------------------- namespaces
namespace Infineon {
namespace Avian {

// ----------------------------------------------------------------------------
/**
 * This constant is a specialty of Hatvan Firmware. That firmware contains some
 * additional logic to detect a FIFO overflow (which is beyond the scope of
 * IPort). When Hatvan Firmware detects a FIFO overflow it invokes the error
 * callback and passes this error code to it.
 */
constexpr uint32_t ERR_FIFO_OVERFLOW = static_cast<uint32_t>(0x012E);

// ---------------------------------------------------------------------------- StrataPort
class StrataPort :
    public Internal::StrataControlPortImpl<HW::IPort<HW::Packed_Raw_Data_t>>,
    public IFrameListener<>
{
public:
    /**
     * \brief This type defines a callback function for notification when
     *        an error occurred.
     *
     * The error callback is called whenever an error occurred.
     *
     * The first argument is a pointer to the instance of
     * \ref StrataAvianPortAdapter.
     *
     * The error code is given as the second argument to the callback. The
     * error code corresponds to IFrame::getStatusCode. On a FIFO overflow
     * the value is given by \ref ERR_FIFO_OVERFLOW. All other values
     * correspond to communication problems (typically data loss between board
     * and host).
     */
    using Error_Callback_t = std::function<void(StrataPort*, uint32_t)>;

    StrataPort(BoardInstance* board);
    ~StrataPort() override;

    void start_reader(HW::Spi_Command_t burst_command, size_t burst_size,
                      Data_Ready_Callback_t callback) override;

    void stop_reader() override;

    void set_buffer(HW::Packed_Raw_Data_t* buffer) override;

    void onNewFrame(IFrame* frame) override;

    /**
     * \brief Register error callback.
     *
     * Register an error callback function. The function is called when
     * an error occurs (FIFO overflow or communication error).
     *
     * To unregister the error callback you pass a nullptr for
     * callback.
     *
     * \param [in]  callback    error callback or nullptr
     */
    void register_error_callback(Error_Callback_t callback);

private:
    IBridgeData* m_bridge_data;
    Error_Callback_t m_errorCallback = nullptr;
    std::mutex m_stop_guard;

    uint8_t m_data_index;

    Avian::HW::Packed_Raw_Data_t* m_buffer = nullptr;
    Data_Ready_Callback_t m_data_ready_callback = nullptr;
    uint16_t m_data_size = 0;

    IData* m_data;
};

/* ------------------------------------------------------------------------ */
}  // namespace Avian
}  // namespace Infineon

#endif /* IFX_AVIAN_STRATA_PORT_H */

/* --- End of File -------------------------------------------------------- */
