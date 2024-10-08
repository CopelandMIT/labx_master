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

classdef MimoseDevice < handle
    %MIMOSEDEVICE Class for an ATR22 device.
    %   An object of this class helps to
    %   - configure a radar device and connect to it
    %   - retrieve radar data from it
    %   - disconnect from it.
    %   This object may be used as a Mimose SDK wrapper in matlab,
    %   as it performs the above tasks similar to Mimose SDK

    properties
        device_handle % mimose device handle
        config
        registers
    end

    properties (Access = private, Hidden)
        active_frame
    end

    methods
        function obj = MimoseDevice(varargin)
            %MIMOSEDEVICE API Constructor for an instance of a Mimose Device.
            % Connects to available ATR22 board or uses the provided
            % recording. The class member 'config' is initialized to the defaults.
            % The device is brought to reset state but is not directly configured.
            % An object of this class helps to
            %   - connect to an attached ATR22 (Mimose) device and configure it
            %   - retrieve radar data from the device.
            %   - disconnect from it.
            %   This object may be used as a Mimose SDK wrapper in matlab,
            %   as it performs the above tasks similar to Mimose SDK
            %
            %   USAGE
            %   Dev = MimoseDevice(); % This creates a Mimose device object
            %   and stores it in variable 'Dev'

            p = inputParser;
            addParameter(p, 'playback_path', '');
            addParameter(p, 'correct_timing', 1);

            parse(p,varargin{:});
            params = p.Results;

            if(isempty(params.playback_path))
                [ec, handle] = DeviceControlM_Mimose('create');
            else
                [ec, handle] = DeviceControlM_Mimose('create_playback_device', params.playback_path, params.correct_timing);
            end
            obj.check_error_code(ec);
            obj.device_handle = handle;
            obj.config = obj.get_config_defaults();
            obj.active_frame = 1;
            obj.registers = MimoseRegisters();
            obj.get_registers();
        end

        function set_config(obj, oDevConf)
            %SET_CONFIG API Configures the board instance with configuration
            % object parameter
            % USAGE
            % Assuming Mimose object 'Dev'
            %
            % Dev.set_config(); % Uses configuration stored in Dev.config
            % and writes this to the attached device.
            %
            % Dev.set_config(new_config); % Uses new configuration
            % object 'new_config' to configure the device. This also
            % updates the Dev.config to new_config.
            %
            % See also MimoseConfig

            % convert all enumerations to integer
            if(nargin < 2)
                config_sdk = obj.convert_configs_mimose_for_sdk(obj.config);
            else
                config_sdk = obj.convert_configs_mimose_for_sdk(oDevConf);
            end
            ec = DeviceControlM_Mimose('set_config', obj.device_handle, ...
                                                     config_sdk.PulseConfig{1}, ...
                                                     config_sdk.PulseConfig{2}, ...
                                                     config_sdk.PulseConfig{3}, ...
                                                     config_sdk.PulseConfig{4}, ...
                                                     config_sdk.FrameConfig{1}, ...
                                                     config_sdk.FrameConfig{2}, ...
                                                     config_sdk.AFC_Config, ...
                                                     config_sdk.ClockConfig);
            %obj.active_pulse_configs = obj.get_active_pulse_configs(oDevConf2.FrameConfig);
            obj.config = obj.get_config();
            obj.get_registers();
            obj.check_error_code(ec);
        end

        function config = get_config(obj)
            %GET_CONFIG API Retrieves configuration from connected device
            %
            % USAGE
            % Assuming Mimose object 'Dev'
            %
            % retrieved_config = Dev.get_config(); % gets configuration
            % from the device and stores in 'retrieved_config'
            %
            % See also MimoseConfig

            config = obj.config;
            [ec, PC1, PC2, PC3, PC4, FC1, FC2, AC, CC] = DeviceControlM_Mimose('get_config', obj.device_handle, ...
                config.PulseConfig{1}, config.FrameConfig{1}, config.AFC_Config, config.ClockConfig);
            obj.check_error_code(ec);
            config.PulseConfig{1} = PC1;
            config.PulseConfig{2} = PC2;
            config.PulseConfig{3} = PC3;
            config.PulseConfig{4} = PC4;
            config.FrameConfig{1} = FC1;
            config.FrameConfig{2} = FC2;
            config.AFC_Config = AC;
            config.ClockConfig = CC;
            config = obj.convert_configs_mimose_from_sdk(config);
        end

        function config = get_config_defaults(obj)
            %GET_CONFIG_DEFAULTS API Returns a Mimose Configuration
            % structure with default configutration values from the SDK.
            %
            % USAGE
            % Assuming Mimose object 'Dev'
            %
            % default_config = Dev.get_config_defaults(); % gets configuration
            % defaults from the SDK and stores in variable of type MimoseConfig 'default_config'
            %
            % See also MimoseConfig

            config = MimoseConfig();
            [ec, PC1, PC2, PC3, PC4, FC1, FC2, AC, CC] = DeviceControlM_Mimose('get_config_defaults', obj.device_handle, ...
                config.PulseConfig{1}, config.FrameConfig{1}, config.AFC_Config, config.ClockConfig);
            obj.check_error_code(ec);
            config.PulseConfig{1} = PC1;
            config.PulseConfig{2} = PC2;
            config.PulseConfig{3} = PC3;
            config.PulseConfig{4} = PC4;
            config.FrameConfig{1} = FC1;
            config.FrameConfig{2} = FC2;
            config.AFC_Config = AC;
            config.ClockConfig = CC;
            config = obj.convert_configs_mimose_from_sdk(config);
        end

        function disconnect(obj)
            %DISCONNECT API disconnect from a Mimose Device attached via USB
            % This method disconnects and frees device from the port where the radar device is connected
            % The device is then made available for connection using other
            % applications
            %
            % Usage
            % Dev.disconnect();

            if obj.device_handle ~= 0
                ec = DeviceControlM_Mimose('destroy', obj.device_handle);
                obj.check_error_code(ec);
                obj.device_handle = 0;
            end
        end

        function RxFrame = get_next_frame(obj, timeout_ms_opt)
            %GET_NEXT_FRAME API Method to fetch radar data from Mimose device
            %   This method fetches data from the radar device in terms of one
            %   frame (all samples of all enabled pulse configurations)
            %   and returns a Frame variable. If the acquisition is not
            %   started, then this is also started
            %
            %USAGE
            %  frame = Dev.get_next_frame() % the returned 'frame' is a complex matrix with
            %  dimensions (num_pulses x num_samples)
            RxFrame = [];
            if exist('timeout_ms_opt','var')
                [ec, cube_dim_1, num_pulses, num_samples, Total_samples] = DeviceControlM_Mimose('get_next_frame_timeout', obj.device_handle, timeout_ms_opt);
            else
                [ec, cube_dim_1, num_pulses, num_samples, Total_samples] = DeviceControlM_Mimose('get_next_frame', obj.device_handle);
            end
            complex_samples = complex(Total_samples(1:2:end), Total_samples(2:2:end));
            obj.check_error_code(ec);
            if(cube_dim_1 ~=1)
                ME = MException(['RadarDevice:error'], 'wrong dimension retrieved');
                throw(ME);
            end
            RxFrame = reshape(complex_samples, num_pulses, num_samples);
        end

        function start_acquisition(obj)
            %START_ACQUISITION API starts the acquisition of raw data
            %   This method starts the acquisition of raw data when the radar device is connected
            %
            % USAGE
            % Dev.start_acquisition();
            [ec] = DeviceControlM_Mimose('start_acquisition', obj.device_handle);
            obj.check_error_code(ec);
        end

        function stop_acquisition(obj)
            %STOP_ACQUISITION API stops the acquisition of raw data
            %   This method stops the acquisition of raw data when the radar device is connected
            %
            % USAGE
            % Dev.stop_acquisition();

            [ec] = DeviceControlM_Mimose('stop_acquisition', obj.device_handle);
            obj.check_error_code(ec);
        end

        function set_register(obj, address, value)
            %SET_REGISTER API sets register value on connected mimose device
            %
            %USAGE
            % Dev.set_register(0x0001, 0x0023); % this writes the value
            %                                   % 0x0023 at address 0x001
            %
            % %Named memory locations can also be used
            % Dev.set_register('CLK_CONF',68);
            %
            % see also get_register

            address_numeric = obj.registers.get_address(address);
            if(isnan(address_numeric))
                ME = MException('MimoseWrapper:error', 'invalid address');
                throw(ME);
            end
            address_value_word = bitand(bitshift(uint32(address_numeric),16),0xFFFF0000) + ...
                bitand(uint32(value),0x0000FFFF);

            [ec] = DeviceControlM_Mimose('set_register', obj.device_handle, uint32(address_value_word));
            obj.get_registers(); % update register list
            obj.check_error_code(ec);
        end

        function regval = get_register(obj, address)
            %GET_REGISTER API Gets register value from the connected device
            % This is especially useful for readout values
            %
            %USAGE
            % val = Dev.get_register(0x0001); % retrieves value from
            %                                       % address 0x0001. This
            %                                       % value can also be
            %                                       % expressed in decimal
            %                                       % format
            % val = Dev.get_register('CLK_CONF'); % named registers can
            %                                     % also be used.
            %
            % see also set_register

            address_numeric = obj.registers.get_address(address);
            [err_code, regval] = DeviceControlM_Mimose('get_register', obj.device_handle, address_numeric);
            obj.get_registers(); % update register list
            obj.check_error_code(err_code);
        end

        function update_rc_lut(obj)
            %UPDATE_RC_LUT API updates RC clocking options look up table at
            % in extreme temperatures or sample variations may lead to a
            % closer to the desired RC clock
            %
            %USAGE
            %
            % % before calling set_config
            % Dev.update_rc_lut(); % retunes the RC look up table

            [err_code] = DeviceControlM_Mimose('update_rc_lut', obj.device_handle);
            obj.check_error_code(err_code);
        end

        function fw_info = get_firmware_information(obj)
            %GET_FIRMWARE_INFORMATION API Gets information about the firmware of the connected
            % baseboad to which the Radar shield is attached. returns
            % firmware version as a string
            %
            % USAGE
            % firmware_version_string = Dev.get_firmware_information();

            [err_code, fw_major, fw_minor, fw_build] = DeviceControlM_Mimose('get_firmware_information', obj.device_handle, oFwInfo);
            obj.check_error_code(err_code);
            fw_info = [num2str(fw_major),'.',num2str(fw_minor),'.',num2str(fw_build)];
        end

        function delete(obj)
            %DELETE API destroyer for the Mimose Device object
            %
            %USAGE
            % Dev.delete(); %disconnects Mimose object and makes it
            % available for other applications
            obj.disconnect();
        end
    end

    methods(Hidden)
        function [ values ]  = get_registers(obj)
            %GET_REGISTERS get register values from SDK register memory
            [ec, registers] = DeviceControlM_Mimose('get_registers', obj.device_handle);
            obj.check_error_code(ec);
            values = bitand(registers,hex2dec('ffff'));
            obj.registers.set_register_values(values);
        end
    end

    methods(Static, Hidden)
        function config_out = convert_configs_mimose_for_sdk(config_in)
            config_out = config_in;
            % adapt PulseConfig
            for idx = 1:4
                config_out.PulseConfig{idx}.channel = int32(config_in.PulseConfig{idx}.channel);
                config_out.PulseConfig{idx}.abb_gain_type = int32(config_in.PulseConfig{idx}.abb_gain_type);
                config_out.PulseConfig{idx}.aoc_mode = int32(config_in.PulseConfig{idx}.aoc_mode);
            end

            config_out.AFC_Config.band = int32(config_in.AFC_Config.band);
            config_out.AFC_Config.afc_repeat_count = int32(config_in.AFC_Config.afc_repeat_count);
        end

        function config_out = convert_configs_mimose_from_sdk(config_in)
            config_out = config_in;
            % adapt PulseConfig
            for idx = 1:4
                config_out.PulseConfig{idx}.channel = MimoseConfigOptions.ifx_Mimose_Channel_t(config_in.PulseConfig{idx}.channel);
                config_out.PulseConfig{idx}.abb_gain_type = MimoseConfigOptions.ifx_Mimose_ABB_type_t(config_in.PulseConfig{idx}.abb_gain_type);
                config_out.PulseConfig{idx}.aoc_mode = MimoseConfigOptions.ifx_Mimose_AOC_Mode_t(config_in.PulseConfig{idx}.aoc_mode);
            end

            config_out.AFC_Config.band = MimoseConfigOptions.ifx_Mimose_RF_Band_t(config_in.AFC_Config.band);
            config_out.AFC_Config.afc_repeat_count = MimoseConfigOptions.ifx_Mimose_AFC_Repeat_Count_t(config_in.AFC_Config.afc_repeat_count);
        end

        function pulse_configs = get_active_pulse_configs(frame_config)
            for i = 1:2
                pulse_configs(1) = frame_config{i}.selected_pulse_config_0 + ...
                                   frame_config{i}.selected_pulse_config_1 + ...
                                   frame_config{i}.selected_pulse_config_2 + ...
                                   frame_config{i}.selected_pulse_config_3;
            end
        end

        function ret = check_error_code(err_code)
            ret = (err_code ~= 0);
            if(ret)
                description = DeviceControlM_Mimose(':describe_error', err_code);
                ME = MException(['RadarDevice:error' num2str(err_code)], description);
                throw(ME);
            end
        end
    end
    
    methods(Static)

        function version = get_version_full()
            % Returns the full SDK version string including
            % the git tag from which this release was built
            %
            % USAGE
            % disp(['Radar SDK Version: ' MimoseDevice.get_version_full()]);
            [~, version] = DeviceControlM_Mimose('get_version_full');
        end

        function version = get_version()
            % Returns the short SDK version string (excluding
            % the git tag from which this release was built)
            %
            %USAGE
            % disp(['Radar SDK Version: ' MimoseDevice.get_version()]);
            [~, version] = DeviceControlM_Mimose('get_version');
        end
    end

end

