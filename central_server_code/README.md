# Lab in a Box - Central Server Code

## Overview
This repository contains the central server code for the "Lab in a Box" system. It is responsible for coordinating data collection from multiple sensors, monitoring time synchronization, and collecting recorded data after the capture process.

## Prerequisites
Before running the central server, ensure that:
1. All sensors are correctly configured and connected to the network. (Go to sensor subfolder if you need to set up a given sensor)
2. Each sensor has an assigned IP address and username for SSH access.
3. The necessary dependencies are installed using `requirements.txt`.

## Setup and Execution

### 1. Start the GUI Control Panel
Run the GUI to configure and start data collection:
```bash
python3 labx_gui_oop_multisensor.py
```

### 2. Configure Devices and Capture Settings
- Enter the **Base Filename** for captured data.
- Set the **Capture Duration** (in seconds).
- Specify the **IP Address** and **Username** for each sensor.
- Select the **Sensor Type** (e.g., camera, radar, etc.).
- Click **Add Device** to register the device.
- Repeat this process for all devices.

### 3. Start the Capture Process
- Click **Start All Captures** to initiate data collection across all connected devices.
- Monitor the **Live Max Offset** plot to track time synchronization accuracy.

### 4. Collect Data After Capture
Once the capture process is complete, click the 'Collect Data' Button or run the `sensor_data_collector.py` script to collect the recorded data from each sensor:
```bash
python3 sensor_data_collector.py
```
This script will:
- Connect to each sensor via SSH.
- Retrieve the recorded data.
- Store the data in the designated local directory (`DEST_ROOT`).

## File Structure
```
central_server_code/
│── archives/                   # Archived logs and previous runs
│── data/                       # Local storage for collected data
│── labx_env/                   # Virtual environment (if applicable)
│── logs/                       # Log files for debugging
│── src/                        # Source code
│   │── central_server_v3.py    # Main central server script
│   │── create_database.py      # Database initialization
│   │── labx_gui_oop_multisensor.py # GUI for setup and monitoring
│   │── plot_multi_rpi_sync_data.py # Script for analyzing time sync data
│   │── requirements.txt         # Dependencies
│   │── sensor_data_collector.py # Script to retrieve data from sensors
│── README.md                   # Documentation
```

## Notes
- Ensure SSH access to all sensors is configured correctly.
- Time synchronization is crucial for accurate data collection.
- If any sensor fails to sync properly, restart the process and verify the network connection.

## Contact
For issues or questions, reach out to the development team or refer to the project's documentation.

