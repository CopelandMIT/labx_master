% ===========================================================================
% Copyright (C) 2021-2023 Infineon Technologies AG
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

classdef AFC_Config
    %DEVICECONFIG Class definition for configuring a RadarDevice
    % This matlab class creates an object with all parameters to configure the radar device,
    % similar to the device configuration of the radar_sdk. The DeviceConfig object is required
    % as an input to instantiate a RadarDevice object. This class also provides a helper
    % function mkDevConf() to create a DeviceConfig object and accepts param-value pairs in
    % any order. It assumes default values for any parameter that is not specified.

    properties
        % 
        band,
        rf_center_frequency_Hz,
        afc_duration_ct,
        afc_threshold_course,
        afc_threshold_fine,
        afc_period,
        afc_repeat_count,
    end

    methods
        function obj = AFC_Config(band, ...
                                  rf_center_frequency_Hz, ...
                                  afc_duration_ct, ...
                                  afc_threshold_course, ...
                                  afc_threshold_fine, ...
                                  afc_period, ...
                                  afc_repeat_count)
            %AFC_CONFIG Construct an instance of this class
            %   The function instantiates a MimoseConfig object
            obj.band = band;
            obj.rf_center_frequency_Hz = rf_center_frequency_Hz;
            obj.afc_duration_ct = afc_duration_ct;
            obj.afc_threshold_course = afc_threshold_course;
            obj.afc_threshold_fine = afc_threshold_fine;
            obj.afc_period = afc_period;
            obj.afc_repeat_count = afc_repeat_count;
        end
    end

    methods(Static)
        function obj = mkConfig(varargin)
            %MKDEVCONF Helper function for creating a MimoseConfig object
            %   This function accepts the config params as param - value pairs
            %   in any order, and uses default values for any unspecified
            %   param, and returns a DeviceConfig object.

            p = inputParser;
            addParameter(p, 'band', MimoseConfigOptions.ifx_Mimose_RF_Band_t.IFX_MIMOSE_RF_BAND_ISM_SRR_24GHZ_100MHz);
            addParameter(p, 'rf_center_frequency_Hz', 24200000000);
            addParameter(p, 'afc_duration_ct', 115);
            addParameter(p, 'afc_threshold_course', 3);
            addParameter(p, 'afc_threshold_fine', 10);
            addParameter(p, 'afc_period', 16);
            addParameter(p, 'afc_repeat_count', MimoseConfigOptions.ifx_Mimose_AFC_Repeat_Count_t.IFX_MIMOSE_AFC_REPEAT_COUNT_1);
            
            parse(p,varargin{:});
            params = p.Results;

            obj = AFC_Config(params.band, ...
                             params.rf_center_frequency_Hz, ...
                             params.afc_duration_ct, ...
                             params.afc_threshold_course, ...
                             params.afc_threshold_fine, ...
                             params.afc_period, ...
                             params.afc_repeat_count);
        end
    end
end

