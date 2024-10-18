/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/
#ifndef IFXRFE_TYPES_H
#define IFXRFE_TYPES_H 1

#ifdef __cplusplus
#include <cstdint>
#else
#include <stdbool.h>
#include <stdint.h>
#endif


#define MAX_RAMPS_SUPPORTED         8


typedef struct
{
    bool        enableMonitoring;   ///< if enabled, this also includes low-power mode in between sequences

    // supported combinations see user manual (incl. decimation factor)
    uint8_t     sampleWidth;        ///< [bits]
    double      sampleRate;         ///< [Hz]

    // TX
    float       txPower;            ///< [%] TX channel output power {0.0: 100.0}

    // RX
    int8_t      lpGain;             ///< [dB] AFE low-pass filter gain, RXS: {-16:6:56}
    uint8_t     mixerGain;          ///< [dB] Mixer gain, RXS: {0,6}
    bool        dcocEnable;         ///< enable DC offset compensation (DCOC)
    uint8_t     dcocShift;          ///< RXS: {0:16} (default 3) N factor for DFE DCOC
} IfxRfe_MmicConfig;

typedef struct
{
    double      fStart;             ///< [Hz] start frequency of ramp
    double      fDelta;             ///< [Hz] delta frequency of ramp

    uint16_t    loops;              ///< {1:1023} total number of ramps should be a power of 2, otherwise automatic padding
    uint16_t    muxOut;            ///< mask with signal output during active segment of ramp, i.e. bit0 : DMUX_A, bit1: DMUX_B, bit2: DMUX_C, bit3: DMUX_D
    uint16_t    txMask;             ///< mask with active TX channels (up to 16 in high-res systems)
    uint16_t    txPhases[16];       ///< phase values for TX channels
} IfxRfe_Ramp;


typedef struct
{
    double      tRamp;              ///< [s] duration of ramp (including start/stop delay). After subtracting the start/stop delay, this should yield a sample number that is a power of 2, otherwise automatic padding
    double      tJump;              ///< [s] duration of jump
    double      tWait;              ///< [s] duration of wait

    double      tRampStartDelay;    ///< [s] time delay before starting LVDS transmission
    double      tRampStopDelay;     ///< [s] time delay before stopping LVDS transmission
//    uint16      options;            ///<
//    bool        startDelayPaOff;
//    bool        startDelayPaDump;

    uint32_t    rxMask;             ///< mask to enable RX channels
    uint16_t    loops;              ///< {1:1023} total number of ramps should be a power of 2, otherwise automatic padding
    uint8_t     rampCount;
    IfxRfe_Ramp *ramps;
} IfxRfe_Sequence;


#endif /* IFXRFE_TYPES_H */
