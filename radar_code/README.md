# **Infineon Radar SDK Setup on Raspberry Pi (32-bit) with Debian Bookworm**

This guide walks you through setting up a 32-bit Raspberry Pi OS (Debian Bookworm) for development with the Infineon Radar SDK. It includes installing the SDK, setting up a Python virtual environment with the required dependencies, and configuring time synchronization using a GPS PPS SBC over LAN with Chrony.

## **Table of Contents**

1. [Flashing and Installing a 32-bit Raspberry Pi OS](#flashing-and-installing-a-32-bit-raspberry-pi-os)
2. [Updating the System](#updating-the-system)
3. [Installing Infineon Radar SDK](#installing-infineon-radar-sdk)
    1. [Cloning the Repository](#1-cloning-the-repository)
    2. [Setting Up Python Virtual Environment](#2-setting-up-python-virtual-environment)
    3. [Installing the Python SDK](#3-installing-the-python-sdk)
4. [Configuring Time Synchronization with Chrony](#configuring-time-synchronization-with-chrony)
    1. [Installing Chrony](#1-installing-chrony)
    2. [Configuring Chrony to Use GPS PPS SBC over LAN](#2-configuring-chrony-to-use-gps-pps-sbc-over-lan)
    3. [Testing the Configuration](#3-testing-the-configuration)
5. [Troubleshooting](#troubleshooting)

---

## **Flashing and Installing a 32-bit Raspberry Pi OS**

### **Requirements:**

- A Raspberry Pi (e.g., Pi 4, Pi 5)
- A microSD card (minimum 64GB recommended)
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
   - Click **Edit Settings** to set your wifi pass word. 
   - **Set Hostname** as "dcope"
   - Configure the Wifi based on your network name and password. 
   - Click **Save**.
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

1. **Open a Terminal on the Raspberry Pi:**

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

## **Configuring Time Synchronization with Chrony**

Accurate time synchronization is crucial for applications that rely on precise timing. This section guides you through installing Chrony and configuring it to use a GPS PPS SBC over LAN as the preferred time source. If you don't have a GPS time server on your LAN, instructions are provided to use Google's NTP servers instead.

### **1. Installing Chrony**

Chrony is a versatile implementation of NTP (Network Time Protocol) that can synchronize the system clock with NTP servers, reference clocks, and manual input.

**Install Chrony:**

```bash
sudo apt update
sudo apt install -y chrony
```

### **2. Configuring Chrony to Use GPS PPS SBC over LAN**

We need to configure Chrony to use the GPS PPS SBC over LAN as the preferred time source. You'll need to determine the IP address of your GPS time server.

**Steps:**

1. **Determine the IP Address of Your GPS PPS SBC:**

   - The GPS PPS SBC should be connected to your LAN.
   - Find its IP address by checking your router's connected devices or using network scanning tools.
   - For example, let's assume the IP address is `192.168.1.100`.

   **Note:** Replace `192.168.1.100` with the actual IP address of your GPS time server.

2. **Configure Chrony:**

   - Edit Chrony's configuration file:

     ```bash
     sudo nano /etc/chrony/chrony.conf
     ```

   - **Remove** or comment out existing `server` or `pool` lines to prevent conflicts.

     ```bash
     # Comment out default servers
     #pool 2.debian.pool.ntp.org iburst
     ```

   - **Add** the GPS PPS SBC as the preferred time source:

     ```bash
     # GPS PPS SBC over LAN
     server 192.168.1.100 iburst prefer
     ```

     - **Explanation:**
       - `server 192.168.1.100`: Specifies the IP address of your GPS time server.
       - `iburst`: Allows faster initial synchronization.
       - `prefer`: Marks this server as the preferred time source.

   - **Optional:** Add Google's NTP servers as fallback options in case the GPS PPS SBC is unavailable.

     ```bash
     # Google NTP servers as fallback
     server time.google.com iburst
     server time2.google.com iburst
     server time3.google.com iburst
     server time4.google.com iburst
     ```

3. **Save and Close the File:**

   - Press `Ctrl+O` to save and `Ctrl+X` to exit the editor.

4. **Restart Chrony:**

   ```bash
   sudo systemctl restart chrony
   ```

### **3. Testing the Configuration**

1. **Verify Time Sources:**

   - Use `chronyc` to check the sources:

     ```bash
     chronyc sources -v
     ```

     - This will display a list of time sources and their status.

   - **Example Output:**

     ```
     MS Name/IP address         Stratum Poll Reach LastRx Last sample
     =============================================================================
     ^* 192.168.1.100                1   6   377    34   -0.000123 +0.000456 0.000
     ^? time.google.com              1   6   377    34   -0.000789 +0.001234 0.000
     ```

     - `^* 192.168.1.100`: The `^*` indicates that the GPS PPS SBC is the currently selected and preferred time source.

2. **Check Tracking Statistics:**

   - To view detailed information about the system's clock performance:

     ```bash
     chronyc tracking
     ```

3. **Monitor Synchronization:**

   - Ensure that the system time is synchronizing with the GPS PPS SBC.
   - You can use the `watch` command to monitor the sources:

     ```bash
     watch -n 10 'chronyc sources -v'
     ```

     - This will update the output every 10 seconds.

---

## **Troubleshooting**

- **GPS Time Server Unavailable:**

  - Ensure that the GPS PPS SBC is powered on and connected to the LAN.
  - Verify the IP address and network connectivity.
  - Use `ping` to test connectivity:

    ```bash
    ping 192.168.1.100
    ```

- **Chrony Not Using GPS Time Server:**

  - Ensure the `prefer` option is set in `chrony.conf`.
  - Verify that there are no typos in the server IP address.
  - Check for firewall rules that may block NTP traffic.

- **No Network Connectivity:**

  - Ensure the Raspberry Pi is connected to the LAN.
  - Check network settings and cables.

- **Using Google's NTP Servers Instead:**

  If you do not have a GPS PPS SBC set up on your LAN, you can configure Chrony to use Google's NTP servers or any other reliable public NTP servers.

  - Edit `/etc/chrony/chrony.conf`:

    ```bash
    sudo nano /etc/chrony/chrony.conf
    ```

  - **Replace** any existing `server` lines with the following:

    ```bash
    # Google NTP servers
    server time.google.com iburst
    server time2.google.com iburst
    server time3.google.com iburst
    server time4.google.com iburst
    ```

  - **Remove** the line pointing to the GPS PPS SBC if it's not available.

  - Save and close the file, then restart Chrony:

    ```bash
    sudo systemctl restart chrony
    ```

- **Chrony Service Not Starting:**

  - Check the status of the Chrony service:

    ```bash
    sudo systemctl status chrony
    ```

  - Look for error messages in the output and address any configuration issues.

- **Permission Errors:**

  - Ensure you use `sudo` when editing system files or restarting services.

---

**Note:** Always activate the virtual environment (`source labx_env/bin/activate`) before running your radar code or scripts that depend on the installed packages and SDK.

