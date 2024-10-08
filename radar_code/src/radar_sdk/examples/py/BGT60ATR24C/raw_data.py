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

import pprint
import numpy as np
from ifxradarsdk import get_version
from ifxradarsdk.fmcw import DeviceFmcw
from ifxradarsdk.fmcw.types import create_dict_from_sequence

# open device: The device will be closed at the end of the block. Instead of
# the with-block you can also use:
#   device = DeviceFmcw()
# However, the with block gives you better control when the device is closed.
with DeviceFmcw() as device:
    print("Radar SDK Version: " + get_version())
    print("UUID of board: " + device.get_board_uuid())
    print("Sensor: " + str(device.get_sensor_type()))

    # A device instance is initialised with the default acquisition
    # sequence for its corresponding radar sensor. This sequence can be
    # simply fetched, analysed or modified by the user.
    first_element = device.get_acquisition_sequence()

    # Print the current device acquisition sequence
    sequence = create_dict_from_sequence(first_element)
    pp = pprint.PrettyPrinter()
    pp.pprint(sequence)

    # Fetch a number of frames
    for frame_number in range(10):
        frame_contents = device.get_next_frame()

        for frame in frame_contents:
            num_rx = np.shape(frame)[0]

            # Do some processing with the obtained frame.
            # In this example we just dump it into the console
            print("Frame " + format(frame_number) + ", num_antennas={}".format(num_rx))

            for iAnt in range(num_rx):
                mat = frame[iAnt, :, :]
                print("Antenna", iAnt, "\n", mat)
