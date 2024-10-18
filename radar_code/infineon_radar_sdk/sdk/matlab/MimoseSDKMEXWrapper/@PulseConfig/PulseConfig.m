% ===========================================================================
% Copyright (C) 2021-2022 Infineon Technologies AG
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

classdef PulseConfig
    %DEVICECONFIG Class definition for configuring a RadarDevice
    % This matlab class creates an object with all parameters to configure the radar device,
    % similar to the device configuration of the radar_sdk. The DeviceConfig object is required
    % as an input to instantiate a RadarDevice object. This class also provides a helper
    % function mkDevConf() to create a DeviceConfig object and accepts param-value pairs in
    % any order. It assumes default values for any parameter that is not specified.

    properties
        % 
        channel,

        % 
        tx_power_level,

        % 
        abb_gain_type,

        % 
        aoc_mode,
    end

    methods
        function obj = PulseConfig(channel, tx_power_level, abb_gain_type, aoc_mode)
            %MIMOSECONFIG Construct an instance of this class
            %   The function instantiates a MimoseConfig object
            obj.channel = channel;
            obj.tx_power_level = tx_power_level;
            obj.abb_gain_type  = abb_gain_type;
            obj.aoc_mode = aoc_mode;
        end
    end

    methods(Static)
        function obj = mkConfig(varargin)
            %MKDEVCONF Helper function for creating a MimoseConfig object
            %   This function accepts the config params as param - value pairs
            %   in any order, and uses default values for any unspecified
            %   param, and returns a DeviceConfig object.

            p = inputParser;
            addParameter(p, 'channel', MimoseConfigOptions.ifx_Mimose_Channel_t.IFX_MIMOSE_CHANNEL_TX1_RX1);
            addParameter(p, 'tx_power_level', 50);
            addParameter(p, 'abb_gain_type', MimoseConfigOptions.ifx_Mimose_ABB_type_t.IFX_MIMOSE_ABB_GAIN_384);
            addParameter(p, 'aoc_mode', MimoseConfigOptions.ifx_Mimose_AOC_Mode_t.IFX_MIMOSE_AOC_OFFSET_VALUE_IS_FULLY_HANDLED_BY_THE_FSM_2);
            
            parse(p,varargin{:});
            params = p.Results;

            obj = PulseConfig(params.channel, ...
                               params.tx_power_level, ...
                               params.abb_gain_type, ...
                               params.aoc_mode);
        end
    end
end

