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

classdef MimoseConfig
    %MIMOSECONFIG Class definition for configuring a RadarDevice
    % This matlab class creates an object with all parameters to configure the radar device,
    % similar to the device configuration of the radar_sdk. The DeviceConfig object is required
    % as an input to instantiate a RadarDevice object. This class also provides a helper
    % function mkDevConf() to create a DeviceConfig object and accepts param-value pairs in
    % any order. It assumes default values for any parameter that is not specified.

    properties
        % 
        PulseConfig,

        % 
        FrameConfig,

        % 
        AFC_Config,

        % 
        ClockConfig,
    end

    methods
        function obj = MimoseConfig()
            %MIMOSECONFIG Construct an instance of this class
            %   The function instantiates a MimoseConfig object with
            %   default values
            obj.PulseConfig{1} = PulseConfig.mkConfig('channel', MimoseConfigOptions.ifx_Mimose_Channel_t.IFX_MIMOSE_CHANNEL_TX1_RX1, ...
                'tx_power_level', 63);
            obj.PulseConfig{2} = PulseConfig.mkConfig('channel', MimoseConfigOptions.ifx_Mimose_Channel_t.IFX_MIMOSE_CHANNEL_TX2_RX2, ...
                'tx_power_level', 63);
            obj.PulseConfig{3} = PulseConfig.mkConfig('channel', MimoseConfigOptions.ifx_Mimose_Channel_t.IFX_MIMOSE_CHANNEL_TX1_RX2);
            obj.PulseConfig{4} = PulseConfig.mkConfig('channel', MimoseConfigOptions.ifx_Mimose_Channel_t.IFX_MIMOSE_CHANNEL_TX2_RX1);

            obj.FrameConfig{1} = FrameConfig.mkConfig('frame_repetition_time_s', 0.250, ...
                'pulse_repetition_time_s', 0.001, ...
                'selected_pulse_config_0', true, ...
                'num_of_samples', 128);
            obj.FrameConfig{2} = FrameConfig.mkConfig();
            obj.AFC_Config  = AFC_Config.mkConfig;
            obj.ClockConfig = ClockConfig.mkConfig;
        end
    end

    methods (Static)
        function printconfig(config)
            disp(config.PulseConfig{1});
            disp(config.PulseConfig{2});
            disp(config.PulseConfig{3});
            disp(config.PulseConfig{4});
            disp(config.FrameConfig{1});
            disp(config.FrameConfig{2});
            disp(config.AFC_Config);
            disp(config.ClockConfig);
        end
    end

end

