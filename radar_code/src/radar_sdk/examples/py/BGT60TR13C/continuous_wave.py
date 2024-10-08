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
from ifxradarsdk import get_version
from ifxradarsdk.cw import DeviceCw

print("Radar SDK Version: " + get_version())

# open device: The device will be closed at the end of the block. Instead of
# the with-block you can also use:
#   device = DeviceCw()
# However, the with block gives you better control when the device is closed.
with DeviceCw() as device:
    # prepare device config
    sensor_info = device.get_sensor_information()

    baseband_config = device.get_baseband_config()
    setattr(baseband_config, "hp_cutoff_Hz", 80_000)
    setattr(baseband_config, "lp_cutoff_Hz", 500_000)

    test_config = device.get_test_signal_generator_config()

    adc_config = device.get_adc_config()

    # apply device config
    device.set_baseband_config(baseband_config)
    device.set_adc_config(adc_config)
    device.set_test_signal_generator_config(test_config)
    print("num samples", baseband_config._get_field("num_samples"))
    device.start_signal()

    # A loop for fetching a finite number of frames comes next..
    for frame_number in range(10):
        try:
            frame = device.capture_frame()
        except ErrorFifoOverflow:
            print("Fifo Overflow")
            exit(1)

        num_rx = np.shape(frame)[0]
        # Do some processing with the obtained frame.
        # In this example we just dump it into the console
        print("Got frame " + format(frame_number) + ", num_antennas={}".format(num_rx))
        for iAnt in range(0, num_rx):
            mat = frame[iAnt, :]
            print("Antenna", iAnt, "\n", mat)
    del frame
