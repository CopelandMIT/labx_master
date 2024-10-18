/**
 * @file ModelBGT60TR11D.cpp
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
#include "ModelBGT60TR11D.hpp"

// ---------------------------------------------------------------------------- namespaces
namespace Infineon {
namespace Avian {
namespace TimingModel {

// ---------------------------------------------------------------------------- ModelBGT60TR11D
ModelBGT60TR11D::ModelBGT60TR11D(const SequenceParameters11D& sParameters,
                                 bool bIgnoreRepetitions) :
    ModelBGT60TRxxD(sParameters, bIgnoreRepetitions),
    m_sParameters11D(sParameters)
{}

// ---------------------------------------------------------------------------- gotoStatePAEN
void ModelBGT60TR11D::gotoStatePAEN(TimerList_t& aTimers)
{
    ModelBGT60TRxxD::gotoStatePAEN(aTimers);

    if (m_sParameters11D.bPowerSensEnabled[m_uShape])
        aTimers.emplace_back("Switch Psens -> MADC", "T_SW", m_sParameters11D.uMadcSwitchTime);
}

// ---------------------------------------------------------------------------- gotoStateSSTART
void ModelBGT60TR11D::gotoStateSSTART(TimerList_t& aTimers)
{
    ModelBGT60TRxxD::gotoStateSSTART(aTimers);

    if (m_sParameters11D.bPowerSensEnabled[m_uShape])
    {
        aTimers.emplace_back("Power Sensing Delay", "T_PSSTART", m_sParameters11D.uPowerSensDelay,
                             [this](TimerList_t& aTimers) -> void {
                                 startPowerMeasurement(aTimers);
                             });
    }
}

// ---------------------------------------------------------------------------- gotoStateWaitForPll
void ModelBGT60TR11D::gotoStateWaitForPll(TimerList_t& aTimers)
{
    ModelBGT60TRxxD::gotoStateWaitForPll(aTimers);

    if (m_sParameters11D.bTemperatureSensEnabled[m_uShape]
        && (m_bDownChirp != getCurrentShapeSettings().bFastDownRamp))
    {
        aTimers.emplace_back("Switch Ptemp -> MADC", "T_SW", m_sParameters11D.uMadcSwitchTime);
    }
}

// ---------------------------------------------------------------------------- gotoStateEDU
void ModelBGT60TR11D::gotoStateEDU(TimerList_t& aTimers)
{
    ModelBGT60TRxxD::gotoStateEDU(aTimers);

    if (m_sParameters11D.bTemperatureSensEnabled[m_uShape]
        && (m_bDownChirp != getCurrentShapeSettings().bFastDownRamp))
    {
        startTemperatureMeasurement(aTimers);
    }
}

// ---------------------------------------------------------------------------- gotoStateEDD
void ModelBGT60TR11D::gotoStateEDD(TimerList_t& aTimers)
{
    ModelBGT60TRxxD::gotoStateEDD(aTimers);

    if (m_sParameters11D.bTemperatureSensEnabled[m_uShape]
        && (!getCurrentShapeSettings().bFastDownRamp))
    {
        startTemperatureMeasurement(aTimers);
    }
}

// ---------------------------------------------------------------------------- startPowerMeasurement
void ModelBGT60TR11D::startPowerMeasurement(TimerList_t& aTimers)
{
    aTimers.emplace_back("Power Sensing", "T_CONV",
                         m_sParameters11D.uMadcAcquisitionTime,
                         [this](TimerList_t& aTimers) -> void {
                             aTimers.emplace_back("Switch RX -> MADC", "T_SW",
                                                  m_sParameters11D.uMadcSwitchTime);
                         });
}

// ---------------------------------------------------------------------------- startTemperatureMeasurement
void ModelBGT60TR11D::startTemperatureMeasurement(TimerList_t& aTimers)
{
    aTimers.emplace_back("Temperature Sensing", "T_CONV",
                         m_sParameters11D.uMadcAcquisitionTime,
                         [this](TimerList_t& aTimers) -> void {
                             aTimers.emplace_back("Switch RX -> MADC", "T_SW",
                                                  m_sParameters11D.uMadcSwitchTime);
                         });
}

/* ------------------------------------------------------------------------ */
}  // namespace TimingModel
}  // namespace Avian
}  // namespace Infineon

/* --- End of File -------------------------------------------------------- */
