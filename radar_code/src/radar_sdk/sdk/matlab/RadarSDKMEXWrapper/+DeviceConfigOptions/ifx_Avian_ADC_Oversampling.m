classdef ifx_Avian_ADC_Oversampling < int32
    %IFX_AVIAN_ADC_OVERSAMPLING  This enumeration type lists the available
    %ADC oversampling modes.
    %   The Avian device's internal Analog-Digital-Converter (ADC) is
    %   capable to repeat the full sample-hold-convert cycle multiple times
    %   and return the average of all cycles.
    enumeration
        IFX_ADC_OVERSAMPLING_OFF  (0) % No oversampling
        IFX_ADC_OVERSAMPLING_2x   (1) % Oversampling factor 2
        IFX_ADC_OVERSAMPLING_4x   (2) % Oversampling factor 4
        IFX_ADC_OVERSAMPLING_8x   (3) % Oversampling factor 8
    end
end
