/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <components/interfaces/IProcessingRadar.hpp>
#include <universal/data_definitions.h>
#include <universal/types/IfxRfe_Types.h>
#include <universal/types/IfxRsp_Types.h>

#include <common/cpp11/iterator.hpp>


template <typename T, typename... Args>
constexpr size_t serialized_sizeof(T *, Args... args)
{
    return serialized_sizeof(T(), std::forward<Args>(args)...);
}


constexpr size_t serialized_sizeof(IfxRfe_MmicConfig)
{
    return sizeof(IfxRfe_MmicConfig().enableMonitoring) +
           sizeof(IfxRfe_MmicConfig().sampleWidth) +
           sizeof(IfxRfe_MmicConfig().sampleRate) +
           sizeof(IfxRfe_MmicConfig().txPower) +
           sizeof(IfxRfe_MmicConfig().lpGain) +
           sizeof(IfxRfe_MmicConfig().mixerGain) +
           sizeof(IfxRfe_MmicConfig().dcocEnable) +
           sizeof(IfxRfe_MmicConfig().dcocShift);
}

constexpr size_t serialized_sizeof(IfxRfe_Ramp)
{
    return sizeof(IfxRfe_Ramp().fStart) +
           sizeof(IfxRfe_Ramp().fDelta) +
           sizeof(IfxRfe_Ramp().loops) +
           sizeof(IfxRfe_Ramp().muxOut) +
           sizeof(IfxRfe_Ramp().txMask) +
           sizeof(IfxRfe_Ramp().txPhases);
}

constexpr size_t serialized_sizeof(IfxRfe_Sequence, uint8_t rampCount = 0)
{
    return rampCount * serialized_sizeof(IfxRfe_Ramp()) +
           sizeof(IfxRfe_Sequence().tRamp) +
           sizeof(IfxRfe_Sequence().tJump) +
           sizeof(IfxRfe_Sequence().tWait) +
           sizeof(IfxRfe_Sequence().tRampStartDelay) +
           sizeof(IfxRfe_Sequence().tRampStopDelay) +
           sizeof(IfxRfe_Sequence().rxMask) +
           sizeof(IfxRfe_Sequence().loops) +
           sizeof(IfxRfe_Sequence().rampCount);
}

constexpr size_t serialized_sizeof(IDataProperties_t)
{
    return sizeof(IDataProperties_t().format) +
           sizeof(IDataProperties_t().rxChannels) +
           sizeof(IDataProperties_t().ramps) +
           sizeof(IDataProperties_t().samples) +
           sizeof(IDataProperties_t().channelSwapping) +
           sizeof(IDataProperties_t().bitWidth);
}

constexpr size_t serialized_sizeof(IProcessingRadarInput_t)
{
    return sizeof(IProcessingRadarInput_t().txChannels) +
           sizeof(IProcessingRadarInput_t().virtualAnt) +
           sizeof(IProcessingRadarInput_t().rampsPerTx) +
           sizeof(IProcessingRadarInput_t().maxRange) +
           sizeof(IProcessingRadarInput_t().maxVelocity);
}

constexpr size_t serialized_sizeof(IfxRsp_FftSetting)
{
    return sizeof(IfxRsp_FftSetting().size) +  ///< 0 = default number of samples (= smallest power of 2 greater than or equal to number of samples)
           sizeof(IfxRsp_FftSetting().acceptedBins) +
           sizeof(IfxRsp_FftSetting().window) +
           sizeof(IfxRsp_FftSetting().windowFormat) +  ///< only Q15 (default) or Q31
           sizeof(IfxRsp_FftSetting().exponent) +      ///< scale result (only for 16bit output data format)
           sizeof(IfxRsp_FftSetting().flags);
}

constexpr size_t serialized_sizeof(IfxRsp_LocalMaxSetting)
{
    return sizeof(IfxRsp_LocalMaxSetting().mode) +
           sizeof(IfxRsp_LocalMaxSetting().threshold) +
           sizeof(IfxRsp_LocalMaxSetting().windowWidth) +
           sizeof(IfxRsp_LocalMaxSetting().combineAnd);
}

constexpr size_t serialized_sizeof(IfxRsp_CfarCaSetting)
{
    return sizeof(IfxRsp_CfarCaSetting().algorithm) +
           sizeof(IfxRsp_CfarCaSetting().guardCells) +
           sizeof(IfxRsp_CfarCaSetting().windowCellsExponent) +
           sizeof(IfxRsp_CfarCaSetting().cashSubWindowExponent) +
           sizeof(IfxRsp_CfarCaSetting().betaThreshold);
}

constexpr size_t serialized_sizeof(IfxRsp_CfarGosSetting)
{
    return sizeof(IfxRsp_CfarGosSetting().algorithm) +
           sizeof(IfxRsp_CfarGosSetting().guardCells) +
           sizeof(IfxRsp_CfarGosSetting().indexLead) +
           sizeof(IfxRsp_CfarGosSetting().indexLag) +
           sizeof(IfxRsp_CfarGosSetting().windowCells) +
           sizeof(IfxRsp_CfarGosSetting().betaThreshold);
}

constexpr size_t serialized_sizeof(IfxRsp_ThresholdingSetting)
{
    return sizeof(IfxRsp_ThresholdingSetting().spectrumExtension) +
           serialized_sizeof(IfxRsp_LocalMaxSetting()) +
           serialized_sizeof(IfxRsp_CfarCaSetting()) +
           serialized_sizeof(IfxRsp_CfarGosSetting());
}

constexpr size_t serialized_sizeof(IfxRsp_DetectionSettings)
{
    return sizeof(IfxRsp_DetectionSettings().maxDetections) +
           sizeof(IfxRsp_DetectionSettings().fftSize) +
           sizeof(IfxRsp_DetectionSettings().flags) +
           serialized_sizeof(IfxRsp_ThresholdingSetting()) * std::size(IfxRsp_DetectionSettings().thresholdingSettings);
}

constexpr size_t serialized_sizeof(IfxRsp_DbfSetting)
{
    return sizeof(IfxRsp_DbfSetting().angles) +             ///< number of angles to calculate
           sizeof(IfxRsp_DbfSetting().coefficientFormat) +  ///< only Complex16 or Complex32 (default)
           sizeof(IfxRsp_DbfSetting().format) +             ///< only Complex16 or Complex32 (default)
           sizeof(IfxRsp_DbfSetting().centerAngle) +        ///< [rad]
           sizeof(IfxRsp_DbfSetting().angularSpacing) +     ///< [rad]
           serialized_sizeof(IfxRsp_ThresholdingSetting());
}

constexpr size_t serialized_sizeof(IfxRsp_AntennaCalibration)
{
    return sizeof(IfxRsp_AntennaCalibration().spacing) +  ///< antenna spacing in multiples of lambda
           sizeof(IfxRsp_AntennaCalibration().temperature) +
           sizeof(IfxRsp_AntennaCalibration().fftSize) +      ///< fftSize used (0 = no Calibration)
           sizeof(IfxRsp_AntennaCalibration().count) +        ///< number of virtual antennae
           sizeof(IfxRsp_AntennaCalibration().indices) +      ///< virtual antenna ordering
           sizeof(IfxRsp_AntennaCalibration().coefficients);  ///< calibration coefficients as complex values (float[2])
}

constexpr size_t serialized_sizeof(IfxRsp_Stages)
{
    return sizeof(IfxRsp_Stages().fftSteps) +   ///< number of FFT stages enabled (0: time data; 1: Range FFT; 2: Doppler)
           sizeof(IfxRsp_Stages().fftFormat) +  ///< output data format for FFT, only Real* or Complex*
           sizeof(IfxRsp_Stages().nciFormat) +
           sizeof(IfxRsp_Stages().virtualChannels) +
           serialized_sizeof(IfxRsp_FftSetting()) * std::size(IfxRsp_Stages().fftSettings) +
           serialized_sizeof(IfxRsp_DetectionSettings()) +
           serialized_sizeof(IfxRsp_DbfSetting()) * std::size(IfxRsp_Stages().dbfSetting);
}

constexpr size_t serialized_sizeof(IfxRsp_Signal)
{
    return sizeof(IfxRsp_Signal().size) +
           sizeof(IfxRsp_Signal().baseAddress) +
           sizeof(IfxRsp_Signal().stride) +
           sizeof(IfxRsp_Signal().rows) +
           sizeof(IfxRsp_Signal().cols) +
           sizeof(IfxRsp_Signal().pages) +
           sizeof(IfxRsp_Signal().format);
}
