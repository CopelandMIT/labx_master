/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/
#ifndef IFXRSP_TYPES_H
#define IFXRSP_TYPES_H 1

#ifdef __cplusplus
#include <cstdint>
#else
#include <stdbool.h>
#include <stdint.h>
#endif

#include <universal/data_definitions.h>


#define MAX_ANTENNAE                32


#define FFT_FLAGS_DISCARD_HALF      (1u << 0)    ///< specify whether symmetric second half of FFT should be discarded (acceptedBins has to be set to 0)
#define FFT_FLAGS_INPLACE           (1u << 1)    ///< overwrite previous FFT data


#define IfxRsp_FftWindow_NoWindow       1
#define IfxRsp_FftWindow_Hann           2
#define IfxRsp_FftWindow_Hamming        3
#define IfxRsp_FftWindow_BlackmanHarris 4

#define IfxRsp_LocalMaxMode_Disable         0
#define IfxRsp_LocalMaxMode_ThresholdOnly   1
#define IfxRsp_LocalMaxMode_LocalMaxOnly    2
#define IfxRsp_LocalMaxMode_Both            3

#define IfxRsp_CfarCaAlgorithm_Disable  0
#define IfxRsp_CfarCaAlgorithm_Cash     1
#define IfxRsp_CfarCaAlgorithm_Ca       2
#define IfxRsp_CfarCaAlgorithm_Cago     3
#define IfxRsp_CfarCaAlgorithm_Caso     4

#define IfxRsp_CfarGosAlgorithm_Disable 0
#define IfxRsp_CfarGosAlgorithm_Gosca   1
#define IfxRsp_CfarGosAlgorithm_Gosgo   2
#define IfxRsp_CfarGosAlgorithm_Gosso   3

#define IfxRsp_VirtualChannel_RawData          (1u << 0)
#define IfxRsp_VirtualChannel_TargetProperties (1u << 1)

#define IfxRsp_Flags_Azimuth   (1 << 0)
#define IfxRsp_Flags_Elevation (1 << 1)

typedef struct _IfxRsp_Signal
{
    uint32_t    size;                   ///< total size
    uint32_t    baseAddress;            ///< The storing-method used is the row-major order. Therefore, consecutive elements of a row are contiguous in memory.
    uint32_t    stride;                 ///< number of bytes between successive rows
    uint16_t    rows;                   ///< dim1
    uint16_t    cols;                   ///< dim2
    uint16_t    pages;                  ///< dim3
    uint8_t     format;
} IfxRsp_Signal;

typedef struct _IfxRsp_FftSetting
{
    uint16_t    size;                   ///< 0 = default number of samples (= smallest power of 2 greater than or equal to number of samples)
    uint16_t    acceptedBins;           ///< 0 = all (disable rejection), otherwise number of accepted bins from the beginning
    uint8_t     window;
    uint8_t     windowFormat;           ///< only Q15 (default) or Q31
    uint8_t     exponent;               ///< (only for 16bit output data format) scale result
    uint8_t     flags;                  ///< specify additional flags for FFT
} IfxRsp_FftSetting;

typedef struct _IfxRsp_LocalMaxSetting
{
    uint8_t     mode;
    uint32_t    threshold;              ///< 0..0xFFFFFFFF
    uint8_t     windowWidth;            ///< 0,1,2
    bool        combineAnd;
} IfxRsp_LocalMaxSetting;

typedef struct _IfxRsp_CfarCaSetting
{
    uint8_t     algorithm;
    uint8_t     guardCells;             ///< 0..32
    uint8_t     windowCellsExponent;    ///< 0..5
    uint8_t     cashSubWindowExponent;  ///< 0..5
    uint16_t    betaThreshold;          ///< 0..0xFFFF
} IfxRsp_CfarCaSetting;

typedef struct _IfxRsp_CfarGosSetting
{
    uint8_t     algorithm;
    uint8_t     guardCells;             ///< 0..32
    uint8_t     indexLead;              ///< 1..32
    uint8_t     indexLag;               ///< 1..32
    uint8_t     windowCells;            ///< 1..32
    uint16_t    betaThreshold;          ///< 0..0xFFFF
} IfxRsp_CfarGosSetting;

typedef struct _IfxRsp_ThresholdingSetting
{
    bool        spectrumExtension;
    IfxRsp_LocalMaxSetting  localMax;   ///< only used when (mode == localMax)
    IfxRsp_CfarCaSetting    cfarCa;     ///< only used when (mode == cfarCa || mode == bothCfar)
    IfxRsp_CfarGosSetting   cfarGos;    ///< only used when (mode == cfarGos || mode == bothCfar)
} IfxRsp_ThresholdingSetting;

typedef struct _IfxRsp_DbfSetting
{
    uint8_t     angles;                 ///< number of angles to calculate
    uint8_t     coefficientFormat;      ///< only Complex16 or Complex32 (default)
    uint8_t     format;                 ///< only Complex16 or Complex32 (default)
    float       centerAngle;            ///< [rad]
    float       angularSpacing;         ///< [rad]
    IfxRsp_ThresholdingSetting  thresholding;
} IfxRsp_DbfSetting;

typedef struct _IfxRsp_DetectionSettings
{
    uint16_t    maxDetections;          ///< maximum number of targets to be detected (0 = disable)
    uint16_t    fftSize;                ///< fftSize to be used (0 = default value)
    uint8_t     flags;           ///< additional calculations (Azimuth, Elevation angle)
    IfxRsp_ThresholdingSetting  thresholdingSettings[2]; ///< thresholdingSettings[0] for range and thresholdingSettings[1] velocity (can only be enabled with nci enabled)
} IfxRsp_DetectionSettings;

typedef struct _IfxRsp_AntennaCalibration
{
    float       spacing;                ///< antenna spacing in multiples of lambda
    float       temperature;            ///< temperature while calibrating
    uint16_t    fftSize;                ///< fftSize used (0 = no Calibration)
    uint8_t     count;                  ///< number of virtual antennae
    uint8_t     indices[MAX_ANTENNAE];  ///< virtual antenna ordering
    float       coefficients[MAX_ANTENNAE][2]; ///< calibration coefficients as complex values (float[2])
} IfxRsp_AntennaCalibration;

typedef struct _IfxRsp_Stages
{
    uint8_t     fftSteps;                ///< number of FFT stages enabled (0: time data; 1: Range FFT; 2: Doppler)
    uint8_t     fftFormat;               ///< output data format for FFT, only Real* or Complex*
    uint8_t     nciFormat;               ///< only Q15 or Q31 (default), 0 to disable (can only be enabled with fftSteps == 2)
    uint8_t     virtualChannels;         ///< channels bit-mask to enable/disable a data output
    IfxRsp_FftSetting           fftSettings[2];          ///< config for each FFT (limitation for second stage: there have to be at least 8 ramps)
    IfxRsp_DetectionSettings    detectionSettings;       ///< target detection (can only be enabled with thresholding enabled)
    IfxRsp_DbfSetting           dbfSetting[2];
} IfxRsp_Stages;


typedef struct
{
    float range;
    float velocity;
    float azimuth;
    float elevation;
    uint32_t  power;
} IfxRsp_TargetProperty;

typedef struct
{
    uint16_t count;
    IfxRsp_TargetProperty *properties;
} IfxRsp_Targets;


#endif /* IFXRSP_TYPES_H */
