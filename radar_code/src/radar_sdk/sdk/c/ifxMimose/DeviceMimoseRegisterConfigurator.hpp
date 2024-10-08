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


/**
 * @file DeviceMimoseRegisterConfigurator.hpp
 */

#pragma once

#include "DeviceMimoseTypes.h"

#include <components/interfaces/IProtocolAtr22.hpp>
#include <components/interfaces/IRegisters.hpp>

#include <array>
#include <vector>

class DeviceMimoseRegisterConfigurator
{
public:
    enum ChipVersion : uint16_t
    {
        CHIP_VERSION_A1 = 0x0000,
        CHIP_VERSION_A2 = 0x0001,
        CHIP_VERSION_B1 = 0x8002,
        CHIP_VERSION_UNKNOWN = 0xFFFF
    };

    using address_t = uint16_t;
    using value_t = uint16_t;
    using BatchType = typename IMemory<address_t, value_t>::BatchType;

    DeviceMimoseRegisterConfigurator(IRegisters<address_t, value_t>* registers);

    void addEnableTrigger2();
    void resetClockConf();
    void addEnableStatusTriggerSignal(uint16_t out_trigger);
    void addEnableDRDPOnlyAcquisition();
    void addEnableTriggerAndDRDPAcquisition(uint16_t triggerCount);
    void addSeqExecute(bool enable, size_t frame_configuration_index);
    void addMemOffsets(uint16_t num_samples, uint16_t num_active_pc, bool use_mem_trg_mem_sens);
    void addAOC_ThStep(uint16_t gain_index);
    void addInterrupt(bool targetDetect, bool sequencerError);
    void addFramePeriod(size_t frame_configuration_index, uint64_t frame_period_clock_cycles);
    void addNumberOfSamples(size_t frame_configuration_index, uint16_t num_samples);
    void addSetPreTrigger(size_t frame_configuration_index, uint16_t num_samples_per_frame, uint16_t active_pc_count, uint32_t pre_trigger_count);
    void addPulseRepetitionTime(size_t pulse_index, size_t frame_configuration_index, uint64_t pulse_repetition_time_per_pc_cycles);
    void addRFChannel(size_t pulse_index, ifx_Mimose_Channel_t channel, int tx_power_level);
    void addRegisterABB_gain(size_t pulse_index, bool agc, int gain_index);
    void addRegisterAOC(size_t pulse_index, ifx_Mimose_AOC_Mode_t aoc_mode);
    void addFFT_Len(size_t pulse, size_t len);
    void addHardcodedConfig();
    void addHardcodedTrigger();
    void addNoHeatingPulses(size_t frame_configuration_index, size_t noHeatingPulses);
    void addRCTbootRefClk(uint64_t t_boot_ref_clk_cycles);
    void addAFC_Duration(uint16_t AFC_DurationCT);
    void addAFC_Threshold(uint16_t thresholdFine, uint16_t thresholdCourse);
    void addAFC_RepetitionPeriod(ifx_Mimose_AFC_Repeat_Count_t repetition);
    void addAFC_Frequency(uint64_t frequencyHz, uint16_t AFC_Duration);
    void addOscilatorSourceRegister(ifx_Mimose_Clock_Config_t clock_config);
    uint32_t getSystemClockRC(uint32_t reference_clock_Hz, uint32_t desired_sys_clock_Hz);
    void updateTrimLutRC(uint32_t reference_clock_Hz);
    uint32_t addClockConfigRegisters(uint32_t reference_clock_Hz,
                                     uint32_t desired_sys_clock_Hz,
                                     bool rc_clock_enabled,
                                     uint32_t hf_on_time_usec,
                                     uint16_t system_clock_divider,
                                     bool system_clock_div_flex,
                                     bool sys_clk_to_i2c);
    static float calcAFC_additionalFrameTime(uint16_t AFC_duration);
    static uint16_t calcAFCConfLoCnt_DelayTime(uint16_t reg_val_BGT24ATR22_VCO_AFC_CONF_REG);

    void getRegisters(uint32_t* registers);
    uint16_t readRegisterValue(uint16_t register_address);  // from hardware
    uint16_t getRegisterValue(uint16_t register_address);   // from SDK
    void reset();
    void reset_register_list();

    void addRegisters(
        BatchType const* array, size_t size);

    template <std::size_t SIZE>
    void addRegisters(
        const std::array<BatchType, SIZE>& regs)
    {
        addRegisters(regs.data(), SIZE);
    }


    void addSetRegister(const BatchType& reg);
    void update_reglist();

    void flushEnqRegisters();

    static float calcPulseDurationTime(uint16_t reg_val_RXABB_HF_ON_T);
    void applyFileConfig();


private:
    enum ChipVersion m_chipVersion;

    enum ChipVersion readoutChipVersion();
    void addXTAL_DutyCycle(bool enable);
    void addTX_DAC_OverwriteChannelC(bool enable);

    value_t getRegisterValue_device(uint16_t addr) const;

    static uint16_t encodeExpMulValue16(uint64_t value, uint8_t exp_bits, uint8_t mul_bits);
    static uint16_t calcI2C_ClockDiv(uint64_t clockHz);

    IRegisters<address_t, value_t>* m_registers;
    std::vector<BatchType> m_registerQueue;
    std::vector<float> m_rcTrimLut;
};
