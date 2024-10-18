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
addpath('../../../sdk/matlab/RadarSDKMEXWrapper');
% load package if this is not matlab
if((exist ("OCTAVE_VERSION", "builtin") > 0))
    pkg load communications
end

disp(['Radar SDK Version: ' RadarDevice.get_version_full()]);

%##########################################################################
% STEP 1: Create one DeviceConfig instance. Please refer to DeviceConfig.m
% for more details about TR13C or ATR24C (MIMO) configuration.
% Example configurations are also provided.
%##########################################################################

% Set device configuration for presence sensing
f_start = 59.0000e9; %FMCW chirp start frequency in Hz
f_end   = 61.9979e9; %FMCW chirp end frequency in Hz
samples_per_chirp  = 64;
chirps_per_frame   = 32;

oDevConf = DeviceConfig(1e6, ... % sample_rate_Hz
                       1, ... % rx_mask
                       1, ... % tx_mask
                       31, ... % tx_power_level
                       33, ... % if_gain_dB
                       f_start, ... % start_frequency_Hz
                       f_end, ... % end_frequency_Hz
                       samples_per_chirp, ... % num_samples_per_chirp
                       chirps_per_frame, ... % num_chirps_per_frame
                       0.000411238, ... % chirp_repetition_time_s
                       0.125, ... % frame_repetition_time_s
                       80000, ... % hp_cutoff_Hz
                       500000, ... % aaf_cutoff_Hz
                       0); % mimo_mode

%##########################################################################
% STEP 2: Create a RadarDevice object and connect to the Radar
%##########################################################################
Dev = RadarDevice();

%##########################################################################
% STEP 3: Configure the radar device with the required config values
%##########################################################################
Dev.set_config(oDevConf);

%##########################################################################
% STEP 4: Algorithm Configurations
%##########################################################################

zero_padding_factor = 2;

%% Range calculation
range_fft_size = (2^zero_padding_factor) * samples_per_chirp;     % zero padding factor of 2 is included
c0 = physconst('LightSpeed');                   % speed of light
BW = f_end - f_start;                           % configured bandwidth
R_max = samples_per_chirp * c0 / (2 * BW);  % maximum range
DistPerBin = R_max / range_fft_size;            % distance per frequency bin
range_len = range_fft_size / 2; % only half of the spectrum has information since the Radar data is real only
range_arr = (0:range_len-1) * DistPerBin;       % vector of distance points corresponding to frequency bins

%% Object Presence sensing algorithm parameters
range_min = 0.2;               		% minimum range of 20cm
range_max = 0.8;               		% maximum range if 80cm
threshold_presence = 0.0007;                        % threshold for object detection
alpha_slow = 0.001;                                 % slow average update coefficient
alpha_med = 0.05;                                   % medium average update coefficient
alpha_fast = 0.6;                                   % fast average update coefficient
presence_status = 0;

% Find range FFT bins for minimum and maximum ranges
temp = find(range_arr >= range_min);
detect_start_bin = temp(1);
temp = find(range_arr >= range_max);
detect_end_bin = temp(1);

%##########################################################################
% STEP 5: Fetch Radar Data
%##########################################################################

% Fetch frame data cube and reshape in the form of an antenna matrix (rows = chirps, columns = samples)
fdata = Dev.get_next_frame();
% Data matrix for the first configured receive antenna
mat = squeeze(fdata(1,:,:));

% Check shape of frame data for one antenna
if(~isequal(size(mat), [chirps_per_frame, samples_per_chirp]))
    clear Dev;
    error('Unexpected data size');
end

zero_padded = zeros(size(mat,1),size(mat,2)*2^(zero_padding_factor));
result = zeros(1,200);

hFig = figure('numbertitle', 'off');
% In this example, show frames till figure is closed.
frame_number = 0;

countdown = 0;
old_status = 0;
%##########################################################################
% STEP 6: Continue fetching Radar data and run desired algorithm on it.
%##########################################################################
while ishandle(hFig)
	%% Fetch data cube from the Radar and format it to an antenna matrix
    fdata = Dev.get_next_frame();
    if (isempty(fdata))
        continue;
    end
    % fetching frame data for the first configured antenna
    mat = squeeze(fdata(1,:,:));
    %% Run algorithm
    % compute window for applying to signal before FFT
    w = window(@blackmanharris,samples_per_chirp);

    % Mean removal and windowing of raw data
    avg_windowed = (mat - mean(mat,2)).*w';

    zero_padded(:,1:samples_per_chirp) = avg_windowed;

    % Range FFT
    range_fft = (fft(zero_padded'))/samples_per_chirp; % normalized FFT
    range_fft = 2*range_fft.'; % compensating for the negative half the spectrum
    % remove the redundant negative spectrum since FFT input was real
    range_fft = range_fft(:,1:(size(range_fft,2)/2));

    % Averaging the FFT values over number of chirps
    fft_avg = mean(range_fft);
    fft_abs = abs(fft_avg);

    if frame_number == 0  % Initialize average updates
        slow_avg = fft_abs;
        fast_avg = fft_abs;
    end

    % update the background slowly in case object is detected
    if ~presence_status
        alpha_used = alpha_med;
    else
        alpha_used = alpha_slow;
    end
    slow_avg = slow_avg*(1-alpha_used) + fft_abs*alpha_used;
    fast_avg = fast_avg*(1-alpha_fast) + fft_abs*alpha_fast;
    data = abs(fast_avg-slow_avg);

    % Detection
    detect_range_max = max(data(detect_start_bin:detect_end_bin));
    presence_status =  (detect_range_max > threshold_presence) && (detect_range_max >= max(data)-threshold_presence);

    if(presence_status == 1)
        countdown = 10;
    end

    if(old_status == 1 && presence_status==0)
        if(countdown>0)
            countdown = countdown-1;
        else
            old_status = 0;
        end
    end
    presence_output = 0;
    if(countdown > 0 )
        old_status = 1;
        presence_output = 1;
    else
        old_status = 0;
    end


    %% Update and display result
    result = [result(2:end) presence_output];
    plot(result, LineWidth=4);
    xlabel('Time',FontSize=20);
    ylabel('Detection Status', FontSize=20);
    set(gca,'xtick',[],'ytick',[]);
    set(gcf, 'name', 'Range window object sensing');
    axis([1 length(result) -0.2 1.2 ]);
    frame_number = frame_number+1;
    if(old_status)
        title(sprintf('Object detected'),FontSize=48);
    else
        title(sprintf('Absent'), FontSize=48);
    end
    drawnow
end

%##########################################################################
% STEP 6: Clear the RadarDevice object. It also stops Radar data and
% disconnects the device from the handle.
%##########################################################################
clear Dev
