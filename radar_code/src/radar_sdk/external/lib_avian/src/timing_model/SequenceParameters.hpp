/**
 * @file SequenceParameters.hpp
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

#ifndef IFX_AVIAN_SEQUENCEPARAMETERS_HPP
#define IFX_AVIAN_SEQUENCEPARAMETERS_HPP

// ---------------------------------------------------------------------------- includes
#include "ifxAvian_TimingModel.hpp"
#include <cstdint>

namespace Infineon {
namespace Avian {
namespace HW {

class RegisterSet;

}  // namespace HW
}  // namespace Avian
}  // namespace Infineon
namespace Infineon {
namespace Avian {

enum class Device_Type;

}  // namespace Avian
}  // namespace Infineon

// ---------------------------------------------------------------------------- namespaces
namespace Infineon {
namespace Avian {
namespace TimingModel {

// ---------------------------------------------------------------------------- ChirpSettings
struct ChirpSettings
{
    double dStartFrequency;
    double dFrequencyInc;
    uint32_t uRampTime;
    uint16_t uNumSamples;
    uint16_t uChirpEndDelay;
    uint8_t uNumAntennas;
};

// ---------------------------------------------------------------------------- ShapeSettings
struct ShapeSettings
{
    ChirpSettings sUp;
    ChirpSettings sDown;
    uint16_t uNumRepetitions;
    uint64_t uShapeEndDelay;
    PowerMode eShapeEndPowerMode;
    bool bFastDownRamp;
};

// ---------------------------------------------------------------------------- SequenceParameters
/**
 * This class contains all timing relevant parameters of BGT60TRxxC.
 */
class SequenceParameters
{
public:
    double dPllMinFrequency;
    double dPllMaxFrequency;
    uint16_t uFifoThreshold;
    uint16_t uFastDownTime;
    uint32_t uWakeUpTime;
    uint16_t uInit0Time;
    uint16_t uInit1Time;
    uint16_t uAdcDivider;
    uint16_t uPreChirpDelay;
    uint16_t uPostChirpDelay;
    uint16_t uPaDelay;
    uint16_t uAdcDelay;
    uint16_t uNumShapeGroupsPerFrame;
    uint64_t uFrameEndDelay;
    PowerMode eFrameEndPowerMode;
    ShapeSettings sShape[4];
    unsigned uFifoSize;
    bool bATR24C;

    SequenceParameters(const Infineon::Avian::HW::RegisterSet& registers,
                       Infineon::Avian::Device_Type device_type,
                       double ref_frequency);

    /*
     * Make this a polymorphic type, because some user classes may want to tell
     * base type from derived types.
     */
    virtual ~SequenceParameters() = default;
};

// ---------------------------------------------------------------------------- SequenceParametersD
/**
 * This class contains all timing relevant parameters of BGT60TRxxD.
 * Basically the parameters are the same as they are for BGT60TRxxC but a few
 * parameters were added.
 */
class SequenceParametersD : public SequenceParameters
{
public:
    uint8_t uStartupDelayPllDivider;
    uint16_t uStartupDelayBandgap;
    uint16_t uStartupDelayMadc;
    uint16_t uStartupDelayPllEnable;
    bool bRefDoublerActive;

    SequenceParametersD(const Infineon::Avian::HW::RegisterSet& registers,
                        Infineon::Avian::Device_Type device_type,
                        double ref_frequency);
};

// ---------------------------------------------------------------------------- SequenceParameters11D
/**
 * This class contains all timing relevant parameters of BGT60TR11D.
 * Basically the parameters are the same as they are for BGT60TRxxD but a few
 * parameters were added.
 */
class SequenceParameters11D : public SequenceParametersD
{
public:
    uint16_t uMadcSwitchTime;
    uint16_t uPowerSensDelay;
    uint16_t uMadcAcquisitionTime;
    bool bPowerSensEnabled[4];
    bool bTemperatureSensEnabled[4];

    SequenceParameters11D(const Infineon::Avian::HW::RegisterSet& registers,
                          Infineon::Avian::Device_Type device_type,
                          double ref_frequency);
};

/* ------------------------------------------------------------------------ */
}  // namespace TimingModel
}  // namespace Avian
}  // namespace Infineon

#endif  // IFX_AVIAN_SEQUENCEPARAMETERS_HPP

/* --- End of File -------------------------------------------------------- */
