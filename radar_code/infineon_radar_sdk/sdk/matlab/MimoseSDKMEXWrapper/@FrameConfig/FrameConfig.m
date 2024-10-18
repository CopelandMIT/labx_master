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

classdef FrameConfig
    %DEVICECONFIG Class definition for configuring a RadarDevice
    % This matlab class creates an object with all parameters to configure the radar device,
    % similar to the device configuration of the radar_sdk. The DeviceConfig object is required
    % as an input to instantiate a RadarDevice object. This class also provides a helper
    % function mkDevConf() to create a DeviceConfig object and accepts param-value pairs in
    % any order. It assumes default values for any parameter that is not specified.

    properties
        % 
        frame_repetition_time_s,
        pulse_repetition_time_s,
        selected_pulse_config_0,
        selected_pulse_config_1,
        selected_pulse_config_2,
        selected_pulse_config_3,
        num_samples,
    end

    methods
        function obj = FrameConfig(frame_repetition_time_s, pulse_repetition_time_s, selected_pulse_config_0, ...
                selected_pulse_config_1, selected_pulse_config_2, selected_pulse_config_3, num_samples)
            %MIMOSECONFIG Construct an instance of this class
            %   The function instantiates a MimoseConfig object
            obj.frame_repetition_time_s = frame_repetition_time_s;
            obj.pulse_repetition_time_s = pulse_repetition_time_s;
            obj.selected_pulse_config_0 = selected_pulse_config_0;
            obj.selected_pulse_config_1 = selected_pulse_config_1;
            obj.selected_pulse_config_2 = selected_pulse_config_2;
            obj.selected_pulse_config_3 = selected_pulse_config_3;
            obj.num_samples = num_samples;
        end
    end

    methods(Static)
        function obj = mkConfig(varargin)
            %MKDEVCONF Helper function for creating a MimoseConfig object
            %   This function accepts the config params as param - value pairs
            %   in any order, and uses default values for any unspecified
            %   param, and returns a DeviceConfig object.

            p = inputParser;
            addParameter(p, 'frame_repetition_time_s', 0.04);
            addParameter(p, 'pulse_repetition_time_s', 0.0005);
            addParameter(p, 'selected_pulse_config_0', false);
            addParameter(p, 'selected_pulse_config_1', false);
            addParameter(p, 'selected_pulse_config_2', false);
            addParameter(p, 'selected_pulse_config_3', false);
            addParameter(p, 'num_of_samples', 16);
            
            parse(p,varargin{:});
            params = p.Results;

            obj = FrameConfig(params.frame_repetition_time_s, ...
                               params.pulse_repetition_time_s, ...
                               params.selected_pulse_config_0, ...
                               params.selected_pulse_config_1, ...
                               params.selected_pulse_config_2, ...
                               params.selected_pulse_config_3, ...
                               params.num_of_samples);
        end
    end
end

