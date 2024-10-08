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

classdef DeviceConfig
    %DEVICECONFIG Class definition for configuring a RadarDevice
    % This matlab class creates an object with all parameters to configure the radar device,
    % similar to the device configuration of the radar_sdk. The DeviceConfig object is required
    % as an input to instantiate a RadarDevice object. This class also provides a helper
    % function mkDevConf() to create a DeviceConfig object and accepts param-value pairs in
    % any order. It assumes default values for any parameter that is not specified.

    properties
        % Sampling rate of the ADC used to acquire the samples during a chirp.
        % The duration of a single chirp depends on the number of samples and
        % the sampling rate.
        sample_rate_Hz,

        % Bitmask where each bit represents one RX antenna of the radar device.
        % If a bit is set the according RX antenna is enabled during the chirps
        % and the signal received through that antenna is captured. The least
        % significant bit corresponds to antenna 1.
        rx_mask,

        % Bitmask where each bit represents one TX antenna.
        tx_mask,

        % This value controls the power of the transmitted RX signal. This is
        % an abstract value between 0 and 31 without any physical meaning.
        tx_power_level,

        % Amplification factor that is applied to the IF signal coming from the
        % RF mixer before it is fed into the ADC.
        if_gain_dB,

        % Start frequency of the FMCW chirp.
        start_frequency_Hz,

        % Stop frequency of the FMCW chirp.
        end_frequency_Hz,

        % This is the number of samples acquired during each chirp of a frame.
        % The duration of a single chirp depends on the number of samples and
        % the sampling rate.
        num_samples_per_chirp,

        % This is the number of chirps a single data frame consists of.
        num_chirps_per_frame,

        % This is the time period that elapses between the beginnings of two
        % consecutive chirps in a frame.  (Also commonly referred to as pulse
        % repetition time or chirp-to-chirp-time.)
        chirp_repetition_time_s,

        % This is the time period that elapses between the beginnings of two
        % consecutive frames. The reciprocal of this parameter is the frame
        % rate. (Also commonly referred to as frame time or frame period.)
        frame_repetition_time_s,

        % Cutoff frequency of the high pass filter (in Hz). The high pass
        % filter is used in order to remove the DC-offset at the output of the
        % RX mixer and also suppress the reflected signal from close in
        % unwanted targets (radome, e.g.).
        %
        % Note: Different sensors support different cutoff frequencies. The
        % frequency provided by the user will be rounded to the closest
        % supported cutoff frequency.  You can read the cutoff frequency that
        % was actually set using \ref ifx_avian_get_config.
        hp_cutoff_Hz,

        % Cutoff frequency of the anti aliasing filter (in Hz)
        aaf_cutoff_Hz,

        % Mode of MIMO: 0 for off, 1 for time-domain-multiplexed MIMO. If MIMO
        % mode is enabled the value for tx_mask will be ignored.
        mimo_mode,
    end

    methods
        function obj = DeviceConfig(sample_rate_Hz, ...
                                    rx_mask, ...
                                    tx_mask, ...
                                    tx_power_level, ...
                                    if_gain_dB, ...
                                    start_frequency_Hz, ...
                                    end_frequency_Hz, ...
                                    num_samples_per_chirp, ...
                                    num_chirps_per_frame, ...
                                    chirp_repetition_time_s, ...
                                    frame_repetition_time_s, ...
                                    hp_cutoff_Hz, ...
                                    aaf_cutoff_Hz, ...
                                    mimo_mode)
            %DEVICECONFIG Construct an instance of this class
            %   The function instantiates a DeviceConfig object
            obj.sample_rate_Hz = sample_rate_Hz;
            obj.rx_mask = rx_mask;
            obj.tx_mask = tx_mask;
            obj.tx_power_level = tx_power_level;
            obj.if_gain_dB = if_gain_dB;
            obj.start_frequency_Hz = start_frequency_Hz;
            obj.end_frequency_Hz = end_frequency_Hz;
            obj.num_samples_per_chirp = num_samples_per_chirp;
            obj.num_chirps_per_frame = num_chirps_per_frame;
            obj.chirp_repetition_time_s = chirp_repetition_time_s;
            obj.frame_repetition_time_s = frame_repetition_time_s;
            obj.hp_cutoff_Hz = hp_cutoff_Hz;
            obj.aaf_cutoff_Hz = aaf_cutoff_Hz;
            obj.mimo_mode = mimo_mode;
        end
    end

    methods(Static)
        function obj = mkDevConf(varargin)
            %MKDEVCONF Helper function for creating a DeviceConfig object
            %   This function accepts the config params as param - value pairs
            %   in any order, and uses default values for any unspecified
            %   param, and returns a DeviceConfig object.
            %   accepted parameters and their default values are as follows:
            %   param: sample_rate_Hz               default val = 1e6; % 1MHz
            %   param: rx_mask                      default val = 7; % RX1 and RX2
            %   param: tx_mask                      default val = 1; % TX1
            %   param: tx_power_level               default val = 31;
            %   param: if_gain_dB                   default val = 33; % 33dB
            %   param: start_frequency_kHz          default val = 60.5e9; % 60.5GHz
            %   param: end_frequency_kHz            default val = 61.5e9; % 61.5GHz
            %   param: num_samples_per_chirp        default val = 512;
            %   param: num_chirps_per_frame         default val = 32;
            %   param: chirp_repetition_time_s      default val = 0.00053861; % 538.61e-6 also known as PRT
            %   param: frame_repetition_time_s      default val = 1;
            %   param: mimo_mode                    default val = 0; % 1 for time-domain-multiplexing

            p = inputParser;
            addParameter(p, 'sample_rate_Hz', 1000000);
            addParameter(p, 'rx_mask', 7);
            addParameter(p, 'tx_mask', 1);
            addParameter(p, 'tx_power_level', 31);
            addParameter(p, 'if_gain_dB', 33);
            addParameter(p, 'start_frequency_Hz', 60500000000);
            addParameter(p, 'end_frequency_Hz', 61500000000);
            addParameter(p, 'num_samples_per_chirp', 512);
            addParameter(p, 'num_chirps_per_frame', 32);
            addParameter(p, 'chirp_repetition_time_s', 0.00053861);
            addParameter(p, 'frame_repetition_time_s', 1);
            addParameter(p, 'hp_cutoff_Hz', 80000);
            addParameter(p, 'aaf_cutoff_Hz', 500000);
            addParameter(p, 'mimo_mode', 0);

            parse(p,varargin{:});
            params = p.Results;

            obj = DeviceConfig(params.sample_rate_Hz, ...
                               params.rx_mask, ...
                               params.tx_mask, ...
                               params.tx_power_level, ...
                               params.if_gain_dB, ...
                               params.start_frequency_Hz, ...
                               params.end_frequency_Hz, ...
                               params.num_samples_per_chirp, ...
                               params.num_chirps_per_frame, ...
                               params.chirp_repetition_time_s, ...
                               params.frame_repetition_time_s, ...
                               params.hp_cutoff_Hz, ...
                               params.aaf_cutoff_Hz, ...
                               params.mimo_mode);
        end
    end
end

