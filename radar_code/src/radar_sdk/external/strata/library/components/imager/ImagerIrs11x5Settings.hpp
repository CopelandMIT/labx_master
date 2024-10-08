/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include "RegistersIrs.hpp"

class ImagerIrs11x5Settings
{
public:
    constexpr static RegistersIrs::BatchType pllSettings26MHz[] = {
        // System PLL settings for 26 MHz
        {0xB062, 0x6383},  // DPHYPLLCFG1 - 26 MHz
        {0xB063, 0x55A8},  // DPHYPLLCFG2 - 26 MHz
        {0xB068, 0x7628},  // DPHYPLLCFG7 - 26 MHz
        {0xB069, 0x03E2},  // DPHYPLLCFG8 - 26 MHz
    };

    constexpr static RegistersIrs::BatchType basicConfig[] = {
        // for better stability charge capacitors at power-up instead of first modulation start
        {0xB01C, 0x0001},  // ANAIP_ADCBG1
        {0xB01D, 0x1888},  // ANAIP_ADCBG2
        {0xB042, 0x0003},  // ANAIP_PIXREFBGEN
        {0xB0A2, 0x0005},  // ANAIP_VMODREG

        // Chip settings for IRS11x5C
        {0xB00C, 0x0647},  // GPIO_CLK_CFG1 - disable internal alive clock divider
        {0xB017, 0x0413},  // ANAIP_PADGPIOCFG9 - GPIO19 tristate
        {0x9813, 0x13FF},  // MTCU_POWERCTRL
        {0xA88F, 0x031E},  // CFGCNT_EXPCFG2 - global-select pattern to avoid memory effect
        {0x981B, 0x7608},  // MTCU_SPARE - enable global select during pre-illumination (default 10 cycles) and warmup (default 30 cycles) phase
        {0xB0AE, 0xEF02},  // ANAIP_SPARE - enable modulation duty cycle control for MOD_SE_P pad and last 16 column fix

        //Interface settings for PIF
        {0xA883, 0x1057},  // CFGCNT_PIFCCFG
        {0xA884, 0x0000},  // CFGCNT_PIFTCFG

        // MIPI phy settings
        //{0xB05A, 0x01C5},  // ANAIP_DPHYDLANE1CFG1
        //{0xB05C, 0x0206},  // ANAIP_DPHYDLANE1CFG3
        //{0xB05D, 0x01C5},  // ANAIP_DPHYDLANE2CFG1
        //{0xB05F, 0x0206},  // ANAIP_DPHYDLANE2CFG3

        // Pin configuration for Evalkit
        //Modulation pads
        //    {0xA892, 0x011D},  // CFGCNT_PSOUT - enable internal MOD_SE_P and MOD_SE_N signal and LVDS
        //    {0xB086, 0x1313},  // ANAIP_PSPADCFG - MOD_SE_P and MOD_SE_N pad push/pull
        {0xA892, 0x031D},  // CFGCNT_PSOUT - enable internal MOD_SE_P and MOD_SE_N signal and disable LVDS
        {0xB086, 0x1513},  // ANAIP_PSPADCFG - MOD_SE_P pad push/pull

        //Illumination interface
        // Evalkit GPIO16 is named GPO19
        {0xB008, 0x0180},  // ANAIP_GPIOMUX5 - GPIO16 = 0  // enable LED1
        {0xB009, 0x31A0},  // ANAIP_GPIOMUX6 - GPO20 = 0  // lower illumination voltage
        //{0xB00A, 0x018D},  // ANAIP_GPIOMUX7 - GPO22 = 0, GPO21 = 1  // enable LED2
        {0xB00A, 0x01AD},  // ANAIP_GPIOMUX7 - GPO22 = 1, GPO21 = 1  // disable LED2
        {0xB016, 0x1315},  // ANAIP_PADGPIOCFG8 - GPIO16 push/pull
        {0xB018, 0x0404},  // ANAIP_PADGPIOCFG10 - GPO20 push/pull, GPO21 push/pull
        {0xB019, 0x0004},  // ANAIP_PADGPIOCFG11 - GPO22 - push/pull

        //Parallel interface
        {0xB00E, 0x1515},  // ANAIP_PADGPIOCFG0 - push/pull (PIF)
        {0xB00F, 0x1515},  // ANAIP_PADGPIOCFG1 - push/pull (PIF)
        {0xB010, 0x1515},  // ANAIP_PADGPIOCFG2 - push/pull (PIF)
        {0xB011, 0x1515},  // ANAIP_PADGPIOCFG3 - push/pull (PIF)
        {0xB012, 0x1515},  // ANAIP_PADGPIOCFG4 - push/pull (PIF)
        {0xB013, 0x1515},  // ANAIP_PADGPIOCFG5 - push/pull (PIF)
        {0xB014, 0x1315},  // ANAIP_PADGPIOCFG6 - GPIO12 push/pull (PIF)
        {0xB015, 0x1513},  // ANAIP_PADGPIOCFG7 - GPIO15 push/pull (PIF)
        //{0xB014, 0x1515},  // ANAIP_PADGPIOCFG6 - GPIO12 push/pull (PIF), GPIO13 push/pull (LED)
        //{0xB015, 0x1515},  // ANAIP_PADGPIOCFG7 - GPIO14 push/pull (LED), GPIO15 push/pull (PIF)
        {0xB016, 0x1515},  // ANAIP_PADGPIOCFG8 - GPIO16 push/pull, GPIO17 push/pull (PIF)
        {0xB017, 0x0415},  // ANAIP_PADGPIOCFG9 - GPIO18 push/pull (PIF), GPIO19 tristate (disable alive toggle)

        // Modulation PLL settings @ 26 MHz
        {0xA893, 0x4041},  // CFGCNT_PLLCFG1_LUT1 - 60,24 MHz
        {0xA894, 0x877B},  // CFGCNT_PLLCFG2_LUT1 - 60,24 MHz
        {0xA895, 0x03C4},  // CFGCNT_PLLCFG3_LUT1 - 60,24 MHz
        {0xA896, 0x4059},  // CFGCNT_PLLCFG1_LUT2 - 80,32 MHz
        {0xA897, 0x5F4F},  // CFGCNT_PLLCFG2_LUT2 - 80,32 MHz
        {0xA898, 0x0BDB},  // CFGCNT_PLLCFG3_LUT2 - 80,32 MHz
    };

    constexpr static RegistersIrs::BatchType greyConfig[] = {
        //Resolution settings
        //ROI: 352x288
        //Readout Time (2 lane): 1.575ms
        {0xA886, 0x0000},  // CFGCNT_ROICMINREG
        {0xA887, 0x015F},  // CFGCNT_ROICMAXREG
        {0xA888, 0x0000},  // CFGCNT_ROIRMINREG
        {0xA889, 0x011F},  // CFGCNT_ROIRMAXREG

        //Sequence settings
        //continuous greyscale measurement
        {0xA88D, 0x0000},  // CFGCNT_CTRLSEQ - 0..0 (1 entries)
        {0xA800, 0x1D6A},  // CFGCNT_S00_EXPOTIME - 1ms @ 60,24 MHz
        {0xA801, 0x32DC},  // CFGCNT_S00_FRAMERATE - 100ms
        {0xA802, 0x200C},  // CFGCNT_S00_PS - grey_on
        {0xA803, 0x0000},  // CFGCNT_S00_PLLSET - 0
    };

    constexpr static RegistersIrs::BatchType calibConfig[] = {
        //Resolution settings
        //ROI: 352x288
        //Readout Time (2 lane): 1.575ms
        {0xA886, 0x0000},  // CFGCNT_ROICMINREG
        {0xA887, 0x015F},  // CFGCNT_ROICMAXREG
        {0xA888, 0x0000},  // CFGCNT_ROIRMINREG
        {0xA889, 0x011F},  // CFGCNT_ROIRMAXREG

        //Sequence settings
        {0xA88D, 0x0000},  // CFGCNT_CTRLSEQ - 0..0 (1 entries)
        {0xA800, 0x0004},  // CFGCNT_S00_EXPOTIME - shortest exposure time for offset measurement
        {0xA801, 0x32DC},  // CFGCNT_S00_FRAMERATE - 100ms
        {0xA802, 0x2000},  // CFGCNT_S00_PS - grey_off
        {0xA803, 0x0000},  // CFGCNT_S00_PLLSET - 0
    };

    constexpr static RegistersIrs::BatchType tangoConfig[] = {
        //Resolution settings
        //ROI: 352x288
        //Readout Time (2 lane): 1.575ms
        {0xA886, 0x0000},  // CFGCNT_ROICMINREG
        {0xA887, 0x015F},  // CFGCNT_ROICMAXREG
        {0xA888, 0x0000},  // CFGCNT_ROIRMINREG
        {0xA889, 0x011F},  // CFGCNT_ROIRMAXREG

        // 5 fps
        {0xA88D, 0x0008},  // CFGCNT_CTRLSEQ - 0..8 (9 entries)
        {0xA800, 0x1D6A},  // CFGCNT_S00_EXPOTIME - 1ms @ 60,24 MHz
        {0xA801, 0x0000},  // CFGCNT_S00_FRAMERATE - 0ms
        {0xA802, 0x2000},  // CFGCNT_S00_PS - grey_off
        {0xA803, 0x0000},  // CFGCNT_S00_PLLSET - 0
        {0xA804, 0x1D6A},  // CFGCNT_S01_EXPOTIME - 1ms @ 60,24 MHz
        {0xA805, 0x0000},  // CFGCNT_S01_FRAMERATE - 0ms
        {0xA806, 0x000C},  // CFGCNT_S01_PS - 0
        {0xA807, 0x0000},  // CFGCNT_S01_PLLSET - 0
        {0xA808, 0x1D6A},  // CFGCNT_S02_EXPOTIME - 1ms @ 60,24 MHz
        {0xA809, 0x0000},  // CFGCNT_S02_FRAMERATE - 0ms
        {0xA80A, 0x0440},  // CFGCNT_S02_PS - 90
        {0xA80B, 0x0000},  // CFGCNT_S02_PLLSET - 0
        {0xA80C, 0x1D6A},  // CFGCNT_S03_EXPOTIME - 1ms @ 60,24 MHz
        {0xA80D, 0x0000},  // CFGCNT_S03_FRAMERATE - 0ms
        {0xA80E, 0x0884},  // CFGCNT_S03_PS - 180
        {0xA80F, 0x0000},  // CFGCNT_S03_PLLSET - 0
        {0xA810, 0x1D6A},  // CFGCNT_S04_EXPOTIME - 1ms @ 60,24 MHz
        {0xA811, 0x0000},  // CFGCNT_S04_FRAMERATE - 0ms
        {0xA812, 0x0CC8},  // CFGCNT_S04_PS - 270
        {0xA813, 0x0000},  // CFGCNT_S04_PLLSET - 0
        {0xA814, 0x2738},  // CFGCNT_S05_EXPOTIME - 1ms @ 80,32 MHz
        {0xA815, 0x0000},  // CFGCNT_S05_FRAMERATE - 0ms
        {0xA816, 0x000C},  // CFGCNT_S05_PS - 0
        {0xA817, 0x0001},  // CFGCNT_S05_PLLSET - 1
        {0xA818, 0x2738},  // CFGCNT_S06_EXPOTIME - 1ms @ 80,32 MHz
        {0xA819, 0x0000},  // CFGCNT_S06_FRAMERATE - 0ms
        {0xA81A, 0x0440},  // CFGCNT_S06_PS - 90
        {0xA81B, 0x0001},  // CFGCNT_S06_PLLSET - 1
        {0xA81C, 0x2738},  // CFGCNT_S07_EXPOTIME - 1ms @ 80,32 MHz
        {0xA81D, 0x0000},  // CFGCNT_S07_FRAMERATE - 0ms
        {0xA81E, 0x0884},  // CFGCNT_S07_PS - 180
        {0xA81F, 0x0001},  // CFGCNT_S07_PLLSET - 1
        {0xA820, 0x2738},  // CFGCNT_S08_EXPOTIME - 1ms @ 80,32 MHz
        {0xA821, 0x58F2},  // CFGCNT_S08_FRAMERATE - 174,88ms
        {0xA822, 0x0CC8},  // CFGCNT_S08_PS - 270
        {0xA823, 0x0001},  // CFGCNT_S08_PLLSET - 1
    };
};


constexpr RegistersIrs::BatchType ImagerIrs11x5Settings::pllSettings26MHz[];

constexpr RegistersIrs::BatchType ImagerIrs11x5Settings::basicConfig[];
constexpr RegistersIrs::BatchType ImagerIrs11x5Settings::greyConfig[];
constexpr RegistersIrs::BatchType ImagerIrs11x5Settings::calibConfig[];
constexpr RegistersIrs::BatchType ImagerIrs11x5Settings::tangoConfig[];
