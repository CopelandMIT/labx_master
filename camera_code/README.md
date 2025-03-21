# Raspberry Pi Camera Data Collector

This project is a Python-based camera data collection tool designed for a Raspberry Pi with a 64-bit OS (desktop version recommended). It captures video data, segments it into batches, and can optionally sync timestamps using an NTP service for precise data synchronization.

## Requirements

- **Hardware**: Raspberry Pi 5 or similar with a connected USB camera ([Recommended Camera](https://www.amazon.com/gp/product/B071DDB1JY/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&th=1))
- **Storage**: 256+ GB storage recommended for local storage and extended recording sessions
- **Software**: Raspberry Pi OS (64-bit) and OpenCV installed

## Raspberry Pi Setup

### Step 1: Flash the Raspberry Pi OS

1. Download the latest **64-bit Raspberry Pi OS** (desktop version) from the [official website](https://www.raspberrypi.com/software/operating-systems/).
2. Use a tool like **Raspberry Pi Imager** to flash the OS onto a microSD card with at least 256 GB of storage.
3. Edit Settinsg with username, password and wifi info. Remeber these!
4. Boot up the Raspberry Pi with the newly flashed OS.
5. If using VNC/SSH, run:
   ```bash
   sudo raspi-config
   ```
   Navigate to "Interface Options" to enable SSH and VNC.

### Step 1.1: Edit Settings on the Raspberry Pi

After booting up, it's important to configure your Raspberry Pi before setting up the environment:
  
- **Enable SSH and VNC:**  
  Within the same `raspi-config` utility, navigate to "Interface Options" and enable both SSH and VNC if you plan to access the Raspberry Pi remotely.

### Step 2: Set Up the Environment on the Raspberry Pi

1. **Clone this repository**:

    ```bash
    git clone https://github.com/CopelandMIT/labx_master.git
    ```

2. **Create and Activate a Virtual Environment** called `labx_env`:

    ```bash
    cd labx_master/camera_code
    python3 -m venv labx_env
    source labx_env/bin/activate
    ```

3. **Install the Dependencies** from `requirements.txt`:

    ```bash
    pip install -r requirements.txt
    ```

4. **Configure Settings** in the `src/CameraDataCollector.py` file if needed (e.g., `central_server_url`, `batch_duration`).

---

### Step 3: Configuring Time Synchronization with Chrony

Accurate time synchronization is crucial for applications that rely on precise timing. This section guides you through installing Chrony and configuring it to use a GPS PPS SBC over LAN as the preferred time source. If you don't have a GPS time server on your LAN, instructions are provided to use Google's NTP servers instead.

#### 1. Installing Chrony

Install Chrony to synchronize with NTP servers and GPS time sources.

```bash
sudo apt update
sudo apt install -y chrony
```

#### 2. Configuring Chrony to Use GPS PPS SBC over LAN

1. **Determine the IP Address of Your GPS PPS SBC**, for example, `192.168.68.126` and `192.168.68.158`.
   
2. **Edit Chrony Configuration**:

   ```bash
   sudo nano /etc/chrony/chrony.conf
   ```
   - Add the following configuration for precise time polling:
     ```bash
     confdir /etc/chrony/conf.d

     # Use PPS GPS RPi and Google Servers
     # Local PPS GPS RPi Servers
     server 192.168.68.126 iburst prefer minpoll 4 maxpoll 4
     server 192.168.68.158 iburst prefer minpoll 4 maxpoll 4

     # Google NTP servers
     server time.google.com iburst minpoll 4 maxpoll 6
     server time2.google.com iburst minpoll 4 maxpoll 6
     server time3.google.com iburst minpoll 4 maxpoll 6
     server time4.google.com iburst minpoll 4 maxpoll 6
     ```

   - **Explanation**:
     - `minpoll` and `maxpoll`: These settings define the interval (in seconds) for polling the time servers. Setting both to `4` ensures more frequent polling for the local GPS servers.

3. **Save and restart Chrony**:
   ```bash
   sudo systemctl restart chrony
   ```
wait a few minutes for the time sync to stabilize. 

#### 3. Testing the Configuration

- Check sources:
  ```bash
  chronyc sources -v
  ```
- Verify tracking status:
  ```bash
  chronyc tracking
  ```

--- 

### Step 4: Running the Data Collector locally

1. Make sure you’re in the virtual environment:
   ```bash
   source labx_env/bin/activate
   cd ~/labx_master/camera_code/src
   ```
2. Start the data collector with desired settings:
   ```bash
   python3 CameraDataCollector.py --capture duration 300
   ```
   This command starts recording for 300 seconds (or your specified duration).

### Step 5: Running the Data Collector on the central server GUI

1. Make sure you follow the central server steps on how to connect this sensor and share keys with this camera sensor's raspberry pi. 

### Optional Arguments

- `--deployed_sensor_id`: Unique ID for your single board computer (default: `CAM001`)
- `--capture duration`: Total recording duration in seconds 

## Troubleshooting

- **Camera Not Detected**: If the camera is not detected, try unplugging and replugging it, or try different USB ports.
- **Frames Dropping or Slow Recording**: Ensure you have sufficient storage and close other applications to free up resources.
- **Stopping Data Collection**: Press `Ctrl+C` to stop, or use `kill` to terminate the process.
  
## Code Structure

- **CameraDataCollector**: Core class to manage camera setup, recording, batching, and timestamp synchronization.
- **TimeSync**: Handles optional NTP synchronization for accurate timestamps across devices.
