/**
 * @file ModelBGT60TRxxC.hpp
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

#ifndef IFX_AVIAN_BGT60TRXXC_HPP
#define IFX_AVIAN_BGT60TRXXC_HPP

// ---------------------------------------------------------------------------- includes
#include "SequenceParameters.hpp"
#include <cstdint>
#include <functional>
#include <list>
#include <string>

// ---------------------------------------------------------------------------- namespaces
namespace Infineon {
namespace Avian {
namespace TimingModel {

// ---------------------------------------------------------------------------- ModelBGT60TRxxC
class ModelBGT60TRxxC
{
public:
    ModelBGT60TRxxC(const SequenceParameters& sParameters,
                    bool bIgnoreRepetitions);
    virtual ~ModelBGT60TRxxC() = default;

    void startFrame();
    int64_t goToNextState();

    PowerMode getPowerMode() const;
    unsigned getNumSamples() const;
    uint8_t getShapeNumber() const;
    uint16_t getShapeRepetition() const;
    uint16_t getShapeSetRepetition() const;

    bool hasFrameFinished() const;

    std::string getStateDescription() const;
    std::string getAssociatedBitfield() const;
    bool isVirtualState() const;
    bool isStartOfShape() const;
    bool isShapeEndDelay() const;

    double getCurrentFrequency() const;
    double getFrequencyIncrement() const;

    double getPowerConsumption() const;
    double getPowerConsumption(PowerMode ePowerMode) const;

protected:
    struct PowerConsumptionTable
    {
        double dPowerDeepSleep;                /* Power consumption in Watts for Deep Sleep Mode */
        double dPowerDeepSleepContinue;        /* Power consumption in Watts for Deep Sleep & Continue Mode */
        double dPowerDeepSleepContinueDoubler; /* Power consumption in Watts for Deep Sleep & Continue Mode with Frequency Doubler */
        double dPowerIdle;                     /* Power consumption in Watts for Idle Mode */
        double dPowerInit0;                    /* Power consumption in Watts for Interchirp Mode during Init0 state */
        double dPowerInterchirp;               /* Power consumption in Watts for Interchirp Mode */
        double dPowerActive;                   /* Power consumption in Watts for Active Mode */
    };

    ModelBGT60TRxxC(const SequenceParameters& sParameters, bool bIgnoreRepetitions,
                    const PowerConsumptionTable& sPowerConsumptionTable);

    const ShapeSettings& getCurrentShapeSettings() const;
    const ChirpSettings& getCurrentChirpSettings() const;

    struct Timer
    {
        typedef std::function<void(std::list<Timer>&)> ExpireFunction_t;

        Timer(std::string sDescription_, std::string sBitfields_,
              uint64_t uCounter_) :
            sDescription(std::move(sDescription_)),
            sBitfields(std::move(sBitfields_)),
            uCounter(uCounter_)
        {}

        Timer(std::string sDescription_, std::string sBitfields_,
              uint64_t uCounter_, ExpireFunction_t fnOnExpire_) :
            sDescription(std::move(sDescription_)),
            sBitfields(std::move(sBitfields_)),
            uCounter(uCounter_),
            fnOnExpire(std::move(fnOnExpire_))
        {}

        std::string sDescription;
        std::string sBitfields;
        uint64_t uCounter;
        ExpireFunction_t fnOnExpire;
    };
    typedef std::list<Timer> TimerList_t;

    const SequenceParameters& m_sParameters;
    bool m_bIgnoreRepetitions;

    // ---------------------------------------------
    // The following members represent the FSM state
    // ---------------------------------------------
    enum class FsmState_e
    {
        Ready,
        Wakeup,
        Init0,
        Init1,
        StartOfShape,  // this is not a real state, it just adds a shape headline to the table
        PaDelay,
        AdcDelay,
        Sampling,
        WaitForPll,
        ChirpEndDelay,
        ShapeEndDelay,
        FrameEndDelay,
        FrameEnded
    } m_eStateFsm;

    PowerMode m_ePowerMode;
    uint8_t m_uShape;
    bool m_bDownChirp;
    uint16_t m_uShapeRepetition;
    uint16_t m_uShapeSetRepetition;
    uint16_t m_uShapeGroupCounter;

    // ---------------------------------------------
    // The following members represent the PLL state
    // ---------------------------------------------
    enum class PllState_e
    {
        Off,
        Locking,
        Ready,
        PreDelay,
        RampInProgress,
        FastDownRamp,
        PostDelay,
        RampFinished
    } m_eStatePll;
    double m_dCurrentFrequency;

    static const uint64_t s_uLockTime;

    // ---------------------------------------------------------
    // The following methods implement the FSM state transitions
    // ---------------------------------------------------------
    virtual void gotoStateWU(TimerList_t& aTimers);
    virtual void gotoStateINIT0(TimerList_t& aTimers);
    virtual void gotoStateINIT1(TimerList_t& aTimers);
    virtual void insertShapeHeader(TimerList_t& aTimers);
    virtual void gotoStatePAEN(TimerList_t& aTimers);
    virtual void gotoStateSSTART(TimerList_t& aTimers);
    virtual void gotoStateACQ(TimerList_t& aTimers);
    virtual void gotoStateWaitForPll(TimerList_t& aTimers);
    virtual void gotoStateEDU(TimerList_t& aTimers);
    virtual void gotoStateEDD(TimerList_t& aTimers);
    virtual void gotoStateSED(TimerList_t& aTimers);
    virtual void gotoStateFED(TimerList_t& aTimers);

    // ---------------------------------------------------------
    // The following methods implement the PLL state transitions
    // ---------------------------------------------------------
    void startPllLocking(TimerList_t& aTimers);
    void startPllPreDelay(TimerList_t& aTimers);
    void startPllRamp(TimerList_t& aTimers);
    void startPllFastDownRamp(TimerList_t& aTimers);
    void startPllPostDelay(TimerList_t& aTimers);

private:
    const PowerConsumptionTable& m_sPowerConsumptionTable;
    TimerList_t m_aTimers;

    static const PowerConsumptionTable s_sPowerConsumptionTableBGT60TR13C;
    static const PowerConsumptionTable s_sPowerConsumptionTableBGT60ATR24C;
};

/* ------------------------------------------------------------------------ */
}  // namespace TimingModel
}  // namespace Avian
}  // namespace Infineon

#endif  // IFX_AVIAN_BGT60TRXXC_HPP

/* --- End of File -------------------------------------------------------- */
