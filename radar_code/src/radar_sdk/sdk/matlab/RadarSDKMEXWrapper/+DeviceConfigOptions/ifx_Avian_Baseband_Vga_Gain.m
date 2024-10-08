classdef ifx_Avian_Baseband_Vga_Gain < int32
    %IFX_AVIAN_BASEBAND_VGA_GAIN  This enumeration type lists the
    %available gain settings of the Avian device's integrated baseband
    %amplifier (VGA).
    enumeration
        IFX_VGA_GAIN_0dB  (0) % The gain of the VGA is 0dB.
        IFX_VGA_GAIN_5dB  (1) % The gain of the VGA is +5dB.
        IFX_VGA_GAIN_10dB (2) % The gain of the VGA is +10dB.
        IFX_VGA_GAIN_15dB (3) % The gain of the VGA is +15dB.
        IFX_VGA_GAIN_20dB (4) % The gain of the VGA is +20dB.
        IFX_VGA_GAIN_25dB (5) % The gain of the VGA is +25dB.
        IFX_VGA_GAIN_30dB (6) % The gain of the VGA is +30dB.
    end
end
