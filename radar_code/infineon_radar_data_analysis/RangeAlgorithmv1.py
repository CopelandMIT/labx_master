# ===========================================================================
# Copyright (C) 2022 Infineon Technologies AG
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
# ===========================================================================

import numpy as np
from scipy import signal, constants
from FFT_spectrum import *


class RangeAlgo:
    """Compute Range map"""

    def __init__(self, num_samples: int, num_chirps_per_frame: int, start_frequency_Hz, end_frequency_Hz, mti_on=False, mti_alpha: float = 0.8):
        """Create Range-Doppler map object

        Parameters:
            - num_samples:          Number of samples in a single chirp
            - num_chirps_per_frame: Number of chirp repetitions within a measurement frame
            - num_ant:              Number of antennas
            - mti_alpha:            Parameter alpha of Moving Target Indicator
        """
        self.num_chirps_per_frame = num_chirps_per_frame
        self.mti_on = mti_on

        # compute Blackman-Harris Window matrix over chirp samples(range)
        self.range_window = signal.windows.blackmanharris(num_samples).reshape(1, num_samples)
        
        bandwidth_hz = abs(end_frequency_Hz - start_frequency_Hz)
        fft_size = num_samples * 2
        self.range_bin_length = constants.c / (2 * bandwidth_hz * fft_size / num_samples)

        if mti_on:
            # parameter for moving target indicator (MTI)
            self.mti_alpha = mti_alpha

            # initialize MTI filter
            self.mti_history = np.zeros((self.num_chirps_per_frame, num_samples))
        
        

    def compute_range_map(self, data: np.ndarray, min_bin_index: int, max_bin_index: int):
        """Compute Range-Doppler map for i-th antennas

        Parameter:
            - data:     Raw-data for one antenna (dimension:
                        num_chirps_per_frame x num_samples)
            - i_ant:    RX antenna index
        """
        # Step 1 - Remove average from signal (mean removal)
        data = data - np.average(data)

        if self.mti_on:
            # Step 2 - MTI processing to remove static objects
            data = data - self.mti_history[:, :]
            self.mti_history[:, :] = data * self.mti_alpha + self.mti_history[:, :] * (1 - self.mti_alpha)

        # Step 3 - calculate fft spectrum for the frame
        range_fft = FFT_spectrum(data, self.range_window)

        # Step 2 - convert to absolute spectrum
        range_fft_abs = abs(range_fft)

        # Step 3 - coherent integration of all chirps
        distance_data = np.divide(range_fft_abs.sum(axis=0), self.num_chirps_per_frame)

        # Step 4 - peak search and distance calculation
        # skip = 8
        distance_peak_idx = min_bin_index + np.argmax(distance_data[min_bin_index:max_bin_index])

        # distance_peak_m = self.range_bin_length * (distance_peak + skip)
        distance_peak_m = self.range_bin_length * (distance_peak_idx + min_bin_index)

        return range_fft, distance_data, distance_peak_idx, distance_peak_m
    

