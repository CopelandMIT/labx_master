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

#include "DeviceMimoseRegisterConfigurator.hpp"

#include "DeviceMimoseConstants.hpp"
#include "registers_BGT24ATR22.hpp"

#include "ifxBase/Exception.hpp"
#include "ifxBase/Log.h"
#include "platform/NamedMemory.hpp"

#include <cassert>
#include <cmath>
#include <memory>

namespace {
constexpr std::array<IRegisters<uint16_t, uint16_t>::BatchType, 5> BGT24ATR22_hardcoded = {{
    {BGT24ATR22_RX_T_BIAS_REG_ADDR, 0x0005},
    {BGT24ATR22_RX_T_RF_REG_ADDR, 0x0007},
    {BGT24ATR22_RX_T_MIX_REG_ADDR, 0x0006},
    //        {BGT24ATR22_RX_BITE0_REG_ADDR, 0x0000},
    //        {BGT24ATR22_RX_BITE1_REG_ADDR, 0x0000},
    {BGT24ATR22_RXABB_T_BIAS_REG_ADDR, 0x0001},
    {BGT24ATR22_RXABB_HF_DELAY_REG_ADDR, 0x0007},
    //        {BGT24ATR22_RXABB_HF_ON_T_REG_ADDR, 0x0131},
    //        {BGT24ATR22_AGC_TH_DIFF_REG_ADDR, 0x0320},
    //        {BGT24ATR22_AOC_CONF_REG_ADDR, 0x0055},
}};

constexpr std::array<DeviceMimoseRegisterConfigurator::BatchType, 4> BGT22ATR22_hard_coded_trigger_config = {{{BGT24ATR22_TRIG0_CONF_REG_ADDR, 0x0013},
                                                                                                              {BGT24ATR22_DRDP_TRIG_MAP_TRG_REG_ADDR, 0x0001},
                                                                                                              {BGT24ATR22_DRDP_TRIG_MAP_CF_REG_ADDR, 0x0008},
                                                                                                              {BGT24ATR22_DRDP_TRIG_MAP_TRG_REG_ADDR, 0x0001}}};

constexpr char const* IFX_MIMOSE_REGISTERS_OVERLOAD_FILE = "atr22_overload.txt";
};  // namespace

template <typename T>
static float_t helperCalcTimeSecFromReg(T reg)
{
    float_t value_mul = reg.MUL;
    float_t value_exp = reg.EXP;
    return value_mul * static_cast<float_t>(std::pow(2, value_exp)) / EXT_OSC_FREQ_HZ;
}

DeviceMimoseRegisterConfigurator::DeviceMimoseRegisterConfigurator(IRegisters<address_t, value_t>* registers) :
    m_registers(registers)
{
    m_registerQueue.reserve(300);
    if (!registers)
    {
        throw rdk::exception::argument_null();
    }

    m_chipVersion = readoutChipVersion();
    if (m_chipVersion != CHIP_VERSION_B1)
    {
        IFX_LOG_ERROR("DeviceMimoseRegisterConfigurator::DeviceMimoseRegisterConfigurator chip version not supported");
        throw rdk::exception::not_supported();
    }
    // initial RC clocks look up table as a fraction of reference clock. can be updated by updateTrimLutRC(...)
    m_rcTrimLut = {0.1310f, 0.1330f, 0.1368f, 0.1392f, 0.1453f, 0.1481f, 0.1531f, 0.1566f,
                   0.1652f, 0.1690f, 0.1756f, 0.1803f, 0.1910f, 0.1969f, 0.2067f, 0.2143f,
                   0.2494f, 0.2567f, 0.2696f, 0.2793f, 0.2999f, 0.3120f, 0.3324f, 0.3491f,
                   0.4060f, 0.4270f, 0.4629f, 0.4931f, 0.5686f, 0.6170f, 0.7058f, 0.7953f};
}

void DeviceMimoseRegisterConfigurator::resetClockConf()
{
    BGT24ATR22::BGT24ATR22_CLK_CONF_REG reg = {};
    reg.value = BGT24ATR22_CLK_CONF_REG_RST;
    DeviceMimoseRegisterConfigurator::BatchType reg_val = {BGT24ATR22_CLK_CONF_REG_ADDR, reg.value};
    addSetRegister(reg_val);
    flushEnqRegisters();
}


void DeviceMimoseRegisterConfigurator::addEnableTrigger2()
{
    {
        BGT24ATR22::BGT24ATR22_TRIG2_CONF_REG reg = {};
        reg.value = 0x3A03;
        DeviceMimoseRegisterConfigurator::BatchType reg_val = {BGT24ATR22_TRIG2_CONF_REG_ADDR, reg.value};
        addSetRegister(reg_val);
    }

    {
        BGT24ATR22::BGT24ATR22_SEQ_TRIG_MAP_AP_REG reg = {};
        reg.value = 0x000F;
        DeviceMimoseRegisterConfigurator::BatchType reg_val = {BGT24ATR22_SEQ_TRIG_MAP_AP_REG_ADDR, reg.value};
        addSetRegister(reg_val);
    }
}

void DeviceMimoseRegisterConfigurator::addEnableStatusTriggerSignal(uint16_t out_trigger)
{
    {
        BGT24ATR22::BGT24ATR22_TRIGX_CONF_REG reg = {};
        reg.value = BGT24ATR22_TRIG1_CONF_REG_RST;
        reg.EN = BGT24ATR22::BGT24ATR22_TRIGX_CONF_EN_ENABLES_PAD;
        reg.DIR = BGT24ATR22::BGT24ATR22_TRIGX_CONF_DIR_SET_DIRECTION_OUTPUT;
        reg.PD_EN = BGT24ATR22::BGT24ATR22_TRIGX_CONF_PD_EN_ENABLES_PULL_DOWN;
        reg.MAP = out_trigger;

        addSetRegister({BGT24ATR22_TRIG1_CONF_REG_ADDR, reg.value});
    }

    // map interrupt router (trig1) to OutputTrig2
    {
        BGT24ATR22::BGT24ATR22_IR_TRIG_MAP_REG reg = {};
        reg.TRIG_MAP = BGT24ATR22::BGT24ATR22_IR_TRIG_MAP_TRIG_MAP_OUTPUT_TRIGGER_2_FOR_INTERRUPT_ROUTER;
        reg.TRIG_EN = BGT24ATR22::BGT24ATR22_IR_TRIG_MAP_TRIG_EN_ENABLES_INTERRUPT_TRIGGER_MAP;

        addSetRegister({BGT24ATR22_IR_TRIG_MAP_REG_ADDR, reg.value});
    }
}

void DeviceMimoseRegisterConfigurator::addEnableDRDPOnlyAcquisition()
{
    std::array<BatchType, 3> registers = {
        BatchType {BGT24ATR22_TRIG0_CONF_REG_ADDR, 0x0023},
        BatchType {BGT24ATR22_DRDP_TRIG_MAP_TRG_REG_ADDR, 0x0001},
        BatchType {BGT24ATR22_DRDP_TRIG_MAP_CF_REG_ADDR, 0x0008}};

    addRegisters(registers);
}

void DeviceMimoseRegisterConfigurator::addEnableTriggerAndDRDPAcquisition(uint16_t triggerCount)
{
    std::array<BatchType, 4> registers = {
        BatchType {BGT24ATR22_DRDP_TRIG_MAP_TRG_REG_ADDR, 0x0001},
        BatchType {BGT24ATR22_DRDP_TRIG_MAP_CF_REG_ADDR, 0x0008},
        BatchType {BGT24ATR22_TRIG3_CONF_REG_ADDR, 0x0303},
        BatchType {BGT24ATR22_TRIG0_CONF_REG_ADDR, 0x1823}};

    addRegisters(registers);

    {
        DeviceMimoseRegisterConfigurator::BatchType reg_val {BGT24ATR22_FRAME0_SEQ_TRIG_MAP_IRQ_REG_ADDR, BGT24ATR22_FRAME0_SEQ_TRIG_MAP_IRQ_REG_RST};

        BGT24ATR22::BGT24ATR22_FRAMEX_SEQ_TRIG_MAP_IRQ_REG reg;
        reg.value = reg_val.value;
        reg.TRIG_EN = 1;
        reg.TRIG_MAP = 3;
        reg.TRIG_COUNT = triggerCount;

        addSetRegister(BatchType {BGT24ATR22_FRAME0_SEQ_TRIG_MAP_IRQ_REG_ADDR, reg.value});
        // To check proper implementation in case of multiple frame config
        // addSetRegister(BatchType{ BGT24ATR22_FRAME1_SEQ_TRIG_MAP_IRQ_REG_ADDR, reg.value });
    }
}

void DeviceMimoseRegisterConfigurator::addSeqExecute(bool enable, size_t frame_configuration_index)
{
    if (frame_configuration_index >= NO_OF_FRAME_CONFIGURATIONS)
    {
        throw rdk::exception::argument_out_of_bounds();
    }
    if (m_chipVersion == CHIP_VERSION_A2)
    {
        // Needed as for A2 version as amplifier will not run on FSM
        addTX_DAC_OverwriteChannelC(enable);
    }

    BatchType reg_val {BGT24ATR22_SEQ_MAIN_CONF_REG_ADDR, BGT24ATR22_SEQ_MAIN_CONF_REG_RST};
    BGT24ATR22::BGT24ATR22_SEQ_MAIN_CONF_REG reg;
    reg.value = reg_val.value;
    if (enable)
    {
        reg.SEQ_EXECUTE = BGT24ATR22::BGT24ATR22_SEQ_MAIN_CONF_SEQ_EXECUTE_START_THE_SEQUENCE_OF_FRAMES_SHOWS_IF_SEQUENCE_IS_BEING_EXECUTED;
    }
    else
    {
        reg.SEQ_EXECUTE = BGT24ATR22::BGT24ATR22_SEQ_MAIN_CONF_SEQ_EXECUTE_DO_NOT_RUN_THE_SEQUENCE_STOP_THE_CURRENT_SEQUENCE_AFTER_THE_CURRENT_FRAME;
    }

    reg.NEXT_FRAME_CONF = frame_configuration_index;
    reg_val.value = reg.value;
    addSetRegister(reg_val);
}

void DeviceMimoseRegisterConfigurator::addTX_DAC_OverwriteChannelC(bool enable)
{
    BGT24ATR22::BGT24ATR22_TX1_TEST_REG reg_tx1;
    reg_tx1.value = BGT24ATR22_ADC_CONF_REG_RST;
    BGT24ATR22::BGT24ATR22_TX2_TEST_REG reg_tx2;
    reg_tx2.value = BGT24ATR22_ADC_CONF_REG_RST;
    if (enable)
    {
        reg_tx1.DAC_OW_EN = BGT24ATR22::BGT24ATR22_TX1_TEST_DAC_OW_EN_ENABLES_THE_DAC_IN_OVERWRITE_MODE;
        reg_tx2.DAC_OW_EN = BGT24ATR22::BGT24ATR22_TX2_TEST_DAC_OW_EN_ENABLES_THE_DAC_IN_OVERWRITE_MODE;
    }
    else
    {
        reg_tx1.DAC_OW_EN = BGT24ATR22::BGT24ATR22_TX1_TEST_DAC_OW_EN_DISABLES_THE_DAC_IN_OVERWRITE_MODE;
        reg_tx2.DAC_OW_EN = BGT24ATR22::BGT24ATR22_TX2_TEST_DAC_OW_EN_DISABLES_THE_DAC_IN_OVERWRITE_MODE;
    }
    addSetRegister({BGT24ATR22_TX1_TEST_REG_ADDR, reg_tx1.value});
    addSetRegister({BGT24ATR22_TX2_TEST_REG_ADDR, reg_tx2.value});
}

void DeviceMimoseRegisterConfigurator::reset()
{
    m_registerQueue.clear();
    BGT24ATR22::BGT24ATR22_SEQ_MAIN_CONF_REG reg;
    reg.value = BGT24ATR22_SEQ_MAIN_CONF_REG_RST;
    reg.SW_RST = 1;
    BatchType regAddrVal = {BGT24ATR22_SEQ_MAIN_CONF_REG_ADDR, reg.value};
    addSetRegister(regAddrVal);
    reg.SW_RST = 0;
    regAddrVal = {BGT24ATR22_SEQ_MAIN_CONF_REG_ADDR, reg.value};
    addSetRegister(regAddrVal);
    flushEnqRegisters();
    // reset SDK register list
    reset_register_list();
}

void DeviceMimoseRegisterConfigurator::reset_register_list()
{
    auto* regs_list = BGT24ATR22::BGT24ATR22_registers.data();
    const auto* reset_regs = BGT24ATR22::BGT24ATR22_registers_reset.data();
    size_t size = BGT24ATR22::BGT24ATR22_registers.size();

    for (uint32_t i = 0; i < size; i++)
    {
        regs_list[i].value = reset_regs[i].value;
    }
    // retrieve MMIC specific registers
    readRegisterValue(BGT24ATR22_XOSC_CLK_CONF_REG_ADDR);
    readRegisterValue(BGT24ATR22_CHIP_TYPE_REG_ADDR);
    readRegisterValue(BGT24ATR22_CHIP0_UID_REG_ADDR);
    readRegisterValue(BGT24ATR22_CHIP1_UID_REG_ADDR);
    readRegisterValue(BGT24ATR22_CHIP2_UID_REG_ADDR);
    readRegisterValue(BGT24ATR22_TEMP_CAL_REG_ADDR);
    readRegisterValue(BGT24ATR22_EFUSE_REG_ADDR);
}


void DeviceMimoseRegisterConfigurator::addInterrupt(bool targetDetect, bool sequencerError)
{
    DeviceMimoseRegisterConfigurator::BatchType reg_val = {BGT24ATR22_IR_EN_REG_ADDR, BGT24ATR22_IR_EN_REG_RST};

    BGT24ATR22::BGT24ATR22_IR_EN_REG reg = {};
    reg.value = reg_val.value;
    reg.EN8 = targetDetect ? BGT24ATR22::BGT24ATR22_IR_EN_EN8_ENABLES_INTERRUPT : BGT24ATR22::BGT24ATR22_IR_EN_EN8_DISABLES_INTERRUPT;
    reg.EN12 = sequencerError ? BGT24ATR22::BGT24ATR22_IR_EN_EN12_ENABLES_INTERRUPT : BGT24ATR22::BGT24ATR22_IR_EN_EN12_DISABLES_INTERRUPT;
    addSetRegister({BGT24ATR22_IR_EN_REG_ADDR, reg.value});
}

void DeviceMimoseRegisterConfigurator::addAOC_ThStep(uint16_t gain_index)
{
    using StepPair = std::pair<uint16_t, uint16_t>;
    using ThreshPair = std::pair<uint16_t, uint16_t>;
    std::vector<StepPair> steps = {
        {0x0001, 0x0001},
        {0x0001, 0x0001},
        {0x0001, 0x0001},
        {0x0001, 0x0001},
        {0x0001, 0x0003},
        {0x0001, 0x0003},
        {0x0001, 0x0003},
        {0x0001, 0x0003}};
    std::vector<ThreshPair> thresh = {
        {0x1FFF, 0x1FFF},
        {0x1FFF, 0x1FFF},
        {0x1FFF, 0x1FFF},
        {0x15E0, 0x1FFF},
        {0x0AF0, 0x1FFF},
        {0x0578, 0x1068},
        {0x02BC, 0x0834},
        {0x015E, 0x041A}};
    const auto lutStepPair = steps[gain_index];
    const auto lutThreshPair = thresh[gain_index];

    const std::array<BatchType, 4> registers = {
        BatchType {BGT24ATR22_AOC_TH_0_REG_ADDR, lutThreshPair.first},
        BatchType {BGT24ATR22_AOC_TH_1_REG_ADDR, lutThreshPair.second},
        BatchType {BGT24ATR22_AOC_STP_0_REG_ADDR, lutStepPair.first},
        BatchType {BGT24ATR22_AOC_STP_1_REG_ADDR, lutStepPair.second}};

    addRegisters(registers);
}

void DeviceMimoseRegisterConfigurator::addFramePeriod(size_t frame_configuration_index, uint64_t frame_period_clock_cycles)
{
    const uint16_t regs_addr[] = {
        BGT24ATR22_FRAME0_TIME_REG_ADDR,
        BGT24ATR22_FRAME1_TIME_REG_ADDR};

    BGT24ATR22::BGT24ATR22_X_TIME_REG reg;

    reg.value = encodeExpMulValue16(frame_period_clock_cycles, 5, 11);
    addSetRegister({regs_addr[frame_configuration_index], reg.value});
}

void DeviceMimoseRegisterConfigurator::addNumberOfSamples(size_t frame_configuration_index, uint16_t num_samples)
{
    IFX_LOG_DEBUG("DeviceMimoseRegisterConfigurator::addNumberOfSamples set number of samples=%u", (unsigned)num_samples);
    if (frame_configuration_index >= NO_OF_FRAME_CONFIGURATIONS)
    {
        throw rdk::exception::argument_out_of_bounds();
    }
    const uint16_t regs_addr[] = {
        BGT24ATR22_FRAME0_LIST_REP_REG_ADDR,
        BGT24ATR22_FRAME1_LIST_REP_REG_ADDR};
    BatchType reg;
    reg.address = regs_addr[frame_configuration_index];
    reg.value = num_samples;
    addSetRegister(reg);
}

void DeviceMimoseRegisterConfigurator::addSetPreTrigger(size_t frame_configuration_index, uint16_t num_samples_per_frame, uint16_t active_pc_count, uint32_t pre_trigger_count)
{
    IFX_LOG_DEBUG("DeviceMimoseRegisterConfigurator::addSetPreTrigger set pre_trigger using count=%u", (uint32_t)pre_trigger_count);
    if (frame_configuration_index >= NO_OF_FRAME_CONFIGURATIONS)
    {
        throw rdk::exception::argument_out_of_bounds();
    }
    const uint16_t regs_addr[] = {
        BGT24ATR22_FRAME0_SEQ_TRIG_MAP_IRQ_REG_ADDR,
        BGT24ATR22_FRAME1_SEQ_TRIG_MAP_IRQ_REG_ADDR};

    BGT24ATR22::BGT24ATR22_FRAMEX_SEQ_TRIG_MAP_IRQ_REG reg;
    reg.value = BGT24ATR22_FRAME0_SEQ_TRIG_MAP_IRQ_REG_RST;
    uint32_t value = num_samples_per_frame * active_pc_count;
    uint32_t trig_samples_max = (1 << 12) - 1;  // from the register bits definition
    if (value < pre_trigger_count)
    {
        reg.TRIG_COUNT = 0;
    }
    else
    {
        uint32_t trig_samples = value - pre_trigger_count;
        if (trig_samples > trig_samples_max)
        {
            throw rdk::exception::not_possible();
        }
        reg.TRIG_COUNT = static_cast<uint16_t>(trig_samples);
    }
    addSetRegister({regs_addr[frame_configuration_index], reg.value});
}

void DeviceMimoseRegisterConfigurator::addPulseRepetitionTime(size_t pulse_index, size_t frame_configuration_index, uint64_t pulse_repetition_time_per_pc_cycles)
{
    IFX_LOG_DEBUG("DeviceMimoseRegisterConfigurator::addPulseRepetitionTimeSec set PC=%u frame+index=%u pulse rep. cycles=%u",
                  pulse_index,
                  frame_configuration_index,
                  pulse_repetition_time_per_pc_cycles);
    if (frame_configuration_index >= NO_OF_FRAME_CONFIGURATIONS)
    {
        throw rdk::exception::argument_out_of_bounds();
    }
    if (pulse_index >= NO_OF_PULSES)
    {
        throw rdk::exception::argument_out_of_bounds();
    }

    // Well, here the power of string manipulation is used, to avoid if-else chains.
    // I hope it is clear, how to select the right register depending on frame and and pulse config index

    {
        const uint16_t regs_addr[] = {
            BGT24ATR22_PC0_CONF_TIME_REG_ADDR,
            BGT24ATR22_PC1_CONF_TIME_REG_ADDR,
            BGT24ATR22_PC2_CONF_TIME_REG_ADDR,
            BGT24ATR22_PC3_CONF_TIME_REG_ADDR};
        BatchType reg_val;
        reg_val.address = regs_addr[pulse_index];
        reg_val.value = encodeExpMulValue16(pulse_repetition_time_per_pc_cycles, 5, 11);
        addSetRegister(reg_val);
    }
    {
        const uint16_t regs_addr[2][4] = {
            {
                BGT24ATR22_FRAME0_LIST0_CONF_REG_ADDR,
                BGT24ATR22_FRAME0_LIST1_CONF_REG_ADDR,
                BGT24ATR22_FRAME0_LIST2_CONF_REG_ADDR,
                BGT24ATR22_FRAME0_LIST3_CONF_REG_ADDR,
            },
            {
                BGT24ATR22_FRAME1_LIST0_CONF_REG_ADDR,
                BGT24ATR22_FRAME1_LIST1_CONF_REG_ADDR,
                BGT24ATR22_FRAME1_LIST2_CONF_REG_ADDR,
                BGT24ATR22_FRAME1_LIST3_CONF_REG_ADDR,
            }};
        BatchType reg_val;
        reg_val.address = regs_addr[frame_configuration_index][pulse_index];
        BGT24ATR22::BGT24ATR22_FRAMEX_LISTX_CONF_REG reg;
        reg.RSVD0 = 0;
        reg.PC_CONF_NR = pulse_index;
        reg.ENTRY_EN = BGT24ATR22::BGT24ATR22_FRAMEX_LISTX_CONF_ENTRY_EN_ENABLES_LIST_ENTRY;
        reg_val.value = reg.value;
        addSetRegister(reg_val);
    }
}

void DeviceMimoseRegisterConfigurator::addRFChannel(size_t pulse_index, ifx_Mimose_Channel_t channel, int tx_power_level)
{
    if (pulse_index >= NO_OF_PULSES)
    {
        throw rdk::exception::argument_out_of_bounds();
    }
    const uint16_t regs_addr_tx1[4] = {
        BGT24ATR22_TX1_PC0_CONF_REG_ADDR,
        BGT24ATR22_TX1_PC1_CONF_REG_ADDR,
        BGT24ATR22_TX1_PC2_CONF_REG_ADDR,
        BGT24ATR22_TX1_PC3_CONF_REG_ADDR};
    const uint16_t regs_addr_tx2[4] = {
        BGT24ATR22_TX2_PC0_CONF_REG_ADDR,
        BGT24ATR22_TX2_PC1_CONF_REG_ADDR,
        BGT24ATR22_TX2_PC2_CONF_REG_ADDR,
        BGT24ATR22_TX2_PC3_CONF_REG_ADDR};
    const uint16_t regs_addr_rx1[4] = {
        BGT24ATR22_RX1_PC0_CONF_REG_ADDR,
        BGT24ATR22_RX1_PC1_CONF_REG_ADDR,
        BGT24ATR22_RX1_PC2_CONF_REG_ADDR,
        BGT24ATR22_RX1_PC3_CONF_REG_ADDR};
    const uint16_t regs_addr_rx2[4] = {
        BGT24ATR22_RX2_PC0_CONF_REG_ADDR,
        BGT24ATR22_RX2_PC1_CONF_REG_ADDR,
        BGT24ATR22_RX2_PC2_CONF_REG_ADDR,
        BGT24ATR22_RX2_PC3_CONF_REG_ADDR};
    BGT24ATR22::BGT24ATR22_TXX_PCX_CONF_REG reg_TX_disable;
    reg_TX_disable.value = BGT24ATR22_TX1_PC0_CONF_REG_RST;
    reg_TX_disable.EN = BGT24ATR22::BGT24ATR22_TXX_PCX_CONF_EN_DISABLES_TX_CHANNEL;

    BGT24ATR22::BGT24ATR22_TXX_PCX_CONF_REG reg_TX_enable;
    reg_TX_enable.value = BGT24ATR22_TX1_PC0_CONF_REG_RST;
    reg_TX_enable.EN = BGT24ATR22::BGT24ATR22_TXX_PCX_CONF_EN_ENABLES_TX_CHANNEL;
    reg_TX_enable.PWR_DAC = tx_power_level;

    BGT24ATR22::BGT24ATR22_RXX_PCX_CONF_REG reg_RX_disable;
    reg_RX_disable.value = 0;
    reg_RX_disable.EN = BGT24ATR22::BGT24ATR22_RXX_PCX_CONF_EN_DISABLES_THE_RX_CHANNEL;

    BGT24ATR22::BGT24ATR22_RXX_PCX_CONF_REG reg_RX_enable;
    reg_RX_enable.value = 0;
    reg_RX_enable.EN = BGT24ATR22::BGT24ATR22_RXX_PCX_CONF_EN_ENABLES_THE_RX_CHANNEL;

    if (channel == IFX_MIMOSE_CHANNEL_TX1_RX1)
    {
        std::array<BatchType, 4> registers {{
            {regs_addr_tx1[pulse_index], reg_TX_enable.value},
            {regs_addr_tx2[pulse_index], reg_TX_disable.value},
            {regs_addr_rx1[pulse_index], reg_RX_enable.value},
            {regs_addr_rx2[pulse_index], reg_RX_disable.value},
        }};
        addRegisters(registers);
    }
    else if (channel == IFX_MIMOSE_CHANNEL_TX2_RX2)
    {
        std::array<BatchType, 4> registers {{
            {regs_addr_tx1[pulse_index], reg_TX_disable.value},
            {regs_addr_tx2[pulse_index], reg_TX_enable.value},
            {regs_addr_rx1[pulse_index], reg_RX_disable.value},
            {regs_addr_rx2[pulse_index], reg_RX_enable.value},
        }};
        addRegisters(registers);
    }
    else if (channel == IFX_MIMOSE_CHANNEL_TX1_RX2)
    {
        std::array<BatchType, 4> registers {{
            {regs_addr_tx1[pulse_index], reg_TX_enable.value},
            {regs_addr_tx2[pulse_index], reg_TX_disable.value},
            {regs_addr_rx1[pulse_index], reg_RX_disable.value},
            {regs_addr_rx2[pulse_index], reg_RX_enable.value},
        }};
        addRegisters(registers);
    }
    else if (channel == IFX_MIMOSE_CHANNEL_TX2_RX1)
    {
        std::array<BatchType, 4> registers {{
            {regs_addr_tx1[pulse_index], reg_TX_disable.value},
            {regs_addr_tx2[pulse_index], reg_TX_enable.value},
            {regs_addr_rx1[pulse_index], reg_RX_enable.value},
            {regs_addr_rx2[pulse_index], reg_RX_disable.value},
        }};
        addRegisters(registers);
    }
    else
    {
        throw rdk::exception::error();
    }
}


void DeviceMimoseRegisterConfigurator::addRegisterABB_gain(size_t pulse_index, bool agc, int gain_index)
{
    IFX_LOG_DEBUG("DeviceMimoseRegisterConfigurator::addRegisterAGC_gain set pulse_index=%u enabled=%s gain=%i", pulse_index, agc ? "true" : "false", gain_index);

    if (pulse_index >= NO_OF_PULSES)
    {
        throw rdk::exception::argument_out_of_bounds();
    }
    if (gain_index < 0)
    {
        throw rdk::exception::argument_out_of_bounds();
    }
    if (gain_index > 7)
    {
        throw rdk::exception::argument_out_of_bounds();
    }

    if (!agc)
    {
        // Correction needed as workaround
        addAOC_ThStep(gain_index);
    }

    BGT24ATR22::BGT24ATR22_PCX_AGC_REG reg;
    BatchType reg_val;
    uint16_t enable_flag;

    if (agc)
    {
        enable_flag = BGT24ATR22::BGT24ATR22_AGC_PCX_AGC_PC_EN_ENABLES_THE_AUTOMATIC_GAIN_CONTROL;
    }
    else
    {
        enable_flag = BGT24ATR22::BGT24ATR22_AGC_PCX_AGC_PC_EN_DISABLES_THE_AUTOMATIC_GAIN_CONTROL;
    }

    switch (pulse_index)
    {
        case 0:
            reg_val = {BGT24ATR22_PC0_AGC_REG_ADDR, BGT24ATR22_PC0_AGC_REG_RST};
            break;
        case 1:
            reg_val = {BGT24ATR22_PC1_AGC_REG_ADDR, BGT24ATR22_PC1_AGC_REG_RST};
            break;
        case 2:
            reg_val = {BGT24ATR22_PC2_AGC_REG_ADDR, BGT24ATR22_PC2_AGC_REG_RST};
            break;
        case 3:
            reg_val = {BGT24ATR22_PC3_AGC_REG_ADDR, BGT24ATR22_PC3_AGC_REG_RST};
            break;
        default:
            throw rdk::exception::error();
    }
    reg.value = reg_val.value;
    reg.AGC_PC_EN = enable_flag;
    reg.AGC_ABB_GAIN = gain_index;
    reg_val.value = reg.value;
    addSetRegister(reg_val);
}


void DeviceMimoseRegisterConfigurator::addRegisterAOC(size_t pulse_index, ifx_Mimose_AOC_Mode_t aoc_mode)
{
    IFX_LOG_DEBUG("DeviceMimoseRegisterConfigurator::addRegisterAOC set pulse_index=%u  mode=%i", pulse_index, aoc_mode);

    if (pulse_index >= NO_OF_PULSES || aoc_mode > NO_OF_MODES)
    {
        throw rdk::exception::argument_out_of_bounds();
    }

    BGT24ATR22::BGT24ATR22_AOC_CONF_REG reg;
    BatchType reg_val {BGT24ATR22_AOC_CONF_REG_ADDR, BGT24ATR22_AOC_CONF_REG_RST};
    reg.value = reg_val.value;

    switch (pulse_index)
    {
        case 0:
            reg.MODE0 = aoc_mode;
            break;
        case 1:
            reg.MODE1 = aoc_mode;
            break;
        case 2:
            reg.MODE2 = aoc_mode;
            break;
        case 3:
            reg.MODE3 = aoc_mode;
            break;
        default:
            throw rdk::exception::error();
    }
    reg_val.value = reg.value;
    addSetRegister(reg_val);
}

void DeviceMimoseRegisterConfigurator::addMemOffsets(uint16_t num_samples, uint16_t num_active_pc, bool use_mem_trg_mem_sens)
{
    const auto total_num_samples(num_samples * num_active_pc);

    if (total_num_samples > IQ_SAMPLES_MAX_READCOUNT_ALLOWED)
    {
        throw rdk::exception::not_supported();
    }

    uint16_t next_mem_offset = 0u;

    addSetRegister({BGT24ATR22_MEM_RAW_REG_ADDR, next_mem_offset});
    addSetRegister({BGT24ATR22_MEM_RAW2_REG_ADDR, next_mem_offset});

    next_mem_offset = ((next_mem_offset + total_num_samples) < IQ_SAMPLES_MAX_READCOUNT_ALLOWED) ? (next_mem_offset + total_num_samples) : IQ_SAMPLES_MAX_READCOUNT_ALLOWED;
    addSetRegister({BGT24ATR22_MEM_FT_REG_ADDR, next_mem_offset});

    if (use_mem_trg_mem_sens)
    {
        next_mem_offset = ((next_mem_offset + total_num_samples) < IQ_SAMPLES_MAX_READCOUNT_ALLOWED) ? (next_mem_offset + total_num_samples) : IQ_SAMPLES_MAX_READCOUNT_ALLOWED;
        addSetRegister({BGT24ATR22_MEM_TRG_REG_ADDR, next_mem_offset});

        next_mem_offset = ((next_mem_offset + 32) < IQ_SAMPLES_MAX_READCOUNT_ALLOWED) ? (next_mem_offset + 32) : IQ_SAMPLES_MAX_READCOUNT_ALLOWED;
        addSetRegister({BGT24ATR22_MEM_SENS_REG_ADDR, next_mem_offset});
    }
    else
    {
        addSetRegister({BGT24ATR22_MEM_TRG_REG_ADDR, IQ_SAMPLES_MAX_READCOUNT_ALLOWED});
        addSetRegister({BGT24ATR22_MEM_SENS_REG_ADDR, IQ_SAMPLES_MAX_READCOUNT_ALLOWED});
    }
}

void DeviceMimoseRegisterConfigurator::addHardcodedConfig()
{
    IFX_LOG_DEBUG("DeviceMimoseRegisterConfigurator::addRegistersHardcodedConfig apply");
    addRegisters(BGT24ATR22_hardcoded.data(), BGT24ATR22_hardcoded.size());
}

void DeviceMimoseRegisterConfigurator::addHardcodedTrigger()
{
    IFX_LOG_DEBUG("DeviceMimoseRegisterConfigurator::addHardcodedTrigger apply");
    addRegisters(BGT22ATR22_hard_coded_trigger_config);
}

void DeviceMimoseRegisterConfigurator::applyFileConfig()
{
    try
    {
        using MyNamedMemory = NamedMemory<address_t, value_t>;
        MyNamedMemory nmem(m_registers);
        std::vector<BatchType> regs;
        nmem.loadConfig(IFX_MIMOSE_REGISTERS_OVERLOAD_FILE, regs);
        nmem.write(regs.data(), static_cast<address_t>(regs.size()), false);
#ifdef IFX_LOG_SEVERITY_DEBUG
        IFX_LOG_DEBUG("DeviceMimoseRegisterConfigurator::applyFileConfig loaded overwrite config with %i registers", regs.size());
        for (auto& reg : regs)
        {
            IFX_LOG_DEBUG("DeviceMimoseRegisterConfigurator::applyFileConfig 0x%04X->0x%04X", reg.address, reg.value);
        }
#endif
    }
    catch (EMemory&)
    {
        IFX_LOG_DEBUG("DeviceMimoseRegisterConfigurator::applyFileConfig no possible to read overwrite config");
    }
}

void DeviceMimoseRegisterConfigurator::addFFT_Len(size_t pulse, size_t len)
{
    uint16_t BGT24ATR22_FTX_CONF_REG_ADDR[] = {
        BGT24ATR22_FT0_CONF_REG_ADDR,
        BGT24ATR22_FT1_CONF_REG_ADDR,
        BGT24ATR22_FT2_CONF_REG_ADDR,
        BGT24ATR22_FT3_CONF_REG_ADDR};
    constexpr uint16_t BGT24ATR22_FTX_CONF_REG_RST[] = {
        BGT24ATR22_FT0_CONF_REG_RST,
        BGT24ATR22_FT1_CONF_REG_RST,
        BGT24ATR22_FT2_CONF_REG_RST,
        BGT24ATR22_FT3_CONF_REG_RST};

    BatchType regAddrVal {BGT24ATR22_FTX_CONF_REG_ADDR[pulse], BGT24ATR22_FTX_CONF_REG_RST[pulse]};
    BGT24ATR22::BGT24ATR22_FTX_CONF_REG reg;
    reg.value = regAddrVal.value;
    if (len <= 16)
    {
        reg.FT_LEN = 0;
    }
    else if (len <= 32)
    {
        reg.FT_LEN = 1;
    }
    else if (len <= 64)
    {
        reg.FT_LEN = 2;
    }
    else
    {
        reg.FT_LEN = 3;
    }
    reg.EN = 1;
    regAddrVal.value = reg.value;
    addSetRegister(regAddrVal);
}

void DeviceMimoseRegisterConfigurator::addNoHeatingPulses(size_t frame_configuration_index, size_t noHeatingPulses)
{
    if (noHeatingPulses > 4)
    {
        throw rdk::exception::argument_out_of_bounds();
    }

    uint16_t addresses[] = {BGT24ATR22_FRAME0_HEATING_REG_ADDR, BGT24ATR22_FRAME1_HEATING_REG_ADDR};
    const auto address = addresses[frame_configuration_index];

    BGT24ATR22::BGT24ATR22_FRAME0_HEATING_REG reg;
    reg.value = 0;
    reg.NR_HEATING_PULSES = noHeatingPulses;

    BatchType regAddrVal = {address, reg.value};
    addSetRegister(regAddrVal);
}

void DeviceMimoseRegisterConfigurator::addRCTbootRefClk(uint64_t t_boot_ref_clk_cycles)
{
    BGT24ATR22::BGT24ATR22_T_BOOT_REF_CLK_REG reg;
    reg.value = BGT24ATR22_T_BOOT_REF_CLK_REG_RST;

    uint16_t reg_exp_bgt_mult = reg.EXP_BG_MULT;
    reg.value = encodeExpMulValue16(t_boot_ref_clk_cycles, 5, 8);
    reg.EXP_BG_MULT = reg_exp_bgt_mult;

    addSetRegister({BGT24ATR22_T_BOOT_REF_CLK_REG_ADDR, reg.value});
}

void DeviceMimoseRegisterConfigurator::addAFC_Duration(uint16_t AFC_DurationCT)
{
    BGT24ATR22::BGT24ATR22_VCO_AFC_DURATION_REG reg;
    reg.value = 0;
    reg.VAL = AFC_DurationCT;
    addSetRegister({BGT24ATR22_VCO_AFC_DURATION_REG_ADDR, reg.value});
}

uint16_t DeviceMimoseRegisterConfigurator::calcAFCConfLoCnt_DelayTime(uint16_t reg_val_BGT24ATR22_VCO_AFC_CONF_REG)
{
    BGT24ATR22::BGT24ATR22_VCO_AFC_CONF_REG regVal;
    regVal.value = reg_val_BGT24ATR22_VCO_AFC_CONF_REG;
    return regVal.LO_CNT_DEL;
}

void DeviceMimoseRegisterConfigurator::addAFC_Threshold(uint16_t thresholdFine, uint16_t thresholdCourse)
{
    BGT24ATR22::BGT24ATR22_VCO_AFC_TH0_REG regTh0;
    BGT24ATR22::BGT24ATR22_VCO_AFC_TH1_REG regTh1;

    regTh0.value = BGT24ATR22_VCO_AFC_TH0_REG_RST;
    regTh1.value = BGT24ATR22_VCO_AFC_TH1_REG_RST;

    regTh0.THRESHOLD = thresholdCourse;
    regTh1.THRESHOLD = thresholdFine;

    addSetRegister({BGT24ATR22_VCO_AFC_TH0_REG_ADDR, regTh0.value});
    addSetRegister({BGT24ATR22_VCO_AFC_TH1_REG_ADDR, regTh1.value});
}

void DeviceMimoseRegisterConfigurator::addAFC_RepetitionPeriod(ifx_Mimose_AFC_Repeat_Count_t repetition)
{
    BGT24ATR22::BGT24ATR22_VCO_AFC_CONF_REG reg;
    reg.value = BGT24ATR22_VCO_AFC_CONF_REG_RST;

    reg.AFC_REP = repetition;

    addSetRegister({BGT24ATR22_VCO_AFC_CONF_REG_ADDR, reg.value});
}

uint32_t DeviceMimoseRegisterConfigurator::getSystemClockRC(uint32_t reference_clock_Hz,
                                                            uint32_t desired_sys_clock_Hz)
{
    const float_t rc_clock_factor = static_cast<float_t>(desired_sys_clock_Hz) / static_cast<float_t>(reference_clock_Hz);

    float_t min_diff = 1000.f;
    size_t min_idx = 1000;
    for (size_t idx = 0; idx < m_rcTrimLut.size(); idx++)
    {
        if (abs(m_rcTrimLut.at(idx) - rc_clock_factor) < min_diff)
        {
            min_diff = abs(m_rcTrimLut.at(idx) - rc_clock_factor);
            min_idx = idx;
        }
    }
    if (min_idx > m_rcTrimLut.size())
    {
        throw rdk::exception::argument_out_of_bounds();
    }

    const float_t precision_hz = 20000.f;
    float_t N_sys_clk = std::ceil(2 * static_cast<float_t>(reference_clock_Hz) / precision_hz);
    if (static_cast<uint32_t>(N_sys_clk) > UINT16_MAX)
    {
        throw rdk::exception::argument_out_of_bounds();
    }
    addSetRegister({BGT24ATR22_RC_T_TRIM_REG_ADDR, static_cast<uint16_t>(N_sys_clk)});

    BGT24ATR22::BGT24ATR22_RC_CLK_CONF_REG reg_clk_conf;
    reg_clk_conf.value = BGT24ATR22_RC_CLK_CONF_REG_RST;
    reg_clk_conf.EN = 1;
    reg_clk_conf.TRIM_VAL = static_cast<uint16_t>(min_idx);
    reg_clk_conf.RC_COUNT = 1;
    addSetRegister({BGT24ATR22_RC_CLK_CONF_REG_ADDR, reg_clk_conf.value});
    flushEnqRegisters();
    uint16_t trim_count = readRegisterValue(BGT24ATR22_RC_TRIM_VAL_REG_ADDR);
    float_t system_clock_Hz = (static_cast<float_t>(reference_clock_Hz) / N_sys_clk) * static_cast<float_t>(trim_count);
    if (static_cast<uint32_t>(system_clock_Hz) > reference_clock_Hz)
    {
        throw rdk::exception::argument_out_of_bounds();
    }
    return static_cast<uint32_t>(system_clock_Hz);
}

void DeviceMimoseRegisterConfigurator::updateTrimLutRC(uint32_t reference_clock_Hz)
{
    const float_t precision_hz = 20000.f;
    float_t N_sys_clk = std::ceil(2 * static_cast<float_t>(reference_clock_Hz) / precision_hz);
    if (static_cast<uint32_t>(N_sys_clk) > static_cast<uint32_t>(UINT16_MAX))
    {
        throw rdk::exception::argument_out_of_bounds();
    }
    addSetRegister({BGT24ATR22_RC_T_TRIM_REG_ADDR, static_cast<uint16_t>(N_sys_clk)});

    BGT24ATR22::BGT24ATR22_RC_CLK_CONF_REG reg_clk_conf;
    reg_clk_conf.value = BGT24ATR22_RC_CLK_CONF_REG_RST;
    reg_clk_conf.EN = 1;
    // Generate look up table
    for (size_t trim_idx = 0; trim_idx < 32; trim_idx++)
    {
        reg_clk_conf.TRIM_VAL = static_cast<uint16_t>(trim_idx);
        reg_clk_conf.RC_COUNT = 1;
        addSetRegister({BGT24ATR22_RC_CLK_CONF_REG_ADDR, reg_clk_conf.value});
        flushEnqRegisters();
        uint16_t trim_value = readRegisterValue(BGT24ATR22_RC_TRIM_VAL_REG_ADDR);
        m_rcTrimLut.at(trim_idx) = static_cast<float_t>(trim_value) / N_sys_clk;
    }
}

uint32_t DeviceMimoseRegisterConfigurator::addClockConfigRegisters(uint32_t reference_clock_Hz,
                                                                   uint32_t desired_sys_clock_Hz,
                                                                   bool rc_clock_enabled,
                                                                   uint32_t hf_on_time_usec,
                                                                   uint16_t system_clock_divider,
                                                                   bool system_clock_div_flex,
                                                                   bool sys_clk_to_i2c)
{
    uint32_t system_clock_Hz;
    BGT24ATR22::BGT24ATR22_CLK_CONF_REG reg_clk_conf;
    reg_clk_conf.value = BGT24ATR22_CLK_CONF_REG_RST;
    // RC clock configuration
    if (rc_clock_enabled)
    {
        // generate lookup table and retrieve closest clock
        // updateTrimLutRC(reference_clock_Hz, rc_clock_Hz);
        system_clock_Hz = getSystemClockRC(reference_clock_Hz, desired_sys_clock_Hz);

        // adapt delays
        float_t rc_clock_ratio = static_cast<float_t>(system_clock_Hz) / static_cast<float_t>(reference_clock_Hz);
        // adapt T_BOOT DELAY registers
        BGT24ATR22::BGT24ATR22_T_BOOT_VCO_FS_REG reg_t_boot_vco_fs;
        reg_t_boot_vco_fs.value = BGT24ATR22_T_BOOT_VCO_FS_REG_RST;
        reg_t_boot_vco_fs.VAL = static_cast<uint16_t>(std::ceil(static_cast<float_t>(reg_t_boot_vco_fs.VAL) * rc_clock_ratio));
        addSetRegister({BGT24ATR22_T_BOOT_VCO_FS_REG_ADDR, reg_t_boot_vco_fs.value});

        BGT24ATR22::BGT24ATR22_T_BOOT_TXCHAIN_REG reg_t_boot_txchain;
        reg_t_boot_txchain.value = BGT24ATR22_T_BOOT_TXCHAIN_REG_RST;
        reg_t_boot_txchain.VAL = static_cast<uint16_t>(std::ceil(static_cast<float_t>(reg_t_boot_txchain.VAL) * rc_clock_ratio));
        addSetRegister({BGT24ATR22_T_BOOT_TXCHAIN_REG_ADDR, reg_t_boot_txchain.value});

        BGT24ATR22::BGT24ATR22_T_BOOT_RXCHAIN_REG reg_t_boot_rxchain;
        reg_t_boot_rxchain.value = BGT24ATR22_T_BOOT_RXCHAIN_REG_RST;
        reg_t_boot_rxchain.VAL = static_cast<uint16_t>(std::ceil(static_cast<float_t>(reg_t_boot_rxchain.VAL) * rc_clock_ratio));
        addSetRegister({BGT24ATR22_T_BOOT_RXCHAIN_REG_ADDR, reg_t_boot_rxchain.value});

        BGT24ATR22::BGT24ATR22_T_BOOT_BANDGAP_REG reg_t_boot_bandgap;
        reg_t_boot_bandgap.value = BGT24ATR22_T_BOOT_BANDGAP_REG_RST;
        reg_t_boot_bandgap.VAL = static_cast<uint16_t>(std::ceil(static_cast<float_t>(reg_t_boot_bandgap.VAL) * rc_clock_ratio));
        addSetRegister({BGT24ATR22_T_BOOT_BANDGAP_REG_ADDR, reg_t_boot_bandgap.value});

        // switch to RC clock
        reg_clk_conf.SYS_CLK_SEL = 1;
    }
    else
    {
        system_clock_Hz = reference_clock_Hz;
    }

    if (system_clock_divider > 0)
    {
        BGT24ATR22::BGT24ATR22_I2C_CONF_REG reg_i2c_conf;
        reg_i2c_conf.value = BGT24ATR22_I2C_CONF_REG_RST;
        reg_i2c_conf.CLK_GATE = 1;
        addSetRegister({BGT24ATR22_I2C_CONF_REG_ADDR, reg_i2c_conf.value});
    }

    if (system_clock_Hz >= 40000000)
    {
        reg_clk_conf.I2C_CLK_DIV = 3;
    }
    else if ((system_clock_Hz >= 30000000) && (system_clock_Hz < 40000000))
    {
        reg_clk_conf.I2C_CLK_DIV = 2;
    }
    else if ((system_clock_Hz >= 20000000) && (system_clock_Hz < 30000000))
    {
        reg_clk_conf.I2C_CLK_DIV = 1;
    }
    else
    {
        reg_clk_conf.I2C_CLK_DIV = 0;
    }
    reg_clk_conf.SYS_CLK_DIV_FLEX = static_cast<uint16_t>(system_clock_div_flex);
    reg_clk_conf.SYS_CLK_TO_I2C = static_cast<uint16_t>(sys_clk_to_i2c);
    reg_clk_conf.SYS_CLK_DIV = system_clock_divider;
    addSetRegister({BGT24ATR22_CLK_CONF_REG_ADDR, reg_clk_conf.value});

    // configure hf on time
    BGT24ATR22::BGT24ATR22_RXABB_HF_ON_T_REG reg_rxabb_hf_on_t;
    auto clock_cycles = static_cast<uint64_t>(std::ceil(static_cast<float_t>(system_clock_Hz)
                                                        * (static_cast<float_t>(hf_on_time_usec) / 1e6)));
    reg_rxabb_hf_on_t.value = encodeExpMulValue16(clock_cycles, 4, 12);
    addSetRegister({BGT24ATR22_RXABB_HF_ON_T_REG_ADDR, reg_rxabb_hf_on_t.value});

    return system_clock_Hz;
}

void DeviceMimoseRegisterConfigurator::addAFC_Frequency(uint64_t frequencyHz, uint16_t AFC_Duration)
{
    BGT24ATR22::BGT24ATR22_VCO_AFC_REF0_REG regRef0;
    BGT24ATR22::BGT24ATR22_VCO_AFC_REF1_REG regRef1;


    regRef0.value = regRef1.value = 0;

    float_t tCount;
    const auto extOscFreqMHz = EXT_OSC_FREQ_HZ * 1e-6f;
    if (m_chipVersion == CHIP_VERSION_A1)
    {
        tCount = 1.f * (AFC_Duration + 1) / extOscFreqMHz;
    }
    else
    {
        tCount = 1.f * AFC_Duration / extOscFreqMHz;
    }
    auto frequencyMHz = 1.f * frequencyHz / 1e6f;
    auto AFC_RefCounter = uint32_t((frequencyMHz / 8.0 * tCount));
    if (AFC_RefCounter > 0xFFFFFF)
    {
        throw rdk::exception::argument_null();
    }
    auto AFC_RefCounterHigh = AFC_RefCounter >> 16;
    auto AFC_RefCounterLow = AFC_RefCounter & 0xFFFF;

    IFX_LOG_DEBUG("AFC FREQ config: duration %u Low %u High %u", AFC_Duration, (unsigned)AFC_RefCounterLow, (unsigned)AFC_RefCounterHigh);
    regRef0.LO_CNT_REF = uint16_t(AFC_RefCounterLow);
    regRef1.LO_CNT_REF = uint16_t(AFC_RefCounterHigh);
    addSetRegister({BGT24ATR22_VCO_AFC_REF0_REG_ADDR, regRef0.value});
    addSetRegister({BGT24ATR22_VCO_AFC_REF1_REG_ADDR, regRef1.value});
}

void DeviceMimoseRegisterConfigurator::getRegisters(uint32_t* registers)
{
    using MyNamedMemory = NamedMemory<address_t, value_t>;
    std::vector<MyNamedMemory::BatchType> regs;
    size_t size = 0;
    size = BGT24ATR22::BGT24ATR22_registers.size();
    regs.reserve(size);
    const auto* src = BGT24ATR22::BGT24ATR22_registers.data();
    copy(src, src + size, back_inserter(regs));

    std::vector<address_t> addresses(size);
    for (size_t i_reg = 0; i_reg < size; i_reg++)
    {
        addresses[i_reg] = regs[i_reg].address;
        value_t btValue = regs[i_reg].value;
        registers[i_reg] = static_cast<uint32_t>((addresses[i_reg] << 16) | (btValue & 0xFFFF));
    }
}

uint16_t DeviceMimoseRegisterConfigurator::getRegisterValue(uint16_t register_address)
{
    size_t size = BGT24ATR22::BGT24ATR22_registers.size();
    const auto* reg_list = BGT24ATR22::BGT24ATR22_registers.data();

    for (size_t i_reg = 0; i_reg < size; i_reg++)
    {
        if (static_cast<uint16_t>(reg_list[i_reg].address) == register_address)
        {
            return static_cast<uint16_t>(reg_list[i_reg].value);
        }
    }
    throw rdk::exception::index_out_of_bounds();
}

uint16_t DeviceMimoseRegisterConfigurator::encodeExpMulValue16(uint64_t value, uint8_t exp_bits, uint8_t mul_bits)
{
    if ((mul_bits + exp_bits) > 16)
    {
        throw rdk::exception::argument_out_of_bounds();
    }

    uint16_t exp_max = (1 << exp_bits) - 1;
    uint64_t mul_mask = (1 << mul_bits) - 1;

    uint16_t exp_val = 0;
    uint64_t temp_val = value;

    uint64_t mul_val = temp_val & mul_mask;

    while (mul_val != temp_val)
    {
        temp_val = (temp_val + 1) >> 1;
        mul_val = temp_val & mul_mask;
        exp_val = exp_val + 1;
        if (exp_val > exp_max)
        {
            throw rdk::exception::not_possible();
        }
    }
    return static_cast<uint16_t>((mul_val << exp_bits) + exp_val);
}

uint16_t DeviceMimoseRegisterConfigurator::readRegisterValue(uint16_t register_address)
{
    size_t size = BGT24ATR22::BGT24ATR22_registers.size();
    auto* reg_list = BGT24ATR22::BGT24ATR22_registers.data();
    for (size_t i_reg = 0; i_reg < size; i_reg++)
    {
        if (static_cast<uint16_t>(reg_list[i_reg].address) == register_address)
        {
            reg_list[i_reg].value = getRegisterValue_device(register_address);
            return static_cast<uint16_t>(reg_list[i_reg].value);
        }
    }
    throw rdk::exception::index_out_of_bounds();
}

void DeviceMimoseRegisterConfigurator::addRegisters(
    BatchType const* array,
    size_t size)
{
    while (size--)
    {
        addSetRegister(*array);
        array++;
    }
}

void DeviceMimoseRegisterConfigurator::addSetRegister(const BatchType& reg)
{
    m_registerQueue.push_back(reg);
}

void DeviceMimoseRegisterConfigurator::update_reglist()
{
    auto* regs_list = BGT24ATR22::BGT24ATR22_registers.data();
    size_t size = BGT24ATR22::BGT24ATR22_registers.size();
    auto* src = m_registerQueue.data();

    for (uint32_t i = 0; i < m_registerQueue.size(); i++)
    {
        bool address_found = false;
        for (uint32_t j = 0; j < size; j++)
        {
            if (static_cast<uint16_t>(regs_list[j].address) == static_cast<uint16_t>(src[i].address))
            {
                regs_list[j].value = src[i].value;
                address_found = true;
                break;
            }
        }
        if (!address_found)
        {
            throw rdk::exception::index_out_of_bounds();
        }
    }
}

void DeviceMimoseRegisterConfigurator::flushEnqRegisters()
{
    update_reglist();
    m_registers->writeBatch(m_registerQueue.data(), static_cast<address_t>(m_registerQueue.size()));
    m_registerQueue.clear();
}

DeviceMimoseRegisterConfigurator::value_t DeviceMimoseRegisterConfigurator::getRegisterValue_device(uint16_t addr) const
{
    return m_registers->read(addr);
}

/* ========================
   HARDCODED INFO
   ========================
*/


float_t DeviceMimoseRegisterConfigurator::calcAFC_additionalFrameTime(uint16_t AFC_duration)
{
    // TODO (@benhmidaines): Update VCO single pulse bootup time, and pre frequency measurement AFC delay as they are dependant on the sys clk
    const auto singlePulseBootupTime = 100e-6f;      // This is default bootup time for VCO, from registers (vco_bootup)
    const auto preFreqMeasuementAFCDelay = 200e-9f;  // T_AFC is fixed to 0x0009

    BGT24ATR22::BGT24ATR22_VCO_AFC_DURATION_REG reg_VCO_AFC_DURATION;
    reg_VCO_AFC_DURATION.value = BGT24ATR22_VCO_AFC_DURATION_REG_RST;
    BGT24ATR22::BGT24ATR22_VCO_AFC_CONF_REG reg_VCO_AFC_CONF;
    reg_VCO_AFC_CONF.value = BGT24ATR22_VCO_AFC_CONF_REG_RST;
    BGT24ATR22::BGT24ATR22_T_BOOT_BANDGAP_REG reg_T_BOOT_BANDGAP_REG;
    reg_T_BOOT_BANDGAP_REG.value = BGT24ATR22_T_BOOT_BANDGAP_REG_RST;

    float_t afc_t_afc_time_sec = AFC_duration * (1.0f / EXT_OSC_FREQ_HZ);

    auto afc_conf_lo_cnt_delay_time_reg = reg_VCO_AFC_CONF.LO_CNT_DEL;
    float_t afc_conf_lo_cnt_delay_time_sec = 0;
    switch (afc_conf_lo_cnt_delay_time_reg)
    {
        case 0:
            afc_conf_lo_cnt_delay_time_sec = 1 * (1.0f / EXT_OSC_FREQ_HZ);
            break;
        case 1:
            afc_conf_lo_cnt_delay_time_sec = 2 * (1.0f / EXT_OSC_FREQ_HZ);
            break;
        case 2:
        case 3:
            afc_conf_lo_cnt_delay_time_sec = 4 * (1.0f / EXT_OSC_FREQ_HZ);
            break;
        default:
            throw rdk::exception::not_supported();
    }

    float_t afc_counting_time_sec = reg_VCO_AFC_DURATION.VAL * (1.0f / EXT_OSC_FREQ_HZ);
    float_t afc_time_sec = afc_counting_time_sec + afc_t_afc_time_sec + 2 * afc_conf_lo_cnt_delay_time_sec;


    IFX_LOG_DEBUG("DeviceMimoseRegisterConfigurator::calcAFC_additionalFrameTime afc time %f", afc_time_sec);
    float_t boot_bandgap_delay_sec = reg_T_BOOT_BANDGAP_REG.VAL * (1.0f / EXT_OSC_FREQ_HZ);
    afc_time_sec += boot_bandgap_delay_sec;

    IFX_LOG_DEBUG("DeviceMimoseRegisterConfigurator::calcAFC_additionalFrameTime afc time with boot %f", afc_time_sec);
    return afc_time_sec + singlePulseBootupTime + preFreqMeasuementAFCDelay;
}

float_t DeviceMimoseRegisterConfigurator::calcPulseDurationTime(uint16_t reg_val_RXABB_HF_ON_T)
{
    BGT24ATR22::BGT24ATR22_RXABB_HF_ON_T_REG reg_RXABB_HF_ON_T;
    reg_RXABB_HF_ON_T.value = reg_val_RXABB_HF_ON_T;
    return helperCalcTimeSecFromReg(reg_RXABB_HF_ON_T);
}


uint16_t DeviceMimoseRegisterConfigurator::calcI2C_ClockDiv(uint64_t clockHz)
{
    uint8_t i2c_clock_div = 0;
    if (clockHz >= 40'000'000)
    {
        i2c_clock_div = 3;
    }
    else if (30'000'000 < clockHz && clockHz <= 40'000'000)
    {
        i2c_clock_div = 2;
    }
    else if (20'000'000 < clockHz && clockHz <= 30'000'000)
    {
        i2c_clock_div = 1;
    }
    return i2c_clock_div;
}

void DeviceMimoseRegisterConfigurator::addXTAL_DutyCycle(bool enable)
{
    BGT24ATR22::BGT24ATR22_XOSC_CLK_CONF_REG reg;
    reg.value = BGT24ATR22_XOSC_CLK_CONF_REG_RST;
    if (enable)
    {
        reg.XOSC_DC_DIS = BGT24ATR22::BGT24ATR22_XOSC_CLK_CONF_XOSC_DC_DIS_ACTIVATE_XTAL_DUTY_CYCLING;
    }
    else
    {
        reg.XOSC_DC_DIS = BGT24ATR22::BGT24ATR22_XOSC_CLK_CONF_XOSC_DC_DIS_DEACTIVATE_XTAL_DUTY_CYCLING_ALWAYS_ON;
    }
    addSetRegister({BGT24ATR22_XOSC_CLK_CONF_REG_ADDR, reg.value});
}

void DeviceMimoseRegisterConfigurator::addOscilatorSourceRegister(ifx_Mimose_Clock_Config_t clock_config)
{
    BGT24ATR22::BGT24ATR22_CLK_CONF_REG reg;
    reg.value = BGT24ATR22_CLK_CONF_REG_RST;
    if (clock_config.rc_clock_enabled)
    {
        reg.REF_CLK_SEL = BGT24ATR22::BGT24ATR22_CLK_CONF_SYS_CLK_SEL_INTERNAL_RC_OSCILLATOR_CLOCK;
        reg.I2C_CLK_DIV = calcI2C_ClockDiv(INT_OSC_FREQ_HZ);
    }
    else
    {
        reg.REF_CLK_SEL = BGT24ATR22::BGT24ATR22_CLK_CONF_SYS_CLK_SEL_REFERENCE_CLOCK;
        reg.I2C_CLK_DIV = calcI2C_ClockDiv(EXT_OSC_FREQ_HZ);
    }

    addSetRegister({BGT24ATR22_CLK_CONF_REG_ADDR, reg.value});
}

enum DeviceMimoseRegisterConfigurator::ChipVersion DeviceMimoseRegisterConfigurator::readoutChipVersion()
{
    value_t btValue = getRegisterValue_device(BGT24ATR22_CHIP_TYPE_REG_ADDR);
    switch (btValue)
    {
        case CHIP_VERSION_A1:
        case CHIP_VERSION_A2:
        case CHIP_VERSION_B1:
            return static_cast<DeviceMimoseRegisterConfigurator::ChipVersion>(btValue);
        default:
            return CHIP_VERSION_UNKNOWN;
    }
}
