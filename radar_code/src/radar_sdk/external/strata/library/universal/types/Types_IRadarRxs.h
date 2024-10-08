/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/
/**
 * \addtogroup      PinsRadar
 * \brief
 * Rxs Pins interface
 * @{
 */
#ifndef TYPES_I_RADAR_RXS_H
#define TYPES_I_RADAR_RXS_H 1

#ifdef __cplusplus
#include <cstdint>
#else
#include <stdbool.h>
#include <stdint.h>
#endif

#include <universal/gpio_definitions.h>


typedef struct
{
    uint8_t devId;            ///< control interface device id
    uint8_t dataIndex;        ///< data interface index
    uint8_t channelSwapping;  ///< swapping mode for receive channels
} IRadarRxsDefinition_t;


#define I_PINS_RXS_DMUX_COUNT 5

typedef struct
{
    uint16_t      gpioReset;                        ///< GPIO pin corresponding to device reset
    uint16_t      gpioStatus;                       ///< GPIO pin corresponding to device OK pin
    uint16_t      gpioDmux[I_PINS_RXS_DMUX_COUNT];  ///< GPIO pins corresponding to device DMUX pins
} IPinsRxsDefinition_t;


#define OPERATION_MODE_SINGLE               0x00
#define OPERATION_MODE_SLAVE_LOIN_TX1       0x02  // use TX2 and TX3
#define OPERATION_MODE_SLAVE_LOIN_TX3       0x03  // use TX1 and TX2
#define OPERATION_MODE_MASTER_LOOUT_TX1     0x10  // use TX2 and TX3
#define OPERATION_MODE_MASTER_LOOUT_TX2     0x20  // use TX1 and TX3
#define OPERATION_MODE_MASTER_LOOUT_TX3     0x30  // use TX1 and TX2
#define OPERATION_MODE_MASTER_LOOUT_TX1_TX3 0x40  // disable TX2

typedef struct
{
    uint8_t operationMode;    ///< see defines
    uint8_t txChannelOffset;  ///< Logical offset for TX1
    uint8_t rxChannelOffset;  ///< Logical offset for RX1
} IRadarRxsRole_t;


#define NO_SYNC_FF 5

typedef struct
{
    bool    ballBreakDetection;             ///< .ballBreakDetection configure ball break detection depending on connection on PCB
    uint8_t triggerSource;                  ///< .triggerSource external sequence trigger source: 0 = none, 1..5 = DMUX1..5, 6..10 = DMUX 1..5 without sync FF
    uint8_t dmuxMap[I_PINS_RXS_DMUX_COUNT]; ///< .dmuxMap default dmux pin configuration (see defines)
} IRadarRxsConfig_t;


#define DMUX_MAP_DEFAULT                                                                              \
    {                                                                                                 \
        DMUX_INPUT_HIGH_Z, DMUX_INPUT_HIGH_Z, DMUX_INPUT_HIGH_Z, DMUX_INPUT_HIGH_Z, DMUX_INPUT_HIGH_Z \
    }

#define DMUX_MAP_INVALID                    0xFF
#define DMUX_INPUT_HIGH_Z                   0x80
#define DMUX_INPUT_PULL_UP                  0x81
#define DMUX_INPUT_PULL_DOWN                0x82
#define DMUX_WRITE                          0x00    ///< value of the corresponding bit from DMUX_WRITE_REG
#define DMUX_SIGNAL_CU_STAT                 0x01    ///< cu_stat bit of the DIG_CTRL_REG register
#define DMUX_SIGNAL_RMP_SEG_SYNC            0x02    ///< Ramper signals the start of a new ramp-segment (the signal's rising/falling edge alternates at every ramp segment)
#define DMUX_SIGNAL_RMP_RDY                 0x03    ///< Ramper ready to execute ramp sequence
#define DMUX_SIGNAL_RMP_TRIGGER_SYNC        0x04    ///< Ramper module start trigger synchronized to digital clock
#define DMUX_SIGNAL_RMP_TRIGGER_ASYNC       0x05    ///< Ramper module start trigger not synchronized to digital clock
#define DMUX_SIGNAL_RMP_START               0x06    ///< Ramper signals the start of a new ramp
#define DMUX_SIGNAL_RMP_RUN                 0x07    ///< Ramper is sequencing (goes low when an error occurs or ramper has finished sequencing)
#define DMUX_SIGNAL_RMP_DMUX_A              0x08    ///< Ramper DMUX A output
#define DMUX_SIGNAL_RMP_DMUX_B              0x09    ///< Ramper DMUX B output
#define DMUX_SIGNAL_RMP_DMUX_C              0x0A    ///< Ramper DMUX C output
#define DMUX_SIGNAL_RMP_DMUX_D              0x0B    ///< Ramper DMUX D output
#define DMUX_SIGNAL_PSUP_MON_RNG_OK         0x0C    ///< Voltage is in functional range
#define DMUX_SIGNAL_PSUP_MON_OVERN_N        0x0D    ///< Overvoltage detection signal (High when below the overvoltage range)
#define DMUX_SIGNAL_PSUP_MON_UNDER_1_N      0x0E    ///< Voltage level detection. High when voltage is above the undervoltage range1
#define DMUX_SIGNAL_RXADC1_CLIP             0x0F    ///< Logical OR combination of RXADC 1 overflow and underflow statuses
#define DMUX_SIGNAL_RXADC1_OVERF            0x10    ///< overflow status of RXADC1
#define DMUX_SIGNAL_RXADC2_CLIP             0x11    ///< Logical OR combination of RXADC 2 overflow and underflow statuses
#define DMUX_SIGNAL_RXADC2_OVERF            0x12    ///< overflow status of RXADC2
#define DMUX_SIGNAL_RXADC2_UNDERF           0x13    ///< underflow status of RXADC2
#define DMUX_SIGNAL_RXADC3_CLIP             0x14    ///< Logical OR combination of RXADC 3 overflow and underflow statuses
#define DMUX_SIGNAL_RXADC3_OVERF            0x15    ///< overflow status of RXADC3
#define DMUX_SIGNAL_RXADC3_UNDERF           0x16    ///< underflow status of RXADC3
#define DMUX_SIGNAL_RXADC4_CLIP             0x17    ///< Logical OR combination of RXADC 4 overflow and underflow statuses
#define DMUX_SIGNAL_RXADC4_OVERF            0x18    ///< overflow status of RXADC4
#define DMUX_SIGNAL_RXADC4_UNDERF           0x19    ///< underflow status of RXADC4


typedef struct
{
    // Config CONF_LOW field
    uint8_t   RMP_DMUX_D_C_B_A;
    uint8_t   TX3_IQM_SEL;
    uint8_t   TX2_IQM_SEL;
    uint8_t   TX1_IQM_SEL;
    uint8_t   TX_OUT_EN;

    // Config CONF_HIGH field
    bool      PA_CTRL_EN;
    uint8_t   TX_PA_EN;
    uint8_t   DFE_SEL;
    bool      MBC_EN;
    uint8_t   AFE_CTRL4_SEL;
    uint8_t   PLL_SEL;
    bool      PLL_UNLOCK;
    bool      RX_FRM;

    // Config header
    bool        CH_LD_CONF_HIGH;
    bool        CH_LD_CONF_LOW;
    uint8_t     NUM_CONF;
    uint8_t     CONF_IDX_START;
} ConfigDescriptor;

typedef struct
{
    // Segment NTIME, NSTART, NSTEP field
    int32_t     NTIME;
    int32_t     NSTART;
    int32_t     NSTEP;
    bool        NTIME_ABS_REL;
    bool        NSTART_ABS_REL;
    bool        NSTEP_ABS_REL;

    // Segment LOOP field
    uint16_t    LOOP_CNT;

    // Segment CONF_SET_SEL field
    uint8_t     CONF_IDX_BEGIN;
    uint8_t     CONF_IDX_LAST;
    bool        CONF_SET_INC;

    // Segment header
    bool        CH_LD_CONF_SET_SEL;
    bool        CH_LD_NSTEP;
    bool        CH_LD_NSTART;
    bool        CH_LD_NTIME;
    bool        LOOP_END;
    bool        LOOP_BEGIN;
    bool        LAST;
    bool        OP;
    uint8_t     SEG_SEL;
} SegmentDescriptor;


#endif /* TYPES_I_RADAR_RXS_H */
/** @} */
