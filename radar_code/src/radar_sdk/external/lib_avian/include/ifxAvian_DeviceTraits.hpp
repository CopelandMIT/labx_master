/**
 * \file ifxAvian_DeviceTraits.hpp
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

#ifndef IFX_AVIAN_DEVICE_TRAITS_H
#define IFX_AVIAN_DEVICE_TRAITS_H

// ---------------------------------------------------------------------------- includes
#include "ifxAvian_Types.hpp"
#include <array>
#include <cstdint>

// ---------------------------------------------------------------------------- namespaces
namespace Infineon {
namespace Avian {

// ---------------------------------------------------------------------------- Device_Traits
/*!
 * This structure contains properties that vary with different derivatives
 * of Avian family devices.
 * Even though this structures is called "Traits" it does not mean trait as it
 * is commonly used in C++. The trait object is evaluated at runtime, it's
 * not a template.
 * Anyhow turning this into a real trait class and turning the driver class
 * into a template class using that trait class is planning for the
 * future...
 */
struct Device_Traits
{
    /*!
     * This type enumerates the different register layout version of the
     * channel set registers.
     */
    enum class Channel_Set_Layout
    {
        Version1,  //!< The legacy layout of Avian C devices.
        Version2,  //!< The new standard layout introduced with Avian D.
        Version3   //!< A slightly modified version introduced with BGT120UTR24.
    };

    //! This type enumerates the different versions of the WU register.
    enum class Wu_Register_Type
    {
        None,            //!< There is no WU register at all.
        T_WU_Only,       //!< WU register contains only T_WU related bit fields.
        T_WU_and_Offset  //!< The WU register additionally contains an T_Offset.
    };

    static const Device_Traits& get(Device_Type device_type);

    const char* description;
    std::pair<uint32_t, uint32_t> allowed_rf_range;
    uint8_t num_tx_antennas;
    uint8_t num_rx_antennas;
    uint8_t num_registers;
    Channel_Set_Layout cs_register_layout;
    bool has_extra_startup_delays;
    bool has_ref_frequency_doubler;
    uint8_t pll_pre_divider;
    uint8_t pll_div_set_80M;
    uint8_t pll_div_set_76M8;
    bool has_sadc;
    bool has_explicit_sadc_bg_div_control;
    Wu_Register_Type wu_register_type;
    bool supports_tx_toggling;
    bool has_programmable_fifo_power_mode;
    bool has_programmable_pad_driver;
    bool has_programmable_pullup_resistors;
    bool has_device_id;
    bool has_reordered_register_layout;
    bool has_internal_oscillator;
    bool has_local_oscillator_frequency_doubler;
    uint16_t fifo_size;

    /*
     * This array holds the SADC input channels the power sensor of each TX
     * antenna is connected to. The output of TX power sensors is differential
     * but SADC input is single ended. That's why each TX antenna requires a
     * pair of SADC input channels.
     */
    std::array<std::pair<uint8_t, uint8_t>, 2> sadc_power_channels;

    /*
     * This array holds the possible cutoff frequency settings of the analog
     * high pass filter in Hz. Not all values are always used. The list
     * of valid elements of the array are followed by -1, meaning end of list.
     */
    std::array<int32_t, 7> hpf_cutoff_settings;

    /*
     * This array holds the possible cutoff frequency settings of the analog
     * anti alias low pass filter in Hz. Not all values are always used. The list
     * of valid elements of the array are followed by -1, meaning end of list.
     */
    std::array<int32_t, 5> aaf_cutoff_settings;

    /*
     * This array holds the possible HP gain settings in dB.
     * Not all values are always used. The list of valid elements of the
     * array are followed by -1, meaning end of list.
     */
    std::array<int8_t, 3> hp_gain_settings;

    /*
     * This array holds the possible VGA gain settings in dB.
     * Not all values are always used. The list of valid elements of the
     * array are followed by -1, meaning end of list.
     */
    std::array<int8_t, 8> vga_gain_settings;


private:
    /*
     * User code must not create local instances. It's only allowed to
     * use references to predefined instances, which represent supported
     * devices.
     */
    Device_Traits() = delete;
    Device_Traits(const Device_Traits&) = delete;
    Device_Traits(Device_Traits&&) = delete;
};

/* ------------------------------------------------------------------------ */
}  // namespace Avian
}  // namespace Infineon

#endif /* IFX_AVIAN_DEVICE_TRAITS_H */

/* --- End of File -------------------------------------------------------- */
