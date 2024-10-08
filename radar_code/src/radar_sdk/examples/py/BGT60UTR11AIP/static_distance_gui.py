# ===========================================================================
# Copyright (C) 2021 Infineon Technologies AG
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
import sys
import time

from PyQt5 import QtWidgets
from pyqtgraph.Qt import QtCore
import pyqtgraph as pg

from ifxradarsdk import get_version_full
from ifxradarsdk.fmcw import DeviceFmcw
from ifxradarsdk.fmcw.types import FmcwSimpleSequenceConfig, FmcwMetrics
from helpers.DistanceAlgo import *


# -------------------------------------------------
# Presentation 
# -------------------------------------------------
class DistanceWindow(QtWidgets.QMainWindow):
    # GUI window showing distances

    def __init__(self, frate=10, nframes=0, parent=None):
        # Create window and it's layout
        # frate:        number of frames per second
        # nframes:      number of acquired frames (0 - infinity)
        # parent:       QT parent window

        super(DistanceWindow, self).__init__(parent)
        self._distance = []
        self._times = []

        # Create Gui Elements
        self._mainbox = QtWidgets.QWidget()
        self.setCentralWidget(self._mainbox)
        self._mainbox.setLayout(QtWidgets.QVBoxLayout())

        self._detection = QtWidgets.QLabel()
        self._mainbox.layout().addWidget(self._detection)

        self._canvas = pg.GraphicsLayoutWidget()
        self._mainbox.layout().addWidget(self._canvas)

        self._label = QtWidgets.QLabel()
        self._mainbox.layout().addWidget(self._label)

        self._button = QtWidgets.QPushButton("Clear")
        self._button.clicked.connect(self._clear)
        self._mainbox.layout().addWidget(self._button)

        self._plot = self._canvas.addPlot()
        self._plot.addLegend()
        self._plot.setLabels(bottom="time [s]", left="distance [m]")
        self._line_distance = self._plot.plot(pen='g', name="distance")

        self._counter = 0
        self._start_time = time.time()

    def update_distance(self, distance):
        # Update single reading

        self._counter = self._counter + 1
        self._distance.append(distance)
        self._times.append(time.time() - self._start_time)
        self._line_distance.setData(self._times, self._distance)
        self._label = "label"

    def _clear(self, event):
        # Clear all data in gui

        self._distance.clear()
        self._times.clear()
        self._counter = 0
        self._start_time = time.time()

    # -------------------------------------------------


# Acquisition in QT timer + Presentation
# -------------------------------------------------
class DistanceUpdaterTimer(QtCore.QTimer):
    # Timer that gets a data from device and move it to GUI

    def __init__(self, device, window, dist_algo):
        # Common variable update, 
        # connect the timer timeout to _update_distance method
        # device:    device from what we extract data
        # window:    window where data is showed
        # dist_algo: algorithm needed for computation of distance   

        super(QtCore.QTimer, self).__init__()
        self._device = device
        self._window = window
        self._dist_algo = dist_algo
        self.timeout.connect(self._update_distance)

    def _update_distance(self):
        frame_contents = self._device.get_next_frame()
        frame = frame_contents[0]
        antenna_samples = frame[i_ant, :, :]
        distance_peak_m, distance_data = self._dist_algo.compute_distance(antenna_samples)

        # GUI update
        self._window.update_distance(distance_peak_m)

    def start(self):
        # Start a timer each 100ms

        super().start(100)


# -------------------------------------------------
# Main logic
# -------------------------------------------------
if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='''Displays distance plot from Radar Data''')
    parser.add_argument('-f', '--frate', type=int, default=5, help="frame rate in Hz, default 5")
    args = parser.parse_args()

    with DeviceFmcw() as device:
        print(f"Radar SDK Version: {get_version_full()}")
        print("Sensor: " + str(device.get_sensor_type()))

        i_ant = 0  # use only 1st RX antenna
        num_rx_antennas = 1

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

        app = QtWidgets.QApplication(sys.argv)
        distance_window = DistanceWindow(args.frate)
        distance_window.show()

        # acquire and present data
        timer = DistanceUpdaterTimer(device, distance_window, algo)
        timer.start()

        app.exec_()
