/**
 * @file SequenceParameters.cpp
 */
/* ===========================================================================
** Copyright (C) 2017-2023 Infineon Technologies AG
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
#include "SequenceParameters.hpp"
#include "../Driver/registers_BGT120TR24E.h"
#include "../Driver/registers_BGT60TR11D.h"
#include "../Driver/registers_BGT60TRxxC.h"
#include "../Driver/registers_BGT60TRxxD.h"
#include "../Driver/registers_BGT60TRxxE.h"
#include "ifxAvian_DeviceTraits.hpp"
#include "ifxAvian_RegisterSet.hpp"
#include "ifxAvian_Types.hpp"

// ---------------------------------------------------------------------------- namespaces
namespace Infineon {
namespace Avian {
namespace TimingModel {

// ---------------------------------------------------------------------------- calculateTimerPeriod
static int64_t calculateTimerPeriod(uint32_t uCounter, uint32_t uMultiplier)
{
    if (uCounter > 0)
        return (int64_t(uCounter) << uMultiplier) * 8 + uMultiplier + 3;
    else
        return 1;
}

// ---------------------------------------------------------------------------- SequenceParameters
SequenceParameters::SequenceParameters(const HW::RegisterSet& registers,
                                       Device_Type device_type,
                                       double ref_frequency) :
    uFifoThreshold(0),
    uWakeUpTime(0),
    uInit0Time(0),
    uInit1Time(0),
    uAdcDivider(0),
    uPreChirpDelay(0),
    uPostChirpDelay(0),
    uPaDelay(0),
    uAdcDelay(0),
    uNumShapeGroupsPerFrame(0),
    uFrameEndDelay(0),
    eFrameEndPowerMode(PowerMode::Deep_Sleep),
    sShape {{{0.0, 0.0, 0, 0, 0, 0}, {0.0, 0.0, 0, 0, 0, 0}, 0, 0, PowerMode::Deep_Sleep_Continue, true},
            {{0.0, 0.0, 0, 0, 0, 0}, {0.0, 0.0, 0, 0, 0, 0}, 0, 0, PowerMode::Deep_Sleep_Continue, true},
            {{0.0, 0.0, 0, 0, 0, 0}, {0.0, 0.0, 0, 0, 0, 0}, 0, 0, PowerMode::Deep_Sleep_Continue, true},
            {{0.0, 0.0, 0, 0, 0, 0}, {0.0, 0.0, 0, 0, 0, 0}, 0, 0, PowerMode::Deep_Sleep_Continue, true}},
    uFifoSize(Device_Traits::get(device_type).fifo_size * 2),
    bATR24C(device_type == Device_Type::BGT60ATR24C)
{
    auto& sDeviceTraits = Device_Traits::get(device_type);

    // extract sequence parameters from register dump
    // ----------------------------------------------
    auto getPowerMode = [](uint32_t uMode, uint32_t uContinue) -> PowerMode {
        switch (uMode)
        {
            case 0: return PowerMode::Interchirp;
            case 1: return PowerMode::Idle;
            case 2:
            default: return (uContinue != 0) ? PowerMode::Deep_Sleep_Continue
                                             : PowerMode::Deep_Sleep;
        };
    };

    // extract relevant parameters from PACR2
    uint32_t uRegPACR2 = registers[BGT60TRxxC_REG_PACR2];
    uint8_t uPllDIVSET = uint16_t(BGT60TRxxC_EXTRACT(PACR2, DIVSET, uRegPACR2));

    uint32_t uFastDownMode = BGT60TRxxC_EXTRACT(PACR2, FSTDNEN, uRegPACR2);
    uFastDownTime = uint16_t(BGT60TRxxC_EXTRACT(PACR2, FSDNTMR, uRegPACR2));
    if (uFastDownMode != 0)
    {
        // set default values hard coded in PLL
        if (uFastDownTime == 0)
        {
            uFastDownTime = (uFastDownMode == 1) ? 40 : (uFastDownMode == 2) ? 56
                                                    : (uFastDownMode == 3)   ? 24
                                                                             : 0;
        }
        else
            uFastDownTime += 1;
    }

    // extract relevant parameters from MAIN
    uint32_t uRegMAIN = registers[BGT60TRxxC_REG_MAIN];
    if (sDeviceTraits.wu_register_type == Device_Traits::Wu_Register_Type::None)
    {
        uWakeUpTime = uint32_t(calculateTimerPeriod(BGT60TRxxC_EXTRACT(MAIN, TWKUP, uRegMAIN),
                                                    BGT60TRxxC_EXTRACT(MAIN, TWKUP_MUL, uRegMAIN)));
    }
    else
    {
        uint32_t uRegWU = registers[BGT60TR11D_REG_WU];
        if (sDeviceTraits.wu_register_type == Device_Traits::Wu_Register_Type::T_WU_Only)
        {
            uWakeUpTime = uint32_t(calculateTimerPeriod(BGT60TR11D_EXTRACT(WU, TR_TWKUP, uRegWU),
                                                        BGT60TR11D_EXTRACT(WU, TR_TWKUP_MUL, uRegWU)));
        }
        else
        {
            uWakeUpTime = uint32_t(calculateTimerPeriod(BGT120TR24E_EXTRACT(WU, TR_TWKUP, uRegWU),
                                                        BGT60TR11D_EXTRACT(WU, TR_TWKUP_MUL, uRegWU)));
        }
    }

    // extract relevant parameters from ADC0
    uint32_t uRegADC0 = registers[BGT60TRxxC_REG_ADC0];
    uAdcDivider = uint16_t(BGT60TRxxC_EXTRACT(ADC0, ADC_DIV, uRegADC0));

    // extract relevant parameters from SFCTL
    uint32_t uRegSFCTL = registers[BGT60TRxxC_REG_SFCTL];
    uFifoThreshold = uint16_t(BGT60TRxxC_EXTRACT(SFCTL, FIFO_CREF, uRegSFCTL) * 2 + 2);

    // extract relevant parameters from CCR0
    uint32_t uRegCCR0 = registers[BGT60TRxxC_REG_CCR0];
    uInit1Time = uint16_t(calculateTimerPeriod(BGT60TRxxC_EXTRACT(CCR0, TR_INIT1, uRegCCR0),
                                               BGT60TRxxC_EXTRACT(CCR0, TR_MUL1, uRegCCR0)));
    uPostChirpDelay = BGT60TRxxC_EXTRACT(CCR0, TMREND, uRegCCR0) * 8
                      + 5 /* Extra cycles from PLL state machine */;

    // extract relevant parameters from CCR1
    uint32_t uRegCCR1 = registers[BGT60TRxxC_REG_CCR1];
    uFrameEndDelay = calculateTimerPeriod(BGT60TRxxC_EXTRACT(CCR1, TFED, uRegCCR1),
                                          BGT60TRxxC_EXTRACT(CCR1, TFED_MUL, uRegCCR1));
    eFrameEndPowerMode = getPowerMode(BGT60TRxxC_EXTRACT(CCR1, PD_MODE, uRegCCR1),
                                      BGT60TRxxC_EXTRACT(CCR0, CONT_MODE, uRegCCR0));
    uPreChirpDelay = BGT60TRxxC_EXTRACT(CCR1, TMRSTRT, uRegCCR1) * 8 + 7 /* syncing between FSM and PLL */
                     + 3 /* Extra cycles from PLL state machine */;

    // extract relevant parameters from CCR2
    uint32_t uRegCCR2 = registers[BGT60TRxxC_REG_CCR2];
    uNumShapeGroupsPerFrame = uint16_t(BGT60TRxxC_EXTRACT(CCR2, FRAME_LEN, uRegCCR2) + 1);

    // extract relevant parameters from CCR3
    uint32_t uRegCCR3 = registers[BGT60TRxxC_REG_CCR3];
    uInit0Time = uint16_t(calculateTimerPeriod(BGT60TRxxC_EXTRACT(CCR3, TR_INIT0, uRegCCR3),
                                               BGT60TRxxC_EXTRACT(CCR3, TR_MUL0, uRegCCR3)));
    uAdcDelay = BGT60TRxxC_EXTRACT(CCR3, T_SSTRT, uRegCCR3) * 8 + 1;
    uPaDelay = BGT60TRxxC_EXTRACT(CCR3, T_PAEN, uRegCCR3) * 8;

    // calculate frequency range
    auto convertFrequency = [ref_frequency, uPllDIVSET, &sDeviceTraits](uint32_t uRegValue, bool bAbsolute) -> double {
        int32_t iSignedValue = (uRegValue < (1 << 23)) ? uRegValue : int32_t(uRegValue) - (1 << 24);

        double dFrequency = double(iSignedValue) / double(1 << 20);
        if (bAbsolute)
            dFrequency += (uPllDIVSET + 2) * 4 + 8;

        return ref_frequency * sDeviceTraits.pll_pre_divider * dFrequency;
    };

    dPllMinFrequency = convertFrequency(0x00800000, true);
    dPllMaxFrequency = convertFrequency(0x007FFFFF, true);

    // now extract shape parameters
    /*
     * If not all shapes are used the according registers may not be programmed. The RegisterSet
     * class throws an exception if not programmed registers are executed. In this case, it's OK if
     * this happens, so the try/catch block makes sure that the extraction continues after the loop.
     */
    try
    {
        for (uint8_t i = 0; i < 4; ++i)
        {
            const uint32_t uRegPLL0 = registers[BGT60TRxxC_REG_PLL1_0 + 8 * i];
            const uint32_t uRegPLL1 = registers[BGT60TRxxC_REG_PLL1_1 + 8 * i];
            const uint32_t uRegPLL2 = registers[BGT60TRxxC_REG_PLL1_2 + 8 * i];
            const uint32_t uRegPLL3 = registers[BGT60TRxxC_REG_PLL1_3 + 8 * i];
            const uint32_t uRegPLL4 = registers[BGT60TRxxC_REG_PLL1_4 + 8 * i];
            const uint32_t uRegPLL5 = registers[BGT60TRxxC_REG_PLL1_5 + 8 * i];
            const uint32_t uRegPLL6 = registers[BGT60TRxxC_REG_PLL1_6 + 8 * i];
            const uint32_t uRegPLL7 = registers[BGT60TRxxC_REG_PLL1_7 + 8 * i];

            sShape[i].sUp.uRampTime = BGT60TRxxC_EXTRACT(PLL1_2, RTU, uRegPLL2) * 8;
            sShape[i].sUp.uNumSamples = BGT60TRxxC_EXTRACT(PLL1_3, APU, uRegPLL3);
            sShape[i].sUp.uChirpEndDelay = BGT60TRxxC_EXTRACT(PLL1_2, T_EDU, uRegPLL2) * 8;
            sShape[i].sUp.uChirpEndDelay += (sShape[i].sUp.uChirpEndDelay != 0) ? 5 : 2; /* two cycles are for syncing between PLL and FSM */
            sShape[i].sUp.dStartFrequency = convertFrequency(BGT60TRxxC_EXTRACT(PLL1_0, FSU, uRegPLL0), true);
            sShape[i].sUp.dFrequencyInc = convertFrequency(BGT60TRxxC_EXTRACT(PLL1_1, RSU, uRegPLL1), false);

            sShape[i].sDown.uRampTime = BGT60TRxxC_EXTRACT(PLL1_6, RTD, uRegPLL6) * 8;
            sShape[i].sDown.uNumSamples = uint16_t(BGT60TRxxC_EXTRACT(PLL1_3, APD, uRegPLL3));
            sShape[i].sDown.uChirpEndDelay = BGT60TRxxC_EXTRACT(PLL1_6, T_EDD, uRegPLL6) * 8;
            sShape[i].sDown.uChirpEndDelay += (sShape[i].sDown.uChirpEndDelay != 0) ? 5 : 2; /* two cycles are for syncing between PLL and FSM */
            sShape[i].sDown.dStartFrequency = convertFrequency(BGT60TRxxC_EXTRACT(PLL1_4, FSD, uRegPLL4), true);
            sShape[i].sDown.dFrequencyInc = convertFrequency(BGT60TRxxC_EXTRACT(PLL1_5, RSD, uRegPLL5), false);

            sShape[i].bFastDownRamp = (BGT60TRxxC_EXTRACT(PLL1_4, FSD, uRegPLL4) == 0)
                                      && (BGT60TRxxC_EXTRACT(PLL1_5, RSD, uRegPLL5) == 0)
                                      && (BGT60TRxxC_EXTRACT(PLL1_6, RTD, uRegPLL6) == 0);

            sShape[i].uShapeEndDelay = calculateTimerPeriod(BGT60TRxxC_EXTRACT(PLL1_7, T_SED, uRegPLL7),
                                                            BGT60TRxxC_EXTRACT(PLL1_7, T_SED_MUL, uRegPLL7));
            sShape[i].eShapeEndPowerMode = getPowerMode(BGT60TRxxC_EXTRACT(PLL1_7, PD_MODE, uRegPLL7),
                                                        BGT60TRxxC_EXTRACT(PLL1_7, CONT_MODE, uRegPLL7));
            sShape[i].uNumRepetitions = BGT60TRxxC_EXTRACT(PLL1_7, SH_EN, uRegPLL7)
                                        * (1 << BGT60TRxxC_EXTRACT(PLL1_7, REPS, uRegPLL7));

            auto countAntennas = [](uint32_t uRegValue) -> uint8_t {
                uint8_t uRxMask = uint8_t(BGT60TRxxC_EXTRACT(CS1_U_1, BBCH_SEL, uRegValue));
                uint8_t uNumAntennas = 0;
                while (uRxMask != 0)
                {
                    uNumAntennas += uRxMask & 1;
                    uRxMask >>= 1;
                }
                return uNumAntennas;
            };

            sShape[i].sUp.uNumAntennas = countAntennas(registers[BGT60TRxxC_REG_CS1_U_1 + 7 * i]);
            if (!sShape[i].bFastDownRamp)
                sShape[i].sDown.uNumAntennas = countAntennas(registers[BGT60TRxxC_REG_CS1_D_1 + 7 * i]);
        }
    }
    catch (...)
    {}
}

// ---------------------------------------------------------------------------- SequenceParametersD
SequenceParametersD::SequenceParametersD(const HW::RegisterSet& registers,
                                         Device_Type device_type,
                                         double ref_frequency) :
    SequenceParameters(registers, device_type, ref_frequency),
    uStartupDelayPllDivider(0),
    uStartupDelayBandgap(0),
    uStartupDelayMadc(0),
    uStartupDelayPllEnable(0),
    bRefDoublerActive(false)
{
    /*
     * The register layout of CCR3 has changed form BGT60TRxxC to BGT60TRxxD.
     * Because of that wrong values have been extracted above for INIT0 period
     * and ADC delay. Extract those timings again, but this time with the proper
     * register layout macros for BGT60TRxxD.
     */
    uint32_t uRegCCR3 = registers[BGT60TRxxC_REG_CCR3];
    uInit0Time = uint16_t(calculateTimerPeriod(BGT60TRxxD_EXTRACT(CCR3, TR_INIT0, uRegCCR3),
                                               BGT60TRxxD_EXTRACT(CCR3, TR_INIT0_MUL, uRegCCR3)));
    uAdcDelay = BGT60TRxxD_EXTRACT(CCR3, TR_SSTART, uRegCCR3) * 8 + 1;

    /*
     * BGT60TRxxD introduces some new parameters for the startup sequence.
     * These are extracted below.
     */
    uint32_t uRegPACR2 = registers[BGT60TRxxC_REG_PACR2];
    uint32_t uRegCSCI = registers[BGT60TRxxD_REG_CSCI];

    uStartupDelayBandgap = uint16_t(64 * BGT60TRxxD_EXTRACT(CSCI, TR_BGEN, uRegCSCI));
    uStartupDelayBandgap += (uStartupDelayBandgap != 0) ? 2 : 1;

    uStartupDelayMadc = uint16_t(64 * BGT60TRxxD_EXTRACT(CSCI, TR_MADCEN, uRegCSCI));
    uStartupDelayMadc += (uStartupDelayMadc != 0) ? 1 : 0;

    uStartupDelayPllEnable = uint16_t(64 * BGT60TRxxD_EXTRACT(CSCI, TR_PLL_ISOPD, uRegCSCI));
    uStartupDelayPllEnable += (uStartupDelayPllEnable != 0) ? 2 : 1;

    uStartupDelayPllDivider = uint8_t(32 * BGT60TRxxD_EXTRACT(PACR2, TR_DIVEN, uRegPACR2) + 1);

    /*
     * BGT60TRxxD introduces a frequency doubler that doubles the external
     * reference frequency.
     */
    auto& sDeviceTraits = Device_Traits::get(device_type);
    if (sDeviceTraits.has_reordered_register_layout)
    {
        uint32_t uRegCLK_IN = registers[BGT60TRxxE_REG_FD];
        bRefDoublerActive = BGT60TRxxE_EXTRACT(FD, CLK_SEL, uRegCLK_IN) != 0;
    }
    else
    {
        uint32_t uRegCLK_IN = registers[BGT60TRxxD_REG_FD];
        bRefDoublerActive = BGT60TRxxD_EXTRACT(FD, CLK_SEL, uRegCLK_IN) != 0;
    }
}

// ---------------------------------------------------------------------------- SequenceParameters11D
SequenceParameters11D::SequenceParameters11D(const HW::RegisterSet& registers,
                                             Device_Type device_type,
                                             double ref_frequency) :
    SequenceParametersD(registers, device_type, ref_frequency),
    uMadcSwitchTime(0),
    uPowerSensDelay(0),
    uMadcAcquisitionTime(0),
    bPowerSensEnabled {false, false, false, false},
    bTemperatureSensEnabled {false, false, false, false}
{
    // BGT60TR11D introduces another timing parameter for power sensing delay.
    uint32_t uRegADC1 = registers[BGT60TR11D_REG_ADC1];
    uPowerSensDelay = uint16_t(8 * BGT60TR11D_EXTRACT(ADC1, TR_PSSTART, uRegADC1) + 2);

    // temperature sensing and power sensing must be checked for each shape ...
    /*
     * If not all shapes are used the according registers may not be programmed. The RegisterSet
     * class throws an exception if not programmed registers are executed. In this case, it's OK if
     * this happens, so the try/catch block makes sure that the extraction continues after the loop.
     */
    try
    {
        for (uint8_t shp = 0; shp < 4; ++shp)
        {
            uint8_t num_pll_regs = BGT60TR11D_REG_PLL2_0 - BGT60TR11D_REG_PLL1_0;
            uint8_t num_cs_regs = BGT60TR11D_REG_CS2_U_0 - BGT60TR11D_REG_CS1_U_0;

            // if shape is disabled, sensing for this and following shapes for sure is off
            if (BGT60TR11D_EXTRACT(PLL1_7, SH_EN, registers[BGT60TR11D_REG_PLL1_7 + num_pll_regs * shp]) == 0)
                break;

            // Power sensing is enabled according to chirp power settings
            if (BGT60TR11D_EXTRACT(CS1_U_0, PD1_EN, registers[BGT60TR11D_REG_CS1_U_0 + num_cs_regs * shp]) != 0)
                bPowerSensEnabled[shp] = true;

            if ((BGT60TR11D_EXTRACT(PLL1_5, RSD, registers[BGT60TR11D_REG_PLL1_5 + num_pll_regs * shp]) == 0)
                && (BGT60TR11D_EXTRACT(PLL1_6, RTD, registers[BGT60TR11D_REG_PLL1_6 + num_pll_regs * shp]) == 0))
            {
                // saw tooth mode -> consider up-chirp for temperature sensing
                if (BGT60TR11D_EXTRACT(CS1_U_0, TEMP_MEAS_EN, registers[BGT60TR11D_REG_CS1_U_0 + num_cs_regs * shp]) != 0)
                    bTemperatureSensEnabled[shp] = true;
            }
            else
            {
                // triangle mode -> consider down-chirp for temperature sensing
                if (BGT60TR11D_EXTRACT(CS1_D_0, TEMP_MEAS_EN,
                                       registers[BGT60TR11D_REG_CS1_D_0 + num_cs_regs * shp])
                    != 0)
                    bTemperatureSensEnabled[shp] = true;
            }
        }
    }
    catch (...)
    {}

    /*
     * Here the time for a single MADC acquisition (sample and convert) is
     * calculated from ADC0 register.
     */
    uint32_t uRegADC0 = registers[BGT60TRxxC_REG_ADC0];
    uMadcAcquisitionTime = 0;

    switch (BGT60TR11D_EXTRACT(ADC0, STC, uRegADC0))
    {
        case 0: uMadcAcquisitionTime += 4; break;
        case 1: uMadcAcquisitionTime += 8; break;
        case 2: uMadcAcquisitionTime += 16; break;
        case 3: uMadcAcquisitionTime += 32; break;
    }

    switch (BGT60TR11D_EXTRACT(ADC0, TRACK_CFG, uRegADC0))
    {
        case 0: uMadcAcquisitionTime += 0; break;
        case 1: uMadcAcquisitionTime += 8; break;
        case 2: uMadcAcquisitionTime += 24; break;
        case 3: uMadcAcquisitionTime += 56; break;
    }

    uMadcAcquisitionTime += BGT60TR11D_EXTRACT(ADC0, MSB_CTRL, uRegADC0) ? 17 : 16;

    uMadcAcquisitionTime <<= BGT60TR11D_EXTRACT(ADC0, ADC_OVERS_CFG, uRegADC0);

    /*
     * The time to switch MADC input channel was found during hardware
     * simulation. It's 1 micro second.
     */
    uMadcSwitchTime = uint16_t(1.0e-6 * ref_frequency + 0.5);
}

/* ------------------------------------------------------------------------ */
}  // namespace TimingModel
}  // namespace Avian
}  // namespace Infineon

/* --- End of File -------------------------------------------------------- */
