% ===========================================================================
% Copyright (C) 2021 Infineon Technologies AG
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

classdef DeviceMetrics
    %DEVICEMETRICS Class definition
    % This matlab class creates a metrics object.
    % This class provides a helper function mkDevMetrics() to create a DeviceMetrics object with default arguments
    % for unspecified parameters.
    % Such helper function assumes default values for any parameter that is not specified.

    properties
        % Sampling rate of the ADC used to acquire the samples
        % during a chirp. The duration of a single chirp depends
        % on the number of samples and the sampling rate.
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

        % The range resolution is the distance between two consecutive
        % bins of the range transform. Note that even though zero
        % padding before the range transform seems to increase this
        % resolution, the true resolution does not change but depends
        % only from the acquisition parameters. Zero padding is just
        % interpolation!
        range_resolution_m,

        % The bins of the range transform represent the range
        % between 0m and this value. (If the time domain input data it
        % is the range-max_range_m ... max_range_m.)
        max_range_m,

        % The bins of the Doppler transform represent the speed values
        % between -max_speed_m_s and max_speed_m_s.
        max_speed_m_s,

        % The speed resolution is the distance between two consecutive
        % bins of the Doppler transform. Note that even though zero
        % padding before the speed transform seems to increase this
        % resolution, the true resolution does not change but depends
        % only from the acquisition parameters. Zero padding is just
        % interpolation!
        speed_resolution_m_s,

        % The desired frame repetition time in seconds (also known
        % as frame time or frame period). The frame repetition time
        % is the inverse of the frame rate
        frame_repetition_time_s,

        % Center frequency of the FMCW chirp. If the value is set to 0
        % the center frequency will be determined from the device
        center_frequency_Hz,
    end

    methods
        function obj = DeviceMetrics(sample_rate_Hz, ...
                                     rx_mask, ...
                                     tx_mask, ...
                                     tx_power_level, ...
                                     if_gain_dB, ...
                                     range_resolution_m, ...
				     max_range_m, ...
				     max_speed_m_s, ...
				     speed_resolution_m_s, ...
				     frame_repetition_time_s, ...
				     center_frequency_Hz)
            obj.sample_rate_Hz = sample_rate_Hz;
            obj.rx_mask = rx_mask;
            obj.tx_mask = tx_mask;
            obj.tx_power_level = tx_power_level;
            obj.if_gain_dB = if_gain_dB;
            obj.range_resolution_m = range_resolution_m;
            obj.max_range_m = max_range_m;
            obj.max_speed_m_s = max_speed_m_s;
            obj.speed_resolution_m_s = speed_resolution_m_s;
            obj.frame_repetition_time_s = frame_repetition_time_s;
            obj.center_frequency_Hz = center_frequency_Hz;
        end
    end

    methods(Static)
        function obj = mkDevMetrics(varargin)
	    p = inputParser;
            addParameter(p, 'sample_rate_Hz', 1000000);
            addParameter(p, 'rx_mask', 7);
            addParameter(p, 'tx_mask', 1);
            addParameter(p, 'tx_power_level', 31);
            addParameter(p, 'if_gain_dB', 33);
            addParameter(p, 'range_resolution_m', 0.3);
            addParameter(p, 'max_range_m', 9.6);
            addParameter(p, 'max_speed_m_s', 2.45);
            addParameter(p, 'speed_resolution_m_s', 0.0765);
            addParameter(p, 'frame_repetition_time_s', 0.1);
            addParameter(p, 'center_frequency_Hz', 60500000000);

            parse(p,varargin{:});
            params = p.Results;

            obj = DeviceMetrics(params.sample_rate_Hz, ...
                                params.rx_mask, ...
                                params.tx_mask, ...
                                params.tx_power_level, ...
                                params.if_gain_dB, ...
                                params.range_resolution_m, ...
                                params.max_range_m, ...
                                params.max_speed_m_s, ...
                                params.speed_resolution_m_s, ...
                                params.frame_repetition_time_s, ...
                                params.center_frequency_Hz);
        end
    end
end

