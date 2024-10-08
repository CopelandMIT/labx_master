/****************************************************************************\
* Copyright (C) 2020 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/
#ifndef DATA_DEFINITIONS_H
#define DATA_DEFINITIONS_H 1

#ifdef __cplusplus
#include <cstdint>
#else
#include <stdbool.h>
#include <stdint.h>
#endif


typedef void (*IData_acquisitionStatusCallback)(bool state);
typedef void (*IData_callback)(void *arg, uint8_t *data, uint32_t length, uint8_t virtualChannel, uint64_t timestamp);

typedef void (*IProcessing_DataCallback)(void *arg, uint32_t data[][2], uint8_t count, uint8_t virtualChannel, uint64_t timestamp);



#define DataFormat_Auto         0xFF
#define DataFormat_Disabled     0x00

#define DataFormat_U8           0x01
#define DataFormat_U16          0x02
#define DataFormat_U32          0x03
#define DataFormat_U64          0x04
#define DataFormat_S8           0x05
#define DataFormat_S16          0x06
#define DataFormat_S32          0x07
#define DataFormat_S64          0x08
#define DataFormat_Bits         0x09

#define DataFormat_Q15          0x0A
#define DataFormat_Q31          0x0B
#define DataFormat_Half         0x0C
#define DataFormat_ComplexQ15   0x0D
#define DataFormat_ComplexQ31   0x0E
#define DataFormat_ComplexHalf  0x0F

#define DataFormat_Packed12     0x10

#define DataFormat_Raw10        0x2B
#define DataFormat_Raw12        0x2C
#define DataFormat_Raw14        0x2D
#define DataFormat_Raw16        DataFormat_U16


#define DataError_NoError           0x00000000
#define DataError_LowLevelError     0x40000001
#define DataError_FrameDropped      0x40000002
#define DataError_FramePoolDepleted 0x40000003
#define DataError_FrameSizeExceeded 0x40000004
#define DataError_FrameQueueTrimmed 0x40000005


#define DATA_INDEX_INVALID        0xFF
#define VIRTUAL_CHANNEL_UNDEFINED 0xFF


#define CHANNEL_SWAPPING_RX_MIRROR (1u << 0)  ///< mirror: swap 0-3 and 1-2
#define CHANNEL_SWAPPING_RX_FLIP   (1u << 1)  ///< flip: swap 0-1 and 2-3
#define CHANNEL_SWAPPING_TX_MIRROR (1u << 2)  ///< mirror: swap 0-2 for 3TX, or 0-1 or 1-2 for 2TX

typedef struct
{
    uint8_t format;           ///< data format identifier
    uint8_t rxChannels;       ///< no. of active receive channels
    uint16_t ramps;           ///< no. of chirps per frame
    uint16_t samples;         ///< no. of samples per chirp

    uint8_t channelSwapping;  ///< swapping mode for channels
    uint8_t bitWidth;         ///< no. of bits per sample
} IDataProperties_t;


#endif /* DATA_DEFINITIONS_H */
