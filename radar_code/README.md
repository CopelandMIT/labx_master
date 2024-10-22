# **Infineon Radar SDK Setup on Raspberry Pi (32-bit)**

This guide walks you through the process of setting up a 32-bit Raspberry Pi OS for development with Infineon Radar SDK and how to install the SDK properly on a Raspberry Pi.

## **Table of Contents**
1. [Flashing and Installing a 32-bit Raspberry Pi OS](#flashing-and-installing-a-32-bit-raspberry-pi-os)
2. [Installing Infineon Radar SDK](#installing-infineon-radar-sdk)
    1. [Cloning the Repository](#1-cloning-the-repository)
    2. [Building the SDK](#2-building-the-sdk)
    3. [Installing the Python SDK](#3-installing-the-python-sdk)

---

## **Flashing and Installing a 32-bit Raspberry Pi OS**

### Requirements:
- A Raspberry Pi (e.g., Pi 3, Pi 4)
- A microSD card (minimum 32GB)
- Raspberry Pi Imager (or another flashing tool)
- A keyboard, mouse, and monitor (or SSH if headless)

### Steps:

1. **Download Raspberry Pi Imager**:
   - Download the [Raspberry Pi Imager](https://www.raspberrypi.org/software/) to your computer and install it.

2. **Prepare the microSD Card**:
   - Insert your microSD card into your computer (using an adapter if needed).

3. **Flash 32-bit Raspberry Pi OS**:
   - Open Raspberry Pi Imager.
   - Select the **OS**: Choose "Raspberry Pi OS (32-bit)".
   - Select the **Storage**: Choose your microSD card.
   - Click **Write** to flash the OS to your microSD card.

4. **Boot the Raspberry Pi**:
   - Insert the flashed microSD card into the Raspberry Pi.
   - Connect a keyboard, mouse, and monitor (or SSH if headless).
   - Power up the Raspberry Pi.
   - Follow the setup instructions (Wi-Fi, locale, etc.).

### Optional: **Enable SSH for Headless Setup**
If you're setting up the Pi without a monitor:
1. After flashing the microSD card, create an empty file named `ssh` (no file extension) in the boot partition.
2. Place the card back in the Pi and boot it. You can now SSH into it using the Pi?s IP address:
   ```bash
   ssh pi@<raspberry-pi-ip-address>
   ```

---

## **Installing Infineon Radar SDK**

### **1. Cloning the Repository**

Start by cloning the SDK repository from GitHub or your source control:

```bash
cd ~
git clone https://github.com/CopelandMIT/labx_master.git
```

Change into the directory:

```bash
cd labx_master/radar_code/infineon_radar_sdk
```

### **2. Building the SDK**

Before installing the Python SDK, you need to build the Infineon Radar SDK itself. Here's how to do it:

1. **Create a Build Directory**:
   ```bash
   mkdir -p build
   cd build
   ```

2. **Run CMake**:
   ```bash
   cmake ..
   ```

   This will configure the SDK and generate the necessary build files.

3. **Build the SDK**:
   ```bash
   make
   ```

   This will compile the SDK and generate the necessary shared libraries and Python SDK components.

### **3. Installing the Python SDK**

Once the SDK has been built, you need to install the Python bindings for the Infineon Radar SDK.

1. **Locate the Python Wheel**:
   The Python SDK wheel (`.whl`) file is located in the `python_wheels` directory. Use the following command to list the available wheels:

   ```bash
   find ~/labx_master/radar_code/infineon_radar_sdk -name "*.whl"
   ```

2. **Install the Correct Wheel for Your Architecture**:
   For Raspberry Pi (32-bit ARM), the correct wheel file is the one for `armv7l`. Use the following command to install it:

   ```bash
   pip install ~/labx_master/radar_code/infineon_radar_sdk/python_wheels/ifxradarsdk-3.5.0+8c595dbb-py3-none-linux_armv7l.whl
   ```

3. **Verify Installation**:
   After installation, verify that the `ifxradarsdk` package has been installed successfully:

   ```bash
   pip list | grep ifxradarsdk
   ```

If everything is successful, you're ready to use the Infineon Radar SDK on your Raspberry Pi.

---

## **Troubleshooting**

- **ModuleNotFoundError**: If you encounter this error while trying to import `ifxradarsdk`, ensure you?ve installed the correct `.whl` file for your Raspberry Pi?s architecture (e.g., `armv7l` for a 32-bit Pi).
  
- **CMake Errors**: Ensure all dependencies like CMake and Python development headers are installed. Run:
  ```bash
  sudo apt-get install cmake python3-dev build-essential
  ```
