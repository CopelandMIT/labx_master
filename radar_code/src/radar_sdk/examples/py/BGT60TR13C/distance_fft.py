# ===========================================================================
# Copyright (C) 2021-2022 Infineon Technologies AG
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

import argparse
import numpy as np
import matplotlib.pyplot as plt

from ifxradarsdk import get_version_full
from ifxradarsdk.fmcw import DeviceFmcw
from ifxradarsdk.fmcw.types import FmcwSimpleSequenceConfig, FmcwMetrics
from helpers.DistanceAlgo import *


# -------------------------------------------------
# Presentation
# -------------------------------------------------
class Draw:
    # Draws plots for data - each antenna is in separated plot

    def __init__(self, max_range_m, num_ant, num_samples):
        # max_range_m:  maximum supported range
        # num_ant:      number of available antennas

        self._num_ant = num_ant
        self._pln = []

        plt.ion()

        self._fig, self._axs = plt.subplots(nrows=1, ncols=num_ant, figsize=((num_ant + 1) // 2, 2))
        self._fig.canvas.manager.set_window_title("Range FFT")
        self._fig.set_size_inches(17 / 3 * num_ant, 4)

        self._dist_points = np.linspace(
            0,
            max_range_m,
            num_samples)

        self._fig.canvas.mpl_connect('close_event', self.close)
        self._is_window_open = True

    def _draw_first_time(self, data_all_antennas):
        # Create common plots as well scale it in same way
        # data_all_antennas: array of raw data for each antenna
        minmin = min([np.min(data) for data in data_all_antennas])
        maxmax = max([np.max(data) for data in data_all_antennas])

        for i_ant in range(self._num_ant):
            # This is a workaround: If there is only one plot then self._axs is
            # an object of the blass type AxesSubplot. However, if multiple
            # axes is available (if more than one RX antenna is activated),
            # then self._axs is an numpy.ndarray of AxesSubplot.
            # The code above gets in both cases the axis.
            if type(self._axs) == np.ndarray:
                ax = self._axs[i_ant]
            else:
                ax = self._axs

            data = data_all_antennas[i_ant]
            pln, = ax.plot(self._dist_points, data)
            ax.set_ylim(minmin, 1.05 * maxmax)
            self._pln.append(pln)

            ax.set_xlabel("distance (m)")
            ax.set_ylabel("FFT magnitude")
            ax.set_title("Antenna #" + str(i_ant))
        self._fig.tight_layout()

    def _draw_next_time(self, data_all_antennas):
        # data_all_antennas: array of raw data for each antenna

        for i_ant in range(0, self._num_ant):
            data = data_all_antennas[i_ant]
            self._pln[i_ant].set_ydata(data)

    def draw(self, data_all_antennas):
        # Draw plots for all antennas
        # data_all_antennas: array of raw data for each antenna
        if self._is_window_open:
            if len(self._pln) == 0:  # handle the first run
                self._draw_first_time(data_all_antennas)
            else:
                self._draw_next_time(data_all_antennas)

            self._fig.canvas.draw_idle()
            self._fig.canvas.flush_events()

    def close(self, event=None):
        if self.is_open():
            self._is_window_open = False
            plt.close(self._fig)
            plt.close('all')  # Needed for Matplotlib ver: 3.4.0 and 3.4.1
            print('Application closed!')

    def is_open(self):
        return self._is_window_open


# -------------------------------------------------
# Helpers
# -------------------------------------------------
def parse_program_arguments(description, def_nframes, def_frate):
    # Parse all program attributes
    # description:   describes program
    # def_nframes:   default number of frames
    # def_frate:     default frame rate in Hz

    parser = argparse.ArgumentParser(description=description)

    parser.add_argument('-n', '--nframes', type=int,
                        default=def_nframes, help="number of frames, default " + str(def_nframes))
    parser.add_argument('-f', '--frate', type=int, default=def_frate,
                        help="frame rate in Hz, default " + str(def_frate))

    return parser.parse_args()


# -------------------------------------------------
# Main logic
# -------------------------------------------------
if __name__ == '__main__':
    args = parse_program_arguments(
        '''Displays distance plot from Radar Data''',
        def_nframes=50,
        def_frate=5)

    with DeviceFmcw() as device:
        print(f"Radar SDK Version: {get_version_full()}")
        print("Sensor: " + str(device.get_sensor_type()))

        # use all available RX antennas
        num_rx_antennas = device.get_sensor_information()["num_rx_antennas"]

        metrics = FmcwMetrics(
            range_resolution_m=0.05,
            max_range_m=1.6,
            max_speed_m_s=3,
            speed_resolution_m_s=0.2,
            center_frequency_Hz=60_750_000_000,
        )

        # create acquisition sequence based on metrics parameters
        sequence = device.create_simple_sequence(FmcwSimpleSequenceConfig())
        sequence.loop.repetition_time_s = 1 / args.frate  # set frame repetition time

        # convert metrics into chirp loop parameters
        chirp_loop = sequence.loop.sub_sequence.contents
        device.sequence_from_metrics(metrics, chirp_loop)

        # set remaining chirp parameters which are not derived from metrics
        chirp = chirp_loop.loop.sub_sequence.contents.chirp
        chirp.sample_rate_Hz = 1_000_000
        chirp.rx_mask = (1 << num_rx_antennas) - 1
        chirp.tx_mask = 1
        chirp.tx_power_level = 31
        chirp.if_gain_dB = 33
        chirp.lp_cutoff_Hz = 500000
        chirp.hp_cutoff_Hz = 80000

        device.set_acquisition_sequence(sequence)

        algo = DistanceAlgo(chirp, chirp_loop.loop.num_repetitions)
        draw = Draw(metrics.max_range_m, num_rx_antennas, chirp.num_samples)

        for frame_number in range(args.nframes):  # for each frame
            if not draw.is_open():
                break
            frame_contents = device.get_next_frame()
            frame_data = frame_contents[0]

            distance_data_all_antennas = []
            distance_peak_m_4_all_ant = []

            for i_ant in range(0, num_rx_antennas):  # for each antenna
                antenna_samples = frame_data[i_ant, :, :]
                distance_peak_m, distance_data = algo.compute_distance(antenna_samples)

                distance_data_all_antennas.append(distance_data)
                distance_peak_m_4_all_ant.append(distance_peak_m)

                print("Distance antenna # " + str(i_ant) + ": " +
                      format(distance_peak_m, "^05.3f") + "m")
            draw.draw(distance_data_all_antennas)

        draw.close()
