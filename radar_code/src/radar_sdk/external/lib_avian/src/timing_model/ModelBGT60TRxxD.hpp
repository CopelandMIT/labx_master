/**
 * @file ModelBGT60TRxxD.hpp
 */
/* ===========================================================================
** Copyright (C) 2016-2023 Infineon Technologies AG
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

#ifndef IFX_AVIAN_MODELBGT60TRXXD_HPP
#define IFX_AVIAN_MODELBGT60TRXXD_HPP

// ---------------------------------------------------------------------------- includes
#include "ModelBGT60TRxxC.hpp"

// ---------------------------------------------------------------------------- namespaces
namespace Infineon {
namespace Avian {
namespace TimingModel {

//----------------------------------------------------------------------------- ModelBGT60TRxxD
class ModelBGT60TRxxD : public ModelBGT60TRxxC
{
public:
    ModelBGT60TRxxD(const SequenceParametersD& sParameters,
                    bool bIgnoreRepetitions);

private:
    const SequenceParametersD& m_sParametersD;

    void gotoStateWU(TimerList_t& aTimers) override;
    void gotoStateINIT0(TimerList_t& aTimers) override;
    void gotoStateINIT1(TimerList_t& aTimers) override;

    static const PowerConsumptionTable s_sPowerConsumptionTableBGT60UTR13D;
};

/* ------------------------------------------------------------------------ */
}  // namespace TimingModel
}  // namespace Avian
}  // namespace Infineon

#endif  // IFX_AVIAN_MODELBGT60TRXXD_HPP

/* --- End of File -------------------------------------------------------- */
