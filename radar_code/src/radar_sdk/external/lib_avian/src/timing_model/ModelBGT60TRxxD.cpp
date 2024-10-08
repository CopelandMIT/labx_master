/**
 * @file ModelBGT60TRxxD.cpp
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
#include "ModelBGT60TRxxD.hpp"

// ---------------------------------------------------------------------------- namespaces
namespace Infineon {
namespace Avian {
namespace TimingModel {

//----------------------------------------------------------------------------- constants
const ModelBGT60TRxxC::PowerConsumptionTable ModelBGT60TRxxD::s_sPowerConsumptionTableBGT60UTR13D =
    {
        /* currentDeepSleep */ 173.0e-6,
        /* dPowerDeepSleepContinue */ 4.5e-3,
        /* dPowerDeepSleepContinueDoubler */ 5.04e-3,
        /* currentIdle */ 5.5e-3,
        /* currentInit0 */ 12.2e-3,
        /* currentInit1 */ 316.0e-3,
        /* currentActive */ 350.0e-3};

//----------------------------------------------------------------------------- ModelBGT60TRxxD
ModelBGT60TRxxD::ModelBGT60TRxxD(const SequenceParametersD& sParameters,
                                 bool bIgnoreRepetitions) :
    ModelBGT60TRxxC(sParameters, bIgnoreRepetitions,
                    s_sPowerConsumptionTableBGT60UTR13D),
    m_sParametersD(sParameters)
{}

//----------------------------------------------------------------------------- gotoStateWU
void ModelBGT60TRxxD::gotoStateWU(TimerList_t& aTimers)
{
    ModelBGT60TRxxC::gotoStateWU(aTimers);

    aTimers.emplace_back("Bandgap Delay", "T_BG_EN",
                         m_sParametersD.uStartupDelayBandgap);
}

//----------------------------------------------------------------------------- gotoStateINIT0
void ModelBGT60TRxxD::gotoStateINIT0(TimerList_t& aTimers)
{
    m_ePowerMode = PowerMode::Interchirp;
    m_eStateFsm = FsmState_e::Init0;

    aTimers.emplace_back("Init 0", "T_INIT0", m_sParameters.uInit0Time,
                         [this](TimerList_t& aTimers) -> void {
                             gotoStateINIT1(aTimers);
                         });

    aTimers.emplace_back("Madc Delay", "T_MADC_EN",
                         m_sParametersD.uStartupDelayMadc);
    aTimers.emplace_back("PLL Enable Delay", "T_PLL_ISOPD",
                         m_sParametersD.uStartupDelayPllEnable);
}

//----------------------------------------------------------------------------- gotoStateINIT1
void ModelBGT60TRxxD::gotoStateINIT1(TimerList_t& aTimers)
{
    ModelBGT60TRxxC::gotoStateINIT1(aTimers);

    aTimers.emplace_back("PLL Divider Delay", "T_DIVEN",
                         m_sParametersD.uStartupDelayPllDivider,
                         [this](TimerList_t& aTimers) -> void {
                             startPllLocking(aTimers);
                         });
}

/* ------------------------------------------------------------------------ */
}  // namespace TimingModel
}  // namespace Avian
}  // namespace Infineon

/* --- End of File -------------------------------------------------------- */
