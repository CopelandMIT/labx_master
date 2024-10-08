classdef ifx_Avian_ADC_SampleTime < int32
    %IFX_AVIAN_ADC_SAMPLETIME  This enumeration type lists the available
    %ADC sample time settings.
    %   The sample time is the time that the sample-and-hold-circuitry of the Avian
    %   device's Analog-Digital-Converter (ADC) takes to sample the voltage at it's
    %   input.
    %
    %   note: The specified timings refer to an external reference clock frequency
    %         of 80MHz. If the clock frequency differs from that, the sample time
    %         periods are scaled accordingly.
    enumeration
        IFX_ADC_SAMPLETIME_50NS    (0) % The voltage is sampled for 50ns
        IFX_ADC_SAMPLETIME_100NS   (1) % The voltage is sampled for 100ns
        IFX_ADC_SAMPLETIME_200NS   (2) % The voltage is sampled for 200ns
        IFX_ADC_SAMPLETIME_400NS   (3) % The voltage is sampled for 400ns
    end
end
