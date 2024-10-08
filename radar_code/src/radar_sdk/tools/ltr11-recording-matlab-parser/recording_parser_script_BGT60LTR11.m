%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Script RECORDING_PARSER_SCRIPT_BGT60LTR11.m
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Copyright (c) 2022, Infineon Technologies AG
% All rights reserved.
%
% Redistribution and use in source and binary forms, with or without modification,are permitted provided that the
% following conditions are met:
%
% Redistributions of source code must retain the above copyright notice, this list of conditions and the following
% disclaimer.
%
% Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following
% disclaimer in the documentation and/or other materials provided with the distribution.
%
% Neither the name of the copyright holders nor the names of its contributors may be used to endorse or promote
% products derived from this software without specific prior written permission.
%
% THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
% INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
% DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE  FOR ANY DIRECT, INDIRECT, INCIDENTAL,
% SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
% SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
% WHETHER IN CONTRACT, STRICT LIABILITY,OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
% OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% DESCRIPTION:
%
% This example Parses recording data from a LTR11 Radar, shows the Doppler processing for the collected raw
% data, computes the velocity of a detected target and presents the results as a plot.
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% NOTES:
%
% For Doppler modulation there is no FMCWEndpoint in the XML file, the
% range FFT has to be omitted and the Doppler FFT has to be directly computed.
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%% Startup
clc;
clear;
close all;
addpath('src');

%% Constants & Parameters
c0 = 3e8;                   % Speed of light in vacuum [m/s]
zero_padding_factor = 4;

%% Parse Raw Data File
% create file name from user selection
[ fname, fpath] = uigetfile('*.raw.bin');
fdata = [fpath, fname(1:end-8)];

% parse data
[frame, Header, frame_count, fCKHz] = ltr11_recording_parser(fdata); % Data Parser

%% Load Raw Data & Radar Settings

% Frame duration
frame_period = Header.Frame_period_sec;

num_Tx_antennas = Header.Num_Tx_Antennas; % Number of Tx antennas
num_Rx_antennas = Header.Num_Rx_Antennas; % Number of Rx antennas

% Number of ADC samples per chirp
NTS = Header.Samples_per_Chirp;

% Number of chirps per frame
PN = Header.Chirps_per_Frame;

% Sampling frequency in Hz
fS = Header.Sampling_Frequency_kHz * 1e3;

% RF frequency in Hz
fC = fCKHz*1e3;

% Doppler threshold
doppler_threshold = 200;

%% Calculate Derived Parameters
lambda = c0 / fC;                               % wavelength

doppler_fft_size = NTS * zero_padding_factor;
doppler_window_func = chebwin(NTS,60);          % Window function for Doppler

doppler_Hz_per_bin =  fS / doppler_fft_size;    
Hz_to_mps_constant = lambda / 2;                % Conversion factor from frequency to speed in m/s
IF_scale = 16 * 3.3;                            % Scaling factor for signal strength

fD_max = fS / 2;                                % Maximum theoretical value of the Doppler frequency
fD_per_bin = fD_max / (doppler_fft_size/2);     % Doppler bin size in Hz
array_bin_fD = ((1:doppler_fft_size) - doppler_fft_size/2 - 1) * -fD_per_bin * Hz_to_mps_constant; % Vector of speed in m/s

%% Initialize Debug Structures
doppler_data_complete             = zeros(doppler_fft_size, frame_count);
doppler_level_complete            = zeros(1, frame_count);
doppler_frequency_complete        = zeros(1, frame_count);
doppler_velocity_complete         = zeros(1, frame_count);
doppler_motion_direction_complete = zeros(1, frame_count);

%% Process Frames
for fr_idx = 1:frame_count                      % Loop over all data frames, while the output window is still open
    raw_data = frame(fr_idx).Chirp;             % Raw data for the frame being processed
    
    raw_data = raw_data * IF_scale;
    raw_data = raw_data - mean(raw_data);
    
    raw_data_windowed = raw_data .* doppler_window_func;
    doppler_data = fftshift(fft(raw_data_windowed,doppler_fft_size)); % Windowing across range and range FFT
    
    doppler_spectrum = abs(doppler_data);
    doppler_spectrum = doppler_spectrum(:,1);
    doppler_spectrum(1) = 0;                    % Ignore maximum speed since the sign of velocity cannot be determined
    
    [doppler_level, max_idx] = max(doppler_spectrum);
    fprintf("[%d]: level: %f, %.0f,",fr_idx, doppler_level, max_idx);
    
    doppler_frequency = 0;
    doppler_speed = 0;
    doppler_velocity = 0;
    motion_dir = 0;
    
    if(doppler_level > doppler_threshold)
        doppler_frequency =  max_idx * doppler_Hz_per_bin;
        doppler_velocity  = array_bin_fD(max_idx);
        
        if (doppler_velocity < 0 )                % Direction of motion detection
            direction       = "departing";
            motion_dir      = 1;
        else
            direction       = "approaching";
            motion_dir      = -1;
        end
        fprintf(" %f Hz, %f m/s, %s", doppler_frequency, doppler_velocity, direction);
    end
    fprintf("\n");
    
    doppler_data_complete(:,fr_idx) = doppler_data;
    doppler_level_complete(fr_idx) = doppler_level;
    doppler_frequency_complete(fr_idx) = doppler_frequency;
    doppler_velocity_complete(fr_idx) = doppler_velocity;
    doppler_motion_direction_complete(fr_idx) = motion_dir;
end

%% Visualization
figure;

plot(1:frame_count,doppler_velocity_complete);
title('Target Velocity');
xlabel('Frames');
ylabel('Velocity (m/s)');

%% End of script
disp('Done!');
