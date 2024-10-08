/**
 * @file StateSequence.cpp
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
#include "ifxAvian_Driver.hpp"
#include "ifxAvian_TimingModel.hpp"
#include "ModelBGT60TR11D.hpp"
#include "ModelBGT60TRxxC.hpp"
#include "ModelBGT60TRxxD.hpp"
#include <cmath>
#include <iomanip>
#include <limits>
#include <memory>
#include <sstream>

// ---------------------------------------------------------------------------- namespaces
namespace Infineon {
namespace Avian {
namespace TimingModel {

// ---------------------------------------------------------------------------- StateList::extendLastStateUntil
void StateSequence::StateList::extendLastStateUntil(Ticks dCurrentTime)
{
    if (!empty())
    {
        StateInfo& sLastState = back();
        sLastState.dDuration = dCurrentTime - sLastState.dStartTime;
        sLastState.dEndFrequency += sLastState.dDuration * dCurrentFrequencyIncrement;
    }
}

// ---------------------------------------------------------------------------- StateList::addStateTransition
StateSequence::StateInfo& StateSequence::StateList::addStateTransition(Ticks dCurrentTime, const StateInfo& sState)
{
    extendLastStateUntil(dCurrentTime);
    push_back(sState);

    return back();
}

// ---------------------------------------------------------------------------- StateList::addState
StateSequence::StateInfo& StateSequence::StateList::addState(Ticks dCurrentTime, const ModelBGT60TRxxC& xFsm)
{
    StateInfo sNewState;
    sNewState.dStartTime = dCurrentTime;
    sNewState.sDescription = xFsm.getStateDescription();
    sNewState.sAssociatedBitfield = xFsm.getAssociatedBitfield();
    sNewState.ePowerMode = xFsm.getPowerMode();
    sNewState.uNumSamples = xFsm.getNumSamples();
    sNewState.dStartFrequency = xFsm.getCurrentFrequency();
    sNewState.dEndFrequency = sNewState.dStartFrequency;
    sNewState.bHeadline = xFsm.isVirtualState();
    sNewState.dPower = xFsm.getPowerConsumption();
    addStateTransition(dCurrentTime, sNewState);

    dCurrentFrequencyIncrement = xFsm.getFrequencyIncrement();

    return back();
}

// ---------------------------------------------------------------------------- StateList::addHeadline
StateSequence::StateInfo& StateSequence::StateList::addHeadline(Ticks dCurrentTime, const std::string& sDescription)
{
    StateInfo sNewState;
    sNewState.dStartTime = dCurrentTime;
    sNewState.sDescription = sDescription;
    sNewState.sAssociatedBitfield = "";
    sNewState.ePowerMode = PowerMode::Idle;
    sNewState.uNumSamples = 0;
    sNewState.dStartFrequency = 0;
    sNewState.dEndFrequency = 0;
    sNewState.bHeadline = true;
    sNewState.dPower = std::numeric_limits<double>::quiet_NaN();
    addStateTransition(dCurrentTime, sNewState);

    return back();
}

// ---------------------------------------------------------------------------- StateList::finishSequence
void StateSequence::StateList::finishSequence(Ticks dCurrentTime)
{
    extendLastStateUntil(dCurrentTime);
    updateStatistics();
    dTime = dCurrentTime;
    uNumStates = size();
}

// ---------------------------------------------------------------------------- StateList::updateStatistics
void StateSequence::StateList::updateStatistics()
{
    dActiveDuration = 0.0;
    dTotalDuration = 0.0;
    dTotalWork = 0.0;


    for (auto& sState : *this)
    {
        if ((sState.dStartTime >= 0.0) && !sState.bHeadline)
        {
            if (!std::isnan(sState.dPower))
            {
                dTotalDuration += sState.dDuration;
                dTotalWork += sState.dPower * sState.dDuration;
            }
            if (sState.ePowerMode == PowerMode::Active)
            {
                dActiveDuration += sState.dDuration;
            }
        }
    }

    dAveragePower = (dTotalDuration > std::numeric_limits<double>::epsilon())
                        ? dTotalWork / dTotalDuration
                        : std::numeric_limits<double>::quiet_NaN();
}

// ---------------------------------------------------------------------------- ShapeGroup_s::addShapeState
void StateSequence::ShapeGroup::addShapeState(Ticks dCurrentTime, const ModelBGT60TRxxC& xFsm)
{
    StateInfo& sNewState = sShapeStates.addState(dCurrentTime, xFsm);
    uNumShapeSamples += sNewState.uNumSamples;
}

// ---------------------------------------------------------------------------- ShapeGroup_s::finishShapeStates
void StateSequence::ShapeGroup::finishShapeStates(Ticks dCurrentTime, const ShapeSettings& sShapeSetting)
{
    sShapeStates.finishSequence(dCurrentTime);

    // remember some shape parameters in first "headline" state
    StateInfo& sHeadline = sShapeStates.front();
    sHeadline.dDuration = dCurrentTime;
    sHeadline.dStartFrequency = sShapeSetting.sUp.dStartFrequency;
    sHeadline.dEndFrequency = sShapeSetting.sUp.dStartFrequency
                              + sShapeSetting.sUp.dFrequencyInc
                                    * sShapeSetting.sUp.uRampTime;
    sHeadline.dPower = sShapeStates.getAveragePower();

    // reuse "associated bit fields" string to indicate triangular shape
    if (!sShapeSetting.bFastDownRamp)
        sHeadline.sAssociatedBitfield = "Tri";

    // process the plain shape group
    uNumShapeRepetitions = sShapeSetting.uNumRepetitions;
}

// ---------------------------------------------------------------------------- ShapeGroup_s::addShapeEndState
void StateSequence::ShapeGroup::addShapeEndState(Ticks dCurrentTime, const ModelBGT60TRxxC& xFsm)
{
    sShapeEndStates.addState(dCurrentTime, xFsm);
}

// ---------------------------------------------------------------------------- ShapeGroup_s::finishShapeEndStates
void StateSequence::ShapeGroup::finishShapeEndStates(Ticks dCurrentTime)
{
    sShapeEndStates.finishSequence(dCurrentTime);

    sGroupDuration.dTime = sShapeStates.dTime
                               * uNumShapeRepetitions
                           + dCurrentTime;
    sGroupDuration.uNumStates = sShapeStates.uNumStates
                                    * uNumShapeRepetitions
                                + sShapeEndStates.size();
}

// ---------------------------------------------------------------------------- ShapeGroup_s::getActiveDuration
Ticks StateSequence::ShapeGroup::getActiveDuration() const
{
    Ticks dActiveShape = uNumShapeRepetitions * sShapeStates.getActiveDuration();
    Ticks dActiveShapeEnd = sShapeEndStates.getActiveDuration();
    return dActiveShape + dActiveShapeEnd;
}

// ---------------------------------------------------------------------------- ShapeGroup_s::getTotalDuration
Ticks StateSequence::ShapeGroup::getTotalDuration() const
{
    Ticks dDurationShape = uNumShapeRepetitions * sShapeStates.dTime;
    Ticks dDurationShapeEnd = sShapeEndStates.dTime;
    return dDurationShape + dDurationShapeEnd;
}

// ---------------------------------------------------------------------------- ShapeGroup_s::getTotalWork
double StateSequence::ShapeGroup::getTotalWork() const
{
    Ticks dDurationShape = uNumShapeRepetitions * sShapeStates.dTime;
    double dWorkShapeStates = sShapeStates.getAveragePower() * dDurationShape;
    double dWorkShapeEnd = sShapeEndStates.getAveragePower() * sShapeEndStates.dTime;
    return dWorkShapeStates + dWorkShapeEnd;
}

// ---------------------------------------------------------------------------- get_ref_frequency_from_driver
static double get_ref_frequency_from_driver(const Driver& driver)
{
    Reference_Clock_Frequency eReferenceFrequency;
    driver.get_reference_clock_frequency(&eReferenceFrequency);
    if ((eReferenceFrequency == Reference_Clock_Frequency::_76_8MHz)
        || (eReferenceFrequency == Reference_Clock_Frequency::_38_4MHz))
        return 76800000.0;
    else
        return 80000000.0;
}

// ---------------------------------------------------------------------------- StateSequence
StateSequence::StateSequence(const Driver& driver) :
    StateSequence(driver.get_device_configuration(), driver.get_device_type(),
                  get_ref_frequency_from_driver(driver))
{}

// ---------------------------------------------------------------------------- StateSequence
StateSequence::StateSequence(const HW::RegisterSet& registers,
                             Device_Type device_type,
                             double ref_frequency) :
    m_dOscFrequency(ref_frequency)
{
    // init state machine
    // ------------------
    std::unique_ptr<SequenceParameters> pParameters;
    std::unique_ptr<ModelBGT60TRxxC> pFsm;

    switch (device_type)
    {
        case Device_Type::BGT60UTR11AIP:
            {
                auto pParameters11D = std::make_unique<SequenceParameters11D>(registers,
                                                                              device_type,
                                                                              m_dOscFrequency);
                pFsm = std::make_unique<ModelBGT60TR11D>(*pParameters11D, true);
                pParameters = std::move(pParameters11D);
            }
            break;

        case Device_Type::BGT60UTR13D:
        case Device_Type::BGT60TR12E:
        case Device_Type::BGT120UTR13E:
        case Device_Type::BGT120UTR24:
        case Device_Type::BGT60ATR24E:
        case Device_Type::BGT24LTR13E:
            {
                auto pParametersD = std::make_unique<SequenceParametersD>(registers,
                                                                          device_type,
                                                                          m_dOscFrequency);
                pFsm = std::make_unique<ModelBGT60TRxxD>(*pParametersD, true);
                pParameters = std::move(pParametersD);
            }
            break;

        case Device_Type::BGT60TR13C:
        case Device_Type::BGT60ATR24C:
        case Device_Type::BGT24LTR24:
        default:
            pParameters = std::make_unique<SequenceParameters>(registers,
                                                               device_type,
                                                               m_dOscFrequency);
            pFsm = std::make_unique<ModelBGT60TRxxC>(*pParameters, true);
            break;
    };

    m_jTotalFrequencyRange = std::make_pair(pParameters->dPllMinFrequency,
                                            pParameters->dPllMaxFrequency);
    m_uFifoSize = pParameters->uFifoSize;

    // now go through state machine
    // ----------------------------

    // process the prefix (= steps until first shape begins)
    m_sPrefixStates.addState(-10.0, *pFsm);
    pFsm->startFrame();

    Ticks dCurrentTime = 0.0;

    const auto avgPowerIdx = m_sPrefixStates.size();
    m_sPrefixStates.addHeadline(dCurrentTime, "Average power consumption");

    while (!pFsm->isStartOfShape())
    {
        m_sPrefixStates.addState(dCurrentTime, *pFsm);
        dCurrentTime += pFsm->goToNextState();
    }
    m_sPrefixStates.finishSequence(dCurrentTime);

    // now process the shape set
    m_uNumSetSamples = 0;
    for (auto uShp = 0; uShp < 4; ++uShp)
    {
        ShapeGroup& sCurrentGroup = m_sGroups[uShp];

        // check that current shape is used
        if (pFsm->getShapeNumber() != uShp + 1)
            break;

        // process the plain shape
        dCurrentTime = 0.0;
        while (!pFsm->isShapeEndDelay())
        {
            sCurrentGroup.addShapeState(dCurrentTime, *pFsm);
            dCurrentTime += pFsm->goToNextState();
        }
        sCurrentGroup.finishShapeStates(dCurrentTime, pParameters->sShape[uShp]);

        dCurrentTime = 0.0;
        while (!pFsm->isStartOfShape())
        {
            sCurrentGroup.addShapeEndState(dCurrentTime, *pFsm);
            dCurrentTime += pFsm->goToNextState();
        }
        sCurrentGroup.finishShapeEndStates(dCurrentTime);

        m_sSetDuration.dTime += sCurrentGroup.sGroupDuration.dTime;
        m_sSetDuration.uNumStates += sCurrentGroup.sGroupDuration.uNumStates;
        m_uNumSetSamples += sCurrentGroup.uNumShapeSamples * sCurrentGroup.uNumShapeRepetitions;
    }

    // calculate end of last shape
    // ---------------------------
    uint8_t uNumUsedShapes = (m_sGroups[3].uNumShapeRepetitions > 0) ? 4 : (m_sGroups[2].uNumShapeRepetitions > 0) ? 3
                                                                       : (m_sGroups[1].uNumShapeRepetitions > 0)   ? 2
                                                                                                                   : 1;
    unsigned uNumSets = pParameters->uNumShapeGroupsPerFrame / uNumUsedShapes;
    uint8_t uNumAdditionalGroups = pParameters->uNumShapeGroupsPerFrame % uNumUsedShapes;
    uint8_t uLastUsedShape = (uNumAdditionalGroups != 0) ? uNumAdditionalGroups - 1 : uNumUsedShapes - 1;
    const ShapeGroup& sLastUsedGroup = m_sGroups[uLastUsedShape];

    // complete sets
    m_sActiveFrameDuration.dTime = m_sPrefixStates.dTime
                                   + uNumSets * m_sSetDuration.dTime;
    m_sActiveFrameDuration.uNumStates = m_sPrefixStates.uNumStates
                                        + uNumSets * m_sSetDuration.uNumStates;
    m_uNumTotalSamples = uNumSets * m_uNumSetSamples;

    // last incomplete set
    for (unsigned uShp = 0; uShp < uNumAdditionalGroups; ++uShp)
    {
        ShapeGroup& sCurrentGroup = m_sGroups[uShp];
        m_sActiveFrameDuration.dTime += sCurrentGroup.sGroupDuration.dTime;
        m_sActiveFrameDuration.uNumStates += sCurrentGroup.sGroupDuration.uNumStates;
        m_uNumTotalSamples = sCurrentGroup.uNumShapeSamples * sCurrentGroup.uNumShapeRepetitions;
    }

    // subtract end states from last used shape
    m_sActiveFrameDuration.dTime -= sLastUsedGroup.sGroupDuration.dTime;
    m_sActiveFrameDuration.dTime += sLastUsedGroup.sShapeStates.dTime
                                    * sLastUsedGroup.uNumShapeRepetitions;

    m_sActiveFrameDuration.uNumStates -= sLastUsedGroup.sGroupDuration.uNumStates;
    m_sActiveFrameDuration.uNumStates += sLastUsedGroup.sShapeStates.uNumStates
                                         * sLastUsedGroup.uNumShapeRepetitions;

    // set frame end state manually
    m_sEndState.dStartTime = 0;
    m_sEndState.dDuration = Ticks(pParameters->uFrameEndDelay);
    m_sEndState.sDescription = "Frame End Delay";
    m_sEndState.sAssociatedBitfield = "T_FED";
    m_sEndState.ePowerMode = pParameters->eFrameEndPowerMode;
    m_sEndState.uNumSamples = 0;
    m_sEndState.dStartFrequency = pFsm->getCurrentFrequency();
    m_sEndState.dEndFrequency = m_sEndState.dStartFrequency;
    m_sEndState.bHeadline = false;
    m_sEndState.dPower = pFsm->getPowerConsumption(m_sEndState.ePowerMode);

    m_uFifoThreshold = pParameters->uFifoThreshold;
    m_uAdcCycles = pParameters->uAdcDivider;

    // calculate the average power of the whole frame
    // ----------------------------------------------
    StateInfo& sTotalPower = m_sPrefixStates.begin()[avgPowerIdx];

    // complete total power headline state
    const Ticks dPrefixDuration = m_sPrefixStates.getTotalDuration();
    const Ticks dPrefixActive = m_sPrefixStates.getActiveDuration();
    const double dPrefixWork = m_sPrefixStates.getTotalWork();

    Ticks dSetDuration = 0;
    Ticks dActive = 0;
    double dSetWork = 0;
    for (ShapeGroup& sGroup : m_sGroups)
    {
        dSetDuration += sGroup.getTotalDuration();
        dSetWork += sGroup.getTotalWork();
        dActive += sGroup.getActiveDuration();
    }
    dSetDuration *= uNumSets;
    dSetWork *= uNumSets;
    dActive *= uNumSets;

    // subtract shape end states of last used shape group
    dSetDuration -= sLastUsedGroup.sShapeEndStates.getTotalDuration();
    dSetWork -= sLastUsedGroup.sShapeEndStates.getTotalWork();

    // frame end states
    const Ticks dEndDuration = m_sEndState.dDuration;
    const double dEndWork = m_sEndState.dPower * dEndDuration;

    // now put it all together
    const Ticks dTotalDuration = dPrefixDuration + dSetDuration + dEndDuration;
    const double dTotalWork = dPrefixWork + dSetWork + dEndWork;

    m_dAveragePowerConsumption = (dTotalDuration > std::numeric_limits<double>::epsilon())
                                     ? dTotalWork / dTotalDuration
                                     : std::numeric_limits<double>::quiet_NaN();

    sTotalPower.dDuration = getTotalTimeRange().second;
    sTotalPower.dPower = m_dAveragePowerConsumption;

    m_dActiveStateTotalDuration = (dActive + dPrefixActive) / m_dOscFrequency;
}

// ---------------------------------------------------------------------------- getNumStates
size_t StateSequence::getNumStates() const
{
    return m_sActiveFrameDuration.uNumStates + 1;
}

// ---------------------------------------------------------------------------- getTotalTimeRange
StateSequence::TickRange StateSequence::getTotalTimeRange() const
{
    return {m_sPrefixStates[0].dStartTime,
            m_sActiveFrameDuration.dTime + m_sEndState.dDuration};
}

// ---------------------------------------------------------------------------- getTotalFrequencyRange
StateSequence::Range StateSequence::getTotalFrequencyRange() const
{
    return m_jTotalFrequencyRange;
}

// ---------------------------------------------------------------------------- findSequenceStep
size_t StateSequence::findSequenceStep(Ticks dTime) const
{
    // first check, if index is in range
    if (dTime >= m_sActiveFrameDuration.dTime)
    {
        return m_sActiveFrameDuration.uNumStates;
    }

    // now check which list to search in
    const StateList* pStateList = nullptr;
    size_t uBaseIdx = 0;

    if (dTime < m_sPrefixStates.dTime)
    {
        pStateList = &m_sPrefixStates;
    }
    else
    {
        // take prefix into accounts
        dTime -= m_sPrefixStates.dTime;
        uBaseIdx = m_sPrefixStates.uNumStates;

        // find shape set repetition
        unsigned uShapeSet = unsigned(dTime / m_sSetDuration.dTime);
        dTime -= m_sSetDuration.dTime * uShapeSet;
        uBaseIdx += m_sSetDuration.uNumStates * uShapeSet;

        // find shape
        const ShapeGroup* pGroup = m_sGroups;
        for (unsigned i = 0; i < 3; ++i)
        {
            if (dTime < pGroup->sGroupDuration.dTime)
                break;

            // take current shape into account
            dTime -= pGroup->sGroupDuration.dTime;
            uBaseIdx += pGroup->sGroupDuration.uNumStates;
            ++pGroup;
        }

        // check if shape or shape end
        Ticks dEndTime = pGroup->sShapeStates.dTime
                         * pGroup->uNumShapeRepetitions;

        if (dTime < dEndTime)
        {
            unsigned uRepetition = unsigned(dTime / pGroup->sShapeStates.dTime);
            dTime -= pGroup->sShapeStates.dTime * uRepetition;
            uBaseIdx += pGroup->sShapeStates.uNumStates * uRepetition;

            pStateList = &pGroup->sShapeStates;
        }
        else
        {
            dTime -= dEndTime;

            uBaseIdx += pGroup->sShapeStates.uNumStates
                        * pGroup->uNumShapeRepetitions;
            pStateList = &pGroup->sShapeEndStates;
        }
    }

    // do a binary search to find the right step
    size_t uSearchStart = 0;
    size_t uSearchEnd = pStateList->size() - 1;

    while (uSearchEnd > uSearchStart)
    {
        size_t uStepToCheck = (uSearchStart + uSearchEnd) >> 1;
        Ticks dStepStart = (*pStateList)[uStepToCheck].dStartTime;
        Ticks dStepEnd = dStepStart + (*pStateList)[uStepToCheck].dDuration;

        if (dTime < dStepStart)
            uSearchEnd = uStepToCheck - 1;
        else if (dTime > dStepEnd)
            uSearchStart = uStepToCheck + 1;
        else
            return uBaseIdx + uStepToCheck;
    }

    return uBaseIdx + uSearchStart;
}

// ---------------------------------------------------------------------------- getChirpToChirpTime
Ticks StateSequence::getChirpToChirpTime(unsigned uShape) const
{
    return (uShape < 4) ? m_sGroups[uShape].sShapeStates.dTime : 0.0;
}

// ---------------------------------------------------------------------------- getSetToSetTime
Ticks StateSequence::getSetToSetTime() const
{
    return m_sSetDuration.dTime;
}

// ---------------------------------------------------------------------------- getFrameDuration
Ticks StateSequence::getFrameDuration() const
{
    // get last sequence step (= last step of frame)
    Iterator xFinalStep(*this, getNumStates() - 1);

    // find wake up step
    Iterator xFirstStep(*this, size_t(0));
    while (xFirstStep.getPowerMode() != PowerMode::Idle)
        ++xFirstStep;

    // if frame does not end in Deep Sleep, next frame won't start with Wake up step
    if ((xFinalStep.getPowerMode() != PowerMode::Deep_Sleep)
        && (xFinalStep.getPowerMode() != PowerMode::Deep_Sleep_Continue))
    {
        // -> skip wake up step
        while (xFirstStep.getPowerMode() != PowerMode::Interchirp)
            ++xFirstStep;

        // if frame does not end in Idle next frame won't do PLL init
        if (xFinalStep.getPowerMode() != PowerMode::Idle)
        {
            // -> skip PLL init
            while (!xFirstStep.isStartOfShape())
                ++xFirstStep;
        }
    }

    // calculate frame length return reciprocal
    return xFinalStep.getEndTime() - xFirstStep.getStartTime();
}

// ---------------------------------------------------------------------------- getDescription
std::string StateSequence::Iterator::getDescription() const
{
    // if this is not the start of shape, return the description as it is
    if (m_pCurrentState->sDescription.find("Start of Shape") == std::string::npos)
        return m_pCurrentState->sDescription;

    // if this is the start of shape, construct the headline
    std::stringstream sStream;
    sStream << "Start of Shape Set " << (m_uShapeSet + 1)
            << ", Shape " << (m_uShape + 1)
            << ", Repetition " << (m_uShapeRepetition + 1);
    return sStream.str();
}

// ---------------------------------------------------------------------------- exportSequenceTable
void StateSequence::exportSequenceTable(std::ostream& sOutStream) const
{
    const int iColWidthTime = 15;
    const int iColWidthDuration = 15;
    const int iColWidthSet = 4;
    const int iColWidthShape = 5;
    const int iColWidthRepetition = 10;
    const int iColWidthDescription = 40;
    const int iColWidthFrequency = 15;
    const int iColWidthBitfields = 20;
    const int iColWidthPowerState = 10;
    const int iColWidthPowerConsumption = 17;

    // write headline
    sOutStream << std::left
               << std::setw(iColWidthTime) << "Start Time"
               << " | "
               << std::setw(iColWidthDuration) << "Duration"
               << " | "
               << std::setw(iColWidthSet) << "Set"
               << " | "
               << std::setw(iColWidthShape) << "Shape"
               << " | "
               << std::setw(iColWidthRepetition) << "Repetition"
               << " | "
               << std::setw(iColWidthDescription) << "Description"
               << " | "
               << std::setw(iColWidthFrequency) << "Start Frequency"
               << " | "
               << std::setw(iColWidthFrequency) << "End Frequency"
               << " | "
               << std::setw(iColWidthBitfields) << "Associated Bitfields"
               << " | "
               << std::setw(iColWidthPowerState) << "Power Mode"
               << " | "
               << std::setw(iColWidthPowerConsumption) << "Power Consumption" << std::endl;

    sOutStream << std::left
               << std::setw(iColWidthTime) << "[12.5ns clks]"
               << " | "
               << std::setw(iColWidthDuration) << "[12.5ns clks]"
               << " | "
               << std::string(iColWidthSet, ' ') << " | "
               << std::string(iColWidthShape, ' ') << " | "
               << std::string(iColWidthRepetition, ' ') << " | "
               << std::string(iColWidthDescription, ' ') << " | "
               << std::setw(iColWidthFrequency) << "[GHz]"
               << " | "
               << std::setw(iColWidthFrequency) << "[GHz]"
               << " | "
               << std::string(iColWidthBitfields, ' ') << " | "
               << std::string(iColWidthPowerState, ' ') << " | "
               << std::setw(iColWidthPowerConsumption) << "[mW]" << std::endl;

    sOutStream << std::string(iColWidthTime, '-') << "-+-"
               << std::string(iColWidthDuration, '-') << "-+-"
               << std::string(iColWidthSet, '-') << "-+-"
               << std::string(iColWidthShape, '-') << "-+-"
               << std::string(iColWidthRepetition, '-') << "-+-"
               << std::string(iColWidthDescription, '-') << "-+-"
               << std::string(iColWidthFrequency, '-') << "-+-"
               << std::string(iColWidthFrequency, '-') << "-+-"
               << std::string(iColWidthBitfields, '-') << "-+-"
               << std::string(iColWidthPowerState, '-') << "-+-"
               << std::string(iColWidthPowerConsumption, '-') << std::endl;

    // now write the data lines
    for (Iterator xStep(*this); !xStep.hasEnded(); ++xStep)
    {
        // skip headlines
        if (xStep.isStartOfShape())
            continue;

        // write start time
        int64_t iValue = int64_t(xStep.getStartTime() + (xStep.getStartTime() >= 0.0 ? 0.5 : -0.5));
        sOutStream << std::right << std::setw(iColWidthTime) << iValue << " | ";

        // write duration
        iValue = int64_t(xStep.getDuration() + 0.5);
        sOutStream << std::right << std::setw(iColWidthDuration) << iValue << " | ";

        // write shape set
        iValue = xStep.getShapeSet();
        sOutStream << std::right << std::setw(iColWidthSet) << iValue << " | ";

        // write shape
        iValue = xStep.getShape();
        sOutStream << std::right << std::setw(iColWidthShape) << iValue << " | ";

        // write shape repetition
        iValue = xStep.getShapeRepetition();
        sOutStream << std::right << std::setw(iColWidthRepetition) << iValue << " | ";

        // write description
        sOutStream << std::left << std::setw(iColWidthDescription) << xStep.getDescription() << " | ";

        // write start frequency
        if ((xStep.getPowerMode() == PowerMode::Interchirp)
            || (xStep.getPowerMode() == PowerMode::Active))
        {
            double dFrequency = xStep.getStartFrequency() * 1.0e-9;
            sOutStream << std::right << std::setw(iColWidthFrequency)
                       << std::fixed << std::setprecision(6) << dFrequency << " | ";
        }
        else
        {
            sOutStream << std::string(iColWidthFrequency, ' ') << " | ";
        }

        // write end frequency
        if ((xStep.getPowerMode() == PowerMode::Interchirp)
            || (xStep.getPowerMode() == PowerMode::Active))
        {
            double dFrequency = xStep.getEndFrequency() * 1.0e-9;
            sOutStream << std::right << std::setw(iColWidthFrequency)
                       << std::fixed << std::setprecision(6) << dFrequency << " | ";
        }
        else
        {
            sOutStream << std::string(iColWidthFrequency, ' ') << " | ";
        }


        // write associated bit fields
        sOutStream << std::left << std::setw(iColWidthBitfields) << xStep.getAssociatedBitfield() << " | ";

        // write power mode
        std::string sMode;
        switch (xStep.getPowerMode())
        {
            case PowerMode::Deep_Sleep: sMode = "Deep Sleep"; break;
            case PowerMode::Deep_Sleep_Continue: sMode = "Deep Sleep & Continue"; break;
            case PowerMode::Idle: sMode = "Idle"; break;
            case PowerMode::Interchirp: sMode = "Interchirp"; break;
            case PowerMode::Active: sMode = "Active"; break;
        }
        sOutStream << std::left << std::setw(iColWidthPowerState) << sMode << " | ";

        // write power consumption
        sOutStream << std::right << std::setw(iColWidthPowerConsumption)
                   << xStep.getPowerConsumption() * 1.0e3 << std::endl;
    }
}

// ---------------------------------------------------------------------------- Iterator::Iterator
StateSequence::Iterator::Iterator(const StateSequence& xSequence) :
    m_xSequence(xSequence),
    m_eStage(Stage_e::Prefix),
    m_uShape(0),
    m_uShapeSet(0),
    m_uShapeRepetition(0),
    m_uSeqIndex(0),
    m_dBaseTime(0.0),
    m_uFifoFillState(0),
    m_pCurrentState(&m_xSequence.m_sPrefixStates[0])
{}

// ---------------------------------------------------------------------------- Iterator::Iterator
StateSequence::Iterator::Iterator(const StateSequence& xSequence, size_t uIndex) :
    m_xSequence(xSequence),
    m_eStage(Stage_e::EndOfSequence),
    m_uShape(0),
    m_uShapeSet(0),
    m_uShapeRepetition(0),
    m_uSeqIndex(0),
    m_dBaseTime(0.0),
    m_uFifoFillState(0),
    m_pCurrentState(nullptr)
{
    // first check, if index is in range
    if (uIndex > m_xSequence.m_sActiveFrameDuration.uNumStates)
    {
        m_dBaseTime = m_xSequence.m_sActiveFrameDuration.dTime + m_xSequence.m_sEndState.dDuration;
        m_pCurrentState = &m_xSequence.m_sEndState;
        m_uFifoFillState = m_xSequence.m_uNumTotalSamples % m_xSequence.m_uFifoThreshold;
        m_eStage = Stage_e::EndOfSequence;
    }
    else if (uIndex == m_xSequence.m_sActiveFrameDuration.uNumStates)
    {
        m_dBaseTime = m_xSequence.m_sActiveFrameDuration.dTime;
        m_pCurrentState = &m_xSequence.m_sEndState;
        m_uFifoFillState = m_xSequence.m_uNumTotalSamples % m_xSequence.m_uFifoThreshold;
        m_eStage = Stage_e::FrameEnd;
    }
    else if (uIndex < m_xSequence.m_sPrefixStates.uNumStates)
    {
        m_uSeqIndex = uint16_t(uIndex);
        m_pCurrentState = &m_xSequence.m_sPrefixStates[m_uSeqIndex];
        m_eStage = Stage_e::Prefix;
    }
    else
    {
        // take prefix into accounts
        uIndex -= m_xSequence.m_sPrefixStates.uNumStates;
        m_dBaseTime += m_xSequence.m_sPrefixStates.dTime;

        // find shape set repetition
        m_uShapeSet = uint16_t(uIndex / m_xSequence.m_sSetDuration.uNumStates);
        uIndex = uIndex % m_xSequence.m_sSetDuration.uNumStates;
        m_dBaseTime += m_xSequence.m_sSetDuration.dTime * m_uShapeSet;
        m_uFifoFillState += m_xSequence.m_uNumSetSamples * m_uShapeSet;

        // find shape
        const ShapeGroup* pGroup = m_xSequence.m_sGroups;
        for (unsigned i = 0; i < 3; ++i)
        {
            if (uIndex < pGroup->sGroupDuration.uNumStates)
                break;

            // take current shape into account
            uIndex -= pGroup->sGroupDuration.uNumStates;
            m_dBaseTime += pGroup->sGroupDuration.dTime;
            m_uFifoFillState += pGroup->uNumShapeSamples * pGroup->uNumShapeRepetitions;
            ++pGroup;
            ++m_uShape;
        }

        // check if shape or shape end
        size_t uEndIndex = pGroup->sShapeStates.uNumStates
                           * pGroup->uNumShapeRepetitions;

        if (uIndex < uEndIndex)
        {
            m_uShapeRepetition = uint16_t(uIndex / pGroup->sShapeStates.uNumStates);
            m_uSeqIndex = uint16_t(uIndex % pGroup->sShapeStates.uNumStates);
            m_dBaseTime += pGroup->sShapeStates.dTime * m_uShapeRepetition;
            m_uFifoFillState += pGroup->uNumShapeSamples * m_uShapeRepetition;
            m_pCurrentState = &pGroup->sShapeStates[m_uSeqIndex];
            m_eStage = Stage_e::Shape;

            for (size_t i = 0; i < m_uSeqIndex; ++i)
                m_uFifoFillState += pGroup->sShapeStates[i].uNumSamples;
        }
        else
        {
            m_uSeqIndex = uint16_t(uIndex - uEndIndex);
            m_dBaseTime += pGroup->sShapeStates.dTime * pGroup->uNumShapeRepetitions;
            m_uFifoFillState += pGroup->uNumShapeSamples * pGroup->uNumShapeRepetitions;
            m_pCurrentState = &pGroup->sShapeEndStates[m_uSeqIndex];
            m_eStage = Stage_e::ShapeEnd;
        }
        m_uFifoFillState %= m_xSequence.getFifoThreshold();
    }
}

// ---------------------------------------------------------------------------- Iterator::Iterator
StateSequence::Iterator::Iterator(const StateSequence& xSequence, Ticks dStartTime) :
    m_xSequence(xSequence),
    m_eStage(Stage_e::EndOfSequence),
    m_uShape(0),
    m_uShapeSet(0),
    m_uShapeRepetition(0),
    m_uSeqIndex(0),
    m_dBaseTime(0.0),
    m_uFifoFillState(0),
    m_pCurrentState(nullptr)
{
    // first check, if index is in range
    if (dStartTime > m_xSequence.m_sActiveFrameDuration.dTime + m_xSequence.m_sEndState.dDuration)
    {
        m_dBaseTime = m_xSequence.m_sActiveFrameDuration.dTime + m_xSequence.m_sEndState.dDuration;
        m_pCurrentState = &m_xSequence.m_sEndState;
        m_uFifoFillState = m_xSequence.m_uNumTotalSamples % m_xSequence.m_uFifoThreshold;
        m_eStage = Stage_e::EndOfSequence;
    }
    else if (dStartTime >= m_xSequence.m_sActiveFrameDuration.dTime)
    {
        m_dBaseTime = m_xSequence.m_sActiveFrameDuration.dTime;
        m_pCurrentState = &m_xSequence.m_sEndState;
        m_uFifoFillState = m_xSequence.m_uNumTotalSamples % m_xSequence.m_uFifoThreshold;
        m_eStage = Stage_e::FrameEnd;
    }
    else
    {
        // now check which list to search in
        const StateList* pStateList = nullptr;

        if (dStartTime < m_xSequence.m_sPrefixStates.dTime)
        {
            m_eStage = Stage_e::Prefix;
            pStateList = &m_xSequence.m_sPrefixStates;
        }
        else
        {
            // take prefix into accounts
            dStartTime -= m_xSequence.m_sPrefixStates.dTime;
            m_dBaseTime += m_xSequence.m_sPrefixStates.dTime;

            // find shape set repetition
            m_uShapeSet = uint16_t(dStartTime / m_xSequence.m_sSetDuration.dTime);
            dStartTime -= m_xSequence.m_sSetDuration.dTime * m_uShapeSet;
            m_dBaseTime += m_xSequence.m_sSetDuration.dTime * m_uShapeSet;
            m_uFifoFillState += m_xSequence.m_uNumSetSamples * m_uShapeSet;

            // find shape
            const ShapeGroup* pGroup = m_xSequence.m_sGroups;
            for (unsigned i = 0; i < 3; ++i)
            {
                if (dStartTime < pGroup->sGroupDuration.dTime)
                    break;

                // take current shape into account
                dStartTime -= pGroup->sGroupDuration.dTime;
                m_dBaseTime += pGroup->sGroupDuration.dTime;
                m_uFifoFillState += pGroup->uNumShapeSamples * pGroup->uNumShapeRepetitions;
                ++pGroup;
                ++m_uShape;
            }

            // check if shape or shape end
            Ticks dEndTime = pGroup->sShapeStates.dTime
                             * pGroup->uNumShapeRepetitions;

            if (dStartTime < dEndTime)
            {
                m_uShapeRepetition = uint16_t(dStartTime / pGroup->sShapeStates.dTime);
                dStartTime -= pGroup->sShapeStates.dTime * m_uShapeRepetition;
                m_dBaseTime += pGroup->sShapeStates.dTime * m_uShapeRepetition;
                m_uFifoFillState += pGroup->uNumShapeSamples * m_uShapeRepetition;
                m_eStage = Stage_e::Shape;
                pStateList = &pGroup->sShapeStates;
            }
            else
            {
                dStartTime -= dEndTime;
                m_dBaseTime += dEndTime;
                m_uFifoFillState += pGroup->uNumShapeSamples * pGroup->uNumShapeRepetitions;

                pStateList = &pGroup->sShapeEndStates;
                m_eStage = Stage_e::ShapeEnd;
            }
        }

        // do a binary search to find the right step
        size_t uSearchStart = 0;
        size_t uSearchEnd = pStateList->size() - 1;

        while (uSearchEnd > uSearchStart)
        {
            size_t uStepToCheck = (uSearchStart + uSearchEnd) >> 1;
            Ticks dStepStart = (*pStateList)[uStepToCheck].dStartTime;
            Ticks dStepEnd = dStepStart + (*pStateList)[uStepToCheck].dDuration;

            if (dStartTime < dStepStart)
                uSearchEnd = uStepToCheck - 1;
            else if (dStartTime > dStepEnd)
                uSearchStart = uStepToCheck + 1;
            else
            {
                uSearchStart = uStepToCheck;
                break;
            }
        }

        m_uSeqIndex = uint16_t(uSearchStart);
        m_pCurrentState = &(*pStateList)[m_uSeqIndex];

        if (m_eStage == Stage_e::Shape)
        {
            for (size_t i = 0; i < m_uSeqIndex; ++i)
                m_uFifoFillState += (*pStateList)[i].uNumSamples;
        }
        m_uFifoFillState %= m_xSequence.getFifoThreshold();
    }
}

// ---------------------------------------------------------------------------- Iterator::operator++
void StateSequence::Iterator::operator++()
{
    const ShapeGroup& sCurrentGroup = m_xSequence.m_sGroups[m_uShape];

    switch (m_eStage)
    {
        case Stage_e::Prefix:
            ++m_uSeqIndex;
            if (m_uSeqIndex >= m_xSequence.m_sPrefixStates.size())
            {
                m_uSeqIndex = 0;
                m_dBaseTime = m_xSequence.m_sPrefixStates.dTime;
                m_eStage = Stage_e::Shape;
            }
            break;

        case Stage_e::Shape:
            if (m_pCurrentState->uNumSamples > 0)
            {
                m_uFifoFillState += m_pCurrentState->uNumSamples;
                m_uFifoFillState %= m_xSequence.getFifoThreshold();
            }
            ++m_uSeqIndex;
            if (m_uSeqIndex >= sCurrentGroup.sShapeStates.size())
            {
                m_uSeqIndex = 0;
                m_dBaseTime += sCurrentGroup.sShapeStates.dTime;
                ++m_uShapeRepetition;
                if (m_uShapeRepetition >= sCurrentGroup.uNumShapeRepetitions)
                {
                    m_uShapeRepetition = 0;
                    m_eStage = (m_dBaseTime >= m_xSequence.m_sActiveFrameDuration.dTime) ? Stage_e::FrameEnd : Stage_e::ShapeEnd;
                }
            }
            break;

        case Stage_e::ShapeEnd:
            ++m_uSeqIndex;
            if (m_uSeqIndex >= m_xSequence.m_sGroups[m_uShape].sShapeEndStates.size())
            {
                m_uSeqIndex = 0;
                m_dBaseTime += sCurrentGroup.sGroupDuration.dTime
                               - (sCurrentGroup.sShapeStates.dTime
                                  * sCurrentGroup.uNumShapeRepetitions);
                m_eStage = Stage_e::Shape;
                ++m_uShape;
                if ((m_uShape == 4)
                    || (m_xSequence.m_sGroups[m_uShape].uNumShapeRepetitions == 0))
                {
                    m_uShape = 0;
                    ++m_uShapeSet;
                }
            }
            break;

        case Stage_e::FrameEnd:
            m_eStage = Stage_e::EndOfSequence;
            m_dBaseTime += m_xSequence.m_sEndState.dDuration;
            break;

        case Stage_e::EndOfSequence:
            // nothing to do, the iterator can't go beyond the end of the sequence
            break;
    }
    updateStatePointer();
}

// ---------------------------------------------------------------------------- Iterator::advanceToGroupEnd
void StateSequence::Iterator::advanceToGroupEnd()
{
    if (m_eStage == Stage_e::Shape)
    {
        const ShapeGroup& sCurrentGroup = m_xSequence.m_sGroups[m_uShape];

        if (m_uSeqIndex == 0)
        {
            m_uFifoFillState += sCurrentGroup.uNumShapeSamples
                                * (sCurrentGroup.uNumShapeRepetitions - m_uShapeRepetition);
        }
        else
        {
            for (uint16_t i = m_uSeqIndex; i < sCurrentGroup.sShapeStates.size(); ++i)
                m_uFifoFillState += sCurrentGroup.sShapeStates[i].uNumSamples;
            m_uFifoFillState += sCurrentGroup.uNumShapeSamples
                                * (sCurrentGroup.uNumShapeRepetitions - (m_uShapeRepetition + 1));
        }
        m_uFifoFillState %= m_xSequence.getFifoThreshold();

        m_dBaseTime += sCurrentGroup.sShapeStates.dTime
                       * (sCurrentGroup.uNumShapeRepetitions - m_uShapeRepetition);
        m_uSeqIndex = 0;
        m_uShapeRepetition = 0;

        m_eStage = (m_dBaseTime >= m_xSequence.m_sActiveFrameDuration.dTime) ? Stage_e::FrameEnd : Stage_e::ShapeEnd;

        updateStatePointer();
    }
}

// ---------------------------------------------------------------------------- Iterator::advanceToShapeEnd
void StateSequence::Iterator::advanceToShapeEnd()
{

    if (m_eStage == Stage_e::Shape)
    {
        const ShapeGroup& sCurrentGroup = m_xSequence.m_sGroups[m_uShape];

        if (m_uSeqIndex == 0)
        {
            m_uFifoFillState += sCurrentGroup.uNumShapeSamples;
        }
        else
        {
            for (uint16_t i = m_uSeqIndex; i < sCurrentGroup.sShapeStates.size(); ++i)
                m_uFifoFillState += sCurrentGroup.sShapeStates[i].uNumSamples;
        }
        m_uFifoFillState %= m_xSequence.getFifoThreshold();

        m_uSeqIndex = 0;
        m_dBaseTime += sCurrentGroup.sShapeStates.dTime;
        ++m_uShapeRepetition;
        if (m_uShapeRepetition >= sCurrentGroup.uNumShapeRepetitions)
        {
            m_uShapeRepetition = 0;
            m_eStage = (m_dBaseTime >= m_xSequence.m_sActiveFrameDuration.dTime) ? Stage_e::FrameEnd : Stage_e::ShapeEnd;
        }
    }
    updateStatePointer();
}

// ---------------------------------------------------------------------------- Iterator::updateStatePointer
void StateSequence::Iterator::updateStatePointer()
{
    // remember a short cut to the current state
    switch (m_eStage)
    {
        case Stage_e::Prefix:
            m_pCurrentState = &m_xSequence.m_sPrefixStates[m_uSeqIndex];
            break;

        case Stage_e::Shape:
            m_pCurrentState = &m_xSequence.m_sGroups[m_uShape].sShapeStates[m_uSeqIndex];
            break;

        case Stage_e::ShapeEnd:
            m_pCurrentState = &m_xSequence.m_sGroups[m_uShape].sShapeEndStates[m_uSeqIndex];
            break;

        case Stage_e::FrameEnd:
        case Stage_e::EndOfSequence:
            m_pCurrentState = &m_xSequence.m_sEndState;
            break;
    }
}

/* ------------------------------------------------------------------------ */
}  // namespace TimingModel
}  // namespace Avian
}  // namespace Infineon

/* --- End of File -------------------------------------------------------- */
