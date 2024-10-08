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

clear all;

% load package if this is not matlab
if((exist ("OCTAVE_VERSION", "builtin") > 0))
    pkg load communications
end

% Point to the generated SDK dlls and mex file
addpath('../../../libs/win32_x64/');

disp(['Radar SDK Version: ' RadarDevice.get_version_full()]);

%##########################################################################
% STEP 1: Create and connect to RadarDevice object
%##########################################################################
Dev = RadarDevice();

%##########################################################################
% STEP 2: create one DeviceConfig instance.
% Please refer to DeviceConfig.m for more details about TR13C or
% ATR24C (MIMO) configuration. The following examples of configs are also
% provided.
%##########################################################################

% The following command creates a DeviceConfig object with all params as default values
%   oDevConf = DeviceConfig.mkDevConf();

% For time-domain-multiplexed MIMO: 'mimo_mode': 1. See also DeviceConfig.m.
%   oDevConf = DeviceConfig.mkDevConf('mimo_mode',1,'rx_mask',15);

% Following command creates a DeviceConfig object with frame reptition time of
% 0.5s (and thus a frame rate of 2Hz) and chirp repetition time of 0.5ms:
%    oDevConf = DeviceConfig.mkDevConf('frame_repetition_time_s',0.5, 'chirp_repetition_time_s',0.0005);

% The following command creates a DeviceConfig object directly from the
% constructor, without using the mkDevConf() helper function.
% NOTE !!! If  mkDevConf() is not used, all the config values
% need to be specified and in the same order as expected.
%
oDevConf = DeviceConfig(1e6, ... % sample_rate_Hz
                       1, ... % rx_mask
                       1, ... % tx_mask
                       31, ... % tx_power_level
                       33, ... % if_gain_dB
                       58e9, ... % start_frequency_Hz
                       62e9, ... % end_frequency_Hz
                       128, ... % num_samples_per_chirp
                       8, ... % num_chirps_per_frame
                       0.0005, ... % chirp_repetition_time_s
                       1, ... % frame_repetition_time_s
                       80000, ... % hp_cutoff_Hz
                       500000, ... % aaf_cutoff_Hz
                       0); % mimo_mode

%##########################################################################
% STEP 3:  Configure the Radar device using the DeviceConfig object created
% in the last STEP.
%##########################################################################

Dev.set_config(oDevConf);

%##########################################################################
% STEP 4: Start fetching and displaying a subset of raw data as example.
%##########################################################################

% In this example, only display datasubset from 10 frames.
for fcount = 1:10
    % Fetch next frame data from the RadarDevice
    last_frame = Dev.get_next_frame();
    if (isempty(last_frame))
        continue;
    end
    % Do some processing with the obtained frame.
    % In this example we display a subset of the samples averaged over
    % chirps.
    num_rx = size(last_frame, 1);
    disp(['Got frame ' num2str(fcount) ', num_antennas=' num2str(num_rx)]);

    for iAnt = 1:num_rx
        mat = squeeze(last_frame(iAnt, :, :));
        disp(['Antenna ' num2str(iAnt)]);
        subset = min(16, size(last_frame, 3)); % use 16 samples or less for display
        disp(mean(mat(:,1:subset),1)); % average subset samples over chirps
        disp('  .... ');
    end
end

%##########################################################################
% STEP 5: Stop the Radar data acquisition trigerred in the last step by the
% function get_next_frame(). Now the device can be reconfigured and
% re-triggered by get_next_frame()
%##########################################################################
Dev.stop_acquisition();

%##########################################################################
% STEP 6: Clear the RadarDevice object. It also automatically disconnects
% from the device.
%##########################################################################
clear Dev
disp('Device disconnected!')

