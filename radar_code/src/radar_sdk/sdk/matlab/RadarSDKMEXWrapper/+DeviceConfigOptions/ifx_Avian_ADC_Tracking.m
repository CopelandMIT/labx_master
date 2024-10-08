classdef ifx_Avian_ADC_Tracking < int32
    %IFX_AVIAN_ADC_TRACKING  This enumeration type lists the available
    %ADC tracking modes.
    %    The Avian device's internal Analog-Digital-Converter (ADC) has 11 bit
    %    resolution and a 12 bit result register. A single conversion always produces
    %    result values with unset LSB. The resolution can be increased by performing
    %    additional tracking conversion. The result will be the average of all
    %    conversions.
    enumeration
        IFX_ADC_NO_SUBCONVERSIONS  (0) % Each sampled voltage value is converted once
        IFX_ADC_1_SUBCONVERSIONS   (1) % Each sampled voltage value is converted two times
        IFX_ADC_3_SUBCONVERSIONS   (2) % Each sampled voltage value is converted four times
        IFX_ADC_7_SUBCONVERSIONS   (3) % Each sampled voltage value is converted eight times
    end
end
