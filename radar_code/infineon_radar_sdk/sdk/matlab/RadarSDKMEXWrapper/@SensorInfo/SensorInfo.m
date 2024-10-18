% ===========================================================================
% Copyright (C) 2022 Infineon Technologies AG
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

classdef SensorInfo
    % SENSORINFO Class definition for configuring the sensor information on a device
    % This matlab class creates an object with all information about a sensor
    % similar to the information in the radar_sdk.

    properties
        % A null terminated string holding a human readable description of the sensor
        description,

        % The minimum RF frequency the sensor can emit
        min_rf_frequency_Hz,

        % The maximum RF frequency the sensor can emit
        max_rf_frequency_Hz,

        % The number of RF antennas used for transmission
        num_tx_antennas,

        % The number of RF antennas used for reception
        num_rx_antennas,

        % The amount of RF transmission power can be controlled in the range of 0 ... max_tx_power
        max_tx_power,

        % The number of available temperature sensors
        num_temp_sensors,

        % If this is 0, the radar data of multiple RX antennas is
        % stored in consecutive data blocks, where each block holds
        % data of one antenna. If this is non-zero, the radar data of
        % multiple RX antennas is stored in one data block, where for
        % each point in time the samples from all RX antennas are
        % stored consecutively before the data of the next point in time follows
        interleaved_rx,

        % sensor specific device id
        device_id,
    end

    methods
        function obj = SensorInfo()
            % SENSORINFO Construct an instance of this class
            %   The function instantiates a Sensor Info object
            obj.description = 0;
            obj.min_rf_frequency_Hz = 0;
            obj.max_rf_frequency_Hz = 0;
            obj.num_tx_antennas = 0;
            obj.num_rx_antennas = 0;
            obj.max_tx_power = 0;
            obj.num_temp_sensors = 0;
            obj.interleaved_rx = 0;
        end
    end
end

