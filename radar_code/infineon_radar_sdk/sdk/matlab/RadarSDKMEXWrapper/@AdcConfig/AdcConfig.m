% ===========================================================================
% Copyright (C) 2021 Infineon Technologies AG
%
% Redistribution and use in source and binary forms, with or without
% modification, are permitted provided that the following conditions are met:
%
% 1. Redistributions of source code must retain the above copyright notice,
%    this list of conditions and the following disclaimer.
% 2. Redistributions in binary form must reproduce the above copyright
%    notice, this list of conditions and the following disclaimer in the
%    documentation and/or other materials provided with the distribution.
% 3. Neither the name of the copyright holder nor the names of its
%    contributors may be used to endorse or promote products derived from
%    this software without specific prior written permission.
%
% THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
% AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
% IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
% ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
% LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
% CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
% SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
% INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
% CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
% ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
% POSSIBILITY OF SUCH DAMAGE.
% ===========================================================================

classdef AdcConfig
    % ADCCONFIG Class definition for configuring a Continuous Wave Control ADC parameters
    % This matlab class creates an object with all parameters to configure the device,
    % similar to the configuration of the radar_sdk. The AdcConfig object is required
    % as an input to set ADC params for Continuous Wave Control. This class also provides a helper
    % function mkAdcConf() to create an AdcConfig object and accepts param-value pairs in
    % any order. It assumes default values for any parameter that is not specified.

    properties
        % Sample rate of the ADC in Hz
        samplerate_Hz,

        % ADC trakcing mode for converting measurements
        tracking,

        % ADC sample time setting
        sample_time,

        % Double MSB time
        double_msb_time,
        
        % ADC oversampling mode
        oversampling,
    end

    methods
        function obj = AdcConfig(samplerate_Hz, ...
                                    tracking, ...
                                    sample_time, ...
                                    double_msb_time, ...
                                    oversampling)
            % ADCCONFIG Construct an instance of this class
            %   The function instantiates an Adc Config object
            obj.samplerate_Hz = samplerate_Hz;
            obj.tracking = tracking;
            obj.sample_time = sample_time;
            obj.double_msb_time = double_msb_time;
            obj.oversampling = oversampling;
        end
    end

    methods(Static)
        function obj = mkAdcConf(varargin)
            % MKADCCONF Helper function for creating an Adc Config object
            %   This function accepts the config params as param - value pairs
            %   in any order, and uses default values for any unspecified
            %   param, and returns an Adc Config object.
            %   accepted parameters and their default values are as follows:
            %   param: samplerate_Hz       default val = 1e6; % 1MHz
            %   param: tracking            default val = 0; % off
            %   param: sample_time         default val = 1; % 100NS 
            %   param: double_msb_time     default val = 1;
            %   param: oversampling        default val = 0; % off

            p = inputParser;
            addParameter(p, 'samplerate_Hz', 2000000);
            addParameter(p, 'tracking', DeviceConfigOptions.ifx_Avian_ADC_Tracking.IFX_ADC_NO_SUBCONVERSIONS);
            addParameter(p, 'sample_time', DeviceConfigOptions.ifx_Avian_ADC_SampleTime.IFX_ADC_SAMPLETIME_100NS);
            addParameter(p, 'double_msb_time', 1);
            addParameter(p, 'oversampling', DeviceConfigOptions.ifx_Avian_ADC_Oversampling.IFX_ADC_OVERSAMPLING_OFF);

            parse(p,varargin{:});
            params = p.Results;

            obj = AdcConfig(double(params.samplerate_Hz), ...
                               double(params.tracking), ...
                               double(params.sample_time), ...
                               double(params.double_msb_time), ...
                               double(params.oversampling));
        end
    end
end

