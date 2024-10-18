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

classdef TestSignalGeneratorConfig
    % TESTSIGNALGENERATORCONFIG Class definition for configuring a Continuous Wave Control Test Signal Generator parameters
    % This matlab class creates an object with all parameters to configure the device,
    % similar to the configuration of the radar_sdk. The TestSignalGeneratorConfig object is required
    % as an input to set baseband paramsfor Continuous Wave Control. This class also provides a helper
    % function mkTestSignalGeneratorConf() to create a TestSignalGeneratorConfig object and accepts param-value pairs in
    % any order. It assumes default values for any parameter that is not specified.

    properties
        % Mode of the Avian device test signal generator
        mode,

        % frequency of the test signal in Hz
        frequency_Hz,
    end

    methods
        function obj = TestSignalGeneratorConfig(mode, ...
                                    frequency_Hz)
            % TESTSIGNALGENERATORCONFIG Construct an instance of this class
            %   The function instantiates a TestSignalGenerator Config object
            obj.mode = mode;
            obj.frequency_Hz = frequency_Hz;
        end
    end

    methods(Static)
        function obj = mkTestSignalGeneratorConf(varargin)
            % MKTESTSIGNALGENERATORCONF Helper function for creating a TestSignalGenerator Config object
            %   This function accepts the config params as param - value pairs
            %   in any order, and uses default values for any unspecified
            %   param, and returns a TestSignalGenerator Config object.
            %   accepted parameters and their default values are as follows:
            %   param: mode           default val = 0; % IFX_TEST_SIGNAL_MODE_OFF
            %   param: frequency_Hz   default val = 60e9; % 60GHz

            p = inputParser;
            addParameter(p, 'mode', DeviceConfigOptions.ifx_Avian_Test_Signal_Generator_Mode.IFX_TEST_SIGNAL_MODE_OFF);
            addParameter(p, 'frequency_Hz', 400000);

            parse(p,varargin{:});
            params = p.Results;

            obj = TestSignalGeneratorConfig(double(params.mode), ...
                               double(params.frequency_Hz));
        end
    end
end

