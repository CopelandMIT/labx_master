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

"""Python wrapper for Infineon Avian Radar sensors"""

from ctypes import *
import typing

import numpy as np

from .AvianTypes import DeviceConfig, DeviceMetrics
from ifxradarsdk.common.base_types import MdaReal
from ifxradarsdk.common.cdll_helper import declare_prototype, load_library
from ifxradarsdk.common.common_types import (
    create_python_list_from_terminated_list,
    DeviceListEntry,
    FirmwareInfo,
    RadarSensor,
    SensorInfo
)
from ifxradarsdk.common.sdk_base import (
    ifx_mda_destroy_r,
    ifx_mem_free,
    move_ifx_list_to_python_list
)


class Device():
    @staticmethod
    def __load_cdll() -> CDLL:
        """Initialize the module and return ctypes handle"""
        # find and load shared library
        dll = load_library("sdk_avian")

        # declare prototypes such that ctypes knows the arguments and return types
        declare_prototype(dll, "ifx_avian_create", None, c_void_p)
        declare_prototype(dll, "ifx_avian_destroy", [c_void_p], None)
        declare_prototype(dll, "ifx_avian_import_register_list", [c_void_p, POINTER(c_uint32), c_size_t], None)
        declare_prototype(dll, "ifx_avian_get_register_list_string", [c_void_p, c_bool], POINTER(c_char))
        declare_prototype(dll, "ifx_avian_create_by_port", [c_char_p], c_void_p)
        declare_prototype(dll, "ifx_avian_get_list", None, c_void_p)
        declare_prototype(dll, "ifx_avian_get_list_by_sensor_type", [c_int], c_void_p)
        declare_prototype(dll, "ifx_avian_create_by_uuid", [c_char_p], c_void_p)
        declare_prototype(dll, "ifx_avian_get_board_uuid", [c_void_p], c_char_p)
        declare_prototype(dll, "ifx_avian_save_register_file", [c_void_p, c_char_p], None)
        declare_prototype(dll, "ifx_avian_load_register_file", [c_void_p, c_char_p], None)
        declare_prototype(dll, "ifx_avian_set_config", [c_void_p, POINTER(DeviceConfig)], None)
        declare_prototype(dll, "ifx_avian_get_config", [c_void_p, POINTER(DeviceConfig)], None)
        declare_prototype(dll, "ifx_avian_get_config_defaults", [c_void_p, POINTER(DeviceConfig)], None)
        declare_prototype(dll, "ifx_avian_start_acquisition", [c_void_p], c_bool)
        declare_prototype(dll, "ifx_avian_stop_acquisition", [c_void_p], c_bool)
        declare_prototype(dll, "ifx_avian_get_next_frame", [c_void_p, POINTER(MdaReal)], POINTER(MdaReal))
        declare_prototype(dll, "ifx_avian_get_next_frame_timeout", [c_void_p, POINTER(MdaReal), c_uint16], POINTER(MdaReal))
        declare_prototype(dll, "ifx_avian_get_temperature", [c_void_p, POINTER(c_float)], None)
        declare_prototype(dll, "ifx_avian_get_firmware_information", [c_void_p], POINTER(FirmwareInfo))
        declare_prototype(dll, "ifx_avian_get_sensor_information", [c_void_p], POINTER(SensorInfo))
        declare_prototype(dll, "ifx_avian_metrics_to_config", [c_void_p, POINTER(DeviceMetrics), c_bool, POINTER(DeviceConfig)], None)
        declare_prototype(dll, "ifx_avian_metrics_from_config", [c_void_p, POINTER(DeviceConfig), POINTER(DeviceMetrics)], None)
        declare_prototype(dll, "ifx_avian_get_hp_cutoff_list", [c_void_p], c_void_p)
        declare_prototype(dll, "ifx_avian_get_aaf_cutoff_list", [c_void_p], c_void_p)

        return dll


    _cdll = __load_cdll.__func__()


    @classmethod
    def get_list(cls, sensor_type : typing.Optional[RadarSensor] = None) -> typing.List[str]:
        """Return list of UUIDs of connected boards

        The function returns a list of unique ids (uuids) that correspond to
        available boards. If sensor_type is specified only boards with this particular
        sensor are returned.

        **Examples**
            uuids_all   = Device.get_list()
            uuids_tr13c = Device.get_list(RadarSensor.BGT60TR13C)

        Parameters:
            sensor_type: Sensor of type RadarSensor
        """

        if sensor_type == None:
            ifx_list = cls._cdll.ifx_avian_get_list()
        else:
            ifx_list = cls._cdll.ifx_avian_get_list_by_sensor_type(int(sensor_type))

        return move_ifx_list_to_python_list(ifx_list,
                                            lambda p: cast(p, POINTER(DeviceListEntry)).contents.uuid.decode("ascii"))


    def __init__(self, uuid : typing.Optional[str] = None, port : typing.Optional[str] = None):
        """Connect to a board

        Search for an Infineon radar sensor board connected to the host machine
        and connect to the first found sensor device.

        The device is automatically closed by the destructor. If you want to
        close the device yourself, you can use the keyword del:
            device = Device()
            # do something with device
            ...
            # close device
            del device

        However, we suggest to use a context manager:
            with open Device() as dev:
                # do something with device

        If port is given, the specific port is opened. If uuid is given and
        port is not given, the radar device with the given uuid is opened. If
        no parameters are given, the first found radar device will be opened.

        Examples:
          - Open first found radar device:
            dev = Device()
          - Open radar device on COM5:
            dev = Device(port="COM5")
          - Open radar device with uuid 0123456789abcdef0123456789abcdef
            dev = Device(uuid="0123456789abcdef0123456789abcdef")

        Optional parameters:
            port:       opens the given port
            uuid:       open the radar device with unique id given by uuid
                        the uuid is represented as a 32 character string of
                        hexadecimal characters. In addition, the uuid may
                        contain dash characters (-) which will be ignored.
                        Both examples are valid and correspond to the same
                        uuid:
                            0123456789abcdef0123456789abcdef
                            01234567-89ab-cdef-0123-456789abcdef
        """
        # The Python garbage collector does not guarantee in which order objects
        # are destroyed when Python is closed. The ctypes instance obtained from
        # cls._cdll is saved in a global variable. If the Python destructor decides to
        # destroy the ctypes instance before this object, then the destructor cannot
        # properly close the connection to the board anymore. This problem for example
        # occurs if a user aborts the script using ctrl+c.
        # To prevent this problem, save the CDLL instance as a member of this object
        # to prevent the garbage collector of destroying the CDLL instance before
        # destroying this object.

        if uuid:
            h = self._cdll.ifx_avian_create_by_uuid(uuid.encode("ascii"))
        elif port:
            h = self._cdll.ifx_avian_create_by_port(port.encode("ascii"))
        else:
            h = self._cdll.ifx_avian_create()

        # The pointer is returned as an integer. Explicitly casting it to a void pointer
        # ensures it is not truncated by integer handling in certain situations
        self.handle = h #c_void_p(h)

    def get_firmware_information(self) -> dict:
        """Gets information about the firmware of a connected device"""
        info_p = self._cdll.ifx_avian_get_firmware_information(self.handle)
        return info_p.contents.to_dict(True)

    def get_sensor_information(self) -> dict:
        """Gets information about the connected device"""
        info_p = self._cdll.ifx_avian_get_sensor_information(self.handle)
        info = info_p.contents.to_dict(True)

        for entry in ["lp_cutoff_list", "hp_cutoff_list", "if_gain_list"]:
            info[entry] = create_python_list_from_terminated_list(info[entry])

        return info

    def save_register_file(self, filename: str) -> None:
        """Save register list to a file"""
        filename_buffer = filename.encode("ascii")
        filename_buffer_p = c_char_p(filename_buffer)
        self._cdll.ifx_avian_save_register_file(self.handle, filename_buffer_p)

    def load_register_file(self, filename: str) -> None:
        """Load register list from a file"""
        filename_buffer = filename.encode("ascii")
        filename_buffer_p = c_char_p(filename_buffer)
        self._cdll.ifx_avian_load_register_file(self.handle, filename_buffer_p)

    def metrics_to_config(self, metrics : DeviceMetrics, power_of_two : bool = True) -> DeviceConfig:
        """Convert metrics configuration to device configuration

        Parameter:
            metrics: metrics configuration
            power_of_two: If true the values for num_samples_per_chirp and
                          num_chirps_per_frame are rounded up to a power of 2
        """
        config = DeviceConfig(0,0,0,0,0,0,0,0,0,0,0)
        self._cdll.ifx_avian_metrics_to_config(self.handle, byref(metrics), power_of_two, byref(config))
        return config

    def metrics_from_config(self, config : DeviceConfig) -> DeviceMetrics:
        metrics = DeviceMetrics(0,0,0,0,0)
        self._cdll.ifx_avian_metrics_from_config(self.handle, byref(config), byref(metrics))
        return metrics

    def set_config(self, config : DeviceConfig) -> None:
        """Set device configuration"""
        self._cdll.ifx_avian_set_config(self.handle, byref(config))

    def get_config(self) -> DeviceConfig:
        """Get current device configuration"""
        config = DeviceConfig(0,0,0,0,0,0,0,0,0,0,0)
        self._cdll.ifx_avian_get_config(self.handle, byref(config))
        return config

    def get_config_defaults(self) -> DeviceConfig:
        """Get default device configuration"""
        config = DeviceConfig(0,0,0,0,0,0,0,0,0,0,0)
        self._cdll.ifx_avian_get_config_defaults(self.handle, byref(config))
        return config

    def start_acquisition(self) -> None:
        """Start acquisition of time domain data

        Starts the acquisition of time domain data from the connected device.
        If the data acquisition is already running the function has no effect.
        """
        self._cdll.ifx_avian_start_acquisition(self.handle)

    def stop_acquisition(self) -> None:
        """Stop acquisition of time domain data

        Stops the acquisition of time domain data from the connected device.
        If the data acquisition is already stopped the function has no effect.
        """
        self._cdll.ifx_avian_stop_acquisition(self.handle)

    def get_next_frame(self, timeout_ms : typing.Optional[int] = None) -> np.ndarray:
        """Retrieve next frame of time domain data from device

        Retrieve the next complete frame of time domain data from the connected
        device. The samples from all chirps and all enabled RX antennas will be
        copied to the provided data structure frame.

        The frame is returned as numpy array with dimensions
        num_virtual_rx_antennas x num_chirps_per_frame x num_samples_per_frame.

        If timeout_ms is given, the exception ErrorTimeout is raised if a
        complete frame is not available within timeout_ms milliseconds.
        """
        if timeout_ms:
            frame = self._cdll.ifx_avian_get_next_frame_timeout(self.handle, None, timeout_ms)
        else:
            frame = self._cdll.ifx_avian_get_next_frame(self.handle, None)
        frame_numpy = frame.contents.to_numpy()
        ifx_mda_destroy_r(frame)
        return frame_numpy

    def get_board_uuid(self) -> str:
        """Get the unique id for the radar board"""
        c_uuid = self._cdll.ifx_avian_get_board_uuid(self.handle)
        return c_uuid.decode("utf-8")

    def get_temperature(self) -> float:
        """Get the temperature of the device in degrees Celsius

        Note that reading the temperature is not supported for UTR11. An
        exception will be raised in this case.
        """
        temperature = c_float(0)
        self._cdll.ifx_avian_get_temperature(self.handle, pointer(temperature))
        return float(temperature.value)

    def get_hp_cutoff_list(self) -> typing.List[str]:
        """Gets a list of available High Pass Cutoff frequencies in [Hz] for a connected device"""
        ifx_list = self._cdll.ifx_avian_get_hp_cutoff_list(self.handle)
        return move_ifx_list_to_python_list(ifx_list,
                                            lambda p: cast(p, POINTER(c_uint32)).contents.value)

    def get_aaf_cutoff_list(self) -> typing.List[str]:
        """Gets a list of available Anti Aliasing Filter Cutoff frequencies in [Hz] for a connected device"""
        ifx_list = self._cdll.ifx_avian_get_aaf_cutoff_list(self.handle)
        return move_ifx_list_to_python_list(ifx_list,
                                            lambda p: cast(p, POINTER(c_uint32)).contents.value)

    def import_register_list(self, registers : typing.List[int]) -> None:
        """Import the registerlist """
        registers_c = (c_uint32 * len(registers))(*registers)
        self._cdll.ifx_avian_import_register_list(self.handle, registers_c, c_size_t(len(registers)))

    def get_register_list_string(self, trigger : bool) -> str:
        """Get the exported register list as a hexadecimal string"""
        ptr = self._cdll.ifx_avian_get_register_list_string(self.handle,trigger)
        reg_list_string = cast(ptr, c_char_p).value
        reg_list_string_py = reg_list_string.decode('ascii')
        ifx_mem_free(ptr)
        return reg_list_string_py

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self._close()

    def _close(self):
        """Destroy device handle"""
        if hasattr(self, "handle") and self.handle:
            self._cdll.ifx_avian_destroy(self.handle)
            self.handle = None

    def __del__(self):
        try:
            self._close()
        except:
            # just like in C++ exceptions raised in a destructor cannot be catched
            # for this reason we ignore them
            pass
