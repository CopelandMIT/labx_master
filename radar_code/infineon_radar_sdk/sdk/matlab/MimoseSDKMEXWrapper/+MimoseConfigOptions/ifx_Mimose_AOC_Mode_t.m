classdef ifx_Mimose_AOC_Mode_t < int32
    %IFX_MIMOSE_AOC_MODE_T  This enumeration type lists the available
    %gain settings of the Avian device's integrated baseband high pass
    %filters.
    enumeration
		IFX_MIMOSE_AOC_OFFSET_VALUE_IS_APPLIED (0)
		IFX_MIMOSE_AOC_OFFSET_VALUE_IS_ONLY_TRACKED (1)    
		IFX_MIMOSE_AOC_OFFSET_VALUE_IS_FULLY_HANDLED_BY_THE_FSM_2 (2)
		IFX_MIMOSE_AOC_OFFSET_VALUE_IS_FULLY_HANDLED_BY_THE_FSM_3 (3)
    end
end
