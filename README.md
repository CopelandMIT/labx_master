# Raspberry Pi Camera Data Collector

This project is a Python-based camera data collection tool designed for a Raspberry Pi with a 64-bit OS (desktop version recommended). It captures video data, segments it into batches, and can optionally sync timestamps using an NTP service for precise data synchronization.

## Requirements

- **Central Server Computer**: Required for monitoring time synchronization across devices.
- **Sensors**: Devices that support Linux or MacOS drivers (RPis are used in examples for compatibility).
- **Time Sync Equipment**: For accurate time synchronization across devices, such as GPS PPS clocks. See `gps_code` for more details on setting up time sync.
- **Hardware**: Raspberry Pi 5 or similar with a connected USB camera ([Recommended Camera](https://www.amazon.com/gp/product/B071DDB1JY/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&th=1))
- **Storage**: 256+ GB storage recommended for local storage and extended recording sessions
- **Software**: Raspberry Pi OS (64-bit), OpenCV, and Chrony installed

## Installation

### Step 1: Flash the Raspberry Pi OS

1. Download the latest **64-bit Raspberry Pi OS** (desktop version) from the [official website](https://www.raspberrypi.com/software/operating-systems/).
2. Use a tool like **Raspberry Pi Imager** to flash the OS onto a microSD card with at least 256 GB of storage.
3. Boot up the Raspberry Pi with the newly flashed OS.
4. If using VNC/SSH, run:
   ```bash
   sudo raspi-config
   ```
   Navigate to "Interface" to enable SSH and VNC.

### Step 2: Set Up the Environment

1. **Clone this repository**:
   ```bash
   git clone https://github.com/CopelandMIT/labx_master.git
   ```
2. **Create and activate a virtual environment** called `labx_env`:
   ```bash
   cd labx_master/camera_code
   python3 -m venv labx_env
   source labx_env/bin/activate
   ```
3. **Install the dependencies** from `requirements.txt`:
   ```bash
   pip install -r requirements.txt
   ```

4. **Configure settings** in `CameraDataCollector.py` (located in `camera_code/src/`) if needed (e.g., `central_server_url`, `batch_duration`).

### Step 3: Configuring Time Synchronization with Chrony

Accurate time synchronization is crucial for applications that rely on precise timing. This section guides you through installing Chrony and configuring it to use a GPS PPS SBC over LAN as the preferred time source. If you don't have a GPS time server on your LAN, instructions are provided to use Google's NTP servers instead.

...

### Step 4: Running the Data Collector

1. Make sure you’re in the virtual environment:
   ```bash
   source labx_env/bin/activate
   ```
2. Start the data collector with desired settings:
   ```bash
   python3 camera_code/src/CameraDataCollector.py --duration 300
   ```
   This command starts recording for 300 seconds (or your specified duration).

### Optional Arguments

- `--sbc_id`: Unique ID for your single board computer (default: `SBC001`)
- `--duration`: Total recording duration in seconds
- `--batch_duration`: Duration in seconds for each batch segment (default: 10 seconds)
- `--disable_data_sync`: Add this flag to disable timestamp synchronization with the central server.
