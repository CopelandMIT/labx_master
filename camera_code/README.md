# Raspberry Pi Camera Data Collector

This project is a Python-based camera data collection tool designed for a Raspberry Pi with a 64-bit OS (desktop version recommended). It captures video data, segments it into batches, and can optionally sync timestamps using an NTP service for precise data synchronization.

## Requirements

- **Hardware**: Raspberry Pi 5 or similar with a connected USB camera ([Recommended Camera](https://www.amazon.com/gp/product/B071DDB1JY/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&th=1))
- **Storage**: 256+ GB storage recommended for local storage and extended recording sessions
- **Software**: Raspberry Pi OS (64-bit) and OpenCV installed

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

4. **Configure settings** in the `camera_code.py` file if needed (e.g., `central_server_url`, `batch_duration`).

### Step 3: Running the Data Collector

1. Make sure you’re in the virtual environment:
   ```bash
   source labx_env/bin/activate
   ```
2. Start the data collector with desired settings:
   ```bash
   python3 camera_code.py --duration 300 --data_directory "data"
   ```
   This command starts recording for 300 seconds (or your specified duration).

### Optional Arguments

- `--sbc_id`: Unique ID for your single board computer (default: `SBC001`)
- `--duration`: Total recording duration in seconds
- `--data_directory`: Directory to store recorded video files
- `--batch_duration`: Duration in seconds for each batch segment (default: 10 seconds)
- `--disable_data_sync`: Add this flag to disable timestamp synchronization with the central server.

## Troubleshooting

- **Camera Not Detected**: If the camera is not detected, try unplugging and replugging it, or try different USB ports.
- **Frames Dropping or Slow Recording**: Ensure you have sufficient storage and close other applications to free up resources.
- **Stopping Data Collection**: Press `Ctrl+C` to stop, or use `kill` to terminate the process.
  
## Code Structure

- **CameraDataCollector**: Core class to manage camera setup, recording, batching, and timestamp synchronization.
- **TimeSync**: Handles optional NTP synchronization for accurate timestamps across devices.
