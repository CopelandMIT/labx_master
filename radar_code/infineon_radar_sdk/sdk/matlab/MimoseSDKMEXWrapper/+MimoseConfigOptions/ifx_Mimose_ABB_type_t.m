classdef ifx_Mimose_ABB_type_t < int32
    %IFX_MIMOSE_ABB_TYPE_T  his type enumerates the
    %available modes of the Avian device's test signal generator.
    enumeration
		IFX_MIMOSE_ABB_GAIN_3 (0)
		IFX_MIMOSE_ABB_GAIN_6 (1)
		IFX_MIMOSE_ABB_GAIN_12 (2)
		IFX_MIMOSE_ABB_GAIN_24 (3)
		IFX_MIMOSE_ABB_GAIN_48 (4)
		IFX_MIMOSE_ABB_GAIN_96 (5)
		IFX_MIMOSE_ABB_GAIN_192 (6)
		IFX_MIMOSE_ABB_GAIN_384 (7)
		IFX_MIMOSE_ABB_GAIN_AUTOMATIC (255)
    end
end
