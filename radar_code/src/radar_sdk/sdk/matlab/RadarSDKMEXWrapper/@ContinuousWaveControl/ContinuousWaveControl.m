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

classdef ContinuousWaveControl < handle
    %CONTINUOUSWAVECONTROL Class for a controller of a continuous wave.
    %   An object of this class helps to
    %   - configure a continuous wave emitter on a connected device in continuous wave mode
    %   - acquire data received through RX antennas

    properties (Access = private)
        dev_handle;         % device handle given by radar sdk
    end

    properties (Access = public)
        cw_handle;           % continuous wave control handle given by radar sdk
    end

    methods
        function obj = ContinuousWaveControl(device_handle)
            %CONTINUOUSWAVECONTROL Constructor for an instance of ContinuousWaveControl
            [ec, obj.cw_handle] = DeviceControlM('cw_create', device_handle);
            obj.dev_handle = device_handle;
            RadarDevice.check_error_code(ec);
        end

        function start_signal(obj)
            %START_EMMISION Enables continuous wave signal
            ec = DeviceControlM('cw_start_signal', obj.cw_handle);
            RadarDevice.check_error_code(ec);
        end

        function stop_signal(obj)
            %STOP_EMMISION Disables continuous wave signal
            ec = DeviceControlM('cw_stop_signal', obj.cw_handle);
            RadarDevice.check_error_code(ec);
        end

        function set_frequency(obj, frequency)
            %SET_FREQUENCY Sets the frequenct of the continuous wave in Hz
            ec = DeviceControlM('cw_set_rf_frequency_Hz', obj.cw_handle, frequency);
            RadarDevice.check_error_code(ec);
        end

        function frequency = get_frequency(obj)
            %GET_FREQUENCY Gets the currently set device frequency in Hz
            [ec, frequency] = DeviceControlM('cw_get_rf_frequency_Hz', obj.cw_handle);
            RadarDevice.check_error_code(ec);
        end

        function set_tx_dac_value(obj, dac_value)
            %SET_TX_DAC_VALUE Sets the DAC value that controls the power of the continuous wave
            ec = DeviceControlM('cw_set_tx_dac_value', obj.cw_handle, dac_value);
            RadarDevice.check_error_code(ec);
        end

        function dac_value = get_tx_dac_value(obj)
            %GET_TX_DAC_VALUE Gets the currently set TX power for CW mode within device
            [ec, dac_value] = DeviceControlM('cw_get_tx_dac_value', obj.cw_handle);
            RadarDevice.check_error_code(ec);
        end

        function enable_tx_antenna(obj, antenna, enable)
            %ENABLE_TX_ANTENNA Enables or disables the specified TX antenna
            ec = DeviceControlM('cw_enable_tx_antenna', obj.cw_handle, antenna, enable);
            RadarDevice.check_error_code(ec);
        end

        function enabled = is_tx_antenna_enabled(obj, antenna)
            %IS_TX_ANTENNA_ENABLED Checks if the specified TX antenna is enabled
            [ec, enabled] = DeviceControlM('cw_tx_antenna_enabled', obj.cw_handle, antenna);
            RadarDevice.check_error_code(ec);
        end

        function enable_rx_antenna(obj, antenna, enable)
            %ENABLE_RX_ANTENNA Enables or disables the specified RX antenna
            ec = DeviceControlM('cw_enable_rx_antenna', obj.cw_handle, antenna, enable);
            RadarDevice.check_error_code(ec);
        end

        function enabled = is_rx_antenna_enabled(obj, antenna)
            %IS_RX_ANTENNA_ENABLED Checks if the specified RX antenna is enabled
            [ec, enabled] = DeviceControlM('cw_rx_antenna_enabled', obj.cw_handle, antenna);
            RadarDevice.check_error_code(ec);
        end

        function set_num_of_samples_per_antenna(obj, num_samples)
            %SET_NUM_OF_SAMPLES_PER_ANTENNA Sets the number of samples per Rx antenna to be captured in single shot
            ec = DeviceControlM('cw_set_num_of_samples_per_antenna', obj.cw_handle, num_samples);
            RadarDevice.check_error_code(ec);
        end

        function num_samples = get_num_of_samples_per_antenna(obj)
            %GET_NUM_OF_SAMPLES_PER_ANTENNA Provides samples count acquired per RX antenna
            [ec, num_samples] = DeviceControlM('cw_get_num_of_samples_per_antenna', obj.cw_handle);
            RadarDevice.check_error_code(ec);
        end

        function set_baseband_params(obj, oBasebandConfig)
            %SET_BASEBAND_PARAMS Configures the Avian device baseband
            ec = DeviceControlM('cw_set_baseband_params', obj.cw_handle, oBasebandConfig);
            RadarDevice.check_error_code(ec);
        end

        function baseband_config = get_baseband_config(obj)
            oBasebandConfig = BasebandConfig.mkBasebandConf();
            %GET_BASEBAND_PARAMS Provides the current baseband configuration of Avian device
            [ec, baseband_config] = DeviceControlM('cw_get_baseband_params', obj.cw_handle, oBasebandConfig);
            RadarDevice.check_error_code(ec);
        end

        function set_adc_config(obj, oAdcConfig)
            %SET_ADC_PARAMS Configures the Avian device ADC parameters
            ec = DeviceControlM('cw_set_adc_params', obj.cw_handle, oAdcConfig);
            RadarDevice.check_error_code(ec);
        end

        function adc_config = get_adc_config(obj)
            %GET_ADC_PARAMS Provides the current ADC configurations of Avian device
            oAdcConfig = AdcConfig.mkAdcConf();
            [ec, adc_config] = DeviceControlM('cw_get_adc_params', obj.cw_handle, oAdcConfig);
            RadarDevice.check_error_code(ec);
        end

        function set_test_signal_generator_config(obj, oTestSigGenConfig)
            %SET_TEST_SIGNAL_GENERATOR_CONFIG Configures the Avian device test signal generator instead of actually TX transmitted signal
            ec = DeviceControlM('cw_set_test_signal_generator_config', obj.cw_handle, oTestSigGenConfig);
            RadarDevice.check_error_code(ec);
        end

        function signal_generator_config = get_test_signal_generator_config(obj)
            %GET_TEST_SIGNAL_GENERATOR_CONFIG Provides the current test signal configurations of Avian device
            oTestSigGenConfig = TestSignalGeneratorConfig.mkTestSignalGeneratorConf();
            [ec, signal_generator_config] = DeviceControlM('cw_get_test_signal_generator_config', obj.cw_handle, oTestSigGenConfig);
            RadarDevice.check_error_code(ec);
        end

        function temperature = measure_temperature(obj)
            %MEASURE_TEMPERATURE Measures and returns the current temperature of the Avian device
            [ec, temperature] = DeviceControlM('cw_measure_temperature', obj.cw_handle);
            RadarDevice.check_error_code(ec);
        end

        function tx_power = measure_tx_power(obj, tx_antenna)
            %MEASURE_TX_POWER Measures and returns the power that is emitted from the specified TX antenna
            [ec, tx_power] = DeviceControlM('cw_measure_tx_power', obj.cw_handle, tx_antenna);
            RadarDevice.check_error_code(ec);
        end

        function RxFrame = capture_frame(obj)
            %CAPTURE_FRAME Captures and returns one frame of raw data for CW mode

            Total_samples = [];
            RxFrame = [];
            try
                [err_code, num_rx, num_samples, received_samples] = DeviceControlM('cw_capture_frame', obj.cw_handle);
                RadarDevice.check_error_code(err_code);
                RxFrame = reshape(received_samples, num_samples, num_rx);
            catch ME
                rethrow(ME);
            end
        end

        function delete(obj)
            %CONTINUOUSWAVECONTROL Destuctor for an instance of ContinuousWaveControl
            ec = DeviceControlM('cw_destroy', obj.cw_handle);
            RadarDevice.check_error_code(ec);
            obj.cw_handle = 0;
        end
    end
end
