/**
 * \file ifxAvian_ParameterExtractor.cpp
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
#include "ifxAvian_ParameterExtractor.hpp"
#include "Driver/registers_BGT120TR24E.h"
#include "Driver/registers_BGT60TR11D.h"
#include "Driver/registers_BGT60TRxxC.h"
#include "Driver/registers_BGT60TRxxD.h"
#include "Driver/registers_BGT60TRxxE.h"
#include "ifxAvian_DeviceTraits.hpp"
#include "ifxAvian_Utilities.hpp"
#include "value_conversion/ifxAvian_RfConversion.hpp"
#include "value_conversion/ifxAvian_TimingConversion.hpp"
#include <algorithm>
#include <string>

// ---------------------------------------------------------------------------- namespaces
namespace Infineon {
namespace Avian {

using namespace Value_Conversion;
using Cs_Layout_t = Device_Traits::Channel_Set_Layout;

// ---------------------------------------------------------------------------- constants
static constexpr uint8_t s_inc_pll = BGT60TRxxC_REG_PLL2_0
                                     - BGT60TRxxC_REG_PLL1_0;
static constexpr uint8_t s_inc_cs = BGT60TRxxC_REG_CS2
                                    - BGT60TRxxC_REG_CS1;
static constexpr uint8_t s_inc_cs_d = BGT60TRxxC_REG_CS1_D_0
                                      - BGT60TRxxC_REG_CS1_U_0;

// ---------------------------------------------------------------------------- Parameter_Extractor
Parameter_Extractor::Parameter_Extractor(const std::vector<uint32_t>& registers,
                                         Device_Type device_type) :
    m_device_type(device_type),
    m_reference_clock(Reference_Clock_Frequency::_80MHz)
{
    /*
     * The internal register list is initialized from the provided register
     * list. For each register value the according address is added.
     */
    uint8_t address = 0;
    for (auto uValue : registers)
        m_registers.emplace(address++, uValue);

    detect_type_and_clock();
}

// ---------------------------------------------------------------------------- Parameter_Extractor
Parameter_Extractor::Parameter_Extractor(std::map<uint8_t, uint32_t> registers,
                                         Device_Type device_type) :
    m_registers(std::move(registers)),
    m_device_type(device_type),
    m_reference_clock(Reference_Clock_Frequency::_80MHz)
{
    detect_type_and_clock();
}

// ---------------------------------------------------------------------------- get_register
/**
 * Other functions should use this function instead of looking up registers
 * directly from m_registers, because this function translates the generic
 * out_of_range exception into an exception with a more meaningful description.
 */
uint32_t Parameter_Extractor::get_register(uint8_t address,
                                           const char* function_name) const
{
    try
    {
        return m_registers.at(address);
    }
    catch (std::out_of_range&)
    {
        throw Error("Register " + std::to_string(address) + " required by"
                    + " \"" + function_name + "\" is not defined.");
    }
}

/*
 * For convenience this macro is defined that automatically adds the function
 * name for the generation of the exception description.
 */
#define GET_REGISTER(ADDRESS) get_register(ADDRESS, __func__)

// ---------------------------------------------------------------------------- detect_type_and_clock
void Parameter_Extractor::detect_type_and_clock()
{
    /*
     * First the device type is detected from the register list and compared
     * to the device type provided to the constructor. An exception is thrown,
     * if device types do not match or if no type was specified and no type
     * could be detected.
     */
    try
    {
        auto reg_CHIP_VERSION = GET_REGISTER(BGT60TRxxC_REG_CHIP_ID);
        auto detected_type = detect_device_type(reg_CHIP_VERSION);

        if (detected_type == Device_Type::Unknown)
            throw Error("Device type could not be detected,"
                        " register CHIP_VERSION contains an unknown IDs.");

        if (m_device_type != Device_Type::Unknown)
        {
            if (m_device_type != detected_type)
                throw Error("Detected device type does not match specified"
                            " device type.");
        }
        else
        {
            m_device_type = detected_type;
        }
    }
    catch (...)
    {
        if (m_device_type == Device_Type::Unknown)
            throw Error("Device type could not be detected,"
                        " register CHIP_VERSION is missing.");
    }

    /*
     * If the device supports doubling of the reference clock, it is
     * expected that the according bit fields are programmed. To detect
     * the frequency of the reference oscillator it is checked, if the
     * doubler is enabled.
     */
    bool double_clock = false;
    auto& sTraits = Device_Traits::get(m_device_type);
    if (sTraits.has_ref_frequency_doubler)
    {
        auto reg_FD = sTraits.has_reordered_register_layout
                          ? GET_REGISTER(BGT60TRxxE_REG_FD)
                          : GET_REGISTER(BGT60TRxxD_REG_FD);
        double_clock = BGT60TRxxD_EXTRACT(FD, CLK_SEL, reg_FD) != 0;
    }

    /*
     * For 60GHz Avian devices also a reference clock of 76.8MHz is
     * supported. Depending on the reference clock the PLL divider offset
     * is programmed differently. This is taken into account here to
     * reconstruct the reference oscillator settings, the imported
     * registers were made for.
     */
    if ((m_device_type != Device_Type::BGT120UTR13E)
        && (m_device_type != Device_Type::BGT24LTR24))
    {
        auto reg_PACR2 = GET_REGISTER(BGT60TRxxC_REG_PACR2);
        auto div_set = BGT60TRxxC_EXTRACT(PACR2, DIVSET, reg_PACR2);

        if (div_set == 20)
        {
            m_reference_clock = double_clock
                                    ? Reference_Clock_Frequency::_40MHz
                                    : Reference_Clock_Frequency::_80MHz;
        }
        else if (div_set == 21)
        {
            m_reference_clock = double_clock
                                    ? Reference_Clock_Frequency::_38_4MHz
                                    : Reference_Clock_Frequency::_76_8MHz;
        }
        else
        {
            throw Error("The value of bit field PACR2:DIV_SET is invalid."
                        " Frequency of reference oscillator could not be"
                        " detected.");
        }
    }
    else
    {
        /*
         * Technically also 120GHz and 24GHz devices could use 76.8MHz
         * clock frequency, but this is not supported yet, and the
         * according DIV_SET values haven't been calculated.
         */
        m_reference_clock = double_clock
                                ? Reference_Clock_Frequency::_40MHz
                                : Reference_Clock_Frequency::_80MHz;
    }
}

// ---------------------------------------------------------------------------- get_clock_frequency
inline uint32_t Parameter_Extractor::get_clock_frequency() const
{
    return ((m_reference_clock == Reference_Clock_Frequency::_38_4MHz)
            || (m_reference_clock == Reference_Clock_Frequency::_76_8MHz))
               ? 76800000
               : 80000000;
}

// ---------------------------------------------------------------------------- to_duration
/**
 * This is a utility function that converts the bit fields of multiplied
 * counters into time steps of 100 picoseconds.
 */
uint64_t Parameter_Extractor::to_100ps(uint32_t tr_x, uint32_t tr_x_mul) const
{
    Scaled_Timer sTimer(static_cast<uint8_t>(tr_x),
                        static_cast<uint8_t>(tr_x_mul));
    return cycles_to_100ps(sTimer.get_clock_cycles(), get_clock_frequency());
}

// ---------------------------------------------------------------------------- get_device_type
Device_Type Parameter_Extractor::get_device_type() const
{
    return m_device_type;
}

// ---------------------------------------------------------------------------- get_reference_clock
Reference_Clock_Frequency Parameter_Extractor::get_reference_clock() const
{
    return m_reference_clock;
}

// ---------------------------------------------------------------------------- get_slice_size
uint16_t Parameter_Extractor::get_slice_size() const
{
    auto reg_SFCTL = GET_REGISTER(BGT60TRxxC_REG_SFCTL);
    auto fifo_cref = BGT60TRxxC_EXTRACT(SFCTL, FIFO_CREF, reg_SFCTL);

    /*
     * The value of bit field FIFO_CREF refers to 24 bit words. Each FIFO data
     * word contains two 12 bit sample values. The state machine internally
     * adds one.
     */
    return uint16_t((fifo_cref + 1) * 2);
}

// ---------------------------------------------------------------------------- get_frame_definition
Frame_Definition Parameter_Extractor::get_frame_definition() const
{
    Frame_Definition frame_def;

    // Power mode extraction is used for shapes and shape set
    auto extract_power_mode = [](uint32_t pd_mode, uint32_t cont_mode,
                                 std::string shape) -> Power_Mode {
        switch (pd_mode)
        {
            case 0: return Power_Mode::Stay_Active;
            case 1: return Power_Mode::Idle;
            case 2: return (cont_mode == 0)
                               ? Power_Mode::Deep_Sleep
                               : Power_Mode::Deep_Sleep_Continue;
            default:
                throw Error("Invalid configuration. Value of bit field PD_MODE"
                            " for shape"
                            + shape + "is reserved.");
        }
    };

    /*
     * This loop extracts the parameter of each shape. The number of shape
     * groups per set (= enabled shapes) is counted during the loop, because
     * it is needed later to calculate the number of shape set repetitions.
     */
    uint8_t num_groups_per_set = 0;
    for (uint8_t shp = 0; shp < 4; ++shp)
    {
        // Relevant bit fields are extracted.
        auto reg_CSCx = GET_REGISTER(BGT60TRxxC_REG_CS1 + shp * s_inc_cs);
        auto reg_PLLx_7 = GET_REGISTER(BGT60TRxxC_REG_PLL1_7 + shp * s_inc_pll);
        auto cs_en = BGT60TRxxC_EXTRACT(CS1, CS_EN, reg_CSCx);
        auto repc = BGT60TRxxC_EXTRACT(CS1, REPC, reg_CSCx);
        auto sh_en = BGT60TRxxC_EXTRACT(PLL1_7, SH_EN, reg_PLLx_7);
        auto reps = BGT60TRxxC_EXTRACT(PLL1_7, REPS, reg_PLLx_7);
        auto pd_mode = BGT60TRxxC_EXTRACT(PLL1_7, PD_MODE, reg_PLLx_7);
        auto cont_mode = BGT60TRxxC_EXTRACT(PLL1_7, CONT_MODE, reg_PLLx_7);
        auto tr_sed = BGT60TRxxC_EXTRACT(PLL1_7, T_SED, reg_PLLx_7);
        auto tr_sed_mul = BGT60TRxxC_EXTRACT(PLL1_7, T_SED_MUL, reg_PLLx_7);

        /*
         * It is expected that shapes and channel set enabling and repetitions
         * are programmed equally, because that's how configurations are
         * generated by class Driver.
         */
        if ((sh_en != cs_en) || (reps != repc))
            throw Error("Invalid configuration. Enabling and/or repetitions"
                        "of shape "
                        + std::to_string(shp) + " and according"
                                                " channel set do not match.");

        // Shape parameters are determined from bit fields.
        auto& shape = frame_def.shapes[shp];
        shape.num_repetitions = (sh_en != 0) ? (1 << reps) : 0;
        if (sh_en != 0)
            ++num_groups_per_set;

        shape.following_power_mode = extract_power_mode(pd_mode, cont_mode,
                                                        std::to_string(shp));
        shape.post_delay_100ps = to_100ps(tr_sed, tr_sed_mul);
    }
    if (num_groups_per_set == 0)
        throw Error("Invalid Configuration. All shapes are disabled.");

    // Relevant bit fields for shape set configuration are extracted.
    auto reg_CCR0 = GET_REGISTER(BGT60TRxxC_REG_CCR0);
    auto reg_CCR1 = GET_REGISTER(BGT60TRxxC_REG_CCR1);
    auto reg_CCR2 = GET_REGISTER(BGT60TRxxC_REG_CCR2);
    auto frame_len = BGT60TRxxC_EXTRACT(CCR2, FRAME_LEN, reg_CCR2);
    auto pd_mode = BGT60TRxxC_EXTRACT(CCR1, PD_MODE, reg_CCR1);
    auto cont_mode = BGT60TRxxC_EXTRACT(CCR0, CONT_MODE, reg_CCR0);
    auto tr_fed = BGT60TRxxC_EXTRACT(CCR1, TFED, reg_CCR1);
    auto tr_fed_mul = BGT60TRxxC_EXTRACT(CCR1, TFED_MUL, reg_CCR1);
    auto max_frame_cnt = BGT60TRxxC_EXTRACT(CCR2, MAX_FRAME_CNT, reg_CCR2);

    // Shape set parameters are determined from bit fields.
    auto& shape_set = frame_def.shape_set;

    uint16_t num_groups_per_frame = uint16_t(frame_len + 1);
    shape_set.num_repetitions = num_groups_per_frame / num_groups_per_set;
    if (num_groups_per_frame % num_groups_per_set != 0)
        throw Error("Invalid Configuration. Value of bit field CCR2:FRAME_LEN"
                    " is not an integer multiple of shape set length.");

    shape_set.following_power_mode = extract_power_mode(pd_mode, cont_mode,
                                                        "set");
    shape_set.post_delay_100ps = to_100ps(tr_fed, tr_fed_mul);

    frame_def.num_frames = uint16_t(max_frame_cnt);

    return frame_def;
}

// ---------------------------------------------------------------------------- get_adc_configuration
Adc_Configuration Parameter_Extractor::get_adc_configuration() const
{
    // Relevant bit fields are extracted.
    auto reg_ADC0 = GET_REGISTER(BGT60TRxxC_REG_ADC0);
    auto adc_div = BGT60TRxxC_EXTRACT(ADC0, ADC_DIV, reg_ADC0);
    auto stc = BGT60TRxxC_EXTRACT(ADC0, STC, reg_ADC0);
    auto track_cfg = BGT60TRxxC_EXTRACT(ADC0, TRACK_CFG, reg_ADC0);
    auto msb_ctrl = BGT60TRxxC_EXTRACT(ADC0, MSB_CTRL, reg_ADC0);
    auto adc_overs_cfg = BGT60TRxxC_EXTRACT(ADC0, ADC_OVERS_CFG, reg_ADC0);

    if (adc_div == 0)
        throw Error("Invalid Configuration."
                    " Bit field ADC0:ADC_DIV must not be 0.");

    // Parameters are determined from bit fields.
    Adc_Configuration config;
    config.samplerate_Hz = ((get_clock_frequency() * 2) / adc_div + 1) / 2;

    switch (stc)
    {
        case 0: config.sample_time = Adc_Sample_Time::_50ns; break;
        case 1: config.sample_time = Adc_Sample_Time::_100ns; break;
        case 2: config.sample_time = Adc_Sample_Time::_200ns; break;
        case 3: config.sample_time = Adc_Sample_Time::_400ns; break;
    }

    switch (track_cfg)
    {
        case 0: config.tracking = Adc_Tracking::None; break;
        case 1: config.tracking = Adc_Tracking::_1_Subconversion; break;
        case 2: config.tracking = Adc_Tracking::_3_Subconversions; break;
        case 3: config.tracking = Adc_Tracking::_7_Subconversions; break;
    }

    config.double_msb_time = msb_ctrl != 0;

    switch (adc_overs_cfg)
    {
        case 0: config.oversampling = Adc_Oversampling::Off; break;
        case 1: config.oversampling = Adc_Oversampling::_2x; break;
        case 2: config.oversampling = Adc_Oversampling::_4x; break;
        case 3: config.oversampling = Adc_Oversampling::_8x; break;
    }

    return config;
}

// ---------------------------------------------------------------------------- get_chirp_timing
Chirp_Timing Parameter_Extractor::get_chirp_timing() const
{
    // Relevant bit fields are extracted.
    auto reg_CCR0 = GET_REGISTER(BGT60TRxxC_REG_CCR0);
    auto reg_CCR1 = GET_REGISTER(BGT60TRxxC_REG_CCR1);
    auto reg_CCR3 = GET_REGISTER(BGT60TRxxC_REG_CCR3);
    auto tr_start = BGT60TRxxC_EXTRACT(CCR1, TMRSTRT, reg_CCR1);
    auto tr_end = BGT60TRxxC_EXTRACT(CCR0, TMREND, reg_CCR0);
    auto tr_paen = BGT60TRxxC_EXTRACT(CCR3, T_PAEN, reg_CCR3);
    auto tr_sstart = Device_Traits::get(m_device_type).has_extra_startup_delays
                         ? BGT60TRxxD_EXTRACT(CCR3, TR_SSTART, reg_CCR3)
                         : BGT60TRxxC_EXTRACT(CCR3, T_SSTRT, reg_CCR3);

    /*
     * Parameters are determined from bit fields. Additional clock cycles
     * introduced by FSM are taken into account.
     */
    Chirp_Timing timing;
    timing.pre_chirp_delay_100ps =
        (uint32_t)cycles_to_100ps(8 * tr_start + 10, get_clock_frequency());

    timing.post_chirp_delay_100ps =
        (uint32_t)cycles_to_100ps(8 * tr_end + 5, get_clock_frequency());

    timing.pa_delay_100ps =
        (uint32_t)cycles_to_100ps(8 * tr_paen, get_clock_frequency());

    timing.adc_delay_100ps =
        (uint16_t)cycles_to_100ps(8 * tr_sstart + 1, get_clock_frequency());

    return timing;
}

// ---------------------------------------------------------------------------- get_startup_timing
Startup_Timing Parameter_Extractor::get_startup_timing() const
{
    // Relevant bit fields are extracted.
    auto reg_CCR0 = GET_REGISTER(BGT60TRxxE_REG_CCR0);
    auto reg_CCR3 = GET_REGISTER(BGT60TRxxE_REG_CCR3);
    auto tr_init0 = Device_Traits::get(m_device_type).has_extra_startup_delays
                        ? BGT60TRxxD_EXTRACT(CCR3, TR_INIT0, reg_CCR3)
                        : BGT60TRxxC_EXTRACT(CCR3, TR_INIT0, reg_CCR3);
    auto tr_init0_mul = BGT60TRxxC_EXTRACT(CCR3, TR_MUL0, reg_CCR3);
    auto tr_init1 = BGT60TRxxC_EXTRACT(CCR0, TR_INIT1, reg_CCR0);
    auto tr_init1_mul = BGT60TRxxC_EXTRACT(CCR0, TR_MUL1, reg_CCR0);

    // Parameters are determined from bit fields.
    uint32_t tr_wu;
    uint32_t tr_wu_mul;
    if (Device_Traits::get(m_device_type).wu_register_type
        != Device_Traits::Wu_Register_Type::None)
    {
        auto reg_WU = GET_REGISTER(BGT60TRxxE_REG_WU);
        tr_wu = BGT60TRxxE_EXTRACT(WU, TR_TWKUP, reg_WU);
        tr_wu_mul = BGT60TRxxE_EXTRACT(WU, TR_TWKUP_MUL, reg_WU);
    }
    else
    {
        auto reg_MAIN = GET_REGISTER(BGT60TRxxC_REG_MAIN);
        tr_wu = BGT60TRxxC_EXTRACT(MAIN, TWKUP, reg_MAIN);
        tr_wu_mul = BGT60TRxxC_EXTRACT(MAIN, TWKUP_MUL, reg_MAIN);
    }

    Startup_Timing timing;
    timing.wake_up_time_100ps = to_100ps(tr_wu, tr_wu_mul);
    timing.pll_settle_time_coarse_100ps = uint32_t(to_100ps(tr_init0,
                                                            tr_init0_mul));
    timing.pll_settle_time_fine_100ps = uint32_t(to_100ps(tr_init1,
                                                          tr_init1_mul));

    return timing;
}

// ---------------------------------------------------------------------------- extract_power_down_config
Power_Down_Configuration
Parameter_Extractor::extract_power_down_config(uint32_t reg_CSCx,
                                               uint32_t reg_CSx_0) const
{
    // Relevant bit fields are extracted.
    auto vco_en = BGT60TRxxC_EXTRACT(CSP_I_0, VCO_EN, reg_CSx_0);
    auto fdiv_en = BGT60TRxxC_EXTRACT(CSP_I_0, FDIV_EN, reg_CSx_0);
    auto abb_isopd = BGT60TRxxC_EXTRACT(CSCI, ABB_ISOPD, reg_CSCx);
    auto rf_isopd = BGT60TRxxC_EXTRACT(CSCI, RF_ISOPD, reg_CSCx);
    auto bg_en = BGT60TRxxC_EXTRACT(CSCI, BG_EN, reg_CSCx);
    auto madc_isopd = BGT60TRxxC_EXTRACT(CSCI, MADC_ISOPD, reg_CSCx);
    auto bg_tmrf_en = BGT60TRxxC_EXTRACT(CSCI, BG_TMRF_EN, reg_CSCx);
    auto pll_isopd = BGT60TRxxC_EXTRACT(CSCI, PLL_ISOPD, reg_CSCx);
    auto sadc_isopd = BGT60TRxxC_EXTRACT(CSCI, SADC_ISOPD, reg_CSCx);

    // Parameters are determined from bit fields.
    Power_Down_Configuration config;
    config.enable_vco = vco_en != 0;
    config.enable_fdiv = fdiv_en != 0;
    config.enable_baseband = abb_isopd == 0;
    config.enable_rf = rf_isopd == 0;
    config.enable_madc_bandgap = bg_en != 0;
    config.enable_madc = madc_isopd == 0;
    config.enable_sadc_bandgap = bg_tmrf_en != 0;
    config.enable_pll = pll_isopd == 0;
    config.enable_sadc = Device_Traits::get(m_device_type).has_sadc
                         && (sadc_isopd == 0);

    return config;
}

// ---------------------------------------------------------------------------- get_idle_configuration
Power_Down_Configuration Parameter_Extractor::get_idle_configuration() const
{
    auto reg_CSI_0 = GET_REGISTER(BGT60TRxxC_REG_CSP_I_0);
    auto reg_CSCI = GET_REGISTER(BGT60TRxxC_REG_CSCI);
    return extract_power_down_config(reg_CSCI, reg_CSI_0);
}

// ---------------------------------------------------------------------------- get_deep_sleep_configuration
Power_Down_Configuration Parameter_Extractor::get_deep_sleep_configuration() const
{
    auto reg_CSD_0 = GET_REGISTER(BGT60TRxxC_REG_CSP_D_0);
    auto reg_CSCDS = GET_REGISTER(BGT60TRxxC_REG_CSCDS);
    return extract_power_down_config(reg_CSCDS, reg_CSD_0);
}

// ---------------------------------------------------------------------------- get_startup_delays
Startup_Delays Parameter_Extractor::get_startup_delays() const
{
    if (!Device_Traits::get(m_device_type).has_extra_startup_delays)
        throw Error("The Avian device does not support this feature.");

    // Relevant bit fields are extracted.
    auto reg_CSCI = GET_REGISTER(BGT60TRxxD_REG_CSCI);
    auto reg_PACR2 = GET_REGISTER(BGT60TRxxD_REG_PACR2);
    auto tr_bgen = BGT60TRxxD_EXTRACT(CSCI, TR_BGEN, reg_CSCI);
    auto tr_madcen = BGT60TRxxD_EXTRACT(CSCI, TR_MADCEN, reg_CSCI);
    auto tr_pll_isopd = BGT60TRxxD_EXTRACT(CSCI, TR_PLL_ISOPD, reg_CSCI);
    auto tr_diven = BGT60TRxxD_EXTRACT(PACR2, TR_DIVEN, reg_PACR2);

    /*
     * Parameters are determined from bit fields. Additional clock cycles
     * introduced by FSM are taken into account.
     */
    Startup_Delays delays;
    delays.bandgap_100ps =
        (uint16_t)cycles_to_100ps(64 * tr_bgen + ((tr_bgen != 0) ? 2 : 1),
                                  get_clock_frequency());

    delays.madc_100ps =
        (uint16_t)cycles_to_100ps(64 * tr_madcen + ((tr_madcen != 0) ? 1 : 0),
                                  get_clock_frequency());

    delays.pll_enable_100ps =
        (uint32_t)cycles_to_100ps(64 * tr_pll_isopd
                                      + ((tr_pll_isopd != 0) ? 2 : 1),
                                  get_clock_frequency());

    delays.pll_divider_100ps =
        (uint16_t)cycles_to_100ps(32 * tr_diven + 1, get_clock_frequency());

    return delays;
}

// ---------------------------------------------------------------------------- get_duty_cycle_correction
Duty_Cycle_Correction_Settings Parameter_Extractor::get_duty_cycle_correction() const
{
    auto& sTraits = Device_Traits::get(m_device_type);
    if (!sTraits.has_ref_frequency_doubler)
        throw Error("The Avian device does not support this feature.");

    // Relevant bit fields are extracted.
    auto reg_FD = sTraits.has_reordered_register_layout
                      ? GET_REGISTER(BGT60TRxxE_REG_FD)
                      : GET_REGISTER(BGT60TRxxD_REG_FD);
    auto clk_sel = BGT60TRxxD_EXTRACT(FD, CLK_SEL, reg_FD);
    auto dc_in = BGT60TRxxD_EXTRACT(FD, DC_IN, reg_FD);
    auto dc_out = BGT60TRxxD_EXTRACT(FD, DC_OUT, reg_FD);

    // Parameters are determined from bit fields.
    Duty_Cycle_Correction_Settings settings;
    switch (clk_sel)
    {
        case 1: settings.mode = Duty_Cycle_Correction_Mode::Only_Out; break;
        case 2: settings.mode = Duty_Cycle_Correction_Mode::In_Out; break;
        case 3: settings.mode = Duty_Cycle_Correction_Mode::SysIn_Out; break;
        default: settings.mode = Duty_Cycle_Correction_Mode::In_Out;
    }

    settings.adjust_in = dc_in & 0x0F;
    settings.invert_input = (dc_in & 0x10) != 0;
    settings.adjust_out = int8_t(dc_out) - 8;

    return settings;
}

// ---------------------------------------------------------------------------- get_fifo_power_mode
Fifo_Power_Mode Parameter_Extractor::get_fifo_power_mode() const
{
    if (!Device_Traits::get(m_device_type).has_programmable_fifo_power_mode)
        throw Error("The Avian device does not support this feature.");

    // Relevant bit fields are extracted.
    auto reg_SFCTL = GET_REGISTER(BGT60TRxxD_REG_SFCTL);
    auto fifo_pd_mode = BGT60TRxxD_EXTRACT(SFCTL, FIFO_PD_MODE, reg_SFCTL);

    // Parameters are determined from bit fields.
    switch (fifo_pd_mode)
    {
        case 0: return Fifo_Power_Mode::Always_On;
        case 1: return Fifo_Power_Mode::Deep_Sleep_Off;
        case 2: return Fifo_Power_Mode::Deep_Sleep_And_Idle_Off;
        default:
            throw Error("Invalid configuration. Value of bit field"
                        " SFCTL:FIFO_PD_MODE is reserved.");
    }
}

// ---------------------------------------------------------------------------- get_pad_driver_mode
Pad_Driver_Mode Parameter_Extractor::get_pad_driver_mode() const
{
    if (!Device_Traits::get(m_device_type).has_programmable_pad_driver)
        throw Error("The Avian device does not support this feature.");

    // Relevant bit fields are extracted.
    auto reg_SFCTL = GET_REGISTER(BGT60TRxxD_REG_SFCTL);
    auto pad_mode = BGT60TRxxD_EXTRACT(SFCTL, PAD_MODE, reg_SFCTL);

    // Parameters are determined from bit fields.
    return pad_mode ? Pad_Driver_Mode::Strong
                    : Pad_Driver_Mode::Normal;
}

// ---------------------------------------------------------------------------- get_pullup_resistor_configuration
Pullup_Resistor_Configuration Parameter_Extractor::get_pullup_resistor_configuration() const
{
    if (!Device_Traits::get(m_device_type).has_programmable_pullup_resistors)
        throw Error("The Avian device does not support this feature.");

    // Relevant bit fields are extracted.
    auto reg_MAIN = GET_REGISTER(BGT60TRxxE_REG_MAIN);
    auto pu_en_spicsn = BGT60TRxxE_EXTRACT(MAIN, PU_EN_SPICSN, reg_MAIN);
    auto pu_en_spiclk = BGT60TRxxE_EXTRACT(MAIN, PU_EN_SPICLK, reg_MAIN);
    auto pu_en_spidi = BGT60TRxxE_EXTRACT(MAIN, PU_EN_SPIDI, reg_MAIN);
    auto pu_en_spido = BGT60TRxxE_EXTRACT(MAIN, PU_EN_SPIDO, reg_MAIN);
    auto pu_en_spidio2 = BGT60TRxxE_EXTRACT(MAIN, PU_EN_SPIDIO2, reg_MAIN);
    auto pu_en_spidio3 = BGT60TRxxE_EXTRACT(MAIN, PU_EN_SPIDIO3, reg_MAIN);
    auto pu_en_irq = BGT60TRxxE_EXTRACT(MAIN, PU_EN_IRQ, reg_MAIN);

    // Parameters are determined from bit fields.
    Pullup_Resistor_Configuration config;
    config.enable_spi_cs = pu_en_spicsn != 0;
    config.enable_spi_clk = pu_en_spiclk != 0;
    config.enable_spi_di = pu_en_spidi != 0;
    config.enable_spi_do = pu_en_spido != 0;
    config.enable_spi_dio2 = pu_en_spidio2 != 0;
    config.enable_spi_dio3 = pu_en_spidio3 != 0;
    config.enable_irq = pu_en_irq != 0;
    return config;
}

// ---------------------------------------------------------------------------- get_power_sens_delay
uint32_t Parameter_Extractor::get_power_sens_delay() const
{
    if (Device_Traits::get(m_device_type).has_sadc)
        throw Error("The Avian device does not support this feature.");

    // Relevant bit fields are extracted.
    auto reg_ADC1 = GET_REGISTER(BGT60TRxxD_REG_CS1_U_0);
    auto tr_psstart = BGT60TR11D_EXTRACT(ADC1, TR_PSSTART, reg_ADC1);

    // Parameters are determined from bit fields.
    return (uint32_t)cycles_to_100ps(tr_psstart * 8 + 2,
                                     get_clock_frequency());
}

// ---------------------------------------------------------------------------- get_power_sens_enabled
bool Parameter_Extractor::get_power_sens_enabled() const
{
    if (Device_Traits::get(m_device_type).has_sadc)
        throw Error("The Avian device does not support this feature.");

    // Relevant bit fields are extracted.
    auto reg_CS1_U_0 = GET_REGISTER(BGT60TRxxD_REG_CS1_U_0);
    auto pd1_en = BGT60TRxxD_EXTRACT(CS1_U_0, PD1_EN, reg_CS1_U_0);

    // Parameters are determined from bit fields.
    return pd1_en != 0;
}

// ---------------------------------------------------------------------------- get_temperature_sens_enabled
bool Parameter_Extractor::get_temperature_sens_enabled() const
{
    if (Device_Traits::get(m_device_type).has_sadc)
        throw Error("The Avian device does not support this feature.");

    // Relevant bit fields are extracted.
    auto reg_CS1_U_0 = GET_REGISTER(BGT60TRxxD_REG_CS1_U_0);
    auto temp_meas_en = BGT60TRxxD_EXTRACT(CS1_U_0, TEMP_MEAS_EN, reg_CS1_U_0);

    // Parameters are determined from bit fields.
    return temp_meas_en != 0;
}

// ---------------------------------------------------------------------------- get_num_samples
uint32_t Parameter_Extractor::get_num_samples(uint8_t shape, bool down) const
{
    // An exception is thrown if chirp is disabled.
    bool second_chirp = is_second_chirp(shape, down);

    // Relevant bit fields are extracted.
    auto reg_PLLx_3 = GET_REGISTER(BGT60TRxxC_REG_PLL1_3 + shape * s_inc_pll);
    auto apu = BGT60TRxxC_EXTRACT(PLL1_3, APU, reg_PLLx_3);
    auto apd = BGT60TRxxC_EXTRACT(PLL1_3, APD, reg_PLLx_3);

    // Parameters are determined from bit fields.
    uint32_t num_samples = second_chirp ? apd : apu;

    /*
     * If number of samples is 0, the chirp is used as a dummy delay. Anyhow
     * the delay duration is controlled indirectly through the number of
     * (not acquired) samples. In this case, the number of samples is
     * calculated from the ramp duration.
     */
    if (num_samples == 0)
    {
        // Relevant bit fields are extracted.
        auto reg_ADC0 = GET_REGISTER(BGT60TRxxC_REG_ADC0);
        auto reg_CCR1 = GET_REGISTER(BGT60TRxxC_REG_CCR1);
        auto reg_CCR3 = GET_REGISTER(BGT60TRxxC_REG_CCR3);
        auto adc_div = BGT60TRxxC_EXTRACT(ADC0, ADC_DIV, reg_ADC0);
        auto tr_start = BGT60TRxxC_EXTRACT(CCR1, TMRSTRT, reg_CCR1);
        auto tr_paen = BGT60TRxxC_EXTRACT(CCR3, T_PAEN, reg_CCR3);
        auto tr_sstart =
            Device_Traits::get(m_device_type).has_extra_startup_delays
                ? BGT60TRxxD_EXTRACT(CCR3, TR_SSTART, reg_CCR3)
                : BGT60TRxxC_EXTRACT(CCR3, T_SSTRT, reg_CCR3);

        uint32_t rtx;
        if (second_chirp)
        {
            auto reg_PLLx_6 = GET_REGISTER(BGT60TRxxC_REG_PLL1_6
                                           + shape * s_inc_pll);
            rtx = BGT60TRxxC_EXTRACT(PLL1_6, RTD, reg_PLLx_6);
        }
        else
        {
            auto reg_PLLx_2 = GET_REGISTER(BGT60TRxxC_REG_PLL1_2
                                           + shape * s_inc_pll);
            rtx = BGT60TRxxC_EXTRACT(PLL1_2, RTU, reg_PLLx_2);
        }

        if (adc_div == 0)
            throw Error("Invalid Configuration."
                        " Bit field ADC0:ADC_DIV must not be 0.");

        /*
         * The driver calculates a ramp duration which is longer than just the
         * time needed to capture ADC samples. The additional clock cycles are
         * calculated from the chirp timing parameters. Those additional clock
         * cycles are compensated here, before ramp duration and ADC sampling
         * rate are used to calculate the number of samples.
         */
        int32_t pre_chirp_delay = tr_start * 8 + 10;
        int32_t pa_delay = tr_paen * 8;
        auto pre_pa_cycles = std::max(pa_delay - pre_chirp_delay, 0);
        auto adc_delay = tr_sstart * 8 + 1;

        int32_t num_ramp_cycles = 8 * rtx;
        num_ramp_cycles -= pre_pa_cycles;
        num_ramp_cycles -= adc_delay;
        num_samples = num_ramp_cycles / adc_div;
    }

    return num_samples;
}

// ---------------------------------------------------------------------------- get_fmcw_configuration
Fmcw_Configuration Parameter_Extractor::get_fmcw_configuration(uint8_t shape) const
{
    // An exception is thrown if shape is disabled.
    check_shape_enabled(shape);

    // Relevant bit fields are extracted.
    auto reg_PLLx_0 = GET_REGISTER(BGT60TRxxC_REG_PLL1_0 + shape * s_inc_pll);
    auto reg_PLLx_1 = GET_REGISTER(BGT60TRxxC_REG_PLL1_1 + shape * s_inc_pll);
    auto reg_PLLx_4 = GET_REGISTER(BGT60TRxxC_REG_PLL1_4 + shape * s_inc_pll);
    auto reg_PLLx_5 = GET_REGISTER(BGT60TRxxC_REG_PLL1_5 + shape * s_inc_pll);
    auto reg_PLLx_6 = GET_REGISTER(BGT60TRxxC_REG_PLL1_6 + shape * s_inc_pll);
    auto reg_CCR1 = GET_REGISTER(BGT60TRxxC_REG_CCR1);
    auto reg_CCR3 = GET_REGISTER(BGT60TRxxC_REG_CCR3);
    auto reg_PACR2 = GET_REGISTER(BGT60TRxxC_REG_PACR2);
    auto reg_ADC0 = GET_REGISTER(BGT60TRxxC_REG_ADC0);
    auto fsu = BGT60TRxxC_EXTRACT(PLL1_0, FSU, reg_PLLx_0);
    auto rsu = BGT60TRxxC_EXTRACT(PLL1_1, RSU, reg_PLLx_1);
    auto fsd = BGT60TRxxC_EXTRACT(PLL1_4, FSD, reg_PLLx_4);
    auto rsd = BGT60TRxxC_EXTRACT(PLL1_5, RSD, reg_PLLx_5);
    auto rtd = BGT60TRxxC_EXTRACT(PLL1_6, RTD, reg_PLLx_6);
    auto divset = BGT60TRxxC_EXTRACT(PACR2, DIVSET, reg_PACR2);
    auto tr_start = BGT60TRxxC_EXTRACT(CCR1, TMRSTRT, reg_CCR1);
    auto tr_paen = BGT60TRxxC_EXTRACT(CCR3, T_PAEN, reg_CCR3);
    auto tr_sstart = BGT60TRxxC_EXTRACT(CCR3, T_SSTRT, reg_CCR3);
    auto adc_div = BGT60TRxxC_EXTRACT(ADC0, ADC_DIV, reg_ADC0);

    // Parameters are determined from bit fields.
    Fmcw_Configuration config;

    /*
     * There is no single bit field the shape type could be read from. The
     * shape is a sawtooth if the down chirp bit fields are all 0.
     * From the Avian device's perspective, the first chirp of a shape is the
     * up-chirp, regardless of the ramp slope. The sign of the first chirp's
     * ramp increment is evaluated to find out the real chirp direction.
     */
    bool sawtooth = (fsd == 0) && (rsd == 0) && (rtd == 0);
    bool first_up = (rsu & 0x00800000) == 0;
    config.shape_type = sawtooth ? (first_up ? Shape_Type::Saw_Up
                                             : Shape_Type::Saw_Down)
                                 : (first_up ? Shape_Type::Tri_Up
                                             : Shape_Type::Tri_Down);

    /*
     * From the driver class' perspective the start frequency is the frequency
     * at that point in time when power amplifier is enabled. The additional
     * clock cycles before that time are calculated to adjust the start
     * frequency later.
     */
    int32_t pre_chirp_delay = tr_start * 8 + 10;
    int32_t pa_delay = tr_paen * 8;
    int32_t additional_cycles = std::max(pa_delay - pre_chirp_delay, 0);

    /*
     * The PLL's start frequency and ramp slope is calculated from the bit
     * fields of the shape's first chirp, because that shape is always used.
     */
    int32_t start_freq = (fsu < (1 << 23)) ? fsu : fsu - (1 << 24);
    int32_t freq_increment = (rsu < (1 << 23)) ? rsu : rsu - (1 << 24);
    start_freq += additional_cycles * freq_increment;

    int32_t end_freq;
    if (sawtooth)
    {
        /*
         * The end frequency is calculated from start frequency, ramp slope
         * and ramp duration in cycles. The ramp duration could directly be
         * calculated from the bit field RTU, but due to rounding effects this
         * introduces an uncertainty of 7 clock cycles compared to the duration
         * used internally by the driver. To be accurate the ramp duration must
         * be calculated in the same way as it is done in the driver.
         *
         * (The imported end frequency may still not be the exact value it was
         * in the configuration the imported registers were generated from, but
         * for a sawtooth shape this does not matter, because a driver instance
         * will generate the same PLL bit field values.)
         */
        int32_t num_ramp_cycles = adc_div * get_num_samples(shape, !first_up)
                                  + 8 * tr_sstart + 1;
        end_freq = start_freq + freq_increment * num_ramp_cycles;
    }
    else
    {
        /*
         * In triangular shapes the start frequency of the second chirp is the
         * end frequency of the first chirp. Reconstructing a chirps's start
         * frequency does not suffer from rounding effects as it does for the
         * end frequency. Therefore for triangular shapes it's preferable to
         * calculate second chirp's start frequency rather than the first
         * chirp's end frequency.
         */
        end_freq = (fsd < (1 << 23)) ? fsd : fsd - (1 << 24);
        int32_t freq_increment_2 = (rsd < (1 << 23)) ? rsd : rsd - (1 << 24);
        end_freq += additional_cycles * freq_increment_2;
    }

    // PLL frequency values are converted to real frequencies.
    RF_Converter converter(get_clock_frequency(), uint8_t(divset),
                           Device_Traits::get(m_device_type).pll_pre_divider);
    if (first_up)
    {
        config.lower_frequency_kHz = converter.pll_to_freq(start_freq);
        config.upper_frequency_kHz = converter.pll_to_freq(end_freq);
    }
    else
    {
        config.lower_frequency_kHz = converter.pll_to_freq(end_freq);
        config.upper_frequency_kHz = converter.pll_to_freq(start_freq);
    }

    /*
     * A DAC bit field does not necessarily contain the TX power, because it
     * may have been programmed to 0 because of the TX mode. Therefore the
     * TX_EN bit fields must also be checked. Search for an enabled TX starts
     * with TX1 during first chirp, then TX2 (if available) is checked.
     */
    auto reg_CSx_U_0 = GET_REGISTER(BGT60TRxxC_REG_CS1_U_0 + shape * s_inc_cs);
    auto reg_CSx_U_1 = GET_REGISTER(BGT60TRxxC_REG_CS1_U_1 + shape * s_inc_cs);
    auto tx1_en = BGT60TRxxC_EXTRACT(CS1_U_0, TX1_EN, reg_CSx_U_0);
    auto tx2_en = BGT60TRxxC_EXTRACT(CS1_U_0, TX2_EN, reg_CSx_U_0);
    auto tx1_dac = BGT60TRxxC_EXTRACT(CS1_U_1, TX1_DAC, reg_CSx_U_1);
    auto tx2_dac = BGT60TRxxC_EXTRACT(CS1_U_1, TX2_DAC, reg_CSx_U_1);

    config.tx_power = 0;
    if (tx1_en != 0)
    {
        config.tx_power = uint8_t(tx1_dac);
    }
    else if ((Device_Traits::get(m_device_type).num_tx_antennas >= 2)
             && (tx2_en != 0))
    {
        config.tx_power = uint8_t(tx2_dac);
    }
    else if (!sawtooth)
    {
        // Try again with second chirp, if shape is triangle.
        auto reg_CSx_D_0 = GET_REGISTER(BGT60TRxxC_REG_CS1_D_0
                                        + shape * s_inc_cs);
        auto reg_CSx_D_1 = GET_REGISTER(BGT60TRxxC_REG_CS1_D_1
                                        + shape * s_inc_cs);
        tx1_en = BGT60TRxxC_EXTRACT(CS1_D_0, TX1_EN, reg_CSx_D_0);
        tx2_en = BGT60TRxxC_EXTRACT(CS1_D_0, TX2_EN, reg_CSx_D_0);
        tx1_dac = BGT60TRxxC_EXTRACT(CS1_D_1, TX1_DAC, reg_CSx_D_1);
        tx2_dac = BGT60TRxxC_EXTRACT(CS1_D_1, TX2_DAC, reg_CSx_D_1);

        if (tx1_en != 0)
        {
            config.tx_power = uint8_t(tx1_dac);
        }
        else if ((Device_Traits::get(m_device_type).num_tx_antennas >= 2)
                 && (tx2_en != 0))
        {
            config.tx_power = uint8_t(tx2_dac);
        }
    }

    return config;
}

// ---------------------------------------------------------------------------- get_frame_format
Frame_Format Parameter_Extractor::get_frame_format(uint8_t shape,
                                                   bool down) const
{
    // An exception is thrown if chirp is disabled.
    bool second_chirp = is_second_chirp(shape, down);

    // Relevant bit fields are extracted.
    auto reg_CSx_y_1 = GET_REGISTER(BGT60TRxxC_REG_CS1_U_1
                                    + shape * s_inc_cs
                                    + (second_chirp ? s_inc_cs_d : 0));
    auto bbch_sel = BGT60TRxxC_EXTRACT(CS1_U_1, BBCH_SEL, reg_CSx_y_1);

    // Parameters are determined from bit fields.
    Frame_Format format;
    format.num_chirps_per_frame = 1;
    format.num_samples_per_chirp = get_num_samples(shape, down);
    format.rx_mask = uint8_t(bbch_sel);
    return format;
}

// ---------------------------------------------------------------------------- get_tx_mode
Tx_Mode Parameter_Extractor::get_tx_mode(uint8_t shape, bool down) const
{
    // An exception is thrown if chirp is disabled.
    bool second_chirp = is_second_chirp(shape, down);

    // Relevant bit fields are extracted.
    auto reg_CSx_y_0 = GET_REGISTER(BGT60TRxxC_REG_CS1_U_0
                                    + shape * s_inc_cs
                                    + (second_chirp ? s_inc_cs_d : 0));
    auto tx1_en = BGT60TRxxC_EXTRACT(CS1_U_0, TX1_EN, reg_CSx_y_0);
    auto tx2_en = BGT60TRxxC_EXTRACT(CS1_U_0, TX2_EN, reg_CSx_y_0);

    if (Device_Traits::get(m_device_type).num_tx_antennas < 2)
        tx2_en = 0;

    // Parameters are determined from bit fields.
    return (tx1_en != 0)
               ? ((tx2_en != 0) ? Tx_Mode::Alternating : Tx_Mode::Tx1_Only)
               : ((tx2_en != 0) ? Tx_Mode::Tx2_Only : Tx_Mode::Off);
}

// ---------------------------------------------------------------------------- get_baseband_configuration
Baseband_Configuration Parameter_Extractor::get_baseband_configuration(uint8_t shape,
                                                                       bool down) const
{
    // An exception is thrown if chirp is disabled.
    bool second_chirp = is_second_chirp(shape, down);

    // Relevant bit fields are extracted.
    auto reg_CSx_y_1 = GET_REGISTER(BGT60TRxxC_REG_CS1_U_1
                                    + shape * s_inc_cs
                                    + (second_chirp ? s_inc_cs_d : 0));
    auto reg_CSx_y_2 = GET_REGISTER(BGT60TRxxC_REG_CS1_U_2
                                    + shape * s_inc_cs
                                    + (second_chirp ? s_inc_cs_d : 0));

    uint32_t hp_gain[4];
    uint32_t hpf_sel[4];
    uint32_t vga_gain[4];
    uint32_t bb_rstcnt;
    if (Device_Traits::get(m_device_type).cs_register_layout == Cs_Layout_t::Version1)
    {
        auto hp_gain_mask = BGT60TRxxC_EXTRACT(CS1_U_2, HP_GAIN, reg_CSx_y_2);
        hp_gain[0] = hp_gain_mask & 1;
        hp_gain[1] = hp_gain_mask & 2;
        hp_gain[2] = hp_gain_mask & 4;
        hp_gain[3] = hp_gain_mask & 8;
        hpf_sel[0] = BGT60TRxxC_EXTRACT(CS1_U_2, HPF_SEL1, reg_CSx_y_2);
        hpf_sel[1] = BGT60TRxxC_EXTRACT(CS1_U_2, HPF_SEL2, reg_CSx_y_2);
        hpf_sel[2] = BGT60TRxxC_EXTRACT(CS1_U_2, HPF_SEL3, reg_CSx_y_2);
        hpf_sel[3] = BGT60TRxxC_EXTRACT(CS1_U_2, HPF_SEL4, reg_CSx_y_2);
        vga_gain[0] = BGT60TRxxC_EXTRACT(CS1_U_2, VGA_GAIN1, reg_CSx_y_2);
        vga_gain[1] = BGT60TRxxC_EXTRACT(CS1_U_2, VGA_GAIN2, reg_CSx_y_2);
        vga_gain[2] = BGT60TRxxC_EXTRACT(CS1_U_2, VGA_GAIN3, reg_CSx_y_2);
        vga_gain[3] = BGT60TRxxC_EXTRACT(CS1_U_2, VGA_GAIN4, reg_CSx_y_2);
        bb_rstcnt = BGT60TRxxC_EXTRACT(CS1_U_1, BB_RSTCNT, reg_CSx_y_1);
    }
    else
    {
        hp_gain[0] = BGT60TRxxD_EXTRACT(CS1_U_1, HP1_GAIN, reg_CSx_y_1);
        hp_gain[1] = BGT60TRxxD_EXTRACT(CS1_U_1, HP2_GAIN, reg_CSx_y_1);
        hp_gain[2] = BGT60TRxxD_EXTRACT(CS1_U_1, HP3_GAIN, reg_CSx_y_1);
        hp_gain[3] = BGT60TRxxD_EXTRACT(CS1_U_1, HP4_GAIN, reg_CSx_y_1);
        hpf_sel[0] = BGT60TRxxD_EXTRACT(CS1_U_2, HPF_SEL1, reg_CSx_y_2);
        hpf_sel[1] = BGT60TRxxD_EXTRACT(CS1_U_2, HPF_SEL2, reg_CSx_y_2);
        hpf_sel[2] = BGT60TRxxD_EXTRACT(CS1_U_2, HPF_SEL3, reg_CSx_y_2);
        hpf_sel[3] = BGT60TRxxD_EXTRACT(CS1_U_2, HPF_SEL4, reg_CSx_y_2);
        vga_gain[0] = BGT60TRxxD_EXTRACT(CS1_U_2, VGA_GAIN1, reg_CSx_y_2);
        vga_gain[1] = BGT60TRxxD_EXTRACT(CS1_U_2, VGA_GAIN2, reg_CSx_y_2);
        vga_gain[2] = BGT60TRxxD_EXTRACT(CS1_U_2, VGA_GAIN3, reg_CSx_y_2);
        vga_gain[3] = BGT60TRxxD_EXTRACT(CS1_U_2, VGA_GAIN4, reg_CSx_y_2);
        bb_rstcnt = BGT60TRxxD_EXTRACT(CS1_U_1, BB_RSTCNT, reg_CSx_y_1);
    }

    // Parameters are determined from bit fields.
    auto to_hp_gain = [](uint32_t bit_field) -> Hp_Gain {
        return (bit_field != 0) ? Hp_Gain::_18dB : Hp_Gain::_30dB;
    };

    auto to_hp_cutoff = [&](uint32_t bit_field) -> int32_t {
        auto& traits = Device_Traits::get(m_device_type);
        auto cutoff_Hz = traits.hpf_cutoff_settings[bit_field];
        if (cutoff_Hz == -1)
        {
            throw Error("Invalid configuration. Value of bit field"
                        " HPF_SELx is reserved.");
        }
        return cutoff_Hz;
    };

    auto to_vga_gain = [](uint32_t bit_field) -> Vga_Gain {
        switch (bit_field)
        {
            case 0: return Vga_Gain::_0dB;
            case 1: return Vga_Gain::_5dB;
            case 2: return Vga_Gain::_10dB;
            case 3: return Vga_Gain::_15dB;
            case 4: return Vga_Gain::_20dB;
            case 5: return Vga_Gain::_25dB;
            case 6: return Vga_Gain::_30dB;
        }
        throw Error("Invalid configuration. Value of bit field"
                    " VGA_GAINx is reserved.");
    };

    Baseband_Configuration config;
    config.hp_gain_1 = to_hp_gain(hp_gain[0]);
    config.hp_cutoff_1_Hz = to_hp_cutoff(hpf_sel[0]);
    config.vga_gain_1 = to_vga_gain(vga_gain[0]);
    config.hp_gain_2 = to_hp_gain(hp_gain[1]);
    config.hp_cutoff_2_Hz = to_hp_cutoff(hpf_sel[1]);
    config.vga_gain_2 = to_vga_gain(vga_gain[1]);
    config.hp_gain_3 = to_hp_gain(hp_gain[2]);
    config.hp_cutoff_3_Hz = to_hp_cutoff(hpf_sel[2]);
    config.vga_gain_3 = to_vga_gain(vga_gain[2]);
    config.hp_gain_4 = to_hp_gain(hp_gain[3]);
    config.hp_cutoff_4_Hz = to_hp_cutoff(hpf_sel[3]);
    config.vga_gain_4 = to_vga_gain(vga_gain[3]);

    if (Device_Traits::get(m_device_type).cs_register_layout == Cs_Layout_t::Version1)
    {
        config.reset_period_100ps =
            (uint32_t)cycles_to_100ps(bb_rstcnt, get_clock_frequency());
    }
    else
    {
        config.reset_period_100ps =
            (uint32_t)cycles_to_100ps(8 * bb_rstcnt, get_clock_frequency());
    }

    return config;
}

// ---------------------------------------------------------------------------- get_chirp_end_delay
uint32_t Parameter_Extractor::get_chirp_end_delay(uint8_t shape,
                                                  bool down) const
{
    // An exception is thrown if chirp is disabled.
    bool second_chirp = is_second_chirp(shape, down);

    // Relevant bit fields are extracted.
    auto reg_PLLx_2 = GET_REGISTER(BGT60TRxxC_REG_PLL1_2 + shape * s_inc_pll);
    auto reg_PLLx_6 = GET_REGISTER(BGT60TRxxC_REG_PLL1_6 + shape * s_inc_pll);
    auto t_edx = second_chirp
                     ? BGT60TRxxC_EXTRACT(PLL1_6, T_EDD, reg_PLLx_6)
                     : BGT60TRxxC_EXTRACT(PLL1_2, T_EDU, reg_PLLx_2);

    // Parameters are determined from bit fields.
    auto clock_cycles = t_edx * 8;
    clock_cycles += (t_edx != 0) ? 5 : 2;

    return (uint32_t)cycles_to_100ps(clock_cycles, get_clock_frequency());
}

// ---------------------------------------------------------------------------- get_anti_alias_filter_settings
Anti_Alias_Filter_Settings Parameter_Extractor::get_anti_alias_filter_settings(uint8_t shape,
                                                                               bool down) const
{
    if (Device_Traits::get(m_device_type).cs_register_layout == Cs_Layout_t::Version1)
        throw Error("The Avian device does not support this feature.");

    // An exception is thrown if chirp is disabled.
    bool second_chirp = is_second_chirp(shape, down);

    // Relevant bit fields are extracted.
    auto reg_CSx_y_0 = GET_REGISTER(BGT60TRxxC_REG_CS1_U_0
                                    + shape * s_inc_cs
                                    + (second_chirp ? s_inc_cs_d : 0));

    uint32_t aab1_aaf_ctrl;
    uint32_t aab2_aaf_ctrl;
    uint32_t aab3_aaf_ctrl;
    uint32_t aab4_aaf_ctrl;
    if (Device_Traits::get(m_device_type).cs_register_layout == Cs_Layout_t::Version2)
    {
        aab1_aaf_ctrl = BGT60TRxxD_EXTRACT(CS1_U_0, ABB1_AAF_CTRL, reg_CSx_y_0);
        aab2_aaf_ctrl = BGT60TRxxD_EXTRACT(CS1_U_0, ABB2_AAF_CTRL, reg_CSx_y_0);
        aab3_aaf_ctrl = BGT60TRxxD_EXTRACT(CS1_U_0, ABB3_AAF_CTRL, reg_CSx_y_0);
        aab4_aaf_ctrl = BGT60TRxxD_EXTRACT(CS1_U_0, ABB4_AAF_CTRL, reg_CSx_y_0);
    }
    else
    {
        aab1_aaf_ctrl = BGT120TR24E_EXTRACT(CS1_U_0, ABB_AAF_CTRL, reg_CSx_y_0);
        aab2_aaf_ctrl = aab1_aaf_ctrl;
        aab3_aaf_ctrl = aab1_aaf_ctrl;
        aab4_aaf_ctrl = aab1_aaf_ctrl;
    }

    // Parameters are determined from bit fields.
    auto to_aaf_cutoff = [&](uint32_t bit_field) -> int32_t {
        auto& traits = Device_Traits::get(m_device_type);
        auto cutoff_Hz = traits.aaf_cutoff_settings[bit_field];
        if (cutoff_Hz == -1)
        {
            throw Error("Invalid configuration. Value of bit field"
                        " ABBx_AAF_CTRL is reserved.");
        }
        return cutoff_Hz;
    };

    Anti_Alias_Filter_Settings settings;
    settings.frequency1_Hz = to_aaf_cutoff(aab1_aaf_ctrl);
    settings.frequency2_Hz = to_aaf_cutoff(aab2_aaf_ctrl);
    settings.frequency3_Hz = to_aaf_cutoff(aab3_aaf_ctrl);
    settings.frequency4_Hz = to_aaf_cutoff(aab4_aaf_ctrl);
    return settings;
}

// ---------------------------------------------------------------------------- check_shape_enabled
void Parameter_Extractor::check_shape_enabled(uint8_t shape) const
{
    // Relevant bit fields are extracted.
    auto reg_PLLx_7 = GET_REGISTER(BGT60TRxxC_REG_PLL1_7 + shape * s_inc_pll);
    auto sh_en = BGT60TRxxC_EXTRACT(PLL1_7, SH_EN, reg_PLLx_7);

    if (sh_en == 0)
        throw Error("Shape " + std::to_string(shape) + " is disabled.");
}

// ---------------------------------------------------------------------------- is_second_chirp
bool Parameter_Extractor::is_second_chirp(uint8_t shape, bool down) const
{
    // An exception is thrown if shape is disabled.
    check_shape_enabled(shape);

    // Relevant bit fields are extracted.
    auto reg_PLLx_1 = GET_REGISTER(BGT60TRxxC_REG_PLL1_1 + shape * s_inc_pll);
    auto reg_PLLx_4 = GET_REGISTER(BGT60TRxxC_REG_PLL1_4 + shape * s_inc_pll);
    auto reg_PLLx_5 = GET_REGISTER(BGT60TRxxC_REG_PLL1_5 + shape * s_inc_pll);
    auto reg_PLLx_6 = GET_REGISTER(BGT60TRxxC_REG_PLL1_6 + shape * s_inc_pll);
    auto rsu = BGT60TRxxC_EXTRACT(PLL1_1, RSU, reg_PLLx_1);
    auto fsd = BGT60TRxxC_EXTRACT(PLL1_4, FSD, reg_PLLx_4);
    auto rsd = BGT60TRxxC_EXTRACT(PLL1_5, RSD, reg_PLLx_5);
    auto rtd = BGT60TRxxC_EXTRACT(PLL1_6, RTD, reg_PLLx_6);

    // Slope of first chirp is evaluated from sign of frequency increment.
    bool sawtooth = (fsd == 0) && (rsd == 0) && (rtd == 0);
    bool first_down = (rsu & 0x00800000) != 0;
    bool second_chirp_requested = (first_down != down);

    // An exception is thrown if the chirp is disabled
    if (second_chirp_requested && sawtooth)
        throw Error(std::string(down ? "Down" : "Up") + " chirp of shape "
                    + std::to_string(shape) + " is disabled.");

    return second_chirp_requested;
}

// ---------------------------------------------------------------------------- has_register
bool Parameter_Extractor::has_register(uint8_t address) const
{
    return m_registers.count(address) > 0;
}

// ---------------------------------------------------------------------------- get_register_value
uint32_t Parameter_Extractor::get_register_value(uint8_t address) const
{
    return m_registers.at(address);
}

/* ------------------------------------------------------------------------ */
}  // namespace Avian
}  // namespace Infineon

/* --- End of File -------------------------------------------------------- */
