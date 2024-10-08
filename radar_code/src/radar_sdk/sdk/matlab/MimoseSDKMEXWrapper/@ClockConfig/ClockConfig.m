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

classdef ClockConfig
    %DEVICECONFIG Class definition for configuring a RadarDevice
    % This matlab class creates an object with all parameters to configure the radar device,
    % similar to the device configuration of the radar_sdk. The DeviceConfig object is required
    % as an input to instantiate a RadarDevice object. This class also provides a helper
    % function mkDevConf() to create a DeviceConfig object and accepts param-value pairs in
    % any order. It assumes default values for any parameter that is not specified.

    properties
        % 
        reference_clock_Hz,
        system_clock_Hz,
        rc_clock_enabled,
        hf_on_time_usec,
        system_clock_divider,
        system_clock_div_flex,
        sys_clk_to_i2c,
    end

    methods
        function obj = ClockConfig(reference_clock_Hz, ...
                                   system_clock_Hz, ...
                                   rc_clock_enabled, ...
                                   hf_on_time_usec, ...
                                   system_clock_divider, ...
                                   system_clock_div_flex, ...
                                   sys_clk_to_i2c)
            %CLOCKCONFIG Construct an instance of this class
            %   The function instantiates a MimoseConfig object
            obj.reference_clock_Hz = reference_clock_Hz;
            obj.system_clock_Hz = system_clock_Hz;
            obj.rc_clock_enabled = rc_clock_enabled;
            obj.hf_on_time_usec = hf_on_time_usec;
            obj.system_clock_divider = system_clock_divider;
            obj.system_clock_div_flex = system_clock_div_flex;
            obj.sys_clk_to_i2c = sys_clk_to_i2c;
        end
    end

    methods(Static)
        function obj = mkConfig(varargin)
            %MKDEVCONF Helper function for creating a MimoseConfig object
            %   This function accepts the config params as param - value pairs
            %   in any order, and uses default values for any unspecified
            %   param, and returns a DeviceConfig object.

            p = inputParser;
            addParameter(p, 'reference_clock_Hz', 38400000);
            addParameter(p, 'system_clock_Hz', 38400000);
            addParameter(p, 'rc_clock_enabled', false);
            addParameter(p, 'hf_on_time_usec', 1);
            addParameter(p, 'system_clock_divider', 0);
            addParameter(p, 'system_clock_div_flex', false);
            addParameter(p, 'sys_clk_to_i2c', false);
            
            parse(p,varargin{:});
            params = p.Results;

            obj = ClockConfig(params.reference_clock_Hz, ...
                params.system_clock_Hz, ...
                params.rc_clock_enabled, ...
                params.hf_on_time_usec, ...
                params.system_clock_divider, ...
                params.system_clock_div_flex, ...
                params.sys_clk_to_i2c);
        end
    end
end

