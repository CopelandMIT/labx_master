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

try 
    disp(['Radar SDK Version: ' MimoseDevice.get_version_full()]);
catch
    ME = MException(['RadarDevice:error'], 'SDK library not found');
    throw(ME);
end

%##########################################################################
% STEP 1: Create a Mimose device object and connect to an attached device
%##########################################################################
Dev = MimoseDevice();


%##########################################################################
% STEP 2: Modify configuration parameters.
%##########################################################################

% Configuration parameters can be modified here before sending them to the
% device
Dev.config.FrameConfig{1}.num_samples = 128;
Dev.config.FrameConfig{1}.selected_pulse_config_1 = true;
%Dev.config.ClockConfig.system_clock_Hz = 20000000;
%Dev.config.ClockConfig.rc_clock_enabled = 1;
%Dev.config.FrameConfig{1}.selected_pulse_config_3 = true;

%  OR

%newConfig = Dev.get_config_defaults();
%newConfig.FrameConfig{1}.selected_pulse_config_1 = true;

%##########################################################################
% STEP 3:  Configure the Radar device using the DeviceConfig object updated
% in the last STEP.
%##########################################################################
Dev.set_config(); %sends the device config to the device

% OR

%Dev.set_config(newConfig);

%##########################################################################
% STEP 4:  If RC clock is enabled and the returned system clock deviates
% more than the desired clock then the RC look up table can be tuned
%##########################################################################
%Dev.update_rc_lut();

%##########################################################################
% STEP 5: Start fetching and displaying a subset of raw data as example.
%##########################################################################
hfig = figure(1);
fcount = 1;
% In this example, only display datasubset from 10 frames.
while ishandle(hfig)
    % Fetch next frame data from the RadarDevice
    last_frame = Dev.get_next_frame();
    if (isempty(last_frame))
        continue;
    end
    % Do some processing with the obtained frame.
    % In this example we display a subset of the samples averaged over
    % chirps.
    num_pulses = size(last_frame, 1);
    num_samples = size(last_frame, 2);
    x_axis = 1:num_samples;
    disp(['Got frame ' num2str(fcount) ', num pulse configs=' num2str(num_pulses)]);

    for pulse_num = 1:size(last_frame,1)
        mat = last_frame(pulse_num, :);
        subplot(size(last_frame,1), 1, pulse_num);
        plot(real(mat));
        axis([1 num_samples 0 1]);
        hold on;
        plot(imag(mat),'r');
        axis([1 num_samples 0 1]);
        hold off;
    end
    fcount = fcount + 1;
    drawnow;
end

%##########################################################################
% STEP 6: Stop the Radar data acquisition trigerred in the last step by the
% function get_next_frame(). Now the device can be reconfigured and
% re-triggered by get_next_frame()
%##########################################################################
Dev.stop_acquisition();

%##########################################################################
% STEP 7: Clear the RadarDevice object. It also automatically disconnects
% from the device.
%##########################################################################
%Dev.set_register('CLK_CONF',68);
clear Dev

