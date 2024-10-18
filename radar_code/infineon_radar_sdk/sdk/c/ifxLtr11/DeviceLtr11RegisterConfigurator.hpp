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
 * @file DeviceLtr11RegisterConfigurator.hpp
 *
 * @brief Header for DeviceLtr11RegisterConfigurator
 */

#pragma once

#include "DeviceLtr11Types.h"
#include <components/interfaces/IRegisters.hpp>
#include <cstdint>
#include <tuple>
#include <vector>


class DeviceLtr11RegisterConfigurator
{
public:
    using address_t = uint8_t;
    using value_t = uint16_t;
    using BatchType = typename IMemory<address_t, value_t>::BatchType;


    IFX_DLL_TEST DeviceLtr11RegisterConfigurator(IRegisters<address_t, value_t>* m_registers);

    IFX_DLL_TEST void addSetPulseConfig(ifx_Ltr11_PRT_t prt, ifx_Ltr11_PulseWidth_t pulseWidth, ifx_Ltr11_TxPowerLevel_t powerLevel);
    ifx_Ltr11_PulseWidth_t getPulseWidth() const;
    ifx_Ltr11_PRT_t getPulseRepetitionTime() const;

    IFX_DLL_TEST void addSetHoldTime(ifx_Ltr11_Hold_Time_t holdTime);
    ifx_Ltr11_Hold_Time_t getHoldTime() const;

    value_t getInternalDetectorThreshold() const;

    ifx_Ltr11_TxPowerLevel_t getTxPowerLevel() const;

    void addSetRX_IF_Gain(ifx_Ltr11_RxIFGain_t rxIfGain);
    ifx_Ltr11_RxIFGain_t getRX_IF_Gain() const;

    IFX_DLL_TEST void addSetRFCenterFrequency(uint64_t freq_Hz);
    uint64_t getRFCenterFrequency() const;

    IFX_DLL_TEST void addSetDetectorThreshold_AdaptivPRT(value_t internalDetectorThreshold, ifx_Ltr11_APRT_Factor_t aprtFactor);
    ifx_Ltr11_APRT_Factor_t getAdaptivePulseRepetitionTime() const;

    void addSetMode(ifx_Ltr11_Mode_t mode);

    void disableInternalDetector();

    bool isJapanBand() const;

    void addSetDefaultRegistersList();

    void dumpRegisters(const char* filename) const;

    template <std::size_t SIZE>
    inline void addRegisters(const std::array<BatchType, SIZE>& regs)
    {
        addRegisters(regs.data(), SIZE);
    };
    void addRegisters(BatchType const* array, size_t size);

    void addSetRegister(const BatchType& reg);

    void flushEnqRegisters();

    inline const std::vector<BatchType>& getQueue()
    {
        return m_registerQueue;
    };

private:
    void resetRegisters();

    IRegisters<address_t, value_t>* m_registers;

    IFX_DLL_TEST static value_t convertRFCFreq_toRegisterValue(uint64_t freq_Hz);
    static uint64_t convertToRfCFreq(uint16_t freq_reg);

    friend uint16_t testGetRegRFFreq(uint64_t freq_Hz);

    value_t getRegisterValue(address_t addr) const;

    std::vector<BatchType> m_registerQueue;
};
