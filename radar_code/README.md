# **Infineon Radar SDK Setup on Raspberry Pi (32-bit) with Debian Bookworm**

This guide walks you through the process of setting up a 32-bit Raspberry Pi OS (Debian Bookworm) for development with the Infineon Radar SDK. It includes installing the SDK, setting up a Python virtual environment with the required dependencies, and configuring time synchronization using GPS PPS with Chrony.

## **Table of Contents**

1. [Flashing and Installing a 32-bit Raspberry Pi OS](#flashing-and-installing-a-32-bit-raspberry-pi-os)
2. [Updating the System](#updating-the-system)
3. [Installing Infineon Radar SDK](#installing-infineon-radar-sdk)
    1. [Cloning the Repository](#1-cloning-the-repository)
    2. [Setting Up Python Virtual Environment](#2-setting-up-python-virtual-environment)
    3. [Installing the Python SDK](#3-installing-the-python-sdk)
4. [Configuring Time Synchronization with GPS PPS using Chrony](#configuring-time-synchronization-with-gps-pps-using-chrony)
    1. [Installing Chrony](#1-installing-chrony)
    2. [Configuring Chrony to Use GPS PPS](#2-configuring-chrony-to-use-gps-pps)
    3. [Testing the Configuration](#3-testing-the-configuration)
5. [Troubleshooting](#troubleshooting)

---

## **Flashing and Installing a 32-bit Raspberry Pi OS**

### **Requirements:**

- A Raspberry Pi (e.g., Pi 3, Pi 4)
- A microSD card (minimum 32GB recommended)
- Raspberry Pi Imager (or another flashing tool)
- A keyboard, mouse, and monitor (or SSH if headless)
- Internet connection

### **Steps:**

1. **Download Raspberry Pi Imager:**

   - Download the [Raspberry Pi Imager](https://www.raspberrypi.org/software/) to your computer and install it.

2. **Prepare the microSD Card:**

   - Insert your microSD card into your computer (using an adapter if needed).

3. **Flash 32-bit Raspberry Pi OS:**

   - Open Raspberry Pi Imager.
   - Select the **OS**: Choose **"Raspberry Pi OS (32-bit)"** (Debian Bookworm).
   - Select the **Storage**: Choose your microSD card.
   - Click **Write** to flash the OS to your microSD card.

4. **Boot the Raspberry Pi:**

   - Insert the flashed microSD card into the Raspberry Pi.
   - Connect a keyboard, mouse, and monitor (or SSH if headless).
   - Power up the Raspberry Pi.
   - Follow the setup instructions (Wi-Fi, locale, etc.).

### **Optional: Enable SSH for Headless Setup**

If you're setting up the Pi without a monitor:

1. After flashing the microSD card, create an empty file named `ssh` (no file extension) in the boot partition.

   - **On Windows:**
     - Open the boot partition of the SD card.
     - Right-click and select **New** > **Text Document**.
     - Rename the file to `ssh` (ensure there is no `.txt` extension).

   - **On macOS/Linux:**

     ```bash
     touch /Volumes/boot/ssh  # Replace /Volumes/boot with the actual mount point
     ```

2. Place the card back in the Pi and boot it. You can now SSH into it using the Pi's IP address:

   ```bash
   ssh pi@<raspberry-pi-ip-address>
   ```

---

## **Updating the System**

After the initial setup, it's important to update the system to ensure all packages are up-to-date.

1. **Open a Terminal:**

   - If you're using the desktop environment, open the terminal application.
   - If you're connected via SSH, you're already in the terminal.

2. **Update the Package List:**

   ```bash
   sudo apt update
   ```

3. **Upgrade Installed Packages:**

   ```bash
   sudo apt upgrade -y
   ```

4. **Optional: Full Upgrade**

   To ensure that any dependencies are also upgraded, you can run:

   ```bash
   sudo apt full-upgrade -y
   ```

5. **Reboot the Raspberry Pi** (if kernel or system packages were upgraded):

   ```bash
   sudo reboot
   ```

---

## **Installing Infineon Radar SDK**

### **1. Cloning the Repository**

Start by cloning the SDK repository from GitHub.

1. **Install Git (if not already installed):**

   ```bash
   sudo apt update
   sudo apt install -y git
   ```

2. **Clone the Repository:**

   ```bash
   cd ~
   git clone https://github.com/CopelandMIT/labx_master.git
   ```

3. **Change into the Directory:**

   ```bash
   cd labx_master/radar_code/infineon_radar_sdk
   ```

### **2. Setting Up Python Virtual Environment**

Setting up the Python virtual environment early ensures that any Python dependencies are managed within the virtual environment.

1. **Install `python3-venv` (if not already installed):**

   ```bash
   sudo apt update
   sudo apt install -y python3-venv
   ```

2. **Navigate to the `radar_code` Directory:**

   ```bash
   cd ~/labx_master/radar_code
   ```

3. **Create a Virtual Environment:**

   ```bash
   python3 -m venv labx_env
   ```

4. **Activate the Virtual Environment:**

   ```bash
   source labx_env/bin/activate
   ```

5. **Upgrade `pip` Inside the Virtual Environment:**

   ```bash
   pip install --upgrade pip
   ```

6. **Install Required Python Packages:**

   ```bash
   pip install -r requirements.txt
   ```

7. **Ensure Necessary Build Tools Are Installed:**

   ```bash
   pip install wheel setuptools
   ```

8. **Install System Packages Required for Building:**

   ```bash
   sudo apt update
   sudo apt install -y cmake python3-dev build-essential
   ```

### **3. Installing the Python SDK**

With the virtual environment set up and dependencies installed, you can now install the Infineon Radar SDK Python package.

1. **Ensure Virtual Environment is Activated:**

   ```bash
   source ~/labx_master/radar_code/labx_env/bin/activate
   ```

2. **Install OpenBLAS Development Libraries:**

   Before installing the Infineon Radar SDK wheel file, install the OpenBLAS development libraries:

   ```bash
   sudo apt-get install -y libopenblas-dev
   ```

3. **Locate the Python Wheel:**

   ```bash
   find ~/labx_master/radar_code/infineon_radar_sdk -name "*.whl"
   ```

4. **Install the Correct Wheel for Your Architecture:**

   ```bash
   pip install ~/labx_master/radar_code/infineon_radar_sdk/python_wheels/ifxradarsdk-3.5.0+8c595dbb-py3-none-linux_armv7l.whl
   ```

   - Ensure you adjust the version number if it's different in your directory.

5. **Verify Installation:**

   ```bash
   pip list | grep ifxradarsdk
   ```

   - You should see output similar to:

     ```
     ifxradarsdk       3.5.0+8c595dbb
     ```

6. **Test the SDK Import:**

   ```bash
   python -c "import ifxradarsdk; print('SDK version:', ifxradarsdk.get_version_full())"
   ```

   - This should output the SDK version without any errors.

---

## **Configuring Time Synchronization with GPS PPS using Chrony**

Accurate time synchronization is crucial for applications that rely on precise timing. This section guides you through installing Chrony and configuring it to use a GPS PPS (Pulse Per Second) source as the preferred time source, along with 4 Google NTP servers.

### **1. Installing Chrony**

Chrony is a versatile implementation of NTP (Network Time Protocol) that can synchronize the system clock with NTP servers, reference clocks (e.g., GPS receivers), and manual input.

**Install Chrony:**

```bash
sudo apt update
sudo apt install -y chrony
```

### **2. Configuring Chrony to Use GPS PPS**

We need to configure Chrony to use the GPS PPS source as the preferred time source and add Google NTP servers for redundancy.

**Steps:**

1. **Identify GPS Device:**

   - Determine the device names for your GPS receiver's NMEA data and PPS signal. Commonly, these are `/dev/ttyS0` for serial NMEA data and `/dev/pps0` for PPS.

2. **Ensure GPSD is Installed and Configured:**

   - Install GPSD and related packages:

     ```bash
     sudo apt install -y gpsd gpsd-clients python3-gps
     ```

   - Configure GPSD to use your GPS device (e.g., `/dev/ttyS0`):

     Edit `/etc/default/gpsd`:

     ```bash
     sudo nano /etc/default/gpsd
     ```

     Update the file to include:

     ```
     START_DAEMON="true"
     GPSD_OPTIONS="-n"
     DEVICES="/dev/ttyS0"
     USBAUTO="false"
     GPSD_SOCKET="/var/run/gpsd.sock"
     ```

   - Restart GPSD:

     ```bash
     sudo systemctl restart gpsd
     ```

3. **Verify GPS is Working:**

   - Use `cgps` or `gpsmon` to check GPS data:

     ```bash
     cgps
     ```

4. **Configure Chrony:**

   - Edit Chrony configuration file:

     ```bash
     sudo nano /etc/chrony/chrony.conf
     ```

   - Add the following lines to configure GPS PPS as the preferred time source and add Google NTP servers:

     ```
     # GPS PPS reference (NMEA serial data)
     refclock SHM 0 offset 0.0 delay 0.2 refid NMEA noselect

     # GPS PPS reference (PPS signal)
     refclock PPS /dev/pps0 refid PPS prefer

     # Google NTP servers
     server time.google.com iburst
     server time2.google.com iburst
     server time3.google.com iburst
     server time4.google.com iburst
     ```

     - **Explanation:**
       - `refclock SHM 0`: Uses shared memory segment 0, which GPSD writes NMEA data to.
       - `noselect`: This source is monitored but not used for synchronization (since NMEA data may not be as precise).
       - `refclock PPS /dev/pps0`: Uses the PPS signal from the GPS device.
       - `prefer`: Marks this source as preferred.
       - `refid`: An identifier for the source.
       - `server ... iburst`: Adds Google NTP servers with the `iburst` option for faster initial synchronization.

5. **Enable PPS Support:**

   - Ensure the `pps-gpio` module is loaded at boot:

     ```bash
     sudo nano /boot/config.txt
     ```

     Add the following lines:

     ```
     dtoverlay=pps-gpio,gpiopin=18
     ```

     - **Note:** Adjust `gpiopin` to match the GPIO pin connected to the PPS signal from the GPS module.

   - Load the module without rebooting:

     ```bash
     sudo dtoverlay pps-gpio gpiopin=18
     ```

6. **Restart Chrony:**

   ```bash
   sudo systemctl restart chrony
   ```

### **3. Testing the Configuration**

1. **Check PPS Device:**

   - Verify that `/dev/pps0` exists:

     ```bash
     ls -l /dev/pps0
     ```

   - Install `ppstest` utility:

     ```bash
     sudo apt install -y pps-tools
     ```

   - Test the PPS signal:

     ```bash
     sudo ppstest /dev/pps0
     ```

     - You should see output indicating PPS signal events.

2. **Monitor Chrony Sources:**

   - Use `chronyc` to check the sources:

     ```bash
     chronyc sources -v
     ```

     - This will display a list of time sources and their status.

   - Check tracking statistics:

     ```bash
     chronyc tracking
     ```

     - This shows information about the system's clock performance.

3. **Ensure GPS PPS is Being Used:**

   - In the output of `chronyc sources -v`, look for the PPS source. The `^*` symbol indicates the source currently being used for synchronization.

     - Example:

       ```
       MS Name/IP address         Stratum Poll Reach LastRx Last sample
       =============================================================================
       #? NMEA                          0   4     0   0     0.000000 0.000000 0.000
       ^* PPS                           0   4   377   5     -0.000001 +0.000001 0.000
       ^? time.google.com               1   6   377  34     -0.000123 +0.000456 0.000
       ```

     - `^* PPS`: Indicates that PPS is the preferred source.

---

## **Troubleshooting**

- **ImportError Related to NumPy and OpenBLAS:**

  If you encounter an error like:

  ```
  ImportError: libopenblas.so.0: cannot open shared object file: No such file or directory
  ```

  **Solution:**

  - Install OpenBLAS libraries:

    ```bash
    sudo apt install -y libopenblas-base libopenblas-dev
    ```

  - Reinstall NumPy in your virtual environment:

    ```bash
    pip uninstall -y numpy
    pip install numpy
    ```

  - Verify NumPy installation:

    ```bash
    python -c "import numpy; print('NumPy version:', numpy.__version__)"
    ```

- **ModuleNotFoundError:**

  Ensure you've installed the correct `.whl` file for your Raspberry Pi's architecture.

- **Virtual Environment Activation Issues:**

  Ensure that the `labx_env` directory exists and that you're in the correct directory.

- **Permission Errors:**

  Do not use `sudo` inside the virtual environment.

- **Missing Dependencies:**

  Install additional system packages:

  ```bash
  sudo apt update
  sudo apt install -y libusb-1.0-0
  ```

- **PPS Device Not Found:**

  - Ensure the GPIO pin is correctly connected and configured.
  - Verify `/dev/pps0` exists. If not, check the `dtoverlay` settings in `/boot/config.txt`.

- **GPS Data Not Received:**

  - Check the GPS antenna and placement.
  - Verify GPSD is configured with the correct device.

- **Chrony Not Using GPS PPS:**

  - Ensure `prefer` is set for the PPS refclock in `chrony.conf`.
  - Verify that the PPS signal is being received using `ppstest`.

---

**Note:** Always activate the virtual environment (`source labx_env/bin/activate`) before running your radar code or scripts that depend on the installed packages and SDK.
