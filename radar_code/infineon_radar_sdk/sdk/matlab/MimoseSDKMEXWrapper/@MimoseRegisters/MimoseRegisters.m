% ===========================================================================
% Copyright (C) 2021-2023 Infineon Technologies AG
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
 
classdef MimoseRegisters  < handle
    %MIMOSEREGISTERS register value storage an display utility class
    %   Detailed explanation goes here

    properties
        reg_names
        reg_address
        reg_values
    end

    methods
        function obj = MimoseRegisters()
            %MIMOSEREGISTERS Construct an instance of this class
            %   Detailed explanation goes here
            reginfo = obj.readjson('atr22_regmap.json');
            reginfo_formatted = struct2cell(reginfo.units.registers);
            obj.reg_names = string(reginfo_formatted(1,:))';
            obj.reg_address = cell2mat(reginfo_formatted(3,:))';
            obj.reg_values = NaN(size(obj.reg_address));
        end

        function view_registers(obj, varargin)
            p = inputParser;
            addParameter(p, 'to_file', 0); %  to file
            addParameter(p, 'hex', 1); % hexadecimal mode
            addParameter(p, 'filename', 'mimose_regs.txt'); % default filename

            parse(p,varargin{:});
            params = p.Results;

            if(params.to_file)
                fid = fopen(params.filename,'w');
                if(params.hex)
                    for i = 1:length(obj.reg_address)
                        if(isnan(obj.reg_values(i)))
                            fprintf(fid, '%s 0x%s NaN\n', obj.reg_names(i), ...
                                dec2hex(obj.reg_address(i),4));
                        else
                            fprintf(fid, '%s 0x%s 0x%s\n', obj.reg_names(i), ...
                                dec2hex(obj.reg_address(i),4), ...
                                dec2hex(obj.reg_values(i),4));
                        end
                    end
                else
                    for i = 1:length(obj.reg_address)
                        fprintf(fid, '%s %d %d\n', obj.reg_names(i), ...
                            obj.reg_address(i), ...
                            obj.reg_values(i));
                    end
                end
                fclose(fid);
            else
                if(params.hex)
                    for i = 1:length(obj.reg_address)
                        if(isnan(obj.reg_values(i)))
                            fprintf('%s, 0x%s, NaN\n', obj.reg_names(i), ...
                                dec2hex(obj.reg_address(i),4));
                        else
                            fprintf('%s, 0x%s, 0x%s\n', obj.reg_names(i), ...
                                dec2hex(obj.reg_address(i),4), ...
                                dec2hex(obj.reg_values(i),4));
                        end
                    end
                else
                    for i = 1:length(obj.reg_address)
                        fprintf('%s, %d, %d\n', obj.reg_names(i), ...
                            obj.reg_address(i), ...
                            obj.reg_values(i));
                    end
                end
            end
        end

        function address_numeric = get_address(obj, address)
            if(isstring(address)||ischar(address))
                regname = address;
            else
                address_numeric = uint32(address);
                return;
            end
            index = find(obj.reg_names==regname);
            if(isempty(index))
                address_numeric = NaN;
            else
                address_numeric = uint32(obj.reg_address(index));
            end
        end

        function obj = set_register_values(obj, values)
            obj.reg_values = values;
        end
    end

    methods(Static, Hidden)
        function [decoded_json] = readjson(filename)
            %READJSON Reads json file
            %   Detailed explanation goes here
            decoded_json = [];
            fidjson = fopen(filename,'rt');
            if(fidjson<0)
                warning("Matching configuration file not found!");
                return;
            end
            jsondata = fread(fidjson);
            fclose(fidjson);
            decoded_json = jsondecode(char(jsondata'));
        end
    end
end


