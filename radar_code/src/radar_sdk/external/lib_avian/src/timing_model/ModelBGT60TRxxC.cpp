/**
 * @file ModelBGT60TRxxC.cpp
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
#include "ModelBGT60TRxxC.hpp"
#include <algorithm>
#include <limits>
#include <sstream>

// ---------------------------------------------------------------------------- namespaces
namespace Infineon {
namespace Avian {
namespace TimingModel {

// ---------------------------------------------------------------------------- constants
const uint64_t ModelBGT60TRxxC::s_uLockTime = 320;  // 4 micro seconds @ 80MHz

const ModelBGT60TRxxC::PowerConsumptionTable ModelBGT60TRxxC::s_sPowerConsumptionTableBGT60TR13C =
    {
        /* currentDeepSleep */ 173.0e-6,
        /* dPowerDeepSleepContinue */ 4.5e-3,
        /* dPowerDeepSleepContinueDoubler */ 4.5e-3,  // C types don't have a doubler
        /* currentIdle */ 5.5e-3,
        /* currentInit0 */ 316.0e-3,
        /* currentInit1 */ 316.0e-3,
        /* currentActive */ 350.0e-3};

const ModelBGT60TRxxC::PowerConsumptionTable ModelBGT60TRxxC::s_sPowerConsumptionTableBGT60ATR24C =
    {
        /* currentDeepSleep */ 322.5e-6,
        /* dPowerDeepSleepContinue */ 4.5e-3,
        /* dPowerDeepSleepContinueDoubler */ 4.5e-3,  // C types don't have a doubler
        /* currentIdle */ 5.8e-3,
        /* currentInit0 */ 354.1e-3,
        /* currentInit1 */ 367.8e-3,
        /* currentActive */ 421.8e-3};

// ---------------------------------------------------------------------------- ModelBGT60TRxxC
ModelBGT60TRxxC::ModelBGT60TRxxC(const SequenceParameters& sParameters,
                                 bool bIgnoreRepetitions) :
    ModelBGT60TRxxC(sParameters, bIgnoreRepetitions,
                    sParameters.bATR24C ? s_sPowerConsumptionTableBGT60ATR24C
                                        : s_sPowerConsumptionTableBGT60TR13C)
{}

// ---------------------------------------------------------------------------- ModelBGT60TRxxC
ModelBGT60TRxxC::ModelBGT60TRxxC(const SequenceParameters& sParameters,
                                 bool bIgnoreRepetitions,
                                 const PowerConsumptionTable& sPowerConsumptionTable) :
    m_sParameters(sParameters),
    m_bIgnoreRepetitions(bIgnoreRepetitions),
    m_eStateFsm(FsmState_e::Ready),
    m_ePowerMode(PowerMode::Deep_Sleep),
    m_uShape(0),
    m_bDownChirp(false),
    m_uShapeRepetition(0),
    m_uShapeSetRepetition(0),
    m_uShapeGroupCounter(0),
    m_eStatePll(PllState_e::Off),
    m_dCurrentFrequency(0.0),
    m_sPowerConsumptionTable(sPowerConsumptionTable)
{
}

// ---------------------------------------------------------------------------- startFrame
void ModelBGT60TRxxC::startFrame()
{
    if (m_eStateFsm == FsmState_e::Ready)
        gotoStateWU(m_aTimers);
}

// ---------------------------------------------------------------------------- goToNextState
int64_t ModelBGT60TRxxC::goToNextState()
{
    // All currently running timers are checked for the one who expires next.
    uint64_t uNumCyclesTillNextState = std::numeric_limits<uint64_t>::max();
    for (const auto& sTimer : m_aTimers)
        uNumCyclesTillNextState = std::min(uNumCyclesTillNextState, sTimer.uCounter);

    /*
     * Now that the time until the next state transition is known all timers
     * are advanced and the PLL frequency is updated.
     */
    for (auto& sTimer : m_aTimers)
        sTimer.uCounter -= uNumCyclesTillNextState;

    m_dCurrentFrequency += uNumCyclesTillNextState * getFrequencyIncrement();

    /*
     * Finally the expired timers are removed from the list. Most of the timers
     * define a function to be executed when the timer expires. These function may
     * start new timers which are inserted at the position of the expired timer.
     */
    auto itTimer = m_aTimers.begin();
    while (itTimer != m_aTimers.end())
    {
        auto itTimerToCheck = itTimer++;
        if (itTimerToCheck->uCounter == 0)
        {
            if (itTimerToCheck->fnOnExpire)
            {
                TimerList_t aNewTimers;
                itTimerToCheck->fnOnExpire(aNewTimers);
                m_aTimers.splice(itTimerToCheck, std::move(aNewTimers));
            }

            m_aTimers.erase(itTimerToCheck);
        }
    }

    /*
     * The FSM state WaitForPll is a special case, because it does not involve
     * a timer. Without a timer there is no expire function that could do the
     * transition to the next state, so that transition is done explicitly here.
     */
    if (m_aTimers.empty() && (m_eStateFsm == FsmState_e::WaitForPll))
    {
        if (m_bDownChirp)
            gotoStateEDD(m_aTimers);
        else
            gotoStateEDU(m_aTimers);
    }

    return uNumCyclesTillNextState;
}

// ---------------------------------------------------------------------------- getPowerMode
PowerMode ModelBGT60TRxxC::getPowerMode() const
{
    return m_ePowerMode;
}

// ---------------------------------------------------------------------------- getNumSamples
unsigned ModelBGT60TRxxC::getNumSamples() const
{
    if (m_eStateFsm != FsmState_e::Sampling)
        return 0;

    return getCurrentChirpSettings().uNumSamples
           * getCurrentChirpSettings().uNumAntennas;
}

// ---------------------------------------------------------------------------- getShapeNumber
uint8_t ModelBGT60TRxxC::getShapeNumber() const
{
    return m_uShape + 1;
}

// ---------------------------------------------------------------------------- getShapeRepetition
uint16_t ModelBGT60TRxxC::getShapeRepetition() const
{
    if ((m_eStateFsm == FsmState_e::StartOfShape)
        || (m_eStateFsm == FsmState_e::PaDelay)
        || (m_eStateFsm == FsmState_e::AdcDelay)
        || (m_eStateFsm == FsmState_e::Sampling)
        || (m_eStateFsm == FsmState_e::WaitForPll)
        || (m_eStateFsm == FsmState_e::ChirpEndDelay))
        return m_uShapeRepetition + 1;
    else
        return 0;
}

// ---------------------------------------------------------------------------- getShapeSetRepetition
uint16_t ModelBGT60TRxxC::getShapeSetRepetition() const
{
    return m_uShapeSetRepetition + 1;
}

// ---------------------------------------------------------------------------- hasFrameFinished
bool ModelBGT60TRxxC::hasFrameFinished() const
{
    return (m_eStateFsm == FsmState_e::FrameEnded);
}

// ---------------------------------------------------------------------------- getStateDescription
std::string ModelBGT60TRxxC::getStateDescription() const
{
    std::string sTimerDescription;

    if (m_eStateFsm == FsmState_e::Ready)
        sTimerDescription = "Before Trigger";
    else if (m_eStateFsm == FsmState_e::WaitForPll)
        sTimerDescription = "Wait for Ramp End";
    else if (m_eStateFsm == FsmState_e::FrameEnded)
        sTimerDescription = "END";

    for (const auto& sTimer : m_aTimers)
    {
        if (!sTimerDescription.empty() && !sTimer.sDescription.empty())
            sTimerDescription += ", ";
        sTimerDescription += sTimer.sDescription;
    }

    return sTimerDescription;
}

// ---------------------------------------------------------------------------- getAssociatedBitfield
std::string ModelBGT60TRxxC::getAssociatedBitfield() const
{
    std::string sTimerBitfieds;
    for (const auto& sTimer : m_aTimers)
    {
        if (!sTimerBitfieds.empty() && !sTimer.sBitfields.empty())
            sTimerBitfieds += ", ";
        sTimerBitfieds += sTimer.sBitfields;
    }

    return sTimerBitfieds;
}

// ---------------------------------------------------------------------------- isVirtualState
bool ModelBGT60TRxxC::isVirtualState() const
{
    return m_eStateFsm == FsmState_e::StartOfShape;
}

// ---------------------------------------------------------------------------- isStartOfShape
bool ModelBGT60TRxxC::isStartOfShape() const
{
    return std::string::npos != getStateDescription().find("Start of Shape");
}

// ---------------------------------------------------------------------------- isShapeEndDelay
bool ModelBGT60TRxxC::isShapeEndDelay() const
{
    return std::string::npos != getStateDescription().find("Shape End Delay");
}

// ---------------------------------------------------------------------------- getCurrentFrequency
double ModelBGT60TRxxC::getCurrentFrequency() const
{
    return m_dCurrentFrequency;
}

// ---------------------------------------------------------------------------- getFrequencyIncrement
double ModelBGT60TRxxC::getFrequencyIncrement() const
{
    switch (m_eStatePll)
    {
        case PllState_e::RampInProgress:
            return getCurrentChirpSettings().dFrequencyInc;

        case PllState_e::FastDownRamp:
            return -(getCurrentChirpSettings().dFrequencyInc * getCurrentChirpSettings().uRampTime)
                   / m_sParameters.uFastDownTime;

        case PllState_e::Locking:
            return (getCurrentChirpSettings().dStartFrequency - m_sParameters.dPllMinFrequency)
                   / s_uLockTime;

        default:
            return 0.f;
    }
}

// ---------------------------------------------------------------------------- getPowerConsumption
double ModelBGT60TRxxC::getPowerConsumption() const
{
    return getPowerConsumption(m_ePowerMode);
}

// ---------------------------------------------------------------------------- getPowerConsumption
double ModelBGT60TRxxC::getPowerConsumption(PowerMode ePowerMode) const
{
    switch (ePowerMode)
    {
        case PowerMode::Deep_Sleep:
            return m_sPowerConsumptionTable.dPowerDeepSleep;

        case PowerMode::Deep_Sleep_Continue:
            if (auto pParametersD = dynamic_cast<const SequenceParametersD*>(&m_sParameters))
            {
                return pParametersD->bRefDoublerActive
                           ? m_sPowerConsumptionTable.dPowerDeepSleepContinueDoubler
                           : m_sPowerConsumptionTable.dPowerDeepSleepContinue;
            }
            else
            {
                return m_sPowerConsumptionTable.dPowerDeepSleepContinue;
            }

        case PowerMode::Idle:
            return m_sPowerConsumptionTable.dPowerIdle;

        case PowerMode::Interchirp:
            return (m_eStateFsm == FsmState_e::Init0)
                       ? m_sPowerConsumptionTable.dPowerInit0
                       : m_sPowerConsumptionTable.dPowerInterchirp;

        case PowerMode::Active:
            return m_sPowerConsumptionTable.dPowerActive;

        default:
            return std::numeric_limits<double>::quiet_NaN();
    }
}

// ---------------------------------------------------------------------------- getCurrentShapeSettings
const ShapeSettings& ModelBGT60TRxxC::getCurrentShapeSettings() const
{
    return m_sParameters.sShape[m_uShape];
}

// ---------------------------------------------------------------------------- getCurrentChirpSettings
const ChirpSettings& ModelBGT60TRxxC::getCurrentChirpSettings() const
{
    return m_bDownChirp ? getCurrentShapeSettings().sDown
                        : getCurrentShapeSettings().sUp;
}

// ---------------------------------------------------------------------------- gotoStateWU
void ModelBGT60TRxxC::gotoStateWU(TimerList_t& aTimers)
{
    m_eStateFsm = FsmState_e::Wakeup;
    m_ePowerMode = PowerMode::Idle;

    aTimers.emplace_back("Wake up", "T_WU", m_sParameters.uWakeUpTime,
                         [this](TimerList_t& aTimers) -> void {
                             gotoStateINIT0(aTimers);
                         });
}

// ---------------------------------------------------------------------------- gotoStateINIT0
void ModelBGT60TRxxC::gotoStateINIT0(TimerList_t& aTimers)
{
    m_ePowerMode = PowerMode::Interchirp;
    m_eStateFsm = FsmState_e::Init0;

    aTimers.emplace_back("Init 0", "T_INIT0", m_sParameters.uInit0Time,
                         [this](TimerList_t& aTimers) -> void {
                             gotoStateINIT1(aTimers);
                         });

    startPllLocking(aTimers);
}

// ---------------------------------------------------------------------------- gotoStateINIT1
void ModelBGT60TRxxC::gotoStateINIT1(TimerList_t& aTimers)
{
    m_eStateFsm = FsmState_e::Init1;

    aTimers.emplace_back("Init 1", "T_INIT1", m_sParameters.uInit1Time,
                         [this](TimerList_t& aTimers) -> void {
                             insertShapeHeader(aTimers);
                         });
}

// ---------------------------------------------------------------------------- insertShapeHeader
void ModelBGT60TRxxC::insertShapeHeader(TimerList_t& aTimers)
{
    m_eStateFsm = FsmState_e::StartOfShape;

    std::stringstream sHeadLine;
    sHeadLine << "Start of Shape Set " << (m_uShapeSetRepetition + 1)
              << ", Shape " << (m_uShape + 1)
              << ", Repetition " << (m_uShapeRepetition + 1);

    aTimers.emplace_back(sHeadLine.str(), std::string(), 0,
                         [this](TimerList_t& aTimers) -> void {
                             gotoStatePAEN(aTimers);
                         });
}

// ---------------------------------------------------------------------------- gotoStatePAEN
void ModelBGT60TRxxC::gotoStatePAEN(TimerList_t& aTimers)
{
    m_eStateFsm = FsmState_e::PaDelay;

    aTimers.emplace_back("PA Delay", "T_PAEN", m_sParameters.uPaDelay,
                         [this](TimerList_t& aTimers) -> void {
                             gotoStateSSTART(aTimers);
                         });

    startPllPreDelay(aTimers);
}

// ---------------------------------------------------------------------------- gotoStateSSTART
void ModelBGT60TRxxC::gotoStateSSTART(TimerList_t& aTimers)
{
    m_eStateFsm = FsmState_e::AdcDelay;
    m_ePowerMode = PowerMode::Active;

    aTimers.emplace_back("ADC Delay", "T_SSTART", m_sParameters.uAdcDelay,
                         [this](TimerList_t& aTimers) -> void {
                             gotoStateACQ(aTimers);
                         });
}

// ---------------------------------------------------------------------------- gotoStateACQ
void ModelBGT60TRxxC::gotoStateACQ(TimerList_t& aTimers)
{
    m_eStateFsm = FsmState_e::Sampling;

    aTimers.emplace_back("Sampling", std::string(m_bDownChirp ? "APD" : "APU") + ", ADC_DIV",
                         getCurrentChirpSettings().uNumSamples * m_sParameters.uAdcDivider,
                         [this](TimerList_t& aTimers) -> void {
                             gotoStateWaitForPll(aTimers);
                         });
}

// ---------------------------------------------------------------------------- gotoStateWaitForPll
void ModelBGT60TRxxC::gotoStateWaitForPll(TimerList_t& /*aTimers*/)
{
    m_eStateFsm = FsmState_e::WaitForPll;
    m_ePowerMode = PowerMode::Interchirp;
}

// ---------------------------------------------------------------------------- gotoStateEDU
void ModelBGT60TRxxC::gotoStateEDU(TimerList_t& aTimers)
{
    m_eStateFsm = FsmState_e::ChirpEndDelay;

    aTimers.emplace_back("Chirp End Delay", "T_EDU", getCurrentChirpSettings().uChirpEndDelay,
                         [this](TimerList_t& aTimers) -> void {
                             // Now, this is the end of the up chirp, down chirp follows
                             m_bDownChirp = true;

                             if (!getCurrentShapeSettings().bFastDownRamp)
                             {
                                 // in triangle mode, a true down chirp follows
                                 m_dCurrentFrequency = getCurrentChirpSettings().dStartFrequency;
                                 m_eStatePll = PllState_e::Ready;

                                 gotoStatePAEN(aTimers);
                             }
                             else
                             {
                                 // in sawtooth mode, only the chirp end delay follows
                                 gotoStateEDD(aTimers);
                             }
                         });
}

// ---------------------------------------------------------------------------- gotoStateEDD
void ModelBGT60TRxxC::gotoStateEDD(TimerList_t& aTimers)
{
    m_eStateFsm = FsmState_e::ChirpEndDelay;

    aTimers.emplace_back("Chirp End Delay", "T_EDD", getCurrentChirpSettings().uChirpEndDelay,
                         [this](TimerList_t& aTimers) -> void {
                             // now, this is the end of the shape, do next shape
                             // ------------------------------------------------

                             // each shape starts with the up chirp
                             m_bDownChirp = false;

                             // do next shape repetition
                             if ((++m_uShapeRepetition < getCurrentShapeSettings().uNumRepetitions)
                                 && !m_bIgnoreRepetitions)
                             {
                                 m_eStatePll = PllState_e::Ready;
                                 m_dCurrentFrequency = getCurrentChirpSettings().dStartFrequency;
                                 insertShapeHeader(aTimers);
                             }
                             else
                             {
                                 // this is the end of the shape group (last shape repetition)
                                 m_uShapeRepetition = 0;

                                 // apply shape end delay
                                 // after the last shape group, shape end delay is replaced by frame end delay
                                 if ((++m_uShapeGroupCounter < m_sParameters.uNumShapeGroupsPerFrame) || m_bIgnoreRepetitions)
                                     gotoStateSED(aTimers);
                                 else
                                     gotoStateFED(aTimers);
                             }
                         });
}

// ---------------------------------------------------------------------------- gotoStateSED
void ModelBGT60TRxxC::gotoStateSED(TimerList_t& aTimers)
{
    m_eStateFsm = FsmState_e::ShapeEndDelay;
    m_ePowerMode = getCurrentShapeSettings().eShapeEndPowerMode;
    if ((m_ePowerMode == PowerMode::Deep_Sleep)
        || (m_ePowerMode == PowerMode::Deep_Sleep_Continue)
        || (m_ePowerMode == PowerMode::Idle))
    {
        m_eStatePll = PllState_e::Off;
        m_dCurrentFrequency = m_sParameters.dPllMinFrequency;
    }

    auto prepare_next_state = [this](TimerList_t& aTimers) -> void {
        // go on to the next shape group
        ++m_uShape;
        if ((m_uShape == 4) || (getCurrentShapeSettings().uNumRepetitions == 0))
        {
            m_uShape = 0;
            ++m_uShapeSetRepetition;
        }

        // next state depends on power mode
        if ((m_ePowerMode == PowerMode::Deep_Sleep)
            || (m_ePowerMode == PowerMode::Deep_Sleep_Continue))
        {
            gotoStateWU(aTimers);
        }
        else if (m_ePowerMode == PowerMode::Idle)
        {
            gotoStateINIT0(aTimers);
        }
        else
        {
            m_eStatePll = PllState_e::Ready;
            m_dCurrentFrequency = getCurrentChirpSettings().dStartFrequency;
            insertShapeHeader(aTimers);
        }
    };
    aTimers.emplace_back("Shape End Delay", "T_SED", getCurrentShapeSettings().uShapeEndDelay, prepare_next_state);
}

// ---------------------------------------------------------------------------- gotoStateFED
void ModelBGT60TRxxC::gotoStateFED(TimerList_t& aTimers)
{
    m_eStateFsm = FsmState_e::FrameEndDelay;
    m_ePowerMode = m_sParameters.eFrameEndPowerMode;
    if ((m_ePowerMode == PowerMode::Deep_Sleep)
        || (m_ePowerMode == PowerMode::Deep_Sleep_Continue)
        || (m_ePowerMode == PowerMode::Idle))
    {
        m_eStatePll = PllState_e::Off;
        m_dCurrentFrequency = m_sParameters.dPllMinFrequency;
    }

    aTimers.emplace_back("Frame End Delay", "T_FED", m_sParameters.uFrameEndDelay,
                         [this](TimerList_t&) -> void {
                             m_eStateFsm = FsmState_e::FrameEnded;
                         });
}

// ---------------------------------------------------------------------------- startPllLocking
void ModelBGT60TRxxC::startPllLocking(TimerList_t& aTimers)
{
    m_eStatePll = PllState_e::Locking;
    m_dCurrentFrequency = m_sParameters.dPllMinFrequency;

    aTimers.emplace_back("PLL locking", "", s_uLockTime,
                         [this](TimerList_t&) -> void {
                             m_eStatePll = PllState_e::Ready;
                         });
}

// ---------------------------------------------------------------------------- startPllPreDelay
void ModelBGT60TRxxC::startPllPreDelay(TimerList_t& aTimers)
{
    m_eStatePll = PllState_e::PreDelay;

    aTimers.emplace_back("Pre-Chirp Delay", "T_START", m_sParameters.uPreChirpDelay,
                         [this](TimerList_t& aTimers) -> void {
                             startPllRamp(aTimers);
                         });
}

// ---------------------------------------------------------------------------- startPllRamp
void ModelBGT60TRxxC::startPllRamp(TimerList_t& aTimers)
{
    m_eStatePll = PllState_e::RampInProgress;
    m_dCurrentFrequency = getCurrentChirpSettings().dStartFrequency;

    aTimers.emplace_back("Ramp in Progress", m_bDownChirp ? "RTD" : "RTU",
                         getCurrentChirpSettings().uRampTime,
                         [this](TimerList_t& aTimers) -> void {
                             if (getCurrentShapeSettings().bFastDownRamp)
                                 startPllFastDownRamp(aTimers);
                             else
                                 startPllPostDelay(aTimers);
                         });
}

// ---------------------------------------------------------------------------- startPllFastDownRamp
void ModelBGT60TRxxC::startPllFastDownRamp(TimerList_t& aTimers)
{
    m_eStatePll = PllState_e::FastDownRamp;

    aTimers.emplace_back("Fast Down Ramp", "FSDNTMR", m_sParameters.uFastDownTime,
                         [this](TimerList_t& aTimers) -> void {
                             startPllPostDelay(aTimers);
                         });
}

// ---------------------------------------------------------------------------- startPllPostDelay
void ModelBGT60TRxxC::startPllPostDelay(TimerList_t& aTimers)
{
    m_eStatePll = PllState_e::PostDelay;

    aTimers.emplace_back("Post-Chirp Delay", "T_END", m_sParameters.uPostChirpDelay,
                         [this](TimerList_t&) -> void {
                             m_eStatePll = PllState_e::RampFinished;
                         });
}

/* ------------------------------------------------------------------------ */
}  // namespace TimingModel
}  // namespace Avian
}  // namespace Infineon

/* --- End of File -------------------------------------------------------- */
