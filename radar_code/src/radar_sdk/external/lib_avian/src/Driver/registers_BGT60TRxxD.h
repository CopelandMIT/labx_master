/**
 * @file registers_BGT60TRxxD.h
 *
 * This file contains register and bit field definition for the BGT60TRxxD
 * device (digital chip ID 6) and derivatives.
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


#ifndef BGT60TRXXD_H_
#define BGT60TRXXD_H_

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/
#include <stdint.h>

/*
==============================================================================
   2. DEFINITIONS
==============================================================================
*/

/**
 * \defgroup Registers_BGT60TRxxD Registers BGT60TRxxD
 *
 * These constants define symbolic names for the SPI register addresses of the
 * BGT60TRxxD.
 *
 * For each register also constants are defined to access the bit masks and offset.
 * Each bit field is defined by two macros with the suffixes _pos and _msk.
 * Don't use those macros directly. Instead use the macros form  \ref AccessMacros.
 * Pass the bit field name without the prefix to those macros.
 *
 * The definitions where generated through this command line:
 * py generate_c_header.py IFX_BGT60TRxxD_ID0006.xml -p BGT60TRxxD
 * @{
 */
#define BGT60TRxxD_REG_MAIN        0x00u
#define BGT60TRxxD_REG_ADC0        0x01u
#define BGT60TRxxD_REG_CHIP_ID     0x02u
#define BGT60TRxxD_REG_STAT1       0x03u
#define BGT60TRxxD_REG_PACR1       0x04u
#define BGT60TRxxD_REG_PACR2       0x05u
#define BGT60TRxxD_REG_SFCTL       0x06u
#define BGT60TRxxD_REG_SADC_CTRL   0x07u
#define BGT60TRxxD_REG_CSP_I_0     0x08u
#define BGT60TRxxD_REG_CSP_I_1     0x09u
#define BGT60TRxxD_REG_CSP_I_2     0x0au
#define BGT60TRxxD_REG_CSCI        0x0bu
#define BGT60TRxxD_REG_CSP_D_0     0x0cu
#define BGT60TRxxD_REG_CSP_D_1     0x0du
#define BGT60TRxxD_REG_CSP_D_2     0x0eu
#define BGT60TRxxD_REG_CSCDS       0x0fu
#define BGT60TRxxD_REG_CS1_U_0     0x10u
#define BGT60TRxxD_REG_CS1_U_1     0x11u
#define BGT60TRxxD_REG_CS1_U_2     0x12u
#define BGT60TRxxD_REG_CS1_D_0     0x13u
#define BGT60TRxxD_REG_CS1_D_1     0x14u
#define BGT60TRxxD_REG_CS1_D_2     0x15u
#define BGT60TRxxD_REG_CS1         0x16u
#define BGT60TRxxD_REG_CS2_U_0     0x17u
#define BGT60TRxxD_REG_CS2_U_1     0x18u
#define BGT60TRxxD_REG_CS2_U_2     0x19u
#define BGT60TRxxD_REG_CS2_D_0     0x1au
#define BGT60TRxxD_REG_CS2_D_1     0x1bu
#define BGT60TRxxD_REG_CS2_D_2     0x1cu
#define BGT60TRxxD_REG_CS2         0x1du
#define BGT60TRxxD_REG_CS3_U_0     0x1eu
#define BGT60TRxxD_REG_CS3_U_1     0x1fu
#define BGT60TRxxD_REG_CS3_U_2     0x20u
#define BGT60TRxxD_REG_CS3_D_0     0x21u
#define BGT60TRxxD_REG_CS3_D_1     0x22u
#define BGT60TRxxD_REG_CS3_D_2     0x23u
#define BGT60TRxxD_REG_CS3         0x24u
#define BGT60TRxxD_REG_CS4_U_0     0x25u
#define BGT60TRxxD_REG_CS4_U_1     0x26u
#define BGT60TRxxD_REG_CS4_U_2     0x27u
#define BGT60TRxxD_REG_CS4_D_0     0x28u
#define BGT60TRxxD_REG_CS4_D_1     0x29u
#define BGT60TRxxD_REG_CS4_D_2     0x2au
#define BGT60TRxxD_REG_CS4         0x2bu
#define BGT60TRxxD_REG_CCR0        0x2cu
#define BGT60TRxxD_REG_CCR1        0x2du
#define BGT60TRxxD_REG_CCR2        0x2eu
#define BGT60TRxxD_REG_CCR3        0x2fu
#define BGT60TRxxD_REG_PLL1_0      0x30u
#define BGT60TRxxD_REG_PLL1_1      0x31u
#define BGT60TRxxD_REG_PLL1_2      0x32u
#define BGT60TRxxD_REG_PLL1_3      0x33u
#define BGT60TRxxD_REG_PLL1_4      0x34u
#define BGT60TRxxD_REG_PLL1_5      0x35u
#define BGT60TRxxD_REG_PLL1_6      0x36u
#define BGT60TRxxD_REG_PLL1_7      0x37u
#define BGT60TRxxD_REG_PLL2_0      0x38u
#define BGT60TRxxD_REG_PLL2_1      0x39u
#define BGT60TRxxD_REG_PLL2_2      0x3au
#define BGT60TRxxD_REG_PLL2_3      0x3bu
#define BGT60TRxxD_REG_PLL2_4      0x3cu
#define BGT60TRxxD_REG_PLL2_5      0x3du
#define BGT60TRxxD_REG_PLL2_6      0x3eu
#define BGT60TRxxD_REG_PLL2_7      0x3fu
#define BGT60TRxxD_REG_PLL3_0      0x40u
#define BGT60TRxxD_REG_PLL3_1      0x41u
#define BGT60TRxxD_REG_PLL3_2      0x42u
#define BGT60TRxxD_REG_PLL3_3      0x43u
#define BGT60TRxxD_REG_PLL3_4      0x44u
#define BGT60TRxxD_REG_PLL3_5      0x45u
#define BGT60TRxxD_REG_PLL3_6      0x46u
#define BGT60TRxxD_REG_PLL3_7      0x47u
#define BGT60TRxxD_REG_PLL4_0      0x48u
#define BGT60TRxxD_REG_PLL4_1      0x49u
#define BGT60TRxxD_REG_PLL4_2      0x4au
#define BGT60TRxxD_REG_PLL4_3      0x4bu
#define BGT60TRxxD_REG_PLL4_4      0x4cu
#define BGT60TRxxD_REG_PLL4_5      0x4du
#define BGT60TRxxD_REG_PLL4_6      0x4eu
#define BGT60TRxxD_REG_PLL4_7      0x4fu
#define BGT60TRxxD_REG_ADC1        0x50u
#define BGT60TRxxD_REG_ADC2        0x51u
#define BGT60TRxxD_REG_ADC3        0x52u
#define BGT60TRxxD_REG_ADC4        0x53u
#define BGT60TRxxD_REG_ADC5        0x54u
#define BGT60TRxxD_REG_RFT0        0x55u
#define BGT60TRxxD_REG_RFT1        0x56u
#define BGT60TRxxD_REG_DFT0        0x57u
#define BGT60TRxxD_REG_DFT1        0x58u
#define BGT60TRxxD_REG_PDFT0       0x59u
#define BGT60TRxxD_REG_PDFT1       0x5au
#define BGT60TRxxD_REG_SADC_DFT0   0x5bu
#define BGT60TRxxD_REG_SADC_DFT1   0x5cu
#define BGT60TRxxD_REG_STAT0       0x5du
#define BGT60TRxxD_REG_SADC_RESULT 0x5eu
#define BGT60TRxxD_REG_FSTAT       0x5fu
#define BGT60TRxxD_REG_DEV_ID0     0x60u
#define BGT60TRxxD_REG_DEV_ID1     0x61u
#define BGT60TRxxD_REG_FD          0x62u
#define BGT60TRxxD_NUM_REGISTERS   0x63u

/* Fields of register MAIN */
/* ----------------------- */
#define BGT60TRxxD_MAIN_FRAME_START_pos     0u
#define BGT60TRxxD_MAIN_FRAME_START_msk     0x000001u
#define BGT60TRxxD_MAIN_SW_RESET_pos        1u
#define BGT60TRxxD_MAIN_SW_RESET_msk        0x000002u
#define BGT60TRxxD_MAIN_FSM_RESET_pos       2u
#define BGT60TRxxD_MAIN_FSM_RESET_msk       0x000004u
#define BGT60TRxxD_MAIN_FIFO_RESET_pos      3u
#define BGT60TRxxD_MAIN_FIFO_RESET_msk      0x000008u
#define BGT60TRxxD_MAIN_TR_TWKUP_pos        4u
#define BGT60TRxxD_MAIN_TR_TWKUP_msk        0x000ff0u
#define BGT60TRxxD_MAIN_TR_TWKUP_MUL_pos    12u
#define BGT60TRxxD_MAIN_TR_TWKUP_MUL_msk    0x00f000u
#define BGT60TRxxD_MAIN_CW_MODE_pos         16u
#define BGT60TRxxD_MAIN_CW_MODE_msk         0x010000u
#define BGT60TRxxD_MAIN_SADC_BG_CLK_DIV_pos 17u
#define BGT60TRxxD_MAIN_SADC_BG_CLK_DIV_msk 0x060000u
#define BGT60TRxxD_MAIN_MADC_BG_CLK_DIV_pos 19u
#define BGT60TRxxD_MAIN_MADC_BG_CLK_DIV_msk 0x180000u
#define BGT60TRxxD_MAIN_LOAD_STRENGTH_pos   21u
#define BGT60TRxxD_MAIN_LOAD_STRENGTH_msk   0x600000u
#define BGT60TRxxD_MAIN_LDO_MODE_pos        23u
#define BGT60TRxxD_MAIN_LDO_MODE_msk        0x800000u

/* Fields of register ADC0 */
/* ----------------------- */
#define BGT60TRxxD_ADC0_ADC_OVERS_CFG_pos 0u
#define BGT60TRxxD_ADC0_ADC_OVERS_CFG_msk 0x000003u
#define BGT60TRxxD_ADC0_BG_TC_TRIM_pos    2u
#define BGT60TRxxD_ADC0_BG_TC_TRIM_msk    0x00001cu
#define BGT60TRxxD_ADC0_BG_CHOP_EN_pos    5u
#define BGT60TRxxD_ADC0_BG_CHOP_EN_msk    0x000020u
#define BGT60TRxxD_ADC0_STC_pos           6u
#define BGT60TRxxD_ADC0_STC_msk           0x0000c0u
#define BGT60TRxxD_ADC0_DSCAL_pos         8u
#define BGT60TRxxD_ADC0_DSCAL_msk         0x000100u
#define BGT60TRxxD_ADC0_TRACK_CFG_pos     9u
#define BGT60TRxxD_ADC0_TRACK_CFG_msk     0x000600u
#define BGT60TRxxD_ADC0_MSB_CTRL_pos      11u
#define BGT60TRxxD_ADC0_MSB_CTRL_msk      0x000800u
#define BGT60TRxxD_ADC0_TRIG_MADC_pos     12u
#define BGT60TRxxD_ADC0_TRIG_MADC_msk     0x001000u
#define BGT60TRxxD_ADC0_ADC_DIV_pos       14u
#define BGT60TRxxD_ADC0_ADC_DIV_msk       0xffc000u

/* Fields of register CHIP_ID */
/* -------------------------- */
#define BGT60TRxxD_CHIP_ID_RF_ID_pos      0u
#define BGT60TRxxD_CHIP_ID_RF_ID_msk      0x0000ffu
#define BGT60TRxxD_CHIP_ID_DIGITAL_ID_pos 8u
#define BGT60TRxxD_CHIP_ID_DIGITAL_ID_msk 0xffff00u

/* Fields of register STAT1 */
/* ------------------------ */
#define BGT60TRxxD_STAT1_SHAPE_GRP_CNT_pos 0u
#define BGT60TRxxD_STAT1_SHAPE_GRP_CNT_msk 0x000fffu
#define BGT60TRxxD_STAT1_FRAME_CNT_pos     12u
#define BGT60TRxxD_STAT1_FRAME_CNT_msk     0xfff000u

/* Fields of register PACR1 */
/* ------------------------ */
#define BGT60TRxxD_PACR1_ANAPON_pos   0u
#define BGT60TRxxD_PACR1_ANAPON_msk   0x000001u
#define BGT60TRxxD_PACR1_VANAREG_pos  1u
#define BGT60TRxxD_PACR1_VANAREG_msk  0x000006u
#define BGT60TRxxD_PACR1_DIGPON_pos   3u
#define BGT60TRxxD_PACR1_DIGPON_msk   0x000008u
#define BGT60TRxxD_PACR1_VDIGREG_pos  4u
#define BGT60TRxxD_PACR1_VDIGREG_msk  0x000030u
#define BGT60TRxxD_PACR1_BGAPEN_pos   6u
#define BGT60TRxxD_PACR1_BGAPEN_msk   0x000040u
#define BGT60TRxxD_PACR1_U2IEN_pos    7u
#define BGT60TRxxD_PACR1_U2IEN_msk    0x000080u
#define BGT60TRxxD_PACR1_VREFSEL_pos  8u
#define BGT60TRxxD_PACR1_VREFSEL_msk  0x000300u
#define BGT60TRxxD_PACR1_RFILTSEL_pos 10u
#define BGT60TRxxD_PACR1_RFILTSEL_msk 0x000400u
#define BGT60TRxxD_PACR1_RLFSEL_pos   11u
#define BGT60TRxxD_PACR1_RLFSEL_msk   0x000800u
#define BGT60TRxxD_PACR1_SPARE_pos    12u
#define BGT60TRxxD_PACR1_SPARE_msk    0x001000u
#define BGT60TRxxD_PACR1_LOCKSEL_pos  13u
#define BGT60TRxxD_PACR1_LOCKSEL_msk  0x00e000u
#define BGT60TRxxD_PACR1_LOCKFORC_pos 16u
#define BGT60TRxxD_PACR1_LOCKFORC_msk 0x010000u
#define BGT60TRxxD_PACR1_ICPSEL_pos   17u
#define BGT60TRxxD_PACR1_ICPSEL_msk   0x0e0000u
#define BGT60TRxxD_PACR1_BIASFORC_pos 20u
#define BGT60TRxxD_PACR1_BIASFORC_msk 0x100000u
#define BGT60TRxxD_PACR1_CPEN_pos     21u
#define BGT60TRxxD_PACR1_CPEN_msk     0x200000u
#define BGT60TRxxD_PACR1_LFEN_pos     22u
#define BGT60TRxxD_PACR1_LFEN_msk     0x400000u
#define BGT60TRxxD_PACR1_OSCCLKEN_pos 23u
#define BGT60TRxxD_PACR1_OSCCLKEN_msk 0x800000u

/* Fields of register PACR2 */
/* ------------------------ */
#define BGT60TRxxD_PACR2_DIVSET_pos    0u
#define BGT60TRxxD_PACR2_DIVSET_msk    0x00001fu
#define BGT60TRxxD_PACR2_DIVEN_pos     5u
#define BGT60TRxxD_PACR2_DIVEN_msk     0x000020u
#define BGT60TRxxD_PACR2_FSTDNEN_pos   6u
#define BGT60TRxxD_PACR2_FSTDNEN_msk   0x0000c0u
#define BGT60TRxxD_PACR2_FSDNTMR_pos   8u
#define BGT60TRxxD_PACR2_FSDNTMR_msk   0x01ff00u
#define BGT60TRxxD_PACR2_TRIVREG_pos   17u
#define BGT60TRxxD_PACR2_TRIVREG_msk   0x020000u
#define BGT60TRxxD_PACR2_DTSEL_pos     18u
#define BGT60TRxxD_PACR2_DTSEL_msk     0x0c0000u
#define BGT60TRxxD_PACR2_PLL_SPARE_pos 20u
#define BGT60TRxxD_PACR2_PLL_SPARE_msk 0x100000u
#define BGT60TRxxD_PACR2_TR_DIVEN_pos  21u
#define BGT60TRxxD_PACR2_TR_DIVEN_msk  0xe00000u

/* Fields of register SFCTL */
/* ------------------------ */
#define BGT60TRxxD_SFCTL_FIFO_CREF_pos    0u
#define BGT60TRxxD_SFCTL_FIFO_CREF_msk    0x001fffu
#define BGT60TRxxD_SFCTL_FIFO_PD_MODE_pos 14u
#define BGT60TRxxD_SFCTL_FIFO_PD_MODE_msk 0x00c000u
#define BGT60TRxxD_SFCTL_MISO_HS_READ_pos 16u
#define BGT60TRxxD_SFCTL_MISO_HS_READ_msk 0x010000u
#define BGT60TRxxD_SFCTL_LFSR_EN_pos      17u
#define BGT60TRxxD_SFCTL_LFSR_EN_msk      0x020000u
#define BGT60TRxxD_SFCTL_PREFIX_EN_pos    18u
#define BGT60TRxxD_SFCTL_PREFIX_EN_msk    0x040000u
#define BGT60TRxxD_SFCTL_PAD_MODE_pos     19u
#define BGT60TRxxD_SFCTL_PAD_MODE_msk     0x080000u
#define BGT60TRxxD_SFCTL_QSPI_WT_pos      20u
#define BGT60TRxxD_SFCTL_QSPI_WT_msk      0xf00000u

/* Fields of register SADC_CTRL */
/* ---------------------------- */
#define BGT60TRxxD_SADC_CTRL_SADC_CHSEL_pos   0u
#define BGT60TRxxD_SADC_CTRL_SADC_CHSEL_msk   0x00000fu
#define BGT60TRxxD_SADC_CTRL_START_SADC_pos   4u
#define BGT60TRxxD_SADC_CTRL_START_SADC_msk   0x000010u
#define BGT60TRxxD_SADC_CTRL_SADC_CLK_DIV_pos 6u
#define BGT60TRxxD_SADC_CTRL_SADC_CLK_DIV_msk 0x0000c0u
#define BGT60TRxxD_SADC_CTRL_SD_EN_pos        8u
#define BGT60TRxxD_SADC_CTRL_SD_EN_msk        0x000100u
#define BGT60TRxxD_SADC_CTRL_OVERS_CFG_pos    9u
#define BGT60TRxxD_SADC_CTRL_OVERS_CFG_msk    0x000600u
#define BGT60TRxxD_SADC_CTRL_SESP_pos         11u
#define BGT60TRxxD_SADC_CTRL_SESP_msk         0x000800u
#define BGT60TRxxD_SADC_CTRL_LVGAIN_pos       12u
#define BGT60TRxxD_SADC_CTRL_LVGAIN_msk       0x001000u
#define BGT60TRxxD_SADC_CTRL_DSCAL_pos        13u
#define BGT60TRxxD_SADC_CTRL_DSCAL_msk        0x002000u
#define BGT60TRxxD_SADC_CTRL_TC_TRIM_pos      14u
#define BGT60TRxxD_SADC_CTRL_TC_TRIM_msk      0x01c000u

/* Fields of register CSP_I_0 */
/* -------------------------- */
#define BGT60TRxxD_CSP_I_0_TX1_EN_pos        0u
#define BGT60TRxxD_CSP_I_0_TX1_EN_msk        0x000001u
#define BGT60TRxxD_CSP_I_0_PD1_EN_pos        1u
#define BGT60TRxxD_CSP_I_0_PD1_EN_msk        0x000002u
#define BGT60TRxxD_CSP_I_0_TX2_EN_pos        2u
#define BGT60TRxxD_CSP_I_0_TX2_EN_msk        0x000004u
#define BGT60TRxxD_CSP_I_0_PD2_EN_pos        3u
#define BGT60TRxxD_CSP_I_0_PD2_EN_msk        0x000008u
#define BGT60TRxxD_CSP_I_0_VCO_EN_pos        4u
#define BGT60TRxxD_CSP_I_0_VCO_EN_msk        0x000010u
#define BGT60TRxxD_CSP_I_0_TEMP_MEAS_EN_pos  5u
#define BGT60TRxxD_CSP_I_0_TEMP_MEAS_EN_msk  0x000020u
#define BGT60TRxxD_CSP_I_0_FDIV_EN_pos       6u
#define BGT60TRxxD_CSP_I_0_FDIV_EN_msk       0x000040u
#define BGT60TRxxD_CSP_I_0_RESERVED1_pos     7u
#define BGT60TRxxD_CSP_I_0_RESERVED1_msk     0x000080u
#define BGT60TRxxD_CSP_I_0_RESERVED2_pos     8u
#define BGT60TRxxD_CSP_I_0_RESERVED2_msk     0x000100u
#define BGT60TRxxD_CSP_I_0_RESERVED3_pos     9u
#define BGT60TRxxD_CSP_I_0_RESERVED3_msk     0x000200u
#define BGT60TRxxD_CSP_I_0_LO_DIST2_EN_pos   10u
#define BGT60TRxxD_CSP_I_0_LO_DIST2_EN_msk   0x000400u
#define BGT60TRxxD_CSP_I_0_LO_DIST1_EN_pos   11u
#define BGT60TRxxD_CSP_I_0_LO_DIST1_EN_msk   0x000800u
#define BGT60TRxxD_CSP_I_0_RX1LOBUF_EN_pos   12u
#define BGT60TRxxD_CSP_I_0_RX1LOBUF_EN_msk   0x001000u
#define BGT60TRxxD_CSP_I_0_RX1MIX_EN_pos     13u
#define BGT60TRxxD_CSP_I_0_RX1MIX_EN_msk     0x002000u
#define BGT60TRxxD_CSP_I_0_RX2LOBUF_EN_pos   14u
#define BGT60TRxxD_CSP_I_0_RX2LOBUF_EN_msk   0x004000u
#define BGT60TRxxD_CSP_I_0_RX2MIX_EN_pos     15u
#define BGT60TRxxD_CSP_I_0_RX2MIX_EN_msk     0x008000u
#define BGT60TRxxD_CSP_I_0_RX3LOBUF_EN_pos   16u
#define BGT60TRxxD_CSP_I_0_RX3LOBUF_EN_msk   0x010000u
#define BGT60TRxxD_CSP_I_0_RX3MIX_EN_pos     17u
#define BGT60TRxxD_CSP_I_0_RX3MIX_EN_msk     0x020000u
#define BGT60TRxxD_CSP_I_0_RX4LOBUF_EN_pos   18u
#define BGT60TRxxD_CSP_I_0_RX4LOBUF_EN_msk   0x040000u
#define BGT60TRxxD_CSP_I_0_RX4MIX_EN_pos     19u
#define BGT60TRxxD_CSP_I_0_RX4MIX_EN_msk     0x080000u
#define BGT60TRxxD_CSP_I_0_ABB1_AAF_CTRL_pos 20u
#define BGT60TRxxD_CSP_I_0_ABB1_AAF_CTRL_msk 0x100000u
#define BGT60TRxxD_CSP_I_0_ABB2_AAF_CTRL_pos 21u
#define BGT60TRxxD_CSP_I_0_ABB2_AAF_CTRL_msk 0x200000u
#define BGT60TRxxD_CSP_I_0_ABB3_AAF_CTRL_pos 22u
#define BGT60TRxxD_CSP_I_0_ABB3_AAF_CTRL_msk 0x400000u
#define BGT60TRxxD_CSP_I_0_ABB4_AAF_CTRL_pos 23u
#define BGT60TRxxD_CSP_I_0_ABB4_AAF_CTRL_msk 0x800000u

/* Fields of register CSP_I_1 */
/* -------------------------- */
#define BGT60TRxxD_CSP_I_1_TX1_DAC_pos       0u
#define BGT60TRxxD_CSP_I_1_TX1_DAC_msk       0x00001fu
#define BGT60TRxxD_CSP_I_1_TX2_DAC_pos       5u
#define BGT60TRxxD_CSP_I_1_TX2_DAC_msk       0x0003e0u
#define BGT60TRxxD_CSP_I_1_HP1_GAIN_pos      10u
#define BGT60TRxxD_CSP_I_1_HP1_GAIN_msk      0x000400u
#define BGT60TRxxD_CSP_I_1_HP2_GAIN_pos      11u
#define BGT60TRxxD_CSP_I_1_HP2_GAIN_msk      0x000800u
#define BGT60TRxxD_CSP_I_1_HP3_GAIN_pos      12u
#define BGT60TRxxD_CSP_I_1_HP3_GAIN_msk      0x001000u
#define BGT60TRxxD_CSP_I_1_HP4_GAIN_pos      13u
#define BGT60TRxxD_CSP_I_1_HP4_GAIN_msk      0x002000u
#define BGT60TRxxD_CSP_I_1_BB_RSTCNT_pos     14u
#define BGT60TRxxD_CSP_I_1_BB_RSTCNT_msk     0x0fc000u
#define BGT60TRxxD_CSP_I_1_MADC_BBCH1_EN_pos 20u
#define BGT60TRxxD_CSP_I_1_MADC_BBCH1_EN_msk 0x100000u
#define BGT60TRxxD_CSP_I_1_MADC_BBCH2_EN_pos 21u
#define BGT60TRxxD_CSP_I_1_MADC_BBCH2_EN_msk 0x200000u
#define BGT60TRxxD_CSP_I_1_MADC_BBCH3_EN_pos 22u
#define BGT60TRxxD_CSP_I_1_MADC_BBCH3_EN_msk 0x400000u
#define BGT60TRxxD_CSP_I_1_MADC_BBCH4_EN_pos 23u
#define BGT60TRxxD_CSP_I_1_MADC_BBCH4_EN_msk 0x800000u

/* Fields of register CSP_I_2 */
/* -------------------------- */
#define BGT60TRxxD_CSP_I_2_HPF_SEL1_pos  0u
#define BGT60TRxxD_CSP_I_2_HPF_SEL1_msk  0x000007u
#define BGT60TRxxD_CSP_I_2_VGA_GAIN1_pos 3u
#define BGT60TRxxD_CSP_I_2_VGA_GAIN1_msk 0x000038u
#define BGT60TRxxD_CSP_I_2_HPF_SEL2_pos  6u
#define BGT60TRxxD_CSP_I_2_HPF_SEL2_msk  0x0001c0u
#define BGT60TRxxD_CSP_I_2_VGA_GAIN2_pos 9u
#define BGT60TRxxD_CSP_I_2_VGA_GAIN2_msk 0x000e00u
#define BGT60TRxxD_CSP_I_2_HPF_SEL3_pos  12u
#define BGT60TRxxD_CSP_I_2_HPF_SEL3_msk  0x007000u
#define BGT60TRxxD_CSP_I_2_VGA_GAIN3_pos 15u
#define BGT60TRxxD_CSP_I_2_VGA_GAIN3_msk 0x038000u
#define BGT60TRxxD_CSP_I_2_HPF_SEL4_pos  18u
#define BGT60TRxxD_CSP_I_2_HPF_SEL4_msk  0x1c0000u
#define BGT60TRxxD_CSP_I_2_VGA_GAIN4_pos 21u
#define BGT60TRxxD_CSP_I_2_VGA_GAIN4_msk 0xe00000u

/* Fields of register CSCI */
/* ----------------------- */
#define BGT60TRxxD_CSCI_REPCS_pos        0u
#define BGT60TRxxD_CSCI_REPCS_msk        0x00000fu
#define BGT60TRxxD_CSCI_CS_EN_pos        4u
#define BGT60TRxxD_CSCI_CS_EN_msk        0x000010u
#define BGT60TRxxD_CSCI_ABB_ISOPD_pos    5u
#define BGT60TRxxD_CSCI_ABB_ISOPD_msk    0x000020u
#define BGT60TRxxD_CSCI_RF_ISOPD_pos     6u
#define BGT60TRxxD_CSCI_RF_ISOPD_msk     0x000040u
#define BGT60TRxxD_CSCI_MADC_BG_EN_pos   7u
#define BGT60TRxxD_CSCI_MADC_BG_EN_msk   0x000080u
#define BGT60TRxxD_CSCI_MADC_ISOPD_pos   8u
#define BGT60TRxxD_CSCI_MADC_ISOPD_msk   0x000100u
#define BGT60TRxxD_CSCI_SADC_ISOPD_pos   9u
#define BGT60TRxxD_CSCI_SADC_ISOPD_msk   0x000200u
#define BGT60TRxxD_CSCI_BG_TMRF_EN_pos   10u
#define BGT60TRxxD_CSCI_BG_TMRF_EN_msk   0x000400u
#define BGT60TRxxD_CSCI_PLL_ISOPD_pos    11u
#define BGT60TRxxD_CSCI_PLL_ISOPD_msk    0x000800u
#define BGT60TRxxD_CSCI_TR_PLL_ISOPD_pos 12u
#define BGT60TRxxD_CSCI_TR_PLL_ISOPD_msk 0x03f000u
#define BGT60TRxxD_CSCI_TR_MADCEN_pos    18u
#define BGT60TRxxD_CSCI_TR_MADCEN_msk    0x1c0000u
#define BGT60TRxxD_CSCI_TR_BGEN_pos      21u
#define BGT60TRxxD_CSCI_TR_BGEN_msk      0xe00000u

/* Fields of register CSP_D_0 */
/* -------------------------- */
#define BGT60TRxxD_CSP_D_0_TX1_EN_pos        0u
#define BGT60TRxxD_CSP_D_0_TX1_EN_msk        0x000001u
#define BGT60TRxxD_CSP_D_0_PD1_EN_pos        1u
#define BGT60TRxxD_CSP_D_0_PD1_EN_msk        0x000002u
#define BGT60TRxxD_CSP_D_0_TX2_EN_pos        2u
#define BGT60TRxxD_CSP_D_0_TX2_EN_msk        0x000004u
#define BGT60TRxxD_CSP_D_0_PD2_EN_pos        3u
#define BGT60TRxxD_CSP_D_0_PD2_EN_msk        0x000008u
#define BGT60TRxxD_CSP_D_0_VCO_EN_pos        4u
#define BGT60TRxxD_CSP_D_0_VCO_EN_msk        0x000010u
#define BGT60TRxxD_CSP_D_0_TEMP_MEAS_EN_pos  5u
#define BGT60TRxxD_CSP_D_0_TEMP_MEAS_EN_msk  0x000020u
#define BGT60TRxxD_CSP_D_0_FDIV_EN_pos       6u
#define BGT60TRxxD_CSP_D_0_FDIV_EN_msk       0x000040u
#define BGT60TRxxD_CSP_D_0_RESERVED1_pos     7u
#define BGT60TRxxD_CSP_D_0_RESERVED1_msk     0x000080u
#define BGT60TRxxD_CSP_D_0_RESERVED2_pos     8u
#define BGT60TRxxD_CSP_D_0_RESERVED2_msk     0x000100u
#define BGT60TRxxD_CSP_D_0_RESERVED3_pos     9u
#define BGT60TRxxD_CSP_D_0_RESERVED3_msk     0x000200u
#define BGT60TRxxD_CSP_D_0_LO_DIST2_EN_pos   10u
#define BGT60TRxxD_CSP_D_0_LO_DIST2_EN_msk   0x000400u
#define BGT60TRxxD_CSP_D_0_LO_DIST1_EN_pos   11u
#define BGT60TRxxD_CSP_D_0_LO_DIST1_EN_msk   0x000800u
#define BGT60TRxxD_CSP_D_0_RX1LOBUF_EN_pos   12u
#define BGT60TRxxD_CSP_D_0_RX1LOBUF_EN_msk   0x001000u
#define BGT60TRxxD_CSP_D_0_RX1MIX_EN_pos     13u
#define BGT60TRxxD_CSP_D_0_RX1MIX_EN_msk     0x002000u
#define BGT60TRxxD_CSP_D_0_RX2LOBUF_EN_pos   14u
#define BGT60TRxxD_CSP_D_0_RX2LOBUF_EN_msk   0x004000u
#define BGT60TRxxD_CSP_D_0_RX2MIX_EN_pos     15u
#define BGT60TRxxD_CSP_D_0_RX2MIX_EN_msk     0x008000u
#define BGT60TRxxD_CSP_D_0_RX3LOBUF_EN_pos   16u
#define BGT60TRxxD_CSP_D_0_RX3LOBUF_EN_msk   0x010000u
#define BGT60TRxxD_CSP_D_0_RX3MIX_EN_pos     17u
#define BGT60TRxxD_CSP_D_0_RX3MIX_EN_msk     0x020000u
#define BGT60TRxxD_CSP_D_0_RX4LOBUF_EN_pos   18u
#define BGT60TRxxD_CSP_D_0_RX4LOBUF_EN_msk   0x040000u
#define BGT60TRxxD_CSP_D_0_RX4MIX_EN_pos     19u
#define BGT60TRxxD_CSP_D_0_RX4MIX_EN_msk     0x080000u
#define BGT60TRxxD_CSP_D_0_ABB1_AAF_CTRL_pos 20u
#define BGT60TRxxD_CSP_D_0_ABB1_AAF_CTRL_msk 0x100000u
#define BGT60TRxxD_CSP_D_0_ABB2_AAF_CTRL_pos 21u
#define BGT60TRxxD_CSP_D_0_ABB2_AAF_CTRL_msk 0x200000u
#define BGT60TRxxD_CSP_D_0_ABB3_AAF_CTRL_pos 22u
#define BGT60TRxxD_CSP_D_0_ABB3_AAF_CTRL_msk 0x400000u
#define BGT60TRxxD_CSP_D_0_ABB4_AAF_CTRL_pos 23u
#define BGT60TRxxD_CSP_D_0_ABB4_AAF_CTRL_msk 0x800000u

/* Fields of register CSP_D_1 */
/* -------------------------- */
#define BGT60TRxxD_CSP_D_1_TX1_DAC_pos       0u
#define BGT60TRxxD_CSP_D_1_TX1_DAC_msk       0x00001fu
#define BGT60TRxxD_CSP_D_1_TX2_DAC_pos       5u
#define BGT60TRxxD_CSP_D_1_TX2_DAC_msk       0x0003e0u
#define BGT60TRxxD_CSP_D_1_HP1_GAIN_pos      10u
#define BGT60TRxxD_CSP_D_1_HP1_GAIN_msk      0x000400u
#define BGT60TRxxD_CSP_D_1_HP2_GAIN_pos      11u
#define BGT60TRxxD_CSP_D_1_HP2_GAIN_msk      0x000800u
#define BGT60TRxxD_CSP_D_1_HP3_GAIN_pos      12u
#define BGT60TRxxD_CSP_D_1_HP3_GAIN_msk      0x001000u
#define BGT60TRxxD_CSP_D_1_HP4_GAIN_pos      13u
#define BGT60TRxxD_CSP_D_1_HP4_GAIN_msk      0x002000u
#define BGT60TRxxD_CSP_D_1_BB_RSTCNT_pos     14u
#define BGT60TRxxD_CSP_D_1_BB_RSTCNT_msk     0x0fc000u
#define BGT60TRxxD_CSP_D_1_MADC_BBCH1_EN_pos 20u
#define BGT60TRxxD_CSP_D_1_MADC_BBCH1_EN_msk 0x100000u
#define BGT60TRxxD_CSP_D_1_MADC_BBCH2_EN_pos 21u
#define BGT60TRxxD_CSP_D_1_MADC_BBCH2_EN_msk 0x200000u
#define BGT60TRxxD_CSP_D_1_MADC_BBCH3_EN_pos 22u
#define BGT60TRxxD_CSP_D_1_MADC_BBCH3_EN_msk 0x400000u
#define BGT60TRxxD_CSP_D_1_MADC_BBCH4_EN_pos 23u
#define BGT60TRxxD_CSP_D_1_MADC_BBCH4_EN_msk 0x800000u

/* Fields of register CSP_D_2 */
/* -------------------------- */
#define BGT60TRxxD_CSP_D_2_HPF_SEL1_pos  0u
#define BGT60TRxxD_CSP_D_2_HPF_SEL1_msk  0x000007u
#define BGT60TRxxD_CSP_D_2_VGA_GAIN1_pos 3u
#define BGT60TRxxD_CSP_D_2_VGA_GAIN1_msk 0x000038u
#define BGT60TRxxD_CSP_D_2_HPF_SEL2_pos  6u
#define BGT60TRxxD_CSP_D_2_HPF_SEL2_msk  0x0001c0u
#define BGT60TRxxD_CSP_D_2_VGA_GAIN2_pos 9u
#define BGT60TRxxD_CSP_D_2_VGA_GAIN2_msk 0x000e00u
#define BGT60TRxxD_CSP_D_2_HPF_SEL3_pos  12u
#define BGT60TRxxD_CSP_D_2_HPF_SEL3_msk  0x007000u
#define BGT60TRxxD_CSP_D_2_VGA_GAIN3_pos 15u
#define BGT60TRxxD_CSP_D_2_VGA_GAIN3_msk 0x038000u
#define BGT60TRxxD_CSP_D_2_HPF_SEL4_pos  18u
#define BGT60TRxxD_CSP_D_2_HPF_SEL4_msk  0x1c0000u
#define BGT60TRxxD_CSP_D_2_VGA_GAIN4_pos 21u
#define BGT60TRxxD_CSP_D_2_VGA_GAIN4_msk 0xe00000u

/* Fields of register CSCDS */
/* ------------------------ */
#define BGT60TRxxD_CSCDS_REPCS_pos      0u
#define BGT60TRxxD_CSCDS_REPCS_msk      0x00000fu
#define BGT60TRxxD_CSCDS_CS_EN_pos      4u
#define BGT60TRxxD_CSCDS_CS_EN_msk      0x000010u
#define BGT60TRxxD_CSCDS_ABB_ISOPD_pos  5u
#define BGT60TRxxD_CSCDS_ABB_ISOPD_msk  0x000020u
#define BGT60TRxxD_CSCDS_RF_ISOPD_pos   6u
#define BGT60TRxxD_CSCDS_RF_ISOPD_msk   0x000040u
#define BGT60TRxxD_CSCDS_MADC_BG_EN_pos 7u
#define BGT60TRxxD_CSCDS_MADC_BG_EN_msk 0x000080u
#define BGT60TRxxD_CSCDS_MADC_ISOPD_pos 8u
#define BGT60TRxxD_CSCDS_MADC_ISOPD_msk 0x000100u
#define BGT60TRxxD_CSCDS_SADC_ISOPD_pos 9u
#define BGT60TRxxD_CSCDS_SADC_ISOPD_msk 0x000200u
#define BGT60TRxxD_CSCDS_BG_TMRF_EN_pos 10u
#define BGT60TRxxD_CSCDS_BG_TMRF_EN_msk 0x000400u
#define BGT60TRxxD_CSCDS_PLL_ISOPD_pos  11u
#define BGT60TRxxD_CSCDS_PLL_ISOPD_msk  0x000800u

/* Fields of register CS1_U_0 */
/* -------------------------- */
#define BGT60TRxxD_CS1_U_0_TX1_EN_pos        0u
#define BGT60TRxxD_CS1_U_0_TX1_EN_msk        0x000001u
#define BGT60TRxxD_CS1_U_0_PD1_EN_pos        1u
#define BGT60TRxxD_CS1_U_0_PD1_EN_msk        0x000002u
#define BGT60TRxxD_CS1_U_0_TX2_EN_pos        2u
#define BGT60TRxxD_CS1_U_0_TX2_EN_msk        0x000004u
#define BGT60TRxxD_CS1_U_0_PD2_EN_pos        3u
#define BGT60TRxxD_CS1_U_0_PD2_EN_msk        0x000008u
#define BGT60TRxxD_CS1_U_0_VCO_EN_pos        4u
#define BGT60TRxxD_CS1_U_0_VCO_EN_msk        0x000010u
#define BGT60TRxxD_CS1_U_0_TEMP_MEAS_EN_pos  5u
#define BGT60TRxxD_CS1_U_0_TEMP_MEAS_EN_msk  0x000020u
#define BGT60TRxxD_CS1_U_0_FDIV_EN_pos       6u
#define BGT60TRxxD_CS1_U_0_FDIV_EN_msk       0x000040u
#define BGT60TRxxD_CS1_U_0_RESERVED1_pos     7u
#define BGT60TRxxD_CS1_U_0_RESERVED1_msk     0x000080u
#define BGT60TRxxD_CS1_U_0_RESERVED2_pos     8u
#define BGT60TRxxD_CS1_U_0_RESERVED2_msk     0x000100u
#define BGT60TRxxD_CS1_U_0_RESERVED3_pos     9u
#define BGT60TRxxD_CS1_U_0_RESERVED3_msk     0x000200u
#define BGT60TRxxD_CS1_U_0_LO_DIST2_EN_pos   10u
#define BGT60TRxxD_CS1_U_0_LO_DIST2_EN_msk   0x000400u
#define BGT60TRxxD_CS1_U_0_LO_DIST1_EN_pos   11u
#define BGT60TRxxD_CS1_U_0_LO_DIST1_EN_msk   0x000800u
#define BGT60TRxxD_CS1_U_0_RX1LOBUF_EN_pos   12u
#define BGT60TRxxD_CS1_U_0_RX1LOBUF_EN_msk   0x001000u
#define BGT60TRxxD_CS1_U_0_RX1MIX_EN_pos     13u
#define BGT60TRxxD_CS1_U_0_RX1MIX_EN_msk     0x002000u
#define BGT60TRxxD_CS1_U_0_RX2LOBUF_EN_pos   14u
#define BGT60TRxxD_CS1_U_0_RX2LOBUF_EN_msk   0x004000u
#define BGT60TRxxD_CS1_U_0_RX2MIX_EN_pos     15u
#define BGT60TRxxD_CS1_U_0_RX2MIX_EN_msk     0x008000u
#define BGT60TRxxD_CS1_U_0_RX3LOBUF_EN_pos   16u
#define BGT60TRxxD_CS1_U_0_RX3LOBUF_EN_msk   0x010000u
#define BGT60TRxxD_CS1_U_0_RX3MIX_EN_pos     17u
#define BGT60TRxxD_CS1_U_0_RX3MIX_EN_msk     0x020000u
#define BGT60TRxxD_CS1_U_0_RX4LOBUF_EN_pos   18u
#define BGT60TRxxD_CS1_U_0_RX4LOBUF_EN_msk   0x040000u
#define BGT60TRxxD_CS1_U_0_RX4MIX_EN_pos     19u
#define BGT60TRxxD_CS1_U_0_RX4MIX_EN_msk     0x080000u
#define BGT60TRxxD_CS1_U_0_ABB1_AAF_CTRL_pos 20u
#define BGT60TRxxD_CS1_U_0_ABB1_AAF_CTRL_msk 0x100000u
#define BGT60TRxxD_CS1_U_0_ABB2_AAF_CTRL_pos 21u
#define BGT60TRxxD_CS1_U_0_ABB2_AAF_CTRL_msk 0x200000u
#define BGT60TRxxD_CS1_U_0_ABB3_AAF_CTRL_pos 22u
#define BGT60TRxxD_CS1_U_0_ABB3_AAF_CTRL_msk 0x400000u
#define BGT60TRxxD_CS1_U_0_ABB4_AAF_CTRL_pos 23u
#define BGT60TRxxD_CS1_U_0_ABB4_AAF_CTRL_msk 0x800000u

/* Fields of register CS1_U_1 */
/* -------------------------- */
#define BGT60TRxxD_CS1_U_1_TX1_DAC_pos       0u
#define BGT60TRxxD_CS1_U_1_TX1_DAC_msk       0x00001fu
#define BGT60TRxxD_CS1_U_1_TX2_DAC_pos       5u
#define BGT60TRxxD_CS1_U_1_TX2_DAC_msk       0x0003e0u
#define BGT60TRxxD_CS1_U_1_HP1_GAIN_pos      10u
#define BGT60TRxxD_CS1_U_1_HP1_GAIN_msk      0x000400u
#define BGT60TRxxD_CS1_U_1_HP2_GAIN_pos      11u
#define BGT60TRxxD_CS1_U_1_HP2_GAIN_msk      0x000800u
#define BGT60TRxxD_CS1_U_1_HP3_GAIN_pos      12u
#define BGT60TRxxD_CS1_U_1_HP3_GAIN_msk      0x001000u
#define BGT60TRxxD_CS1_U_1_HP4_GAIN_pos      13u
#define BGT60TRxxD_CS1_U_1_HP4_GAIN_msk      0x002000u
#define BGT60TRxxD_CS1_U_1_BB_RSTCNT_pos     14u
#define BGT60TRxxD_CS1_U_1_BB_RSTCNT_msk     0x0fc000u
#define BGT60TRxxD_CS1_U_1_MADC_BBCH1_EN_pos 20u
#define BGT60TRxxD_CS1_U_1_MADC_BBCH1_EN_msk 0x100000u
#define BGT60TRxxD_CS1_U_1_MADC_BBCH2_EN_pos 21u
#define BGT60TRxxD_CS1_U_1_MADC_BBCH2_EN_msk 0x200000u
#define BGT60TRxxD_CS1_U_1_MADC_BBCH3_EN_pos 22u
#define BGT60TRxxD_CS1_U_1_MADC_BBCH3_EN_msk 0x400000u
#define BGT60TRxxD_CS1_U_1_MADC_BBCH4_EN_pos 23u
#define BGT60TRxxD_CS1_U_1_MADC_BBCH4_EN_msk 0x800000u

/* Fields of register CS1_U_2 */
/* -------------------------- */
#define BGT60TRxxD_CS1_U_2_HPF_SEL1_pos  0u
#define BGT60TRxxD_CS1_U_2_HPF_SEL1_msk  0x000007u
#define BGT60TRxxD_CS1_U_2_VGA_GAIN1_pos 3u
#define BGT60TRxxD_CS1_U_2_VGA_GAIN1_msk 0x000038u
#define BGT60TRxxD_CS1_U_2_HPF_SEL2_pos  6u
#define BGT60TRxxD_CS1_U_2_HPF_SEL2_msk  0x0001c0u
#define BGT60TRxxD_CS1_U_2_VGA_GAIN2_pos 9u
#define BGT60TRxxD_CS1_U_2_VGA_GAIN2_msk 0x000e00u
#define BGT60TRxxD_CS1_U_2_HPF_SEL3_pos  12u
#define BGT60TRxxD_CS1_U_2_HPF_SEL3_msk  0x007000u
#define BGT60TRxxD_CS1_U_2_VGA_GAIN3_pos 15u
#define BGT60TRxxD_CS1_U_2_VGA_GAIN3_msk 0x038000u
#define BGT60TRxxD_CS1_U_2_HPF_SEL4_pos  18u
#define BGT60TRxxD_CS1_U_2_HPF_SEL4_msk  0x1c0000u
#define BGT60TRxxD_CS1_U_2_VGA_GAIN4_pos 21u
#define BGT60TRxxD_CS1_U_2_VGA_GAIN4_msk 0xe00000u

/* Fields of register CS1_D_0 */
/* -------------------------- */
#define BGT60TRxxD_CS1_D_0_TX1_EN_pos        0u
#define BGT60TRxxD_CS1_D_0_TX1_EN_msk        0x000001u
#define BGT60TRxxD_CS1_D_0_PD1_EN_pos        1u
#define BGT60TRxxD_CS1_D_0_PD1_EN_msk        0x000002u
#define BGT60TRxxD_CS1_D_0_TX2_EN_pos        2u
#define BGT60TRxxD_CS1_D_0_TX2_EN_msk        0x000004u
#define BGT60TRxxD_CS1_D_0_PD2_EN_pos        3u
#define BGT60TRxxD_CS1_D_0_PD2_EN_msk        0x000008u
#define BGT60TRxxD_CS1_D_0_VCO_EN_pos        4u
#define BGT60TRxxD_CS1_D_0_VCO_EN_msk        0x000010u
#define BGT60TRxxD_CS1_D_0_TEMP_MEAS_EN_pos  5u
#define BGT60TRxxD_CS1_D_0_TEMP_MEAS_EN_msk  0x000020u
#define BGT60TRxxD_CS1_D_0_FDIV_EN_pos       6u
#define BGT60TRxxD_CS1_D_0_FDIV_EN_msk       0x000040u
#define BGT60TRxxD_CS1_D_0_RESERVED1_pos     7u
#define BGT60TRxxD_CS1_D_0_RESERVED1_msk     0x000080u
#define BGT60TRxxD_CS1_D_0_RESERVED2_pos     8u
#define BGT60TRxxD_CS1_D_0_RESERVED2_msk     0x000100u
#define BGT60TRxxD_CS1_D_0_RESERVED3_pos     9u
#define BGT60TRxxD_CS1_D_0_RESERVED3_msk     0x000200u
#define BGT60TRxxD_CS1_D_0_LO_DIST2_EN_pos   10u
#define BGT60TRxxD_CS1_D_0_LO_DIST2_EN_msk   0x000400u
#define BGT60TRxxD_CS1_D_0_LO_DIST1_EN_pos   11u
#define BGT60TRxxD_CS1_D_0_LO_DIST1_EN_msk   0x000800u
#define BGT60TRxxD_CS1_D_0_RX1LOBUF_EN_pos   12u
#define BGT60TRxxD_CS1_D_0_RX1LOBUF_EN_msk   0x001000u
#define BGT60TRxxD_CS1_D_0_RX1MIX_EN_pos     13u
#define BGT60TRxxD_CS1_D_0_RX1MIX_EN_msk     0x002000u
#define BGT60TRxxD_CS1_D_0_RX2LOBUF_EN_pos   14u
#define BGT60TRxxD_CS1_D_0_RX2LOBUF_EN_msk   0x004000u
#define BGT60TRxxD_CS1_D_0_RX2MIX_EN_pos     15u
#define BGT60TRxxD_CS1_D_0_RX2MIX_EN_msk     0x008000u
#define BGT60TRxxD_CS1_D_0_RX3LOBUF_EN_pos   16u
#define BGT60TRxxD_CS1_D_0_RX3LOBUF_EN_msk   0x010000u
#define BGT60TRxxD_CS1_D_0_RX3MIX_EN_pos     17u
#define BGT60TRxxD_CS1_D_0_RX3MIX_EN_msk     0x020000u
#define BGT60TRxxD_CS1_D_0_RX4LOBUF_EN_pos   18u
#define BGT60TRxxD_CS1_D_0_RX4LOBUF_EN_msk   0x040000u
#define BGT60TRxxD_CS1_D_0_RX4MIX_EN_pos     19u
#define BGT60TRxxD_CS1_D_0_RX4MIX_EN_msk     0x080000u
#define BGT60TRxxD_CS1_D_0_ABB1_AAF_CTRL_pos 20u
#define BGT60TRxxD_CS1_D_0_ABB1_AAF_CTRL_msk 0x100000u
#define BGT60TRxxD_CS1_D_0_ABB2_AAF_CTRL_pos 21u
#define BGT60TRxxD_CS1_D_0_ABB2_AAF_CTRL_msk 0x200000u
#define BGT60TRxxD_CS1_D_0_ABB3_AAF_CTRL_pos 22u
#define BGT60TRxxD_CS1_D_0_ABB3_AAF_CTRL_msk 0x400000u
#define BGT60TRxxD_CS1_D_0_ABB4_AAF_CTRL_pos 23u
#define BGT60TRxxD_CS1_D_0_ABB4_AAF_CTRL_msk 0x800000u

/* Fields of register CS1_D_1 */
/* -------------------------- */
#define BGT60TRxxD_CS1_D_1_TX1_DAC_pos       0u
#define BGT60TRxxD_CS1_D_1_TX1_DAC_msk       0x00001fu
#define BGT60TRxxD_CS1_D_1_TX2_DAC_pos       5u
#define BGT60TRxxD_CS1_D_1_TX2_DAC_msk       0x0003e0u
#define BGT60TRxxD_CS1_D_1_HP1_GAIN_pos      10u
#define BGT60TRxxD_CS1_D_1_HP1_GAIN_msk      0x000400u
#define BGT60TRxxD_CS1_D_1_HP2_GAIN_pos      11u
#define BGT60TRxxD_CS1_D_1_HP2_GAIN_msk      0x000800u
#define BGT60TRxxD_CS1_D_1_HP3_GAIN_pos      12u
#define BGT60TRxxD_CS1_D_1_HP3_GAIN_msk      0x001000u
#define BGT60TRxxD_CS1_D_1_HP4_GAIN_pos      13u
#define BGT60TRxxD_CS1_D_1_HP4_GAIN_msk      0x002000u
#define BGT60TRxxD_CS1_D_1_BB_RSTCNT_pos     14u
#define BGT60TRxxD_CS1_D_1_BB_RSTCNT_msk     0x0fc000u
#define BGT60TRxxD_CS1_D_1_MADC_BBCH1_EN_pos 20u
#define BGT60TRxxD_CS1_D_1_MADC_BBCH1_EN_msk 0x100000u
#define BGT60TRxxD_CS1_D_1_MADC_BBCH2_EN_pos 21u
#define BGT60TRxxD_CS1_D_1_MADC_BBCH2_EN_msk 0x200000u
#define BGT60TRxxD_CS1_D_1_MADC_BBCH3_EN_pos 22u
#define BGT60TRxxD_CS1_D_1_MADC_BBCH3_EN_msk 0x400000u
#define BGT60TRxxD_CS1_D_1_MADC_BBCH4_EN_pos 23u
#define BGT60TRxxD_CS1_D_1_MADC_BBCH4_EN_msk 0x800000u

/* Fields of register CS1_D_2 */
/* -------------------------- */
#define BGT60TRxxD_CS1_D_2_HPF_SEL1_pos  0u
#define BGT60TRxxD_CS1_D_2_HPF_SEL1_msk  0x000007u
#define BGT60TRxxD_CS1_D_2_VGA_GAIN1_pos 3u
#define BGT60TRxxD_CS1_D_2_VGA_GAIN1_msk 0x000038u
#define BGT60TRxxD_CS1_D_2_HPF_SEL2_pos  6u
#define BGT60TRxxD_CS1_D_2_HPF_SEL2_msk  0x0001c0u
#define BGT60TRxxD_CS1_D_2_VGA_GAIN2_pos 9u
#define BGT60TRxxD_CS1_D_2_VGA_GAIN2_msk 0x000e00u
#define BGT60TRxxD_CS1_D_2_HPF_SEL3_pos  12u
#define BGT60TRxxD_CS1_D_2_HPF_SEL3_msk  0x007000u
#define BGT60TRxxD_CS1_D_2_VGA_GAIN3_pos 15u
#define BGT60TRxxD_CS1_D_2_VGA_GAIN3_msk 0x038000u
#define BGT60TRxxD_CS1_D_2_HPF_SEL4_pos  18u
#define BGT60TRxxD_CS1_D_2_HPF_SEL4_msk  0x1c0000u
#define BGT60TRxxD_CS1_D_2_VGA_GAIN4_pos 21u
#define BGT60TRxxD_CS1_D_2_VGA_GAIN4_msk 0xe00000u

/* Fields of register CS1 */
/* ---------------------- */
#define BGT60TRxxD_CS1_REPCS_pos      0u
#define BGT60TRxxD_CS1_REPCS_msk      0x00000fu
#define BGT60TRxxD_CS1_CS_EN_pos      4u
#define BGT60TRxxD_CS1_CS_EN_msk      0x000010u
#define BGT60TRxxD_CS1_ABB_ISOPD_pos  5u
#define BGT60TRxxD_CS1_ABB_ISOPD_msk  0x000020u
#define BGT60TRxxD_CS1_RF_ISOPD_pos   6u
#define BGT60TRxxD_CS1_RF_ISOPD_msk   0x000040u
#define BGT60TRxxD_CS1_MADC_BG_EN_pos 7u
#define BGT60TRxxD_CS1_MADC_BG_EN_msk 0x000080u
#define BGT60TRxxD_CS1_MADC_ISOPD_pos 8u
#define BGT60TRxxD_CS1_MADC_ISOPD_msk 0x000100u
#define BGT60TRxxD_CS1_SADC_ISOPD_pos 9u
#define BGT60TRxxD_CS1_SADC_ISOPD_msk 0x000200u
#define BGT60TRxxD_CS1_BG_TMRF_EN_pos 10u
#define BGT60TRxxD_CS1_BG_TMRF_EN_msk 0x000400u
#define BGT60TRxxD_CS1_PLL_ISOPD_pos  11u
#define BGT60TRxxD_CS1_PLL_ISOPD_msk  0x000800u

/* Fields of register CS2_U_0 */
/* -------------------------- */
#define BGT60TRxxD_CS2_U_0_TX1_EN_pos        0u
#define BGT60TRxxD_CS2_U_0_TX1_EN_msk        0x000001u
#define BGT60TRxxD_CS2_U_0_PD1_EN_pos        1u
#define BGT60TRxxD_CS2_U_0_PD1_EN_msk        0x000002u
#define BGT60TRxxD_CS2_U_0_TX2_EN_pos        2u
#define BGT60TRxxD_CS2_U_0_TX2_EN_msk        0x000004u
#define BGT60TRxxD_CS2_U_0_PD2_EN_pos        3u
#define BGT60TRxxD_CS2_U_0_PD2_EN_msk        0x000008u
#define BGT60TRxxD_CS2_U_0_VCO_EN_pos        4u
#define BGT60TRxxD_CS2_U_0_VCO_EN_msk        0x000010u
#define BGT60TRxxD_CS2_U_0_TEMP_MEAS_EN_pos  5u
#define BGT60TRxxD_CS2_U_0_TEMP_MEAS_EN_msk  0x000020u
#define BGT60TRxxD_CS2_U_0_FDIV_EN_pos       6u
#define BGT60TRxxD_CS2_U_0_FDIV_EN_msk       0x000040u
#define BGT60TRxxD_CS2_U_0_RESERVED1_pos     7u
#define BGT60TRxxD_CS2_U_0_RESERVED1_msk     0x000080u
#define BGT60TRxxD_CS2_U_0_RESERVED2_pos     8u
#define BGT60TRxxD_CS2_U_0_RESERVED2_msk     0x000100u
#define BGT60TRxxD_CS2_U_0_RESERVED3_pos     9u
#define BGT60TRxxD_CS2_U_0_RESERVED3_msk     0x000200u
#define BGT60TRxxD_CS2_U_0_LO_DIST2_EN_pos   10u
#define BGT60TRxxD_CS2_U_0_LO_DIST2_EN_msk   0x000400u
#define BGT60TRxxD_CS2_U_0_LO_DIST1_EN_pos   11u
#define BGT60TRxxD_CS2_U_0_LO_DIST1_EN_msk   0x000800u
#define BGT60TRxxD_CS2_U_0_RX1LOBUF_EN_pos   12u
#define BGT60TRxxD_CS2_U_0_RX1LOBUF_EN_msk   0x001000u
#define BGT60TRxxD_CS2_U_0_RX1MIX_EN_pos     13u
#define BGT60TRxxD_CS2_U_0_RX1MIX_EN_msk     0x002000u
#define BGT60TRxxD_CS2_U_0_RX2LOBUF_EN_pos   14u
#define BGT60TRxxD_CS2_U_0_RX2LOBUF_EN_msk   0x004000u
#define BGT60TRxxD_CS2_U_0_RX2MIX_EN_pos     15u
#define BGT60TRxxD_CS2_U_0_RX2MIX_EN_msk     0x008000u
#define BGT60TRxxD_CS2_U_0_RX3LOBUF_EN_pos   16u
#define BGT60TRxxD_CS2_U_0_RX3LOBUF_EN_msk   0x010000u
#define BGT60TRxxD_CS2_U_0_RX3MIX_EN_pos     17u
#define BGT60TRxxD_CS2_U_0_RX3MIX_EN_msk     0x020000u
#define BGT60TRxxD_CS2_U_0_RX4LOBUF_EN_pos   18u
#define BGT60TRxxD_CS2_U_0_RX4LOBUF_EN_msk   0x040000u
#define BGT60TRxxD_CS2_U_0_RX4MIX_EN_pos     19u
#define BGT60TRxxD_CS2_U_0_RX4MIX_EN_msk     0x080000u
#define BGT60TRxxD_CS2_U_0_ABB1_AAF_CTRL_pos 20u
#define BGT60TRxxD_CS2_U_0_ABB1_AAF_CTRL_msk 0x100000u
#define BGT60TRxxD_CS2_U_0_ABB2_AAF_CTRL_pos 21u
#define BGT60TRxxD_CS2_U_0_ABB2_AAF_CTRL_msk 0x200000u
#define BGT60TRxxD_CS2_U_0_ABB3_AAF_CTRL_pos 22u
#define BGT60TRxxD_CS2_U_0_ABB3_AAF_CTRL_msk 0x400000u
#define BGT60TRxxD_CS2_U_0_ABB4_AAF_CTRL_pos 23u
#define BGT60TRxxD_CS2_U_0_ABB4_AAF_CTRL_msk 0x800000u

/* Fields of register CS2_U_1 */
/* -------------------------- */
#define BGT60TRxxD_CS2_U_1_TX1_DAC_pos       0u
#define BGT60TRxxD_CS2_U_1_TX1_DAC_msk       0x00001fu
#define BGT60TRxxD_CS2_U_1_TX2_DAC_pos       5u
#define BGT60TRxxD_CS2_U_1_TX2_DAC_msk       0x0003e0u
#define BGT60TRxxD_CS2_U_1_HP1_GAIN_pos      10u
#define BGT60TRxxD_CS2_U_1_HP1_GAIN_msk      0x000400u
#define BGT60TRxxD_CS2_U_1_HP2_GAIN_pos      11u
#define BGT60TRxxD_CS2_U_1_HP2_GAIN_msk      0x000800u
#define BGT60TRxxD_CS2_U_1_HP3_GAIN_pos      12u
#define BGT60TRxxD_CS2_U_1_HP3_GAIN_msk      0x001000u
#define BGT60TRxxD_CS2_U_1_HP4_GAIN_pos      13u
#define BGT60TRxxD_CS2_U_1_HP4_GAIN_msk      0x002000u
#define BGT60TRxxD_CS2_U_1_BB_RSTCNT_pos     14u
#define BGT60TRxxD_CS2_U_1_BB_RSTCNT_msk     0x0fc000u
#define BGT60TRxxD_CS2_U_1_MADC_BBCH1_EN_pos 20u
#define BGT60TRxxD_CS2_U_1_MADC_BBCH1_EN_msk 0x100000u
#define BGT60TRxxD_CS2_U_1_MADC_BBCH2_EN_pos 21u
#define BGT60TRxxD_CS2_U_1_MADC_BBCH2_EN_msk 0x200000u
#define BGT60TRxxD_CS2_U_1_MADC_BBCH3_EN_pos 22u
#define BGT60TRxxD_CS2_U_1_MADC_BBCH3_EN_msk 0x400000u
#define BGT60TRxxD_CS2_U_1_MADC_BBCH4_EN_pos 23u
#define BGT60TRxxD_CS2_U_1_MADC_BBCH4_EN_msk 0x800000u

/* Fields of register CS2_U_2 */
/* -------------------------- */
#define BGT60TRxxD_CS2_U_2_HPF_SEL1_pos  0u
#define BGT60TRxxD_CS2_U_2_HPF_SEL1_msk  0x000007u
#define BGT60TRxxD_CS2_U_2_VGA_GAIN1_pos 3u
#define BGT60TRxxD_CS2_U_2_VGA_GAIN1_msk 0x000038u
#define BGT60TRxxD_CS2_U_2_HPF_SEL2_pos  6u
#define BGT60TRxxD_CS2_U_2_HPF_SEL2_msk  0x0001c0u
#define BGT60TRxxD_CS2_U_2_VGA_GAIN2_pos 9u
#define BGT60TRxxD_CS2_U_2_VGA_GAIN2_msk 0x000e00u
#define BGT60TRxxD_CS2_U_2_HPF_SEL3_pos  12u
#define BGT60TRxxD_CS2_U_2_HPF_SEL3_msk  0x007000u
#define BGT60TRxxD_CS2_U_2_VGA_GAIN3_pos 15u
#define BGT60TRxxD_CS2_U_2_VGA_GAIN3_msk 0x038000u
#define BGT60TRxxD_CS2_U_2_HPF_SEL4_pos  18u
#define BGT60TRxxD_CS2_U_2_HPF_SEL4_msk  0x1c0000u
#define BGT60TRxxD_CS2_U_2_VGA_GAIN4_pos 21u
#define BGT60TRxxD_CS2_U_2_VGA_GAIN4_msk 0xe00000u

/* Fields of register CS2_D_0 */
/* -------------------------- */
#define BGT60TRxxD_CS2_D_0_TX1_EN_pos        0u
#define BGT60TRxxD_CS2_D_0_TX1_EN_msk        0x000001u
#define BGT60TRxxD_CS2_D_0_PD1_EN_pos        1u
#define BGT60TRxxD_CS2_D_0_PD1_EN_msk        0x000002u
#define BGT60TRxxD_CS2_D_0_TX2_EN_pos        2u
#define BGT60TRxxD_CS2_D_0_TX2_EN_msk        0x000004u
#define BGT60TRxxD_CS2_D_0_PD2_EN_pos        3u
#define BGT60TRxxD_CS2_D_0_PD2_EN_msk        0x000008u
#define BGT60TRxxD_CS2_D_0_VCO_EN_pos        4u
#define BGT60TRxxD_CS2_D_0_VCO_EN_msk        0x000010u
#define BGT60TRxxD_CS2_D_0_TEMP_MEAS_EN_pos  5u
#define BGT60TRxxD_CS2_D_0_TEMP_MEAS_EN_msk  0x000020u
#define BGT60TRxxD_CS2_D_0_FDIV_EN_pos       6u
#define BGT60TRxxD_CS2_D_0_FDIV_EN_msk       0x000040u
#define BGT60TRxxD_CS2_D_0_RESERVED1_pos     7u
#define BGT60TRxxD_CS2_D_0_RESERVED1_msk     0x000080u
#define BGT60TRxxD_CS2_D_0_RESERVED2_pos     8u
#define BGT60TRxxD_CS2_D_0_RESERVED2_msk     0x000100u
#define BGT60TRxxD_CS2_D_0_RESERVED3_pos     9u
#define BGT60TRxxD_CS2_D_0_RESERVED3_msk     0x000200u
#define BGT60TRxxD_CS2_D_0_LO_DIST2_EN_pos   10u
#define BGT60TRxxD_CS2_D_0_LO_DIST2_EN_msk   0x000400u
#define BGT60TRxxD_CS2_D_0_LO_DIST1_EN_pos   11u
#define BGT60TRxxD_CS2_D_0_LO_DIST1_EN_msk   0x000800u
#define BGT60TRxxD_CS2_D_0_RX1LOBUF_EN_pos   12u
#define BGT60TRxxD_CS2_D_0_RX1LOBUF_EN_msk   0x001000u
#define BGT60TRxxD_CS2_D_0_RX1MIX_EN_pos     13u
#define BGT60TRxxD_CS2_D_0_RX1MIX_EN_msk     0x002000u
#define BGT60TRxxD_CS2_D_0_RX2LOBUF_EN_pos   14u
#define BGT60TRxxD_CS2_D_0_RX2LOBUF_EN_msk   0x004000u
#define BGT60TRxxD_CS2_D_0_RX2MIX_EN_pos     15u
#define BGT60TRxxD_CS2_D_0_RX2MIX_EN_msk     0x008000u
#define BGT60TRxxD_CS2_D_0_RX3LOBUF_EN_pos   16u
#define BGT60TRxxD_CS2_D_0_RX3LOBUF_EN_msk   0x010000u
#define BGT60TRxxD_CS2_D_0_RX3MIX_EN_pos     17u
#define BGT60TRxxD_CS2_D_0_RX3MIX_EN_msk     0x020000u
#define BGT60TRxxD_CS2_D_0_RX4LOBUF_EN_pos   18u
#define BGT60TRxxD_CS2_D_0_RX4LOBUF_EN_msk   0x040000u
#define BGT60TRxxD_CS2_D_0_RX4MIX_EN_pos     19u
#define BGT60TRxxD_CS2_D_0_RX4MIX_EN_msk     0x080000u
#define BGT60TRxxD_CS2_D_0_ABB1_AAF_CTRL_pos 20u
#define BGT60TRxxD_CS2_D_0_ABB1_AAF_CTRL_msk 0x100000u
#define BGT60TRxxD_CS2_D_0_ABB2_AAF_CTRL_pos 21u
#define BGT60TRxxD_CS2_D_0_ABB2_AAF_CTRL_msk 0x200000u
#define BGT60TRxxD_CS2_D_0_ABB3_AAF_CTRL_pos 22u
#define BGT60TRxxD_CS2_D_0_ABB3_AAF_CTRL_msk 0x400000u
#define BGT60TRxxD_CS2_D_0_ABB4_AAF_CTRL_pos 23u
#define BGT60TRxxD_CS2_D_0_ABB4_AAF_CTRL_msk 0x800000u

/* Fields of register CS2_D_1 */
/* -------------------------- */
#define BGT60TRxxD_CS2_D_1_TX1_DAC_pos       0u
#define BGT60TRxxD_CS2_D_1_TX1_DAC_msk       0x00001fu
#define BGT60TRxxD_CS2_D_1_TX2_DAC_pos       5u
#define BGT60TRxxD_CS2_D_1_TX2_DAC_msk       0x0003e0u
#define BGT60TRxxD_CS2_D_1_HP1_GAIN_pos      10u
#define BGT60TRxxD_CS2_D_1_HP1_GAIN_msk      0x000400u
#define BGT60TRxxD_CS2_D_1_HP2_GAIN_pos      11u
#define BGT60TRxxD_CS2_D_1_HP2_GAIN_msk      0x000800u
#define BGT60TRxxD_CS2_D_1_HP3_GAIN_pos      12u
#define BGT60TRxxD_CS2_D_1_HP3_GAIN_msk      0x001000u
#define BGT60TRxxD_CS2_D_1_HP4_GAIN_pos      13u
#define BGT60TRxxD_CS2_D_1_HP4_GAIN_msk      0x002000u
#define BGT60TRxxD_CS2_D_1_BB_RSTCNT_pos     14u
#define BGT60TRxxD_CS2_D_1_BB_RSTCNT_msk     0x0fc000u
#define BGT60TRxxD_CS2_D_1_MADC_BBCH1_EN_pos 20u
#define BGT60TRxxD_CS2_D_1_MADC_BBCH1_EN_msk 0x100000u
#define BGT60TRxxD_CS2_D_1_MADC_BBCH2_EN_pos 21u
#define BGT60TRxxD_CS2_D_1_MADC_BBCH2_EN_msk 0x200000u
#define BGT60TRxxD_CS2_D_1_MADC_BBCH3_EN_pos 22u
#define BGT60TRxxD_CS2_D_1_MADC_BBCH3_EN_msk 0x400000u
#define BGT60TRxxD_CS2_D_1_MADC_BBCH4_EN_pos 23u
#define BGT60TRxxD_CS2_D_1_MADC_BBCH4_EN_msk 0x800000u

/* Fields of register CS2_D_2 */
/* -------------------------- */
#define BGT60TRxxD_CS2_D_2_HPF_SEL1_pos  0u
#define BGT60TRxxD_CS2_D_2_HPF_SEL1_msk  0x000007u
#define BGT60TRxxD_CS2_D_2_VGA_GAIN1_pos 3u
#define BGT60TRxxD_CS2_D_2_VGA_GAIN1_msk 0x000038u
#define BGT60TRxxD_CS2_D_2_HPF_SEL2_pos  6u
#define BGT60TRxxD_CS2_D_2_HPF_SEL2_msk  0x0001c0u
#define BGT60TRxxD_CS2_D_2_VGA_GAIN2_pos 9u
#define BGT60TRxxD_CS2_D_2_VGA_GAIN2_msk 0x000e00u
#define BGT60TRxxD_CS2_D_2_HPF_SEL3_pos  12u
#define BGT60TRxxD_CS2_D_2_HPF_SEL3_msk  0x007000u
#define BGT60TRxxD_CS2_D_2_VGA_GAIN3_pos 15u
#define BGT60TRxxD_CS2_D_2_VGA_GAIN3_msk 0x038000u
#define BGT60TRxxD_CS2_D_2_HPF_SEL4_pos  18u
#define BGT60TRxxD_CS2_D_2_HPF_SEL4_msk  0x1c0000u
#define BGT60TRxxD_CS2_D_2_VGA_GAIN4_pos 21u
#define BGT60TRxxD_CS2_D_2_VGA_GAIN4_msk 0xe00000u

/* Fields of register CS2 */
/* ---------------------- */
#define BGT60TRxxD_CS2_REPCS_pos      0u
#define BGT60TRxxD_CS2_REPCS_msk      0x00000fu
#define BGT60TRxxD_CS2_CS_EN_pos      4u
#define BGT60TRxxD_CS2_CS_EN_msk      0x000010u
#define BGT60TRxxD_CS2_ABB_ISOPD_pos  5u
#define BGT60TRxxD_CS2_ABB_ISOPD_msk  0x000020u
#define BGT60TRxxD_CS2_RF_ISOPD_pos   6u
#define BGT60TRxxD_CS2_RF_ISOPD_msk   0x000040u
#define BGT60TRxxD_CS2_MADC_BG_EN_pos 7u
#define BGT60TRxxD_CS2_MADC_BG_EN_msk 0x000080u
#define BGT60TRxxD_CS2_MADC_ISOPD_pos 8u
#define BGT60TRxxD_CS2_MADC_ISOPD_msk 0x000100u
#define BGT60TRxxD_CS2_SADC_ISOPD_pos 9u
#define BGT60TRxxD_CS2_SADC_ISOPD_msk 0x000200u
#define BGT60TRxxD_CS2_BG_TMRF_EN_pos 10u
#define BGT60TRxxD_CS2_BG_TMRF_EN_msk 0x000400u
#define BGT60TRxxD_CS2_PLL_ISOPD_pos  11u
#define BGT60TRxxD_CS2_PLL_ISOPD_msk  0x000800u

/* Fields of register CS3_U_0 */
/* -------------------------- */
#define BGT60TRxxD_CS3_U_0_TX1_EN_pos        0u
#define BGT60TRxxD_CS3_U_0_TX1_EN_msk        0x000001u
#define BGT60TRxxD_CS3_U_0_PD1_EN_pos        1u
#define BGT60TRxxD_CS3_U_0_PD1_EN_msk        0x000002u
#define BGT60TRxxD_CS3_U_0_TX2_EN_pos        2u
#define BGT60TRxxD_CS3_U_0_TX2_EN_msk        0x000004u
#define BGT60TRxxD_CS3_U_0_PD2_EN_pos        3u
#define BGT60TRxxD_CS3_U_0_PD2_EN_msk        0x000008u
#define BGT60TRxxD_CS3_U_0_VCO_EN_pos        4u
#define BGT60TRxxD_CS3_U_0_VCO_EN_msk        0x000010u
#define BGT60TRxxD_CS3_U_0_TEMP_MEAS_EN_pos  5u
#define BGT60TRxxD_CS3_U_0_TEMP_MEAS_EN_msk  0x000020u
#define BGT60TRxxD_CS3_U_0_FDIV_EN_pos       6u
#define BGT60TRxxD_CS3_U_0_FDIV_EN_msk       0x000040u
#define BGT60TRxxD_CS3_U_0_RESERVED1_pos     7u
#define BGT60TRxxD_CS3_U_0_RESERVED1_msk     0x000080u
#define BGT60TRxxD_CS3_U_0_RESERVED2_pos     8u
#define BGT60TRxxD_CS3_U_0_RESERVED2_msk     0x000100u
#define BGT60TRxxD_CS3_U_0_RESERVED3_pos     9u
#define BGT60TRxxD_CS3_U_0_RESERVED3_msk     0x000200u
#define BGT60TRxxD_CS3_U_0_LO_DIST2_EN_pos   10u
#define BGT60TRxxD_CS3_U_0_LO_DIST2_EN_msk   0x000400u
#define BGT60TRxxD_CS3_U_0_LO_DIST1_EN_pos   11u
#define BGT60TRxxD_CS3_U_0_LO_DIST1_EN_msk   0x000800u
#define BGT60TRxxD_CS3_U_0_RX1LOBUF_EN_pos   12u
#define BGT60TRxxD_CS3_U_0_RX1LOBUF_EN_msk   0x001000u
#define BGT60TRxxD_CS3_U_0_RX1MIX_EN_pos     13u
#define BGT60TRxxD_CS3_U_0_RX1MIX_EN_msk     0x002000u
#define BGT60TRxxD_CS3_U_0_RX2LOBUF_EN_pos   14u
#define BGT60TRxxD_CS3_U_0_RX2LOBUF_EN_msk   0x004000u
#define BGT60TRxxD_CS3_U_0_RX2MIX_EN_pos     15u
#define BGT60TRxxD_CS3_U_0_RX2MIX_EN_msk     0x008000u
#define BGT60TRxxD_CS3_U_0_RX3LOBUF_EN_pos   16u
#define BGT60TRxxD_CS3_U_0_RX3LOBUF_EN_msk   0x010000u
#define BGT60TRxxD_CS3_U_0_RX3MIX_EN_pos     17u
#define BGT60TRxxD_CS3_U_0_RX3MIX_EN_msk     0x020000u
#define BGT60TRxxD_CS3_U_0_RX4LOBUF_EN_pos   18u
#define BGT60TRxxD_CS3_U_0_RX4LOBUF_EN_msk   0x040000u
#define BGT60TRxxD_CS3_U_0_RX4MIX_EN_pos     19u
#define BGT60TRxxD_CS3_U_0_RX4MIX_EN_msk     0x080000u
#define BGT60TRxxD_CS3_U_0_ABB1_AAF_CTRL_pos 20u
#define BGT60TRxxD_CS3_U_0_ABB1_AAF_CTRL_msk 0x100000u
#define BGT60TRxxD_CS3_U_0_ABB2_AAF_CTRL_pos 21u
#define BGT60TRxxD_CS3_U_0_ABB2_AAF_CTRL_msk 0x200000u
#define BGT60TRxxD_CS3_U_0_ABB3_AAF_CTRL_pos 22u
#define BGT60TRxxD_CS3_U_0_ABB3_AAF_CTRL_msk 0x400000u
#define BGT60TRxxD_CS3_U_0_ABB4_AAF_CTRL_pos 23u
#define BGT60TRxxD_CS3_U_0_ABB4_AAF_CTRL_msk 0x800000u

/* Fields of register CS3_U_1 */
/* -------------------------- */
#define BGT60TRxxD_CS3_U_1_TX1_DAC_pos       0u
#define BGT60TRxxD_CS3_U_1_TX1_DAC_msk       0x00001fu
#define BGT60TRxxD_CS3_U_1_TX2_DAC_pos       5u
#define BGT60TRxxD_CS3_U_1_TX2_DAC_msk       0x0003e0u
#define BGT60TRxxD_CS3_U_1_HP1_GAIN_pos      10u
#define BGT60TRxxD_CS3_U_1_HP1_GAIN_msk      0x000400u
#define BGT60TRxxD_CS3_U_1_HP2_GAIN_pos      11u
#define BGT60TRxxD_CS3_U_1_HP2_GAIN_msk      0x000800u
#define BGT60TRxxD_CS3_U_1_HP3_GAIN_pos      12u
#define BGT60TRxxD_CS3_U_1_HP3_GAIN_msk      0x001000u
#define BGT60TRxxD_CS3_U_1_HP4_GAIN_pos      13u
#define BGT60TRxxD_CS3_U_1_HP4_GAIN_msk      0x002000u
#define BGT60TRxxD_CS3_U_1_BB_RSTCNT_pos     14u
#define BGT60TRxxD_CS3_U_1_BB_RSTCNT_msk     0x0fc000u
#define BGT60TRxxD_CS3_U_1_MADC_BBCH1_EN_pos 20u
#define BGT60TRxxD_CS3_U_1_MADC_BBCH1_EN_msk 0x100000u
#define BGT60TRxxD_CS3_U_1_MADC_BBCH2_EN_pos 21u
#define BGT60TRxxD_CS3_U_1_MADC_BBCH2_EN_msk 0x200000u
#define BGT60TRxxD_CS3_U_1_MADC_BBCH3_EN_pos 22u
#define BGT60TRxxD_CS3_U_1_MADC_BBCH3_EN_msk 0x400000u
#define BGT60TRxxD_CS3_U_1_MADC_BBCH4_EN_pos 23u
#define BGT60TRxxD_CS3_U_1_MADC_BBCH4_EN_msk 0x800000u

/* Fields of register CS3_U_2 */
/* -------------------------- */
#define BGT60TRxxD_CS3_U_2_HPF_SEL1_pos  0u
#define BGT60TRxxD_CS3_U_2_HPF_SEL1_msk  0x000007u
#define BGT60TRxxD_CS3_U_2_VGA_GAIN1_pos 3u
#define BGT60TRxxD_CS3_U_2_VGA_GAIN1_msk 0x000038u
#define BGT60TRxxD_CS3_U_2_HPF_SEL2_pos  6u
#define BGT60TRxxD_CS3_U_2_HPF_SEL2_msk  0x0001c0u
#define BGT60TRxxD_CS3_U_2_VGA_GAIN2_pos 9u
#define BGT60TRxxD_CS3_U_2_VGA_GAIN2_msk 0x000e00u
#define BGT60TRxxD_CS3_U_2_HPF_SEL3_pos  12u
#define BGT60TRxxD_CS3_U_2_HPF_SEL3_msk  0x007000u
#define BGT60TRxxD_CS3_U_2_VGA_GAIN3_pos 15u
#define BGT60TRxxD_CS3_U_2_VGA_GAIN3_msk 0x038000u
#define BGT60TRxxD_CS3_U_2_HPF_SEL4_pos  18u
#define BGT60TRxxD_CS3_U_2_HPF_SEL4_msk  0x1c0000u
#define BGT60TRxxD_CS3_U_2_VGA_GAIN4_pos 21u
#define BGT60TRxxD_CS3_U_2_VGA_GAIN4_msk 0xe00000u

/* Fields of register CS3_D_0 */
/* -------------------------- */
#define BGT60TRxxD_CS3_D_0_TX1_EN_pos        0u
#define BGT60TRxxD_CS3_D_0_TX1_EN_msk        0x000001u
#define BGT60TRxxD_CS3_D_0_PD1_EN_pos        1u
#define BGT60TRxxD_CS3_D_0_PD1_EN_msk        0x000002u
#define BGT60TRxxD_CS3_D_0_TX2_EN_pos        2u
#define BGT60TRxxD_CS3_D_0_TX2_EN_msk        0x000004u
#define BGT60TRxxD_CS3_D_0_PD2_EN_pos        3u
#define BGT60TRxxD_CS3_D_0_PD2_EN_msk        0x000008u
#define BGT60TRxxD_CS3_D_0_VCO_EN_pos        4u
#define BGT60TRxxD_CS3_D_0_VCO_EN_msk        0x000010u
#define BGT60TRxxD_CS3_D_0_TEMP_MEAS_EN_pos  5u
#define BGT60TRxxD_CS3_D_0_TEMP_MEAS_EN_msk  0x000020u
#define BGT60TRxxD_CS3_D_0_FDIV_EN_pos       6u
#define BGT60TRxxD_CS3_D_0_FDIV_EN_msk       0x000040u
#define BGT60TRxxD_CS3_D_0_RESERVED1_pos     7u
#define BGT60TRxxD_CS3_D_0_RESERVED1_msk     0x000080u
#define BGT60TRxxD_CS3_D_0_RESERVED2_pos     8u
#define BGT60TRxxD_CS3_D_0_RESERVED2_msk     0x000100u
#define BGT60TRxxD_CS3_D_0_RESERVED3_pos     9u
#define BGT60TRxxD_CS3_D_0_RESERVED3_msk     0x000200u
#define BGT60TRxxD_CS3_D_0_LO_DIST2_EN_pos   10u
#define BGT60TRxxD_CS3_D_0_LO_DIST2_EN_msk   0x000400u
#define BGT60TRxxD_CS3_D_0_LO_DIST1_EN_pos   11u
#define BGT60TRxxD_CS3_D_0_LO_DIST1_EN_msk   0x000800u
#define BGT60TRxxD_CS3_D_0_RX1LOBUF_EN_pos   12u
#define BGT60TRxxD_CS3_D_0_RX1LOBUF_EN_msk   0x001000u
#define BGT60TRxxD_CS3_D_0_RX1MIX_EN_pos     13u
#define BGT60TRxxD_CS3_D_0_RX1MIX_EN_msk     0x002000u
#define BGT60TRxxD_CS3_D_0_RX2LOBUF_EN_pos   14u
#define BGT60TRxxD_CS3_D_0_RX2LOBUF_EN_msk   0x004000u
#define BGT60TRxxD_CS3_D_0_RX2MIX_EN_pos     15u
#define BGT60TRxxD_CS3_D_0_RX2MIX_EN_msk     0x008000u
#define BGT60TRxxD_CS3_D_0_RX3LOBUF_EN_pos   16u
#define BGT60TRxxD_CS3_D_0_RX3LOBUF_EN_msk   0x010000u
#define BGT60TRxxD_CS3_D_0_RX3MIX_EN_pos     17u
#define BGT60TRxxD_CS3_D_0_RX3MIX_EN_msk     0x020000u
#define BGT60TRxxD_CS3_D_0_RX4LOBUF_EN_pos   18u
#define BGT60TRxxD_CS3_D_0_RX4LOBUF_EN_msk   0x040000u
#define BGT60TRxxD_CS3_D_0_RX4MIX_EN_pos     19u
#define BGT60TRxxD_CS3_D_0_RX4MIX_EN_msk     0x080000u
#define BGT60TRxxD_CS3_D_0_ABB1_AAF_CTRL_pos 20u
#define BGT60TRxxD_CS3_D_0_ABB1_AAF_CTRL_msk 0x100000u
#define BGT60TRxxD_CS3_D_0_ABB2_AAF_CTRL_pos 21u
#define BGT60TRxxD_CS3_D_0_ABB2_AAF_CTRL_msk 0x200000u
#define BGT60TRxxD_CS3_D_0_ABB3_AAF_CTRL_pos 22u
#define BGT60TRxxD_CS3_D_0_ABB3_AAF_CTRL_msk 0x400000u
#define BGT60TRxxD_CS3_D_0_ABB4_AAF_CTRL_pos 23u
#define BGT60TRxxD_CS3_D_0_ABB4_AAF_CTRL_msk 0x800000u

/* Fields of register CS3_D_1 */
/* -------------------------- */
#define BGT60TRxxD_CS3_D_1_TX1_DAC_pos       0u
#define BGT60TRxxD_CS3_D_1_TX1_DAC_msk       0x00001fu
#define BGT60TRxxD_CS3_D_1_TX2_DAC_pos       5u
#define BGT60TRxxD_CS3_D_1_TX2_DAC_msk       0x0003e0u
#define BGT60TRxxD_CS3_D_1_HP1_GAIN_pos      10u
#define BGT60TRxxD_CS3_D_1_HP1_GAIN_msk      0x000400u
#define BGT60TRxxD_CS3_D_1_HP2_GAIN_pos      11u
#define BGT60TRxxD_CS3_D_1_HP2_GAIN_msk      0x000800u
#define BGT60TRxxD_CS3_D_1_HP3_GAIN_pos      12u
#define BGT60TRxxD_CS3_D_1_HP3_GAIN_msk      0x001000u
#define BGT60TRxxD_CS3_D_1_HP4_GAIN_pos      13u
#define BGT60TRxxD_CS3_D_1_HP4_GAIN_msk      0x002000u
#define BGT60TRxxD_CS3_D_1_BB_RSTCNT_pos     14u
#define BGT60TRxxD_CS3_D_1_BB_RSTCNT_msk     0x0fc000u
#define BGT60TRxxD_CS3_D_1_MADC_BBCH1_EN_pos 20u
#define BGT60TRxxD_CS3_D_1_MADC_BBCH1_EN_msk 0x100000u
#define BGT60TRxxD_CS3_D_1_MADC_BBCH2_EN_pos 21u
#define BGT60TRxxD_CS3_D_1_MADC_BBCH2_EN_msk 0x200000u
#define BGT60TRxxD_CS3_D_1_MADC_BBCH3_EN_pos 22u
#define BGT60TRxxD_CS3_D_1_MADC_BBCH3_EN_msk 0x400000u
#define BGT60TRxxD_CS3_D_1_MADC_BBCH4_EN_pos 23u
#define BGT60TRxxD_CS3_D_1_MADC_BBCH4_EN_msk 0x800000u

/* Fields of register CS3_D_2 */
/* -------------------------- */
#define BGT60TRxxD_CS3_D_2_HPF_SEL1_pos  0u
#define BGT60TRxxD_CS3_D_2_HPF_SEL1_msk  0x000007u
#define BGT60TRxxD_CS3_D_2_VGA_GAIN1_pos 3u
#define BGT60TRxxD_CS3_D_2_VGA_GAIN1_msk 0x000038u
#define BGT60TRxxD_CS3_D_2_HPF_SEL2_pos  6u
#define BGT60TRxxD_CS3_D_2_HPF_SEL2_msk  0x0001c0u
#define BGT60TRxxD_CS3_D_2_VGA_GAIN2_pos 9u
#define BGT60TRxxD_CS3_D_2_VGA_GAIN2_msk 0x000e00u
#define BGT60TRxxD_CS3_D_2_HPF_SEL3_pos  12u
#define BGT60TRxxD_CS3_D_2_HPF_SEL3_msk  0x007000u
#define BGT60TRxxD_CS3_D_2_VGA_GAIN3_pos 15u
#define BGT60TRxxD_CS3_D_2_VGA_GAIN3_msk 0x038000u
#define BGT60TRxxD_CS3_D_2_HPF_SEL4_pos  18u
#define BGT60TRxxD_CS3_D_2_HPF_SEL4_msk  0x1c0000u
#define BGT60TRxxD_CS3_D_2_VGA_GAIN4_pos 21u
#define BGT60TRxxD_CS3_D_2_VGA_GAIN4_msk 0xe00000u

/* Fields of register CS3 */
/* ---------------------- */
#define BGT60TRxxD_CS3_REPCS_pos      0u
#define BGT60TRxxD_CS3_REPCS_msk      0x00000fu
#define BGT60TRxxD_CS3_CS_EN_pos      4u
#define BGT60TRxxD_CS3_CS_EN_msk      0x000010u
#define BGT60TRxxD_CS3_ABB_ISOPD_pos  5u
#define BGT60TRxxD_CS3_ABB_ISOPD_msk  0x000020u
#define BGT60TRxxD_CS3_RF_ISOPD_pos   6u
#define BGT60TRxxD_CS3_RF_ISOPD_msk   0x000040u
#define BGT60TRxxD_CS3_MADC_BG_EN_pos 7u
#define BGT60TRxxD_CS3_MADC_BG_EN_msk 0x000080u
#define BGT60TRxxD_CS3_MADC_ISOPD_pos 8u
#define BGT60TRxxD_CS3_MADC_ISOPD_msk 0x000100u
#define BGT60TRxxD_CS3_SADC_ISOPD_pos 9u
#define BGT60TRxxD_CS3_SADC_ISOPD_msk 0x000200u
#define BGT60TRxxD_CS3_BG_TMRF_EN_pos 10u
#define BGT60TRxxD_CS3_BG_TMRF_EN_msk 0x000400u
#define BGT60TRxxD_CS3_PLL_ISOPD_pos  11u
#define BGT60TRxxD_CS3_PLL_ISOPD_msk  0x000800u

/* Fields of register CS4_U_0 */
/* -------------------------- */
#define BGT60TRxxD_CS4_U_0_TX1_EN_pos        0u
#define BGT60TRxxD_CS4_U_0_TX1_EN_msk        0x000001u
#define BGT60TRxxD_CS4_U_0_PD1_EN_pos        1u
#define BGT60TRxxD_CS4_U_0_PD1_EN_msk        0x000002u
#define BGT60TRxxD_CS4_U_0_TX2_EN_pos        2u
#define BGT60TRxxD_CS4_U_0_TX2_EN_msk        0x000004u
#define BGT60TRxxD_CS4_U_0_PD2_EN_pos        3u
#define BGT60TRxxD_CS4_U_0_PD2_EN_msk        0x000008u
#define BGT60TRxxD_CS4_U_0_VCO_EN_pos        4u
#define BGT60TRxxD_CS4_U_0_VCO_EN_msk        0x000010u
#define BGT60TRxxD_CS4_U_0_TEMP_MEAS_EN_pos  5u
#define BGT60TRxxD_CS4_U_0_TEMP_MEAS_EN_msk  0x000020u
#define BGT60TRxxD_CS4_U_0_FDIV_EN_pos       6u
#define BGT60TRxxD_CS4_U_0_FDIV_EN_msk       0x000040u
#define BGT60TRxxD_CS4_U_0_RESERVED1_pos     7u
#define BGT60TRxxD_CS4_U_0_RESERVED1_msk     0x000080u
#define BGT60TRxxD_CS4_U_0_RESERVED2_pos     8u
#define BGT60TRxxD_CS4_U_0_RESERVED2_msk     0x000100u
#define BGT60TRxxD_CS4_U_0_RESERVED3_pos     9u
#define BGT60TRxxD_CS4_U_0_RESERVED3_msk     0x000200u
#define BGT60TRxxD_CS4_U_0_LO_DIST2_EN_pos   10u
#define BGT60TRxxD_CS4_U_0_LO_DIST2_EN_msk   0x000400u
#define BGT60TRxxD_CS4_U_0_LO_DIST1_EN_pos   11u
#define BGT60TRxxD_CS4_U_0_LO_DIST1_EN_msk   0x000800u
#define BGT60TRxxD_CS4_U_0_RX1LOBUF_EN_pos   12u
#define BGT60TRxxD_CS4_U_0_RX1LOBUF_EN_msk   0x001000u
#define BGT60TRxxD_CS4_U_0_RX1MIX_EN_pos     13u
#define BGT60TRxxD_CS4_U_0_RX1MIX_EN_msk     0x002000u
#define BGT60TRxxD_CS4_U_0_RX2LOBUF_EN_pos   14u
#define BGT60TRxxD_CS4_U_0_RX2LOBUF_EN_msk   0x004000u
#define BGT60TRxxD_CS4_U_0_RX2MIX_EN_pos     15u
#define BGT60TRxxD_CS4_U_0_RX2MIX_EN_msk     0x008000u
#define BGT60TRxxD_CS4_U_0_RX3LOBUF_EN_pos   16u
#define BGT60TRxxD_CS4_U_0_RX3LOBUF_EN_msk   0x010000u
#define BGT60TRxxD_CS4_U_0_RX3MIX_EN_pos     17u
#define BGT60TRxxD_CS4_U_0_RX3MIX_EN_msk     0x020000u
#define BGT60TRxxD_CS4_U_0_RX4LOBUF_EN_pos   18u
#define BGT60TRxxD_CS4_U_0_RX4LOBUF_EN_msk   0x040000u
#define BGT60TRxxD_CS4_U_0_RX4MIX_EN_pos     19u
#define BGT60TRxxD_CS4_U_0_RX4MIX_EN_msk     0x080000u
#define BGT60TRxxD_CS4_U_0_ABB1_AAF_CTRL_pos 20u
#define BGT60TRxxD_CS4_U_0_ABB1_AAF_CTRL_msk 0x100000u
#define BGT60TRxxD_CS4_U_0_ABB2_AAF_CTRL_pos 21u
#define BGT60TRxxD_CS4_U_0_ABB2_AAF_CTRL_msk 0x200000u
#define BGT60TRxxD_CS4_U_0_ABB3_AAF_CTRL_pos 22u
#define BGT60TRxxD_CS4_U_0_ABB3_AAF_CTRL_msk 0x400000u
#define BGT60TRxxD_CS4_U_0_ABB4_AAF_CTRL_pos 23u
#define BGT60TRxxD_CS4_U_0_ABB4_AAF_CTRL_msk 0x800000u

/* Fields of register CS4_U_1 */
/* -------------------------- */
#define BGT60TRxxD_CS4_U_1_TX1_DAC_pos       0u
#define BGT60TRxxD_CS4_U_1_TX1_DAC_msk       0x00001fu
#define BGT60TRxxD_CS4_U_1_TX2_DAC_pos       5u
#define BGT60TRxxD_CS4_U_1_TX2_DAC_msk       0x0003e0u
#define BGT60TRxxD_CS4_U_1_HP1_GAIN_pos      10u
#define BGT60TRxxD_CS4_U_1_HP1_GAIN_msk      0x000400u
#define BGT60TRxxD_CS4_U_1_HP2_GAIN_pos      11u
#define BGT60TRxxD_CS4_U_1_HP2_GAIN_msk      0x000800u
#define BGT60TRxxD_CS4_U_1_HP3_GAIN_pos      12u
#define BGT60TRxxD_CS4_U_1_HP3_GAIN_msk      0x001000u
#define BGT60TRxxD_CS4_U_1_HP4_GAIN_pos      13u
#define BGT60TRxxD_CS4_U_1_HP4_GAIN_msk      0x002000u
#define BGT60TRxxD_CS4_U_1_BB_RSTCNT_pos     14u
#define BGT60TRxxD_CS4_U_1_BB_RSTCNT_msk     0x0fc000u
#define BGT60TRxxD_CS4_U_1_MADC_BBCH1_EN_pos 20u
#define BGT60TRxxD_CS4_U_1_MADC_BBCH1_EN_msk 0x100000u
#define BGT60TRxxD_CS4_U_1_MADC_BBCH2_EN_pos 21u
#define BGT60TRxxD_CS4_U_1_MADC_BBCH2_EN_msk 0x200000u
#define BGT60TRxxD_CS4_U_1_MADC_BBCH3_EN_pos 22u
#define BGT60TRxxD_CS4_U_1_MADC_BBCH3_EN_msk 0x400000u
#define BGT60TRxxD_CS4_U_1_MADC_BBCH4_EN_pos 23u
#define BGT60TRxxD_CS4_U_1_MADC_BBCH4_EN_msk 0x800000u

/* Fields of register CS4_U_2 */
/* -------------------------- */
#define BGT60TRxxD_CS4_U_2_HPF_SEL1_pos  0u
#define BGT60TRxxD_CS4_U_2_HPF_SEL1_msk  0x000007u
#define BGT60TRxxD_CS4_U_2_VGA_GAIN1_pos 3u
#define BGT60TRxxD_CS4_U_2_VGA_GAIN1_msk 0x000038u
#define BGT60TRxxD_CS4_U_2_HPF_SEL2_pos  6u
#define BGT60TRxxD_CS4_U_2_HPF_SEL2_msk  0x0001c0u
#define BGT60TRxxD_CS4_U_2_VGA_GAIN2_pos 9u
#define BGT60TRxxD_CS4_U_2_VGA_GAIN2_msk 0x000e00u
#define BGT60TRxxD_CS4_U_2_HPF_SEL3_pos  12u
#define BGT60TRxxD_CS4_U_2_HPF_SEL3_msk  0x007000u
#define BGT60TRxxD_CS4_U_2_VGA_GAIN3_pos 15u
#define BGT60TRxxD_CS4_U_2_VGA_GAIN3_msk 0x038000u
#define BGT60TRxxD_CS4_U_2_HPF_SEL4_pos  18u
#define BGT60TRxxD_CS4_U_2_HPF_SEL4_msk  0x1c0000u
#define BGT60TRxxD_CS4_U_2_VGA_GAIN4_pos 21u
#define BGT60TRxxD_CS4_U_2_VGA_GAIN4_msk 0xe00000u

/* Fields of register CS4_D_0 */
/* -------------------------- */
#define BGT60TRxxD_CS4_D_0_TX1_EN_pos        0u
#define BGT60TRxxD_CS4_D_0_TX1_EN_msk        0x000001u
#define BGT60TRxxD_CS4_D_0_PD1_EN_pos        1u
#define BGT60TRxxD_CS4_D_0_PD1_EN_msk        0x000002u
#define BGT60TRxxD_CS4_D_0_TX2_EN_pos        2u
#define BGT60TRxxD_CS4_D_0_TX2_EN_msk        0x000004u
#define BGT60TRxxD_CS4_D_0_PD2_EN_pos        3u
#define BGT60TRxxD_CS4_D_0_PD2_EN_msk        0x000008u
#define BGT60TRxxD_CS4_D_0_VCO_EN_pos        4u
#define BGT60TRxxD_CS4_D_0_VCO_EN_msk        0x000010u
#define BGT60TRxxD_CS4_D_0_TEMP_MEAS_EN_pos  5u
#define BGT60TRxxD_CS4_D_0_TEMP_MEAS_EN_msk  0x000020u
#define BGT60TRxxD_CS4_D_0_FDIV_EN_pos       6u
#define BGT60TRxxD_CS4_D_0_FDIV_EN_msk       0x000040u
#define BGT60TRxxD_CS4_D_0_RESERVED1_pos     7u
#define BGT60TRxxD_CS4_D_0_RESERVED1_msk     0x000080u
#define BGT60TRxxD_CS4_D_0_RESERVED2_pos     8u
#define BGT60TRxxD_CS4_D_0_RESERVED2_msk     0x000100u
#define BGT60TRxxD_CS4_D_0_RESERVED3_pos     9u
#define BGT60TRxxD_CS4_D_0_RESERVED3_msk     0x000200u
#define BGT60TRxxD_CS4_D_0_LO_DIST2_EN_pos   10u
#define BGT60TRxxD_CS4_D_0_LO_DIST2_EN_msk   0x000400u
#define BGT60TRxxD_CS4_D_0_LO_DIST1_EN_pos   11u
#define BGT60TRxxD_CS4_D_0_LO_DIST1_EN_msk   0x000800u
#define BGT60TRxxD_CS4_D_0_RX1LOBUF_EN_pos   12u
#define BGT60TRxxD_CS4_D_0_RX1LOBUF_EN_msk   0x001000u
#define BGT60TRxxD_CS4_D_0_RX1MIX_EN_pos     13u
#define BGT60TRxxD_CS4_D_0_RX1MIX_EN_msk     0x002000u
#define BGT60TRxxD_CS4_D_0_RX2LOBUF_EN_pos   14u
#define BGT60TRxxD_CS4_D_0_RX2LOBUF_EN_msk   0x004000u
#define BGT60TRxxD_CS4_D_0_RX2MIX_EN_pos     15u
#define BGT60TRxxD_CS4_D_0_RX2MIX_EN_msk     0x008000u
#define BGT60TRxxD_CS4_D_0_RX3LOBUF_EN_pos   16u
#define BGT60TRxxD_CS4_D_0_RX3LOBUF_EN_msk   0x010000u
#define BGT60TRxxD_CS4_D_0_RX3MIX_EN_pos     17u
#define BGT60TRxxD_CS4_D_0_RX3MIX_EN_msk     0x020000u
#define BGT60TRxxD_CS4_D_0_RX4LOBUF_EN_pos   18u
#define BGT60TRxxD_CS4_D_0_RX4LOBUF_EN_msk   0x040000u
#define BGT60TRxxD_CS4_D_0_RX4MIX_EN_pos     19u
#define BGT60TRxxD_CS4_D_0_RX4MIX_EN_msk     0x080000u
#define BGT60TRxxD_CS4_D_0_ABB1_AAF_CTRL_pos 20u
#define BGT60TRxxD_CS4_D_0_ABB1_AAF_CTRL_msk 0x100000u
#define BGT60TRxxD_CS4_D_0_ABB2_AAF_CTRL_pos 21u
#define BGT60TRxxD_CS4_D_0_ABB2_AAF_CTRL_msk 0x200000u
#define BGT60TRxxD_CS4_D_0_ABB3_AAF_CTRL_pos 22u
#define BGT60TRxxD_CS4_D_0_ABB3_AAF_CTRL_msk 0x400000u
#define BGT60TRxxD_CS4_D_0_ABB4_AAF_CTRL_pos 23u
#define BGT60TRxxD_CS4_D_0_ABB4_AAF_CTRL_msk 0x800000u

/* Fields of register CS4_D_1 */
/* -------------------------- */
#define BGT60TRxxD_CS4_D_1_TX1_DAC_pos       0u
#define BGT60TRxxD_CS4_D_1_TX1_DAC_msk       0x00001fu
#define BGT60TRxxD_CS4_D_1_TX2_DAC_pos       5u
#define BGT60TRxxD_CS4_D_1_TX2_DAC_msk       0x0003e0u
#define BGT60TRxxD_CS4_D_1_HP1_GAIN_pos      10u
#define BGT60TRxxD_CS4_D_1_HP1_GAIN_msk      0x000400u
#define BGT60TRxxD_CS4_D_1_HP2_GAIN_pos      11u
#define BGT60TRxxD_CS4_D_1_HP2_GAIN_msk      0x000800u
#define BGT60TRxxD_CS4_D_1_HP3_GAIN_pos      12u
#define BGT60TRxxD_CS4_D_1_HP3_GAIN_msk      0x001000u
#define BGT60TRxxD_CS4_D_1_HP4_GAIN_pos      13u
#define BGT60TRxxD_CS4_D_1_HP4_GAIN_msk      0x002000u
#define BGT60TRxxD_CS4_D_1_BB_RSTCNT_pos     14u
#define BGT60TRxxD_CS4_D_1_BB_RSTCNT_msk     0x0fc000u
#define BGT60TRxxD_CS4_D_1_MADC_BBCH1_EN_pos 20u
#define BGT60TRxxD_CS4_D_1_MADC_BBCH1_EN_msk 0x100000u
#define BGT60TRxxD_CS4_D_1_MADC_BBCH2_EN_pos 21u
#define BGT60TRxxD_CS4_D_1_MADC_BBCH2_EN_msk 0x200000u
#define BGT60TRxxD_CS4_D_1_MADC_BBCH3_EN_pos 22u
#define BGT60TRxxD_CS4_D_1_MADC_BBCH3_EN_msk 0x400000u
#define BGT60TRxxD_CS4_D_1_MADC_BBCH4_EN_pos 23u
#define BGT60TRxxD_CS4_D_1_MADC_BBCH4_EN_msk 0x800000u

/* Fields of register CS4_D_2 */
/* -------------------------- */
#define BGT60TRxxD_CS4_D_2_HPF_SEL1_pos  0u
#define BGT60TRxxD_CS4_D_2_HPF_SEL1_msk  0x000007u
#define BGT60TRxxD_CS4_D_2_VGA_GAIN1_pos 3u
#define BGT60TRxxD_CS4_D_2_VGA_GAIN1_msk 0x000038u
#define BGT60TRxxD_CS4_D_2_HPF_SEL2_pos  6u
#define BGT60TRxxD_CS4_D_2_HPF_SEL2_msk  0x0001c0u
#define BGT60TRxxD_CS4_D_2_VGA_GAIN2_pos 9u
#define BGT60TRxxD_CS4_D_2_VGA_GAIN2_msk 0x000e00u
#define BGT60TRxxD_CS4_D_2_HPF_SEL3_pos  12u
#define BGT60TRxxD_CS4_D_2_HPF_SEL3_msk  0x007000u
#define BGT60TRxxD_CS4_D_2_VGA_GAIN3_pos 15u
#define BGT60TRxxD_CS4_D_2_VGA_GAIN3_msk 0x038000u
#define BGT60TRxxD_CS4_D_2_HPF_SEL4_pos  18u
#define BGT60TRxxD_CS4_D_2_HPF_SEL4_msk  0x1c0000u
#define BGT60TRxxD_CS4_D_2_VGA_GAIN4_pos 21u
#define BGT60TRxxD_CS4_D_2_VGA_GAIN4_msk 0xe00000u

/* Fields of register CS4 */
/* ---------------------- */
#define BGT60TRxxD_CS4_REPCS_pos      0u
#define BGT60TRxxD_CS4_REPCS_msk      0x00000fu
#define BGT60TRxxD_CS4_CS_EN_pos      4u
#define BGT60TRxxD_CS4_CS_EN_msk      0x000010u
#define BGT60TRxxD_CS4_ABB_ISOPD_pos  5u
#define BGT60TRxxD_CS4_ABB_ISOPD_msk  0x000020u
#define BGT60TRxxD_CS4_RF_ISOPD_pos   6u
#define BGT60TRxxD_CS4_RF_ISOPD_msk   0x000040u
#define BGT60TRxxD_CS4_MADC_BG_EN_pos 7u
#define BGT60TRxxD_CS4_MADC_BG_EN_msk 0x000080u
#define BGT60TRxxD_CS4_MADC_ISOPD_pos 8u
#define BGT60TRxxD_CS4_MADC_ISOPD_msk 0x000100u
#define BGT60TRxxD_CS4_SADC_ISOPD_pos 9u
#define BGT60TRxxD_CS4_SADC_ISOPD_msk 0x000200u
#define BGT60TRxxD_CS4_BG_TMRF_EN_pos 10u
#define BGT60TRxxD_CS4_BG_TMRF_EN_msk 0x000400u
#define BGT60TRxxD_CS4_PLL_ISOPD_pos  11u
#define BGT60TRxxD_CS4_PLL_ISOPD_msk  0x000800u

/* Fields of register CCR0 */
/* ----------------------- */
#define BGT60TRxxD_CCR0_TR_RED_pos    0u
#define BGT60TRxxD_CCR0_TR_RED_msk    0x0001ffu
#define BGT60TRxxD_CCR0_CONT_MODE_pos 9u
#define BGT60TRxxD_CCR0_CONT_MODE_msk 0x000200u
#define BGT60TRxxD_CCR0_REPT_pos      10u
#define BGT60TRxxD_CCR0_REPT_msk      0x003c00u
#define BGT60TRxxD_CCR0_TR_INIT1_pos  14u
#define BGT60TRxxD_CCR0_TR_INIT1_msk  0x3fc000u
#define BGT60TRxxD_CCR0_TR_MUL1_pos   22u
#define BGT60TRxxD_CCR0_TR_MUL1_msk   0xc00000u

/* Fields of register CCR1 */
/* ----------------------- */
#define BGT60TRxxD_CCR1_TR_START_pos   0u
#define BGT60TRxxD_CCR1_TR_START_msk   0x0001ffu
#define BGT60TRxxD_CCR1_PD_MODE_pos    9u
#define BGT60TRxxD_CCR1_PD_MODE_msk    0x000600u
#define BGT60TRxxD_CCR1_TR_FED_pos     11u
#define BGT60TRxxD_CCR1_TR_FED_msk     0x07f800u
#define BGT60TRxxD_CCR1_TR_FED_MUL_pos 19u
#define BGT60TRxxD_CCR1_TR_FED_MUL_msk 0xf80000u

/* Fields of register CCR2 */
/* ----------------------- */
#define BGT60TRxxD_CCR2_MAX_FRAME_CNT_pos 0u
#define BGT60TRxxD_CCR2_MAX_FRAME_CNT_msk 0x000fffu
#define BGT60TRxxD_CCR2_FRAME_LEN_pos     12u
#define BGT60TRxxD_CCR2_FRAME_LEN_msk     0xfff000u

/* Fields of register CCR3 */
/* ----------------------- */
#define BGT60TRxxD_CCR3_TR_PAEN_pos      0u
#define BGT60TRxxD_CCR3_TR_PAEN_msk      0x0001ffu
#define BGT60TRxxD_CCR3_TR_SSTART_pos    9u
#define BGT60TRxxD_CCR3_TR_SSTART_msk    0x007e00u
#define BGT60TRxxD_CCR3_TR_INIT0_pos     15u
#define BGT60TRxxD_CCR3_TR_INIT0_msk     0x3f8000u
#define BGT60TRxxD_CCR3_TR_INIT0_MUL_pos 22u
#define BGT60TRxxD_CCR3_TR_INIT0_MUL_msk 0xc00000u

/* Fields of register PLL1_0 */
/* ------------------------- */
#define BGT60TRxxD_PLL1_0_FSU_pos 0u
#define BGT60TRxxD_PLL1_0_FSU_msk 0xffffffu

/* Fields of register PLL1_1 */
/* ------------------------- */
#define BGT60TRxxD_PLL1_1_RSU_pos 0u
#define BGT60TRxxD_PLL1_1_RSU_msk 0xffffffu

/* Fields of register PLL1_2 */
/* ------------------------- */
#define BGT60TRxxD_PLL1_2_RTU_pos  0u
#define BGT60TRxxD_PLL1_2_RTU_msk  0x003fffu
#define BGT60TRxxD_PLL1_2_TEDU_pos 16u
#define BGT60TRxxD_PLL1_2_TEDU_msk 0xff0000u

/* Fields of register PLL1_3 */
/* ------------------------- */
#define BGT60TRxxD_PLL1_3_APU_pos 0u
#define BGT60TRxxD_PLL1_3_APU_msk 0x000fffu
#define BGT60TRxxD_PLL1_3_APD_pos 12u
#define BGT60TRxxD_PLL1_3_APD_msk 0xfff000u

/* Fields of register PLL1_4 */
/* ------------------------- */
#define BGT60TRxxD_PLL1_4_FSD_pos 0u
#define BGT60TRxxD_PLL1_4_FSD_msk 0xffffffu

/* Fields of register PLL1_5 */
/* ------------------------- */
#define BGT60TRxxD_PLL1_5_RSD_pos 0u
#define BGT60TRxxD_PLL1_5_RSD_msk 0xffffffu

/* Fields of register PLL1_6 */
/* ------------------------- */
#define BGT60TRxxD_PLL1_6_RTD_pos  0u
#define BGT60TRxxD_PLL1_6_RTD_msk  0x003fffu
#define BGT60TRxxD_PLL1_6_TEDD_pos 16u
#define BGT60TRxxD_PLL1_6_TEDD_msk 0xff0000u

/* Fields of register PLL1_7 */
/* ------------------------- */
#define BGT60TRxxD_PLL1_7_REPS_pos       0u
#define BGT60TRxxD_PLL1_7_REPS_msk       0x00000fu
#define BGT60TRxxD_PLL1_7_SH_EN_pos      4u
#define BGT60TRxxD_PLL1_7_SH_EN_msk      0x000010u
#define BGT60TRxxD_PLL1_7_CONT_MODE_pos  8u
#define BGT60TRxxD_PLL1_7_CONT_MODE_msk  0x000100u
#define BGT60TRxxD_PLL1_7_PD_MODE_pos    9u
#define BGT60TRxxD_PLL1_7_PD_MODE_msk    0x000600u
#define BGT60TRxxD_PLL1_7_TR_SED_pos     11u
#define BGT60TRxxD_PLL1_7_TR_SED_msk     0x07f800u
#define BGT60TRxxD_PLL1_7_TR_SED_MUL_pos 19u
#define BGT60TRxxD_PLL1_7_TR_SED_MUL_msk 0xf80000u

/* Fields of register PLL2_0 */
/* ------------------------- */
#define BGT60TRxxD_PLL2_0_FSU_pos 0u
#define BGT60TRxxD_PLL2_0_FSU_msk 0xffffffu

/* Fields of register PLL2_1 */
/* ------------------------- */
#define BGT60TRxxD_PLL2_1_RSU_pos 0u
#define BGT60TRxxD_PLL2_1_RSU_msk 0xffffffu

/* Fields of register PLL2_2 */
/* ------------------------- */
#define BGT60TRxxD_PLL2_2_RTU_pos  0u
#define BGT60TRxxD_PLL2_2_RTU_msk  0x003fffu
#define BGT60TRxxD_PLL2_2_TEDU_pos 16u
#define BGT60TRxxD_PLL2_2_TEDU_msk 0xff0000u

/* Fields of register PLL2_3 */
/* ------------------------- */
#define BGT60TRxxD_PLL2_3_APU_pos 0u
#define BGT60TRxxD_PLL2_3_APU_msk 0x000fffu
#define BGT60TRxxD_PLL2_3_APD_pos 12u
#define BGT60TRxxD_PLL2_3_APD_msk 0xfff000u

/* Fields of register PLL2_4 */
/* ------------------------- */
#define BGT60TRxxD_PLL2_4_FSD_pos 0u
#define BGT60TRxxD_PLL2_4_FSD_msk 0xffffffu

/* Fields of register PLL2_5 */
/* ------------------------- */
#define BGT60TRxxD_PLL2_5_RSD_pos 0u
#define BGT60TRxxD_PLL2_5_RSD_msk 0xffffffu

/* Fields of register PLL2_6 */
/* ------------------------- */
#define BGT60TRxxD_PLL2_6_RTD_pos  0u
#define BGT60TRxxD_PLL2_6_RTD_msk  0x003fffu
#define BGT60TRxxD_PLL2_6_TEDD_pos 16u
#define BGT60TRxxD_PLL2_6_TEDD_msk 0xff0000u

/* Fields of register PLL2_7 */
/* ------------------------- */
#define BGT60TRxxD_PLL2_7_REPS_pos       0u
#define BGT60TRxxD_PLL2_7_REPS_msk       0x00000fu
#define BGT60TRxxD_PLL2_7_SH_EN_pos      4u
#define BGT60TRxxD_PLL2_7_SH_EN_msk      0x000010u
#define BGT60TRxxD_PLL2_7_CONT_MODE_pos  8u
#define BGT60TRxxD_PLL2_7_CONT_MODE_msk  0x000100u
#define BGT60TRxxD_PLL2_7_PD_MODE_pos    9u
#define BGT60TRxxD_PLL2_7_PD_MODE_msk    0x000600u
#define BGT60TRxxD_PLL2_7_TR_SED_pos     11u
#define BGT60TRxxD_PLL2_7_TR_SED_msk     0x07f800u
#define BGT60TRxxD_PLL2_7_TR_SED_MUL_pos 19u
#define BGT60TRxxD_PLL2_7_TR_SED_MUL_msk 0xf80000u

/* Fields of register PLL3_0 */
/* ------------------------- */
#define BGT60TRxxD_PLL3_0_FSU_pos 0u
#define BGT60TRxxD_PLL3_0_FSU_msk 0xffffffu

/* Fields of register PLL3_1 */
/* ------------------------- */
#define BGT60TRxxD_PLL3_1_RSU_pos 0u
#define BGT60TRxxD_PLL3_1_RSU_msk 0xffffffu

/* Fields of register PLL3_2 */
/* ------------------------- */
#define BGT60TRxxD_PLL3_2_RTU_pos  0u
#define BGT60TRxxD_PLL3_2_RTU_msk  0x003fffu
#define BGT60TRxxD_PLL3_2_TEDU_pos 16u
#define BGT60TRxxD_PLL3_2_TEDU_msk 0xff0000u

/* Fields of register PLL3_3 */
/* ------------------------- */
#define BGT60TRxxD_PLL3_3_APU_pos 0u
#define BGT60TRxxD_PLL3_3_APU_msk 0x000fffu
#define BGT60TRxxD_PLL3_3_APD_pos 12u
#define BGT60TRxxD_PLL3_3_APD_msk 0xfff000u

/* Fields of register PLL3_4 */
/* ------------------------- */
#define BGT60TRxxD_PLL3_4_FSD_pos 0u
#define BGT60TRxxD_PLL3_4_FSD_msk 0xffffffu

/* Fields of register PLL3_5 */
/* ------------------------- */
#define BGT60TRxxD_PLL3_5_RSD_pos 0u
#define BGT60TRxxD_PLL3_5_RSD_msk 0xffffffu

/* Fields of register PLL3_6 */
/* ------------------------- */
#define BGT60TRxxD_PLL3_6_RTD_pos  0u
#define BGT60TRxxD_PLL3_6_RTD_msk  0x003fffu
#define BGT60TRxxD_PLL3_6_TEDD_pos 16u
#define BGT60TRxxD_PLL3_6_TEDD_msk 0xff0000u

/* Fields of register PLL3_7 */
/* ------------------------- */
#define BGT60TRxxD_PLL3_7_REPS_pos       0u
#define BGT60TRxxD_PLL3_7_REPS_msk       0x00000fu
#define BGT60TRxxD_PLL3_7_SH_EN_pos      4u
#define BGT60TRxxD_PLL3_7_SH_EN_msk      0x000010u
#define BGT60TRxxD_PLL3_7_CONT_MODE_pos  8u
#define BGT60TRxxD_PLL3_7_CONT_MODE_msk  0x000100u
#define BGT60TRxxD_PLL3_7_PD_MODE_pos    9u
#define BGT60TRxxD_PLL3_7_PD_MODE_msk    0x000600u
#define BGT60TRxxD_PLL3_7_TR_SED_pos     11u
#define BGT60TRxxD_PLL3_7_TR_SED_msk     0x07f800u
#define BGT60TRxxD_PLL3_7_TR_SED_MUL_pos 19u
#define BGT60TRxxD_PLL3_7_TR_SED_MUL_msk 0xf80000u

/* Fields of register PLL4_0 */
/* ------------------------- */
#define BGT60TRxxD_PLL4_0_FSU_pos 0u
#define BGT60TRxxD_PLL4_0_FSU_msk 0xffffffu

/* Fields of register PLL4_1 */
/* ------------------------- */
#define BGT60TRxxD_PLL4_1_RSU_pos 0u
#define BGT60TRxxD_PLL4_1_RSU_msk 0xffffffu

/* Fields of register PLL4_2 */
/* ------------------------- */
#define BGT60TRxxD_PLL4_2_RTU_pos  0u
#define BGT60TRxxD_PLL4_2_RTU_msk  0x003fffu
#define BGT60TRxxD_PLL4_2_TEDU_pos 16u
#define BGT60TRxxD_PLL4_2_TEDU_msk 0xff0000u

/* Fields of register PLL4_3 */
/* ------------------------- */
#define BGT60TRxxD_PLL4_3_APU_pos 0u
#define BGT60TRxxD_PLL4_3_APU_msk 0x000fffu
#define BGT60TRxxD_PLL4_3_APD_pos 12u
#define BGT60TRxxD_PLL4_3_APD_msk 0xfff000u

/* Fields of register PLL4_4 */
/* ------------------------- */
#define BGT60TRxxD_PLL4_4_FSD_pos 0u
#define BGT60TRxxD_PLL4_4_FSD_msk 0xffffffu

/* Fields of register PLL4_5 */
/* ------------------------- */
#define BGT60TRxxD_PLL4_5_RSD_pos 0u
#define BGT60TRxxD_PLL4_5_RSD_msk 0xffffffu

/* Fields of register PLL4_6 */
/* ------------------------- */
#define BGT60TRxxD_PLL4_6_RTD_pos  0u
#define BGT60TRxxD_PLL4_6_RTD_msk  0x003fffu
#define BGT60TRxxD_PLL4_6_TEDD_pos 16u
#define BGT60TRxxD_PLL4_6_TEDD_msk 0xff0000u

/* Fields of register PLL4_7 */
/* ------------------------- */
#define BGT60TRxxD_PLL4_7_REPS_pos       0u
#define BGT60TRxxD_PLL4_7_REPS_msk       0x00000fu
#define BGT60TRxxD_PLL4_7_SH_EN_pos      4u
#define BGT60TRxxD_PLL4_7_SH_EN_msk      0x000010u
#define BGT60TRxxD_PLL4_7_CONT_MODE_pos  8u
#define BGT60TRxxD_PLL4_7_CONT_MODE_msk  0x000100u
#define BGT60TRxxD_PLL4_7_PD_MODE_pos    9u
#define BGT60TRxxD_PLL4_7_PD_MODE_msk    0x000600u
#define BGT60TRxxD_PLL4_7_TR_SED_pos     11u
#define BGT60TRxxD_PLL4_7_TR_SED_msk     0x07f800u
#define BGT60TRxxD_PLL4_7_TR_SED_MUL_pos 19u
#define BGT60TRxxD_PLL4_7_TR_SED_MUL_msk 0xf80000u

/* Fields of register ADC1 */
/* ----------------------- */
#define BGT60TRxxD_ADC1_TDIG_I_pos 0u
#define BGT60TRxxD_ADC1_TDIG_I_msk 0x00ffffu

/* Fields of register ADC2 */
/* ----------------------- */
#define BGT60TRxxD_ADC2_TDIG_ADC0_O_pos 0u
#define BGT60TRxxD_ADC2_TDIG_ADC0_O_msk 0x00ffffu

/* Fields of register ADC3 */
/* ----------------------- */
#define BGT60TRxxD_ADC3_TDIG_ADC1_O_pos 0u
#define BGT60TRxxD_ADC3_TDIG_ADC1_O_msk 0x00ffffu

/* Fields of register ADC4 */
/* ----------------------- */
#define BGT60TRxxD_ADC4_TDIG_ADC2_O_pos 0u
#define BGT60TRxxD_ADC4_TDIG_ADC2_O_msk 0x00ffffu

/* Fields of register ADC5 */
/* ----------------------- */
#define BGT60TRxxD_ADC5_TDIG_ADC3_O_pos 0u
#define BGT60TRxxD_ADC5_TDIG_ADC3_O_msk 0x00ffffu

/* Fields of register RFT0 */
/* ----------------------- */
#define BGT60TRxxD_RFT0_RFTSIGCLK_DIV_pos    0u
#define BGT60TRxxD_RFT0_RFTSIGCLK_DIV_msk    0x001fffu
#define BGT60TRxxD_RFT0_RFTSIGCLK_DIV_EN_pos 13u
#define BGT60TRxxD_RFT0_RFTSIGCLK_DIV_EN_msk 0x002000u
#define BGT60TRxxD_RFT0_TEST_SIG_RF1_EN_pos  14u
#define BGT60TRxxD_RFT0_TEST_SIG_RF1_EN_msk  0x004000u
#define BGT60TRxxD_RFT0_TEST_SIG_RF2_EN_pos  15u
#define BGT60TRxxD_RFT0_TEST_SIG_RF2_EN_msk  0x008000u
#define BGT60TRxxD_RFT0_TEST_SIG_RF3_EN_pos  16u
#define BGT60TRxxD_RFT0_TEST_SIG_RF3_EN_msk  0x010000u
#define BGT60TRxxD_RFT0_TEST_SIG_RF4_EN_pos  17u
#define BGT60TRxxD_RFT0_TEST_SIG_RF4_EN_msk  0x020000u
#define BGT60TRxxD_RFT0_TEST_SIG_IF1_EN_pos  18u
#define BGT60TRxxD_RFT0_TEST_SIG_IF1_EN_msk  0x040000u
#define BGT60TRxxD_RFT0_TEST_SIG_IF2_EN_pos  19u
#define BGT60TRxxD_RFT0_TEST_SIG_IF2_EN_msk  0x080000u
#define BGT60TRxxD_RFT0_TEST_SIG_IF3_EN_pos  20u
#define BGT60TRxxD_RFT0_TEST_SIG_IF3_EN_msk  0x100000u
#define BGT60TRxxD_RFT0_TEST_SIG_IF4_EN_pos  21u
#define BGT60TRxxD_RFT0_TEST_SIG_IF4_EN_msk  0x200000u
#define BGT60TRxxD_RFT0_RF_TEST_MODE_pos     22u
#define BGT60TRxxD_RFT0_RF_TEST_MODE_msk     0xc00000u

/* Fields of register RFT1 */
/* ----------------------- */
#define BGT60TRxxD_RFT1_TEST_DIV_EN_pos     0u
#define BGT60TRxxD_RFT1_TEST_DIV_EN_msk     0x000001u
#define BGT60TRxxD_RFT1_RESERVED1_pos       1u
#define BGT60TRxxD_RFT1_RESERVED1_msk       0x000002u
#define BGT60TRxxD_RFT1_CTRL_MIX1_IFBUF_pos 2u
#define BGT60TRxxD_RFT1_CTRL_MIX1_IFBUF_msk 0x00000cu
#define BGT60TRxxD_RFT1_CTRL_MIX2_IFBUF_pos 4u
#define BGT60TRxxD_RFT1_CTRL_MIX2_IFBUF_msk 0x000030u
#define BGT60TRxxD_RFT1_CTRL_MIX3_IFBUF_pos 6u
#define BGT60TRxxD_RFT1_CTRL_MIX3_IFBUF_msk 0x0000c0u
#define BGT60TRxxD_RFT1_CTRL_MIX4_IFBUF_pos 8u
#define BGT60TRxxD_RFT1_CTRL_MIX4_IFBUF_msk 0x000300u
#define BGT60TRxxD_RFT1_RESERVED2_pos       10u
#define BGT60TRxxD_RFT1_RESERVED2_msk       0x07fc00u
#define BGT60TRxxD_RFT1_TX1_MOD_EN_pos      19u
#define BGT60TRxxD_RFT1_TX1_MOD_EN_msk      0x080000u
#define BGT60TRxxD_RFT1_TX2_MOD_EN_pos      20u
#define BGT60TRxxD_RFT1_TX2_MOD_EN_msk      0x100000u
#define BGT60TRxxD_RFT1_TX1_SPARE_pos       21u
#define BGT60TRxxD_RFT1_TX1_SPARE_msk       0x200000u
#define BGT60TRxxD_RFT1_TX2_SPARE_pos       22u
#define BGT60TRxxD_RFT1_TX2_SPARE_msk       0xc00000u

/* Fields of register DFT0 */
/* ----------------------- */
#define BGT60TRxxD_DFT0_MODE_pos             0u
#define BGT60TRxxD_DFT0_MODE_msk             0x000003u
#define BGT60TRxxD_DFT0_SMONEN_pos           2u
#define BGT60TRxxD_DFT0_SMONEN_msk           0x00000cu
#define BGT60TRxxD_DFT0_EFUSE_EN_pos         4u
#define BGT60TRxxD_DFT0_EFUSE_EN_msk         0x000010u
#define BGT60TRxxD_DFT0_ALG_pos              5u
#define BGT60TRxxD_DFT0_ALG_msk              0x0001e0u
#define BGT60TRxxD_DFT0_NHOLD_pos            9u
#define BGT60TRxxD_DFT0_NHOLD_msk            0x000200u
#define BGT60TRxxD_DFT0_DBG_pos              10u
#define BGT60TRxxD_DFT0_DBG_msk              0x000400u
#define BGT60TRxxD_DFT0_CTRL_pos             11u
#define BGT60TRxxD_DFT0_CTRL_msk             0x000800u
#define BGT60TRxxD_DFT0_EFUSE_SENSE_LOW_pos  12u
#define BGT60TRxxD_DFT0_EFUSE_SENSE_LOW_msk  0x001000u
#define BGT60TRxxD_DFT0_EFUSE_SENSE_HIGH_pos 13u
#define BGT60TRxxD_DFT0_EFUSE_SENSE_HIGH_msk 0x002000u
#define BGT60TRxxD_DFT0_EFUSE_SENSE_pos      14u
#define BGT60TRxxD_DFT0_EFUSE_SENSE_msk      0x004000u
#define BGT60TRxxD_DFT0_EFUSE_PRG_pos        15u
#define BGT60TRxxD_DFT0_EFUSE_PRG_msk        0x008000u
#define BGT60TRxxD_DFT0_DEBUG_DIG_pos        16u
#define BGT60TRxxD_DFT0_DEBUG_DIG_msk        0x0f0000u

/* Fields of register DFT1 */
/* ----------------------- */
#define BGT60TRxxD_DFT1_FAIL_pos         3u
#define BGT60TRxxD_DFT1_FAIL_msk         0x000008u
#define BGT60TRxxD_DFT1_DONE_pos         4u
#define BGT60TRxxD_DFT1_DONE_msk         0x000010u
#define BGT60TRxxD_DFT1_EFUSE_LOCKED_pos 8u
#define BGT60TRxxD_DFT1_EFUSE_LOCKED_msk 0x000100u
#define BGT60TRxxD_DFT1_EFUSE_READY_pos  9u
#define BGT60TRxxD_DFT1_EFUSE_READY_msk  0x000200u

/* Fields of register PDFT0 */
/* ------------------------ */
#define BGT60TRxxD_PDFT0_BYPRMPEN_pos 0u
#define BGT60TRxxD_PDFT0_BYPRMPEN_msk 0x000001u
#define BGT60TRxxD_PDFT0_BYPSDMEN_pos 1u
#define BGT60TRxxD_PDFT0_BYPSDMEN_msk 0x000002u
#define BGT60TRxxD_PDFT0_BYPSDM_pos   2u
#define BGT60TRxxD_PDFT0_BYPSDM_msk   0x00003cu

/* Fields of register PDFT1 */
/* ------------------------ */
#define BGT60TRxxD_PDFT1_FSDNDEB_pos  0u
#define BGT60TRxxD_PDFT1_FSDNDEB_msk  0x000001u
#define BGT60TRxxD_PDFT1_FRCUPLO_pos  1u
#define BGT60TRxxD_PDFT1_FRCUPLO_msk  0x000002u
#define BGT60TRxxD_PDFT1_FRCUPHI_pos  2u
#define BGT60TRxxD_PDFT1_FRCUPHI_msk  0x000004u
#define BGT60TRxxD_PDFT1_FRCDNHI_pos  3u
#define BGT60TRxxD_PDFT1_FRCDNHI_msk  0x000008u
#define BGT60TRxxD_PDFT1_FRCDNLO_pos  4u
#define BGT60TRxxD_PDFT1_FRCDNLO_msk  0x000010u
#define BGT60TRxxD_PDFT1_BOOSTCP_pos  5u
#define BGT60TRxxD_PDFT1_BOOSTCP_msk  0x000020u
#define BGT60TRxxD_PDFT1_DFTANAEN_pos 6u
#define BGT60TRxxD_PDFT1_DFTANAEN_msk 0x000040u
#define BGT60TRxxD_PDFT1_SPARE0_pos   7u
#define BGT60TRxxD_PDFT1_SPARE0_msk   0x000180u
#define BGT60TRxxD_PDFT1_DFTDIGEN_pos 9u
#define BGT60TRxxD_PDFT1_DFTDIGEN_msk 0x000200u
#define BGT60TRxxD_PDFT1_DFTDIGSE_pos 10u
#define BGT60TRxxD_PDFT1_DFTDIGSE_msk 0x000c00u
#define BGT60TRxxD_PDFT1_SPAREDFT_pos 12u
#define BGT60TRxxD_PDFT1_SPAREDFT_msk 0x00f000u
#define BGT60TRxxD_PDFT1_SPARE1_pos   16u
#define BGT60TRxxD_PDFT1_SPARE1_msk   0x030000u
#define BGT60TRxxD_PDFT1_DFTANASE_pos 18u
#define BGT60TRxxD_PDFT1_DFTANASE_msk 0x1c0000u
#define BGT60TRxxD_PDFT1_DITHEN_pos   21u
#define BGT60TRxxD_PDFT1_DITHEN_msk   0x200000u
#define BGT60TRxxD_PDFT1_DITHSEL_pos  22u
#define BGT60TRxxD_PDFT1_DITHSEL_msk  0xc00000u

/* Fields of register SADC_DFT0 */
/* ---------------------------- */
#define BGT60TRxxD_SADC_DFT0_TST_DIG_I_pos 0u
#define BGT60TRxxD_SADC_DFT0_TST_DIG_I_msk 0x0fffffu

/* Fields of register SADC_DFT1 */
/* ---------------------------- */
#define BGT60TRxxD_SADC_DFT1_CAL_RAM_pos 0u
#define BGT60TRxxD_SADC_DFT1_CAL_RAM_msk 0x00001fu
#define BGT60TRxxD_SADC_DFT1_DISCHARGE_COUNTER_pos5u
#define BGT60TRxxD_SADC_DFT1_DISCHARGE_COUNTER_msk 0x1fffe0u

/* Fields of register STAT0 */
/* ------------------------ */
#define BGT60TRxxD_STAT0_SADC_RDY_pos  0u
#define BGT60TRxxD_STAT0_SADC_RDY_msk  0x000001u
#define BGT60TRxxD_STAT0_MADC_RDY_pos  1u
#define BGT60TRxxD_STAT0_MADC_RDY_msk  0x000002u
#define BGT60TRxxD_STAT0_MADC_BGUP_pos 2u
#define BGT60TRxxD_STAT0_MADC_BGUP_msk 0x000004u
#define BGT60TRxxD_STAT0_LDO_RDY_pos   3u
#define BGT60TRxxD_STAT0_LDO_RDY_msk   0x000008u
#define BGT60TRxxD_STAT0_PM_pos        5u
#define BGT60TRxxD_STAT0_PM_msk        0x0000e0u
#define BGT60TRxxD_STAT0_CH_IDX_pos    8u
#define BGT60TRxxD_STAT0_CH_IDX_msk    0x000700u
#define BGT60TRxxD_STAT0_SH_IDX_pos    11u
#define BGT60TRxxD_STAT0_SH_IDX_msk    0x003800u

/* Fields of register SADC_RESULT */
/* ------------------------------ */
#define BGT60TRxxD_SADC_RESULT_SADC_RESULT_pos 0u
#define BGT60TRxxD_SADC_RESULT_SADC_RESULT_msk 0x0003ffu
#define BGT60TRxxD_SADC_RESULT_SADC_BUSY_pos   10u
#define BGT60TRxxD_SADC_RESULT_SADC_BUSY_msk   0x000400u
#define BGT60TRxxD_SADC_RESULT_SADC_RAW_pos    12u
#define BGT60TRxxD_SADC_RESULT_SADC_RAW_msk    0x3ff000u

/* Fields of register FSTAT */
/* ------------------------ */
#define BGT60TRxxD_FSTAT_FILL_STATUS_pos  0u
#define BGT60TRxxD_FSTAT_FILL_STATUS_msk  0x003fffu
#define BGT60TRxxD_FSTAT_RAM_PWR_DOWN_pos 14u
#define BGT60TRxxD_FSTAT_RAM_PWR_DOWN_msk 0x004000u
#define BGT60TRxxD_FSTAT_CLK_NUM_ERR_pos  17u
#define BGT60TRxxD_FSTAT_CLK_NUM_ERR_msk  0x020000u
#define BGT60TRxxD_FSTAT_BURST_ERR_pos    18u
#define BGT60TRxxD_FSTAT_BURST_ERR_msk    0x040000u
#define BGT60TRxxD_FSTAT_FUF_ERR_pos      19u
#define BGT60TRxxD_FSTAT_FUF_ERR_msk      0x080000u
#define BGT60TRxxD_FSTAT_EMPTY_pos        20u
#define BGT60TRxxD_FSTAT_EMPTY_msk        0x100000u
#define BGT60TRxxD_FSTAT_CREF_pos         21u
#define BGT60TRxxD_FSTAT_CREF_msk         0x200000u
#define BGT60TRxxD_FSTAT_FULL_pos         22u
#define BGT60TRxxD_FSTAT_FULL_msk         0x400000u
#define BGT60TRxxD_FSTAT_FOF_ERR_pos      23u
#define BGT60TRxxD_FSTAT_FOF_ERR_msk      0x800000u

/* Fields of register DEV_ID0 */
/* -------------------------- */
#define BGT60TRxxD_DEV_ID0_DEVICE_ID_pos 0u
#define BGT60TRxxD_DEV_ID0_DEVICE_ID_msk 0xffffffu

/* Fields of register DEV_ID1 */
/* -------------------------- */
#define BGT60TRxxD_DEV_ID1_DEVICE_ID_pos 0u
#define BGT60TRxxD_DEV_ID1_DEVICE_ID_msk 0xffffffu

/* Fields of register FD */
/* --------------------- */
#define BGT60TRxxD_FD_CLK_SEL_pos    0u
#define BGT60TRxxD_FD_CLK_SEL_msk    0x000003u
#define BGT60TRxxD_FD_DC_IN_pos      2u
#define BGT60TRxxD_FD_DC_IN_msk      0x00007cu
#define BGT60TRxxD_FD_DC_OUT_pos     7u
#define BGT60TRxxD_FD_DC_OUT_msk     0x000780u
#define BGT60TRxxD_FD_IRQ_FD_SEL_pos 11u
#define BGT60TRxxD_FD_IRQ_FD_SEL_msk 0x000800u

/*
 * \brief This is the burst prefix word to read the data memory of BGT60TRxxD.
 *
 * The prefix is a 32 bit word must be transmitted to the BGT60TRxxD before
 * reading the sample memory. The CS signal must not go high between the
 * transmission of the burst command word and the sample data.
 *
 * The word is constructed as follows:
 * FF: Burst command code
 * C6: Address 0x63, R/W bit = 0
 * 00: Infinite burst length --> burst length controlled by CS line
 * 63: Again Address 0x63 (this is only for quad SPI mode)
 */
#define BGT60TRxxD_MEMORY_BURST_READ_CMD 0xFFC60063
/**@}*/

/**
 * \defgroup AccessMacros AccessMacros
 *
 * These macros can be used to access the bit fields of BGT60TRxx registers.
 * It is recommended to use these macros instead of accessing the bit fields
 * directly.
 *
 * @{
 */

/**
 * \brief This macro generates the SPI word to set a bit field of BGT60TRxxD.
 *
 * Pass the register and bit field names and the value to be set to that bit
 * field. The value will be masked and shifted to the right position.
 *
 * The generates word will also have the right register address in the upper
 * 7 bit and the write bit set, so that the word can be transmitted to the
 * BGT60TRxx immediately.
 *
 * Since all other bit fields in the generated word are 0, those bit field
 * will be cleared when the macro result is sent to the BGT60TRxx. So it is
 * recommended to create the SPI word for all bit fields of a register and
 * combine them with a logical OR. The result can be sent to the BGT60TRxx to
 * write the whole register.
 */
#define BGT60TRxxD_SET(REGISTER, FIELD, X) \
    ((((uint32_t)(X) << BGT60TRxxD_##REGISTER##_##FIELD##_pos) & BGT60TRxxD_##REGISTER##_##FIELD##_msk) | (BGT60TRxxD_REG_##REGISTER << 25) | 0x01000000)

/*
 * \brief This macro creates an SPI data word to read a register of the
 *        BGT60TRxxD.
 *
 * The macro constructs a 32 bit word that can must be transmitted to the
 * BGT60TRxxD. The transfer must be done in a full duplex transmission,
 * because the register data will be returned this transfer.
 */
#define BGT60TRxxD_REGISTER_READ_CMD(REGISTER) \
    (BGT60TRxxD_REG_##REGISTER << 25)

/**
 * \brief This macro extracts a certain bit field from an SPI register value.
 *
 * Pass the register and bit field names and a 32 bit integer value that
 * contains the bit field in its lower 24 bits. The value of the bit field
 * is masked and shifted. The result is the value of the bit field.
 */
#define BGT60TRxxD_EXTRACT(REGISTER, FIELD, X) \
    (((uint32_t)(X)&BGT60TRxxD_##REGISTER##_##FIELD##_msk) >> BGT60TRxxD_##REGISTER##_##FIELD##_pos)

/* Extract fields of register GSR0 */
/* ------------------------------- */
#define BGT60TRxxD_GET_GSR0_FOU_ERR(X)            ((X)&0x08000000)
#define BGT60TRxxD_GET_GSR0_MISO_HS_READ(X)       ((X)&0x04000000)
#define BGT60TRxxD_GET_GSR0_BURST_ERROR(X)        ((X)&0x02000000)
#define BGT60TRxxD_GET_GSR0_CLOCK_NUMBER_ERROR(X) ((X)&0x01000000)

/** @} */


#endif /* BGT60TRXXD_H_ */
