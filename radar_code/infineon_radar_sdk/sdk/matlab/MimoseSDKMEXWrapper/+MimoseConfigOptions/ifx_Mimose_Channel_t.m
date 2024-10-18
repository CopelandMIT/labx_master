classdef ifx_Mimose_Channel_t < int32
    %IFX_MIMOSE_CHANNEL_T  This enumeration type lists the
    %available gain settings of the Avian device's integrated baseband
    %amplifier (VGA).
    enumeration
        IFX_MIMOSE_CHANNEL_TX1_RX1 (0)
        IFX_MIMOSE_CHANNEL_TX2_RX2 (1)
        IFX_MIMOSE_CHANNEL_TX1_RX2 (2)
        IFX_MIMOSE_CHANNEL_TX2_RX1 (3)
    end
end
