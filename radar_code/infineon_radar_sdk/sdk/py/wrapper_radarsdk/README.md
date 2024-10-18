# ifxradarsdk: Python Wrapper for Radar SDK

## Description

The Radar SDK Python wrapper is designed as a single Python package, providing access
to the Infineon 60GHz radar sensors through Python.

Its implementation relies on underlying (wrapped) shared libraries, which are included
as part of the Python wheel. Therefore, wheels are platform specific, as wheel for
Windows contains `.dll` files, while wheel for Linux contains `.so` files.

### Supported radar sensors

* BGT60LTR11AIP
* BGT60UTR11AIP
* BGT60TR13C
* BGT60ATR24C

### Supported platforms

The wrapper has been tested with `Python 3.8` on the following platforms:

* Windows 10 (64-bit)
* Linux Ubuntu 22.04 (Jammy Jellyfish, 64-bit)
* macOS 12.1 (Monterey, 64-bit ARM and Intel based processors)

Additional wheels are provided (but are not thoroughly tested):

* Raspberry Pi OS 5 (Debian Buster based, 32-bit)

## Installation

Python Wrapper wheel, is distributed for several supported platforms, and it can
be installed using the well known `pip install` command.

> **Hint**: We recommend installing `ifxradarsdk` in virtual environnement
([venv](https://docs.python.org/3/library/venv.html)),
to ensure "clean" installation without influence of any, already present,
system level Python packages.
This can be achieved using the following commands:
>
> ```bash
> # create Python virtual environment
> python -m venv .venv
> # activate Python virtual environment
> source .venv/bin/activate  # on Linux
> .venv/Scripts/activate     # on Windows
> ```

Installation of `ifxradarsdk` is then achieved using the following command
(replace `ifxradarsdk` wheel path with the actual wheel path on your system):

```bash
python -m pip install /path/to/ifxradarsdk.whl
```

> **Notes**: `ifxradarsdk` requires a 64-bit Python installation
(except in case of Raspberry Pi OS).
The following error message indicates that you are using a 32-bit Python installation:
`OSError: [WinError 193] %1 is not a valid Win32 application`.

## Usage

After installation, Python wrapper may be imported and used as following:

```Python
from ifxradarsdk import get_version_full


get_version_full()
```

or to access device specific functionality:

```Python
from ifxradarsdk.ltr11 import DeviceLtr11


with DeviceLtr11() as device:
    defaults = device.get_config_defaults()
```

### Examples

Additional application examples can be found README.md file, located in the examples
folder.

## API and documentation

A full documentation is available as docstring. You can access documentation
of the specific `ifxradarsdk` module using the `help` command. For example, to get
docstring of `DeviceLtr11`, we can do the following:

```Python
from ifxradarsdk.ltr11 import DeviceLtr11


help(DeviceLtr11)
```
