# labx_master

**labx_master** is the central repository for setting up a synchronized distributed sensor network, integrating multiple sensors like a 60GHz Infineon FMCW radar, USB cameras, and GPS modules. It uses NTP and PPS GPS for time synchronization and includes tools for data collection and analysis.

## Features

- **Multi-sensor support**: Radar, cameras, GPS
- **NTP and PPS GPS time synchronization**: High-precision time synchronization across all sensor nodes
- **Scalable architecture**: Easily add or remove sensors as needed
- **Data collection and analysis tools**: Central server to manage and analyze collected data

---

## Table of Contents

- [Installation and Setup](#installation-and-setup)
  - [1. Clone the Repository](#1-clone-the-repository)
  - [2. Build the GPS Time Server](#2-build-the-gps-time-server)
  - [3. Configure Sensor Nodes](#3-configure-sensor-nodes)
  - [4. Test Sensor Synchronization with Central Server](#4-test-sensor-synchronization-with-central-server)
- [Usage](#usage)
  - [Central Server](#central-server)
  - [Sensor Nodes](#sensor-nodes)
- [License](#license)

---

## Installation and Setup

Follow these general steps to set up the Lab in a Box:

### 1. Clone the Repository

Start by cloning the `labx_master` repository to your local machine:

```bash
git clone https://github.com/CopelandMIT/labx_master.git
cd labx_master
```

### 2. Build the GPS Time Server

The GPS time server provides a precise time reference for all sensor nodes in the network.

- **Navigate to the GPS code directory:**

  ```bash
  cd gps_code
  ```

- **Set Up the GPS Time Server:**

  Follow the instructions in the `gps_code` folder's README to build and configure the GPS time server. This typically involves:

  - Installing necessary packages (e.g., GPSD, Chrony)
  - Configuring GPS hardware (e.g., GPS module with PPS output)
  - Updating `/etc/chrony/chrony.conf` to use the GPS as the primary time source

- **Sample `chrony.conf` Configuration:**

  ```plaintext
  # Use GPS PPS as the preferred time source
  refclock SHM 0 offset 0.0 delay 0.2 refid NMEA noselect
  refclock PPS /dev/pps0 refid PPS prefer

  # Allow local network access
  allow 192.168.0.0/16

  # Other configurations...
  ```

- **Start and Verify the GPS Time Server:**

  ```bash
  sudo systemctl restart chrony
  chronyc sources -v
  ```

  Ensure that the GPS PPS source is active and preferred.

### 3. Configure Sensor Nodes
Set up each sensor node by configuring time synchronization and integrating sensors using the `shared_sensor_code`.

- **Navigate to the Shared Sensor Code Directory:**

  ```bash
  cd ../shared_sensor_code
  ```

- **Set Up Time Synchronization:**

  - **Install Chrony:**

    ```bash
    sudo apt update
    sudo apt install -y chrony
    ```

  - **Update `/etc/chrony/chrony.conf`:**

    Configure each sensor node to synchronize time with the GPS time server and fallback NTP servers.

    ```plaintext
    # Use Local GPS Time Server
    server <GPS_SERVER_IP> iburst prefer minpoll 4 maxpoll 4

    # Google NTP servers as fallback
    server time.google.com iburst minpoll 4 maxpoll 6
    server time2.google.com iburst minpoll 4 maxpoll 6
    server time3.google.com iburst minpoll 4 maxpoll 6
    server time4.google.com iburst minpoll 4 maxpoll 6

    # Allow NTP traffic from local network
    allow 192.168.0.0/16

    # Other configurations...
    ```

    - Replace `<GPS_SERVER_IP>` with the IP address of your GPS time server.

  - **Restart Chrony and Verify Synchronization:**

    ```bash
    sudo systemctl restart chrony
    chronyc sources -v
    ```

- **Integrate `TimeSync` Class:**

  - The `TimeSync` class sends timestamps and Chrony tracking data to the central server for synchronization analysis.

  - **Import and Initialize `TimeSync`:**

    ```python
    from timesync import TimeSync

    # Initialize TimeSync
    time_sync = TimeSync(
        sbc_id='SBC001',
        central_server_url='http://<CENTRAL_SERVER_IP>:5000/receive_data',
        polling_interval=15  # Adjust as needed
    )
    ```

    - Replace `<CENTRAL_SERVER_IP>` with the IP address of your central server.

  - **Start Time Synchronization Thread:**

    ```python
    time_sync.start()
    ```

- **Set Up Sensor Data Collection:**

  - Integrate your sensors (e.g., radar, cameras) using the provided code in `shared_sensor_code`.

  - **Example for Camera Data Collector:**

    ```python
    from camera_data_collector import CameraDataCollector
    import threading

    stop_event = threading.Event()
    camera_collector = CameraDataCollector(
        stop_event=stop_event,
        sbc_id='SBC001',
        central_server_url='http://<CENTRAL_SERVER_IP>:5000/receive_data',
        data_collection_interval=10
    )

    camera_collector.start()
    ```


### 4. Test Sensor Synchronization with Central Server

Ensure that all sensor nodes are correctly synchronized with the central server.

- **Navigate to the Central Server Code Directory:**

  ```bash
  cd ../central_server_code
  ```

- **Run the Central Server:**

  ```bash
  python central_server.py
  ```

- **Verify Data Reception:**

  - Check the central server console output to verify that it is receiving data from the sensor nodes.

  - Ensure that timestamps are accurate and synchronized.

---

## Usage

### Central Server

Run `central_server.py` to manage sensor data collection and synchronization.

```bash
cd central_server_code
python central_server.py
```

- The central server listens for incoming data from sensor nodes and performs synchronization analysis.

### Sensor Nodes

Each sensor node should run its respective data collection script as per the instructions in the `shared_sensor_code` folder.

- **Example: Running the Camera Data Collector**

  ```bash
  cd shared_sensor_code
  python camera_data_collector.py
  ```

- **Ensure Time Synchronization is Active:**

  - Before starting sensor data collection, make sure the `TimeSync` thread is running to send synchronization data to the central server.

---

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

## Additional Information

- **Monitoring Time Synchronization:**

  - Use `chronyc tracking` and `chronyc sources -v` to monitor the synchronization status on both the GPS server and sensor nodes.

- **Adjusting Polling Intervals:**

  - Polling intervals in `chrony.conf` and the `TimeSync` class can be adjusted based on network performance and synchronization requirements.

- **Error Handling:**

  - Implement error handling in your sensor node scripts to manage network interruptions or hardware issues gracefully.

- **Extending the Network:**

  - The architecture is scalable. Additional sensors can be integrated by following the same setup steps.

---

**Note:** Accurate time synchronization is crucial for data integrity in distributed sensor networks. Ensure that all steps, especially those involving Chrony configuration and `TimeSync` integration, are carefully followed.

---

**Disclaimer:** This guide provides a general overview of the setup process. Depending on your specific hardware and network configuration, additional steps may be required. Always test the system thoroughly in a controlled environment before deploying it in a production setting.

---

**Contact Information:**

- For issues or questions, please open an issue on the [GitHub repository](https://github.com/CopelandMIT/labx_master/issues).

---
