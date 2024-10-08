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

classdef ShieldType

    properties
        value_as_uint,
    end

    methods
        function obj = ShieldType(uint_value)
            obj.value_as_uint = uint_value;
        end

        function setValue(obj, uint_value)
            obj.value_as_uint = uint_value;
        end

        function [out] = getValue(obj)
            if obj.value_as_uint == 0
                out = "MISSING";
            elseif obj.value_as_uint == 1
                out = "UNKNOWN";
            elseif obj.value_as_uint == 512
                out = "BGT60TR13AIP";
            elseif obj.value_as_uint == 513
                out = "BGT60ATR24AIP";
            elseif obj.value_as_uint == 514
                out = "BGT60UTR11AIP";
            elseif obj.value_as_uint == 515
                out = "BGT60UTR13D";
            elseif obj.value_as_uint == 768
                out = "BGT60LTR11";
            elseif obj.value_as_uint == 769
                out = "BGT60LTR11_MONOSTAT";
            elseif obj.value_as_uint == 770
                out = "BGT60LTR11_B11";
            elseif obj.value_as_uint == 1024
                out = "BGT24ATR22_ES";
            elseif obj.value_as_uint == 1025
                out = "BGT24ATR22_PROD";
            elseif obj.value_as_uint == 65535
                out = "DEVICE_ANY";
            else
                out = "UNRECOGNIZED";
            end
        end
    end

end
