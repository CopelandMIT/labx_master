classdef ifx_Avian_Baseband_Hp_Gain < int32
    %IFX_AVIAN_BASEBAND_HP_GAIN  This enumeration type lists the available
    %gain settings of the Avian device's integrated baseband high pass
    %filters.
    enumeration
        IFX_HP_GAIN_18dB    (0) % The gain of the high pass filter is +18dB.
        IFX_HP_GAIN_30dB    (1) % The gain of the high pass filter is +30dB.
    end
end
