/**
 * @file ModelBGT60TR11D.hpp
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

#ifndef IFX_AVIAN_MODELBGT60TR11D_HPP
#define IFX_AVIAN_MODELBGT60TR11D_HPP

// ---------------------------------------------------------------------------- includes
#include "ModelBGT60TRxxD.hpp"

// ---------------------------------------------------------------------------- namespaces
namespace Infineon {
namespace Avian {
namespace TimingModel {

// ---------------------------------------------------------------------------- ModelBGT60TR11D
class ModelBGT60TR11D : public ModelBGT60TRxxD
{
public:
    ModelBGT60TR11D(const SequenceParameters11D& sParameters,
                    bool bIgnoreRepetitions);

private:
    const SequenceParameters11D& m_sParameters11D;

    void gotoStatePAEN(TimerList_t& aTimers) override;
    void gotoStateSSTART(TimerList_t& aTimers) override;
    void gotoStateWaitForPll(TimerList_t& aTimers) override;
    void gotoStateEDU(TimerList_t& aTimers) override;
    void gotoStateEDD(TimerList_t& aTimers) override;

    void startPowerMeasurement(TimerList_t& aTimers);
    void startTemperatureMeasurement(TimerList_t& aTimers);
};

/* ------------------------------------------------------------------------ */
}  // namespace TimingModel
}  // namespace Avian
}  // namespace Infineon

#endif  // IFX_AVIAN_MODELBGT60TR11D_HPP

/* --- End of File -------------------------------------------------------- */
