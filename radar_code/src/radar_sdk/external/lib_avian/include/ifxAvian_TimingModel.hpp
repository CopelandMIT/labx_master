/**
 * @file ifxAvian_TimingModel.hpp
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

#ifndef IFX_AVIAN_TIMING_MODEL_HPP
#define IFX_AVIAN_TIMING_MODEL_HPP

// ---------------------------------------------------------------------------- includes
#include <cstdint>
#include <ostream>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------- namespaces
namespace Infineon {
namespace Avian {

class Driver;
enum class Device_Type;

namespace HW {

class RegisterSet;

}  // namespace HW

namespace TimingModel {

class ModelBGT60TRxxC;
struct ShapeSettings;

// ---------------------------------------------------------------------------- Ticks
/** Timing based on oscillator clock ticks */
using Ticks = double;

// ---------------------------------------------------------------------------- PowerMode
enum class PowerMode
{
    Deep_Sleep,
    Deep_Sleep_Continue,
    Idle,
    Interchirp,
    Active
};

// ---------------------------------------------------------------------------- StateSequence
class StateSequence
{
public:
    using Range = std::pair<double, double>;
    using TickRange = std::pair<Ticks, Ticks>;
    class Iterator;

    explicit StateSequence(const Driver& driver);
    StateSequence(const HW::RegisterSet& registers, Device_Type device_type,
                  double ref_frequency = 80.0e6);

    /**
     * @brief This function returns the total number of step in the sequence.
     *
     * @return The number of steps in the sequence
     */
    size_t getNumStates() const;

    /**
     * @brief This functions returns the time range of the sequence.
     *
     * @return A pair containing the minimum and maximum time of
     *         the range in clock cycles
     */
    TickRange getTotalTimeRange() const;

    /**
     * @brief This functions returns the supported RF frequency range.
     *
     * @return A pair containing the minimum and maximum supported
     *         RF frequency in Hz.
     */
    Range getTotalFrequencyRange() const;


    /**
     * @brief This functions returns the index of that step in the sequence, that
     *        contains the specified point in time.
     *
     * @param[in] dTime      The point in time that is searched for in the
     *                       sequence. The time is specified in clock cycles.
     *
     * @return The index of the searched step in the sequence.
     */
    size_t findSequenceStep(Ticks dTime) const;

    /**
     * @brief This functions returns the FIFO threshold of the sequence.
     *
     * @return When this number of samples are present in the FIFO memory,
     *         BGT60TRxx issues an IRQ to request a data read.
     */
    unsigned getFifoThreshold() const
    {
        return m_uFifoThreshold;
    }

    /**
     * @brief This function returns the ADC time  to capture a single value.
     *
     * @return The number of clock cycles needed by the ADC to sample and convert
     *         a single value. Dividing the clock oscillator frequency by this
     *         value results in the ADC sampling rate.
     */
    unsigned getNumAdcCycles() const
    {
        return m_uAdcCycles;
    }

    /**
     * @brief This function returns the amount of time spent in active mode during a frame.
     *
     * @return The active time inside a frame
     */
    double getFrameActiveDuration() const
    {
        return m_dActiveStateTotalDuration;
    }

    /**
     * @brief This function returns the average power consumption during a frame.
     *
     * @return The active time inside a frame
     */
    double getFrameAveragePowerConsumption() const
    {
        return m_dAveragePowerConsumption;
    }

    /**
     * @brief This function returns the time period between two consecutive
     *        chirps.
     *
     * @param[in] uShape     The index of the shape of interest.
     *                       Valid range is 0...3.
     *
     * @return The time period in clock cycles between the start of one chirp
     *         and the start of the following chirp of the same shape. This
     *         is the reciprocal of the repetition rate in a shape group.
     */
    Ticks getChirpToChirpTime(unsigned uShape) const;

    /**
     * @brief This function returns the time period between two consecutive
     *        shape sets.
     *
     * @return The time period in clock cycles between the start of one shape set
     *         and the start of the following shape set. This is the reciprocal of
     *         the repetition rate of the shape set.
     */
    Ticks getSetToSetTime() const;

    /**
     * @brief This function returns the time peried between the two consecutive
     *        frames.
     *
     * @return The time period in clock cycles between the start of one frame
     *         and the start of the following frame. This is the reciprocal of
     *         the frame rate.
     */
    Ticks getFrameDuration() const;

    /**
     * @brief This function writes the complete state sequence to a stream.
     *
     * @param[in] sOutStream  The stream the sequence should be written to.
     */
    void exportSequenceTable(std::ostream& sOutStream) const;

    /**
     * @brief This function returns the full size of the FIFO memory.
     *
     * @return The FIFO size in number of samples.
     */
    unsigned getFifoSize() const
    {
        return m_uFifoSize;
    }

    /**
     * @brief Convert Ticks into seconds.
     *
     * @param[in] ticks    the number of ticks (clock cycles)
     *
     * @return the time in seconds corresponding to the specified number of ticks
     *
     */
    inline double toSeconds(Ticks ticks) const
    {
        return ticks / m_dOscFrequency;
    }

private:
    struct Duration
    {
        Ticks dTime = 0.0;
        size_t uNumStates = 0;
    };

    struct StateInfo
    {
        Ticks dStartTime;
        Ticks dDuration;
        std::string sDescription;
        std::string sAssociatedBitfield;
        PowerMode ePowerMode;
        unsigned uNumSamples;
        double dStartFrequency;
        double dEndFrequency;
        bool bHeadline;
        double dPower;
    };
    typedef std::vector<StateInfo> StateListBase;

    class StateList : public StateListBase, public Duration
    {
    public:
        using StateListBase::StateListBase;

        StateInfo& addState(Ticks dCurrentTime, const ModelBGT60TRxxC& xFsm);
        StateInfo& addHeadline(Ticks dCurrentTime, const std::string& sDescription);
        void finishSequence(Ticks dCurrentTime);
        Ticks getActiveDuration() const
        {
            return dActiveDuration;
        }
        double getAveragePower() const
        {
            return dAveragePower;
        }
        double getTotalWork() const
        {
            return dTotalWork;
        }
        Ticks getTotalDuration() const
        {
            return dTotalDuration;
        }

    private:
        StateInfo& addStateTransition(Ticks dCurrentTime, const StateInfo& sState);
        void extendLastStateUntil(Ticks dCurrentTime);
        void updateStatistics(); /* update average power and time spend in active modes */

        double dCurrentFrequencyIncrement = 0.0;
        Ticks dActiveDuration = 0.0;
        double dTotalWork = 0.0;
        Ticks dTotalDuration = 0.0;
        double dAveragePower = 0.0;
    };

    struct ShapeGroup
    {
        void addShapeState(Ticks dCurrentTime, const ModelBGT60TRxxC& xFsm);
        void finishShapeStates(Ticks dCurrentTime, const ShapeSettings& sShapeSetting);
        void addShapeEndState(Ticks dCurrentTime, const ModelBGT60TRxxC& xFsm);
        void finishShapeEndStates(Ticks dCurrentTime);
        Ticks getActiveDuration() const;
        Ticks getTotalDuration() const;
        double getTotalWork() const;

        Duration sGroupDuration;
        unsigned uNumShapeRepetitions = 0;
        StateList sShapeStates;
        StateList sShapeEndStates;
        unsigned uNumShapeSamples = 0;
    };

    StateList m_sPrefixStates;

    unsigned m_uNumSetSamples;
    Duration m_sSetDuration;
    ShapeGroup m_sGroups[4];

    unsigned m_uNumTotalSamples;
    Duration m_sActiveFrameDuration;
    StateInfo m_sEndState;

    unsigned m_uFifoThreshold;
    unsigned m_uAdcCycles;

    double m_dOscFrequency;
    Range m_jTotalFrequencyRange;

    Ticks m_dActiveStateTotalDuration = 0.0; /* Total Time spent in active state during a frame */
    double m_dAveragePowerConsumption = 0.0; /* Average Power Consumption during a frame */
    unsigned m_uFifoSize;
};

// ---------------------------------------------------------------------------- StateSequence::Iterator
class StateSequence::Iterator
{
public:
    /**
     * @brief Creates new state iterator pointing to
     *        the beginning of the sequence
     *
     * This constructor creates a new state sequence iterator object that points
     * to the first step of the provided state sequence.
     *
     * The iterator object keeps a reference to the provided sequence object
     * so the state object must not be destroyed before the iterator object
     * is destroyed.
     *
     * @param[in]  xSequence  The sequence object the new iterator object will
     *                        refer to.
     */
    explicit Iterator(const StateSequence& xSequence);

    /**
     * @brief Creates a new state iterator object pointing to
     *        a specified step
     *
     * This constructor creates a new state sequence iterator object that points
     * to the step of the provided state sequence indicated by index. If the
     * index exceeds the number of steps in the sequence, the iterator is
     * initialized to the last step of the sequence.
     *
     * The iterator object keeps a reference to the provided sequence object
     * so the state object must not be destroyed before the iterator object
     * is destroyed.
     *
     * @param[in]  xSequence  The sequence object the new iterator object will
     *                        refer to.
     * @param[in]  uIndex     The index of the state at which the iterator is
     *                        initialized.
     */
    Iterator(const StateSequence& xSequence, size_t uIndex);

    /**
     * @brief Creates a new state iterator object pointing to
     *        a specified point in time
     *
     * This function creates a new state sequence iterator object that points to
     * the step in the provided state sequence that contains the specified point
     * in time indicated by index. If the specified time exceeds the end time of
     * the sequence, the iterator is initialized to the last step of the sequence.
     *
     * The iterator object keeps a reference to the provided sequence object
     * so the state object must not be destroyed before the iterator object
     * is destroyed.
     *
     * @param[in]  xSequence   The sequence object the new iterator object will
     *                         refer to.
     * @param[in]  dStartTime  The point in time that should be covered by that
     *                         step of the sequence the iterator is initialized
     *                         to. This is specified in clock cycles.
     */
    Iterator(const StateSequence& xSequence, Ticks dStartTime);

    /**
     * @brief Advances the iterator to the next step.
     *
     * If the iterator has reached the last step of the sequence,
     * this function does nothing.
     */
    void operator++();

    /**
     * @brief This function advances the iterator to the last step of the current
     *        shape.
     *
     * This function advances the iterator by one or more steps until the end of
     * the current shape is reached. If the iterator has reached the last step of
     * the sequence, this function does nothing.
     */
    void advanceToShapeEnd();

    /**
     * @brief This function advances the iterator to the last step of the current
     *        shape group.
     *
     * This function advances the iterator by one or more steps until the end of
     * the current shape group is reached. If the iterator has reached the last
     * step of the sequence, this function does nothing.
     */
    void advanceToGroupEnd();

    /**
     * @brief This function checks if the iterator points to the end of the
     *        sequence
     *
     * @return True if the iterator points to last step of the sequence, false
     *         otherwise.
     */
    inline bool hasEnded() const;

    /**
     * @brief This function checks if the iterator points to the start of a
     *        shape.
     *
     * @return True if the iterator points to the first step of a shape, false
     *         otherwise.
     */
    inline bool isStartOfShape() const;

    /**
     * @brief This function checks if the iterator points to a
     *        headline shape.
     *
     * @return True if the iterator points to a headline shape, false
     *         otherwise.
     */
    inline bool isHeadline() const;

    /**
     * @brief This function checks if the current step is part of a shape
     *
     * @return True if the step the iterator is currently pointing to is part
     *         of a shape, false if that step is part of the wake up phase,
     *         shape end delay, or frame and delay. If iterator is pointing at
     *         the end of the sequence this is also false.
     */
    inline bool isActiveShape() const;

    /**
     * @brief This function returns the duration of the current shape.
     *
     * @return The duration in clock cycles of the shape the iterator is currently
     *         pointing to. This is not taking the shape repetitions into account.
     */
    inline Ticks getSingleShapeDuration() const;

    /**
     * @brief This function returns the duration of the current shape group.
     *
     * @return The duration in clock cycles of the shape group the iterator is
     *         currently pointing to. This includes all shape repetitions of the
     *         shape group.
     */
    inline Ticks getGroupStartTime() const;

    /**
     * @brief This function returns the number of repetitions of the current shape
     *
     * @return The number of repetitions of the shape the iterator is currently
     *         pointing to.
     */
    inline unsigned getNumRepetitions() const;

    /**
     * @brief This function returns the start frequency of the current shape.
     *
     * @return The start frequency in Hz of the shape the iterator is currently
     *         pointing to.
     */
    inline double getShapeStartFrequency() const;

    /**
     * @brief This function returns the end frequency of the current shape.
     *
     *
     * @return The end frequency in Hz of the shape the iterator is currently
     *         pointing to.
     */
    inline double getShapeEndFrequency() const;

    /**
     * @brief This function checks if the current shape is a triangle shape.
     *
     * @return True if the iterator is currently pointing to a step in a
     *         triangle shape. False if that shape is a sawtooth shape.
     */
    inline bool isTriangleShape() const;

    /**
     * @brief This function returns the number of samples captured during the
     *        current shape.
     *
     * @return The number of samples captured during the complete shape (up chirp
     *         and down chirp) the iterator is currently pointing to.
     */
    inline unsigned getNumShapeSamples() const;

    /**
     * @brief This function returns the start time of the current step.
     *
     * @return The start time in clock cycles of the step the iterator is
     *         currently pointing to.
     */
    inline Ticks getStartTime() const;

    /**
     * @brief This function returns the duration of the current step.
     *
     * @return The duration in clock cycles of the step the iterator is
     *         currently pointing to.
     */
    inline Ticks getDuration() const;

    /**
     * @brief This function returns the end time of the current step.
     *
     * @return The start end in clock cycles of the step the iterator is
     *         currently pointing to.
     */
    inline Ticks getEndTime() const;

    /**
     * @brief This functions returns a description of the current step
     *
     * This function generates a zero terminated string that describes the step
     * the iterator is currently pointing to.
     *
     * The caller must allocate a buffer and pass it to the function. The function
     * writes the description to that buffer. If the buffer is not too small, the
     * string is truncated.
     *
     * @return The description of the current step
     */
    std::string getDescription() const;

    /**
     * @brief This function returns a list of BGT60TRxx register bit fields that
     *        have an influence on the current steps
     *
     * The returned buffer is owned by this library and must not be freed by the
     * caller.
     *
     * @return A zero-terminated string listing the BGT60Trxx bit fields that
     *         influence the step the iterator is currently pointing to.
     */
    inline const std::string& getAssociatedBitfield() const;

    /**
     * @brief This function returns the power mode of the current step
     *
     * @return The power mode of the step, the iterator is currently pointing to.
     */
    inline PowerMode getPowerMode() const;

    /**
     * @brief This function returns the power consumption of the current step
     *
     * @return The power consumption of the current step (or NaN if not applicable for this step).
     */
    inline double getPowerConsumption() const;

    /**
     * @brief This function checks if the ADC is currently capturing
     *
     * @return True if the ADC is capturing sampling during the step the iterator
     *         is currently pointing to, false otherwise.
     */
    inline bool isSampling() const;

    /**
     * @brief This function returns the number of samples captured during the
     *        current step.
     *
     * @return The number of samples captured during the step the iterator is
     *         currently pointing to.
     */
    inline unsigned getNumSamples() const;

    /**
     * @brief This function returns the current repetition number of the whole
     *        shape set.
     *
     * @return The repetition number of the shape set the iterator is currently
     *         pointing to. The first shape set is 1.
     */
    inline unsigned getShapeSet() const;

    /**
     * @brief This function returns the index of the shape group within the
     *        shape set.
     *
     * @return The shape group index in the range of 1...4.
     */
    inline uint8_t getShape() const;

    /**
     * @brief This function returns the repetition number of the current shape.
     *
     * @return The repetition number of the shape the iterator is currently
     *         pointing to. The first shape within the shape group is 1.
     */
    inline unsigned getShapeRepetition() const;

    /**
     * @brief This function returns the frequency at the beginning of the current
     *        step.
     *
     * @return The frequency in Hz at the beginning of the step the iterator is
     *         currently pointing to.
     */
    inline double getStartFrequency() const;

    /**
     * @brief This function returns the frequency at the end of the current step.
     *
     * @return The frequency in Hz at the end of the step the iterator is
     *         currently pointing to.
     */
    inline double getEndFrequency() const;

    /**
     * @brief This function returns the FIFO fill state at the beginning of the
     *        current step.
     *
     * @return The number of samples in the FIFO at the beginning of the step the
     *         iterator is currently pointing to.
     */
    inline unsigned getFifoFillState() const;

private:
    void updateStatePointer();

    const StateSequence& m_xSequence;

    // current sequence position
    enum class Stage_e : uint8_t
    {
        Prefix,
        Shape,
        ShapeEnd,
        FrameEnd,
        EndOfSequence
    };
    Stage_e m_eStage;
    uint8_t m_uShape;
    uint16_t m_uShapeSet;
    uint16_t m_uShapeRepetition;
    uint16_t m_uSeqIndex;
    Ticks m_dBaseTime;
    unsigned m_uFifoFillState;
    const StateInfo* m_pCurrentState;

    Iterator operator=(const Iterator&) = delete;
};

// ---------------------------------------------------------------------------- StateSequence::Iterator::hasEnded
inline bool StateSequence::Iterator::hasEnded() const
{
    return (m_eStage == Stage_e::EndOfSequence);
}

// ---------------------------------------------------------------------------- StateSequence::Iterator::isStartOfShape
inline bool StateSequence::Iterator::isStartOfShape() const
{
    return (m_eStage == Stage_e::Shape) && (m_uSeqIndex == 0);
}

// ---------------------------------------------------------------------------- StateSequence::Iterator::isHeadline
inline bool StateSequence::Iterator::isHeadline() const
{
    return (m_pCurrentState != nullptr) && m_pCurrentState->bHeadline;
}

// ---------------------------------------------------------------------------- StateSequence::Iterator::isActiveShape
inline bool StateSequence::Iterator::isActiveShape() const
{
    return (m_eStage == Stage_e::Shape);
}

// ---------------------------------------------------------------------------- StateSequence::Iterator::getSingleShapeDuration
inline Ticks StateSequence::Iterator::getSingleShapeDuration() const
{
    return m_xSequence.m_sGroups[m_uShape].sShapeStates.dTime;
}

// ---------------------------------------------------------------------------- StateSequence::Iterator::getGroupStartTime
inline Ticks StateSequence::Iterator::getGroupStartTime() const
{
    return m_dBaseTime
           - m_xSequence.m_sGroups[m_uShape].sShapeStates.dTime
                 * m_uShapeRepetition;
}

// ---------------------------------------------------------------------------- StateSequence::Iterator::getNumRepetitions
inline unsigned StateSequence::Iterator::getNumRepetitions() const
{
    return m_xSequence.m_sGroups[m_uShape].uNumShapeRepetitions;
}

// ---------------------------------------------------------------------------- StateSequence::Iterator::getShapeStartFrequency
inline double StateSequence::Iterator::getShapeStartFrequency() const
{
    return m_xSequence.m_sGroups[m_uShape].sShapeStates[0].dStartFrequency;
}

// ---------------------------------------------------------------------------- StateSequence::Iterator::getShapeEndFrequency
inline double StateSequence::Iterator::getShapeEndFrequency() const
{
    return m_xSequence.m_sGroups[m_uShape].sShapeStates[0].dEndFrequency;
}

// ---------------------------------------------------------------------------- StateSequence::Iterator::isTriangleShape
inline bool StateSequence::Iterator::isTriangleShape() const
{
    return m_xSequence.m_sGroups[m_uShape].sShapeStates[0].sAssociatedBitfield == "Tri";
}

// ---------------------------------------------------------------------------- StateSequence::Iterator::getNumShapeSamples
inline unsigned StateSequence::Iterator::getNumShapeSamples() const
{
    return m_xSequence.m_sGroups[m_uShape].uNumShapeSamples;
}

// ---------------------------------------------------------------------------- StateSequence::Iterator::getStartTime
inline Ticks StateSequence::Iterator::getStartTime() const
{
    return m_pCurrentState->dStartTime + m_dBaseTime;
}

// ---------------------------------------------------------------------------- StateSequence::Iterator::getDuration
inline Ticks StateSequence::Iterator::getDuration() const
{
    return m_pCurrentState->dDuration;
}

// ---------------------------------------------------------------------------- StateSequence::Iterator::getEndTime
inline Ticks StateSequence::Iterator::getEndTime() const
{
    return m_dBaseTime + m_pCurrentState->dStartTime + m_pCurrentState->dDuration;
}

// ---------------------------------------------------------------------------- StateSequence::Iterator::getAssociatedBitfield
inline const std::string& StateSequence::Iterator::getAssociatedBitfield() const
{
    return m_pCurrentState->sAssociatedBitfield;
}

// ---------------------------------------------------------------------------- StateSequence::Iterator::getPowerMode
inline PowerMode StateSequence::Iterator::getPowerMode() const
{
    return m_pCurrentState->ePowerMode;
}

// ---------------------------------------------------------------------------- StateSequence::Iterator::getPowerConsumption
inline double StateSequence::Iterator::getPowerConsumption() const
{
    return m_pCurrentState->dPower;
}

// ---------------------------------------------------------------------------- StateSequence::Iterator::isSampling
inline bool StateSequence::Iterator::isSampling() const
{
    return m_pCurrentState->uNumSamples != 0;
}

// ---------------------------------------------------------------------------- StateSequence::Iterator::getNumSamples
inline unsigned StateSequence::Iterator::getNumSamples() const
{
    return m_pCurrentState->uNumSamples;
}

// ---------------------------------------------------------------------------- StateSequence::Iterator::getShapeSet
inline unsigned StateSequence::Iterator::getShapeSet() const
{
    unsigned uShapeSet = m_uShapeSet + 1;
    // wake up time of shape end officially belongs to the following state
    if ((m_eStage == Stage_e::ShapeEnd) && (m_uSeqIndex > 0)
        && ((m_uShape >= 3) || (m_xSequence.m_sGroups[m_uShape + 1].uNumShapeRepetitions == 0)))
        ++uShapeSet;
    return uShapeSet;
}

// ---------------------------------------------------------------------------- StateSequence::Iterator::getShape
inline uint8_t StateSequence::Iterator::getShape() const
{
    uint8_t uShape = m_uShape;
    if ((m_eStage == Stage_e::ShapeEnd) && (m_uSeqIndex > 0))
    {
        uShape = (uShape + 1) & 0x03;
        if (m_xSequence.m_sGroups[uShape].uNumShapeRepetitions == 0)
            uShape = 0;
    }
    return uShape + 1;
}

// ---------------------------------------------------------------------------- StateSequence::Iterator::getShapeRepetition
inline unsigned StateSequence::Iterator::getShapeRepetition() const
{
    return (m_eStage == Stage_e::Shape) ? m_uShapeRepetition + 1 : 0;
}

// ---------------------------------------------------------------------------- StateSequence::Iterator::getStartFrequency
inline double StateSequence::Iterator::getStartFrequency() const
{
    return m_pCurrentState->dStartFrequency;
}

// ---------------------------------------------------------------------------- StateSequence::Iterator::getEndFrequency
inline double StateSequence::Iterator::getEndFrequency() const
{
    return m_pCurrentState->dEndFrequency;
}

// ---------------------------------------------------------------------------- StateSequence::Iterator::getFifoFillState
inline unsigned StateSequence::Iterator::getFifoFillState() const
{
    return m_uFifoFillState;
}

/* ------------------------------------------------------------------------ */
}  // namespace TimingModel
}  // namespace Avian
}  // namespace Infineon

#endif  // IFX_AVIAN_TIMING_MODEL_HPP

/* --- End of File -------------------------------------------------------- */
