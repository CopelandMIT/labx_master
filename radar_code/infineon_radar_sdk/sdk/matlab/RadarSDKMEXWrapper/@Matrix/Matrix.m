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

classdef Matrix < handle
    %MATRIX object that stores data from one antenna of a RadarDevice
    %   Consists of all samples and all chirps of a frame from one antenna

    properties
        chirps     % Number of chirps
        samples    % Number of samples per chirp
        mData      % 2D matrix with row as chirps and cols as samples
    end

    methods
        function obj = Matrix(rows,cols)
            %MATRIX Construct an instance of Matrix
            %   creates the matrox of required size
            obj.chirps = rows;
            obj.samples = cols;
            obj.mData = zeros(rows,cols);
        end

        function obj = set_data(obj,aData)
            %SET_DATA Takes in a stream of data from one antenna
            %   And allocates into the mData matrix of the Matrix object
            for r = 1:obj.chirps
                obj.mData(r,:) = aData(((r-1)*obj.samples)+1:r*obj.samples);
            end
        end

        function val = at(obj,row,col)
            %AT returns the value of the specified sample (col) of the
            %   specified chirp (row)
            val = obj.mData(row,col);
        end

        function aData = get_chirp_data(obj,row)
            %GET_CHIRP_DATA returns all samples (cols) of the specified chirp (row)
            aData = obj.mData(row,:);
        end

    end
end

