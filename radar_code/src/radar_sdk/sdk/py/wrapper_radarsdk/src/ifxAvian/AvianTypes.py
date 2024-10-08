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

"""Definitions of the various structures for the Avian wrapper

This file contains definitions of enumerations and structures for the Avian
wrapper.
"""

from ctypes import *

from ifxradarsdk.common.common_types import ifxStructure


# structs
class DeviceConfig(ifxStructure):
    """Device configuration for radar sensor

    sample_rate_Hz:
        Sampling rate of the ADC used to acquire the samples during a
        chirp. The duration of a single chirp depends on the number of
        samples and the sampling rate.

    rx_mask:
        Bitmask where each bit represents one RX antenna of the radar
        device. If a bit is set the according RX antenna is enabled
        during the chirps and the signal received through that antenna
        is captured. The least significant bit corresponds to antenna
        1.

    tx_mask:
        Bitmask where each bit represents one TX antenna. Analogous to
        rx_mask.

    tx_power_level:
        This value controls the power of the transmitted RX signal.
        This is an abstract value between 0 and 31 without any physical
        meaning.

    if_gain_dB:
        Amplification factor that is applied to the IF signal coming
        from the RF mixer before it is fed into the ADC.

    start_frequency_Hz:
        Start frequency of the FMCW chirp.

    end_frequency_Hz:
        Stop frequency of the FMCW chirp.

    num_samples_per_chirp:
        This is the number of samples acquired during each chirp of a
        frame. The duration of a single chirp depends on the number of
        samples and the sampling rate.

    num_chirps_per_frame:
        This is the number of chirps a single data frame consists of.

    chirp_repetition_time_s:
        This is the time period that elapses between the beginnings of
        two consecutive chirps in a frame. (Also commonly referred to as
        pulse repetition time or chirp-to-chirp time.)

    frame_repetition_time_s:
        This is the time period that elapses between the beginnings of
        two consecutive frames. The reciprocal of this parameter is the
        frame rate. (Also commonly referred to as frame time or frame
        period.)

    hp_cutoff_frequency_Hz:
        Cutoff frequency of the high pass filter (in Hz).  The high pass filter
        is used in order to remove the DC-offset at the output of the RX mixer
        and also suppress the reflected signal from close in unwanted targets
        (radome, e.g.).

        Note: Different sensors support different cutoff frequencies. The
        frequency provided by the user will be rounded to the closest supported
        cutoff frequency. You can read the cutoff frequency that was actually
        set using the method get_config.

    aaf_cutoff_frequency_Hz:
        Cutoff frequency of the anti aliasing filter (in Hz).

    mimo_mode:
        Mode of MIMO. Allowed values are "tdm" for time-domain-multiplexed MIMO
        or "off" for MIMO deactivated. If mimo_mode is "tdm" the value for
        tx_mask will be ignored.
    """
    _fields_ = (
        ("sample_rate_Hz", c_uint32),
        ("rx_mask", c_uint32),
        ("tx_mask", c_uint32),
        ("tx_power_level", c_uint32),
        ("if_gain_dB", c_uint32),
        ("start_frequency_Hz", c_uint64),
        ("end_frequency_Hz", c_uint64),
        ("num_samples_per_chirp", c_uint32),
        ("num_chirps_per_frame", c_uint32),
        ("chirp_repetition_time_s", c_float),
        ("frame_repetition_time_s", c_float),
        ("hp_cutoff_Hz", c_uint32),
        ("aaf_cutoff_Hz", c_uint32),
        ("mimo_mode", c_int))

    def __init__(self, sample_rate_Hz : float, rx_mask : int, tx_mask : int,
                 tx_power_level : int, if_gain_dB : int, start_frequency_Hz : float,
                 end_frequency_Hz : float, num_samples_per_chirp : int,
                 num_chirps_per_frame : float, chirp_repetition_time_s : float,
                 frame_repetition_time_s : float, hp_cutoff_Hz : int = 80_000, aaf_cutoff_Hz : int = 500_000, mimo_mode : str = "off"):
        # Adjustments to parameters and conversion
        sample_rate_Hz = c_uint32(int(sample_rate_Hz))
        start_frequency_Hz = c_uint64(int(start_frequency_Hz))
        end_frequency_Hz = c_uint64(int(end_frequency_Hz))

        self._initialize_fields(locals())


    def __setattr__(self, field, value):
        if field == "mimo_mode":
            if value == "off":
                value = 0
            elif value == "tdm":
                value = 1
            else:
                raise ValueError(f"Invalid value for mimo_mode: {value}")
        super().__setattr__(field, value)


    def __getattribute__(self, field):
        value = super().__getattribute__(field)
        if field == "mimo_mode":
            return "off" if value == 0 else "tdm"
        return value


class DeviceMetrics(ifxStructure):
    """DeviceMetrics for radar sensor

    range_resolution_m:
        The range resolution is the distance between two consecutive
        bins of the range transform. Note that even though zero
        padding before the range transform seems to increase this
        resolution, the true resolution does not change but depends
        only from the acquisition parameters. Zero padding is just
        interpolation!

    max_range_m:
        The bins of the Doppler transform represent the speed values
        between -max_speed_m_s and max_speed_m_s.

    max_speed_m_s:
        The bins of the Doppler transform represent the speed values
        between -max_speed_m_s and max_speed_m_s.

    speed_resolution_m_s:
        The speed resolution is the distance between two consecutive
        bins of the Doppler transform. Note that even though zero
        padding before the speed transform seems to increase this
        resolution, the true resolution does not change but depends
        only from the acquisition parameters. Zero padding is just
        interpolation!

    center_frequency_Hz:
        Sampling center frequency of the FMCW chirp. If the value is set to 0 the center
        frequency will be determined by the device. (Default: 0)
    """
    _fields_ = (
        ("range_resolution_m", c_float),
        ("max_range_m", c_float),
        ("max_speed_m_s", c_float),
        ("speed_resolution_m_s", c_float),
        ("center_frequency_Hz", c_double))

    def __init__(self, range_resolution_m : float, max_range_m : float,
                 max_speed_m_s : float, speed_resolution_m_s : float,
                 center_frequency_Hz : float = 0):

        self._initialize_fields(locals())
