/* ===========================================================================
** Copyright (C) 2023 Infineon Technologies AG
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

#include "DeviceLtr11RegisterConfigurator.hpp"
#include "ifxBase/Exception.hpp"

#include <components/interfaces/IRadarLtr11.hpp>
#include <components/interfaces/IRegisters.hpp>

#include <cmath>
#include <cstring>
#include <fstream>
#include <iomanip>

namespace {
constexpr auto LTR11_REGISTERS_NUMB = 0x0039;

// Register addresses
constexpr auto DAR_REG0_REG_ADDR = 0x0000;
constexpr auto DAR_REG1_REG_ADDR = 0x0001;
constexpr auto THOLD_REG2_REG_ADDR = 0x0002;
constexpr auto PLL_CONFIG1_REG4_REG_ADDR = 0x0004;
constexpr auto PLL_CONFIG2_REG5_REG_ADDR = 0x0005;
constexpr auto PLL_CONFIG3_REG6_REG_ADDR = 0x0006;
constexpr auto DC_TMG_PD_MPA_REG7_REG_ADDR = 0x0007;
constexpr auto DIV_REG8_REG_ADDR = 0x0008;
constexpr auto BB_REG9_REG_ADDR = 0x0009;
constexpr auto HT_REG10_REG_ADDR = 0x000A;
constexpr auto ALGO1_REG13_REG_ADDR = 0x000D;
constexpr auto ALGO2_REG14_REG_ADDR = 0x000E;
constexpr auto DIGI_CTRL_REG15_REG_ADDR = 0x000F;
constexpr auto E_FUSES_REG55_REG_ADDR = 0x0037;

constexpr auto THOLD_REG2_REG_RST = 0x0000;
constexpr auto DC_TMG_PD_MPA_REG7_REG_RST = 0x0000;
constexpr auto BB_REG9_REG_RST = 0x0000;
constexpr auto PLL_CONFIG2_REG5_REG_RST = 0x0000;
constexpr auto DIGI_CTRL_REG15_REG_RST = 0x0000;
constexpr auto ALGO1_REG13_REG_RST = 0x0000;
constexpr auto DAR_REG1_REG_RST = 0x0000;

constexpr auto dc_on_pulse_len_BITS = 8;
constexpr auto dc_on_pulse_len_MASK = (0x03 << dc_on_pulse_len_BITS);  // bits 8,9

constexpr auto dc_rep_rate_BITS = 10;
constexpr auto dc_rep_rate_MASK = (0x03 << dc_rep_rate_BITS);  // bits 10,11

constexpr auto thres_MASK = 0x1FFF;

constexpr auto mpa_ctrl_MASK = 0x07;

constexpr auto bb_ctrl_gain_MASK = 0x0F;

constexpr auto pll_fcw_MASK = 0x0FFF;

constexpr auto aprt_BITS = 14;
constexpr auto aprt_MASK = (0x01 << aprt_BITS);

// digital control (pulse mode and continuous wave mode) related - begin
constexpr auto start_pm_BITS = 14;
constexpr auto start_pm_MASK = (0x01 << start_pm_BITS);

constexpr auto start_cw_BITS = 12;
constexpr auto start_cw_MASK = (0x01 << start_cw_BITS);

constexpr auto miso_drv_BITS = 6;
constexpr auto miso_drv_MASK = (0x01 << miso_drv_BITS);

constexpr auto stat_mux_MASK = 0x0F;

constexpr auto digital_control_pulse_mode_MASK = (stat_mux_MASK | miso_drv_MASK | start_cw_MASK | start_pm_MASK);
// digital control related - end

constexpr auto prt_mult_MASK = 0x03;

constexpr auto pll_japan_mode_BIT = 15;

constexpr auto status_multiplexer_amplitude = 6;

constexpr auto bb_dig_det_en_BITS = 7;
constexpr auto bb_dig_det_en_MASK = 1 << bb_dig_det_en_BITS;

const std::vector<DeviceLtr11RegisterConfigurator::BatchType> defaultRegistersList = {{
    {DAR_REG0_REG_ADDR, 0x0000},
    {DAR_REG1_REG_ADDR, 0x0000},
    {PLL_CONFIG1_REG4_REG_ADDR, 0x0F3A},
    {PLL_CONFIG3_REG6_REG_ADDR, 0x6800},
    {DC_TMG_PD_MPA_REG7_REG_ADDR, 0x0457},
    {DIV_REG8_REG_ADDR, 0x0000},
    {BB_REG9_REG_ADDR, 0x0068},
    {ALGO2_REG14_REG_ADDR, 0x4000},
}};

/* coefficient of the line equation: y = m *x + n */
const float m = 4.16667E-07f;
const float n = -21504.0F;

}  // end of anonymous namespace

DeviceLtr11RegisterConfigurator::DeviceLtr11RegisterConfigurator(IRegisters<address_t, value_t>* registers) :
    m_registers {registers}
{
}

void DeviceLtr11RegisterConfigurator::addSetDefaultRegistersList()
{
    m_registerQueue.insert(m_registerQueue.end(), defaultRegistersList.begin(), defaultRegistersList.end());
}

void DeviceLtr11RegisterConfigurator::addSetPulseConfig(const ifx_Ltr11_PRT_t prt, const ifx_Ltr11_PulseWidth_t pulseWidth, const ifx_Ltr11_TxPowerLevel_t powerLevel)
{
    value_t value = DC_TMG_PD_MPA_REG7_REG_RST;

    value &= ~dc_on_pulse_len_MASK;
    value |= (pulseWidth << dc_on_pulse_len_BITS);
    value &= ~dc_rep_rate_MASK;
    value |= (prt << dc_rep_rate_BITS);
    value &= ~mpa_ctrl_MASK;
    value |= powerLevel;

    addSetRegister({DC_TMG_PD_MPA_REG7_REG_ADDR, value});
}

ifx_Ltr11_PulseWidth_t DeviceLtr11RegisterConfigurator::getPulseWidth() const
{
    const auto regVal = getRegisterValue(DC_TMG_PD_MPA_REG7_REG_ADDR);
    auto pulseWidth = ((regVal & dc_on_pulse_len_MASK) >> dc_on_pulse_len_BITS);
    return static_cast<ifx_Ltr11_PulseWidth_t>(pulseWidth);
}

ifx_Ltr11_PRT_t DeviceLtr11RegisterConfigurator::getPulseRepetitionTime() const
{
    const auto regVal = getRegisterValue(DC_TMG_PD_MPA_REG7_REG_ADDR);
    auto prt = ((regVal & dc_rep_rate_MASK) >> dc_rep_rate_BITS);
    return static_cast<ifx_Ltr11_PRT_t>(prt);
}

void DeviceLtr11RegisterConfigurator::addSetHoldTime(const ifx_Ltr11_Hold_Time_t holdTime)
{
    BatchType regHoldTime = {HT_REG10_REG_ADDR, static_cast<value_t>(holdTime)};
    addSetRegister(regHoldTime);
}

ifx_Ltr11_Hold_Time_t DeviceLtr11RegisterConfigurator::getHoldTime() const
{
    return static_cast<ifx_Ltr11_Hold_Time_t>(getRegisterValue(HT_REG10_REG_ADDR));
}

void DeviceLtr11RegisterConfigurator::addSetDetectorThreshold_AdaptivPRT(const value_t internalDetectorThreshold, const ifx_Ltr11_APRT_Factor_t aprtFactor)
{
    const auto aptr_enabled = (aprtFactor != IFX_LTR11_APRT_FACTOR_1);

    value_t reg2Value = THOLD_REG2_REG_RST;

    reg2Value &= ~thres_MASK;
    reg2Value |= (internalDetectorThreshold & thres_MASK);
    reg2Value &= ~aprt_MASK;
    reg2Value |= static_cast<int>(aptr_enabled) << aprt_BITS;

    if (aptr_enabled)
    {
        value_t reg13Value = ALGO1_REG13_REG_RST;
        reg13Value &= ~prt_mult_MASK;
        reg13Value |= aprtFactor;
        addSetRegister({ALGO1_REG13_REG_ADDR, reg13Value});
    }
    addSetRegister({THOLD_REG2_REG_ADDR, reg2Value});
}

DeviceLtr11RegisterConfigurator::value_t DeviceLtr11RegisterConfigurator::getInternalDetectorThreshold() const
{
    const auto internalDetectorThreshold = (getRegisterValue(THOLD_REG2_REG_ADDR) & thres_MASK);
    return internalDetectorThreshold;
}

ifx_Ltr11_TxPowerLevel_t DeviceLtr11RegisterConfigurator::getTxPowerLevel() const
{
    const auto powerLevel = (getRegisterValue(DC_TMG_PD_MPA_REG7_REG_ADDR) & mpa_ctrl_MASK);
    return static_cast<ifx_Ltr11_TxPowerLevel_t>(powerLevel);
}

void DeviceLtr11RegisterConfigurator::addSetRX_IF_Gain(const ifx_Ltr11_RxIFGain_t rxIfGain)
{
    value_t value = BB_REG9_REG_RST;
    value &= ~bb_ctrl_gain_MASK;
    value |= rxIfGain;

    addSetRegister({BB_REG9_REG_ADDR, value});
}

ifx_Ltr11_RxIFGain_t DeviceLtr11RegisterConfigurator::getRX_IF_Gain() const
{
    const auto rx_if_gain = (getRegisterValue(BB_REG9_REG_ADDR) & bb_ctrl_gain_MASK);

    return static_cast<ifx_Ltr11_RxIFGain_t>(rx_if_gain);
}

void DeviceLtr11RegisterConfigurator::addSetRFCenterFrequency(const uint64_t freq_Hz)
{
    auto freq_reg = convertRFCFreq_toRegisterValue(freq_Hz);

    value_t value = PLL_CONFIG2_REG5_REG_RST;

    value &= ~pll_fcw_MASK;
    value |= freq_reg;

    addSetRegister({PLL_CONFIG2_REG5_REG_ADDR, value});
}

uint64_t DeviceLtr11RegisterConfigurator::getRFCenterFrequency() const
{
    const auto freq_reg = (getRegisterValue(PLL_CONFIG2_REG5_REG_ADDR) & pll_fcw_MASK);
    return convertToRfCFreq(freq_reg);
}

ifx_Ltr11_APRT_Factor_t DeviceLtr11RegisterConfigurator::getAdaptivePulseRepetitionTime() const
{
    const auto aprt_enabled = (getRegisterValue(2) & aprt_MASK) >> aprt_BITS;

    if (!aprt_enabled)
    {
        return IFX_LTR11_APRT_FACTOR_1;
    }

    return static_cast<ifx_Ltr11_APRT_Factor_t>(getRegisterValue(ALGO1_REG13_REG_ADDR) & prt_mult_MASK);
}


DeviceLtr11RegisterConfigurator::value_t DeviceLtr11RegisterConfigurator::getRegisterValue(address_t addr) const
{
    if (m_registers == nullptr)
    {
        throw rdk::exception::missing_interface();
    }

    return m_registers->read(addr);
}

// ================
// register getters
// ================
void DeviceLtr11RegisterConfigurator::addSetMode(ifx_Ltr11_Mode_t mode)
{
    value_t value = DIGI_CTRL_REG15_REG_RST;

    value &= ~digital_control_pulse_mode_MASK;

    const value_t start_pm_value = 1;
    const value_t start_cw_value = mode;
    const value_t miso_drv_value = 1;
    const value_t stat_mux_value = status_multiplexer_amplitude;

    value |= (start_pm_value << start_pm_BITS);
    value |= (start_cw_value << start_cw_BITS);
    value |= (miso_drv_value << miso_drv_BITS);
    value |= stat_mux_value;

    addSetRegister({DIGI_CTRL_REG15_REG_ADDR, value});
}

DeviceLtr11RegisterConfigurator::value_t DeviceLtr11RegisterConfigurator::convertRFCFreq_toRegisterValue(uint64_t freq_Hz)
{
    auto f_freq_Hz = static_cast<float>(freq_Hz);
    float value = m * f_freq_Hz + n;

    auto ret_value = static_cast<value_t>(std::floor(value + 0.5));  // round to closest

    return ret_value;
}

uint64_t DeviceLtr11RegisterConfigurator::convertToRfCFreq(uint16_t freq_reg)
{
    float value_Hz = (static_cast<float>(freq_reg) - n) / m;

    auto ret_value_Hz = static_cast<uint64_t>(std::floor(value_Hz));

    return ret_value_Hz;
}

void DeviceLtr11RegisterConfigurator::dumpRegisters(const char* filename) const
{
    std::ofstream f;
    f.open(filename);
    if (!f)
    {
        throw rdk::exception::opening_file();
    }

    for (auto address = 0; address < LTR11_REGISTERS_NUMB; address++)
    {
        f << "REG_" << address << " ";
        f << std::hex << std::setfill('0') << std::setw(sizeof(address)) << address << " ";
        f << std::dec << getRegisterValue(address) << "\n";
    }
    f.close();
}

bool DeviceLtr11RegisterConfigurator::isJapanBand() const
{
    const uint16_t japanMode = 0;
    const uint16_t registerBit = getRegisterValue(E_FUSES_REG55_REG_ADDR) & (1 << pll_japan_mode_BIT);
    return registerBit == japanMode;
}

void DeviceLtr11RegisterConfigurator::disableInternalDetector()
{
    value_t value = DAR_REG1_REG_RST;

    /* The value to set to Register 1 is xored with the register value set by the main controller.
       Since register 1 is initialized to 0, in order to disable the internal detector, we need to write 1
       to the bb_dig_det bit.
    */
    value &= ~bb_dig_det_en_MASK;
    value |= 1 << bb_dig_det_en_BITS;

    addSetRegister({DAR_REG1_REG_ADDR, value});
}

void DeviceLtr11RegisterConfigurator::addSetRegister(const BatchType& reg)
{
    m_registerQueue.push_back(reg);
}

void DeviceLtr11RegisterConfigurator::flushEnqRegisters()
{
    if (m_registers == nullptr)
    {
        throw rdk::exception::missing_interface();
    }
    m_registers->writeBatch(m_registerQueue.data(), static_cast<address_t>(m_registerQueue.size()));
    m_registerQueue.clear();
}
