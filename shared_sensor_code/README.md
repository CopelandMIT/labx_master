# Shared Sensor Code

This repository contains code for synchronizing sensor data timestamps across multiple devices using a central server. The `TimeSync` class allows your application to send timestamps and Chrony tracking data to the central server for synchronization analysis. By using threading, you can send timestamps in the background without blocking your main application logic.

This README provides instructions on how to import and use the `TimeSync` class within your application, specifically in the context of a `CameraDataCollector` class. Additionally, it includes the necessary configuration changes to the `/etc/chrony/chrony.conf` file to optimize time synchronization using Chrony.

## Table of Contents

- [Prerequisites](#prerequisites)
- [Installation](#installation)
- [Chrony Configuration](#chrony-configuration)
  - [Updating `/etc/chrony/chrony.conf`](#updating-etcchronychronyconf)
- [Usage](#usage)
  - [Importing the `TimeSync` Class](#importing-the-timesync-class)
  - [Integrating `TimeSync` in Your Application](#integrating-timesync-in-your-application)
  - [Starting the Time Synchronization Thread](#starting-the-time-synchronization-thread)
- [Example Code](#example-code)
- [Additional Notes](#additional-notes)
- [License](#license)

---

## Prerequisites

- **Python 3.6** or higher
- **Network Connection** to the central server
- **Chrony** installed and configured for time synchronization
- **Required Python Packages** (listed in `requirements.txt`):
  - `requests`
  - Additional packages as needed (e.g., `opencv-python` for camera operations)

---

## Installation

1. **Clone the Repository**

   ```bash
   git clone https://github.com/YourUsername/shared_sensor_code.git
   cd shared_sensor_code
   ```

2. **Create and Activate a Virtual Environment** (Recommended)

   ```bash
   python3 -m venv venv
   source venv/bin/activate
   ```

3. **Install Dependencies**

   ```bash
   pip install -r requirements.txt
   ```

---

## Chrony Configuration

To ensure precise time synchronization across your devices, you need to configure Chrony to use a local GPS PPS server and reliable NTP servers. This section guides you through updating the `/etc/chrony/chrony.conf` file.

### Updating `/etc/chrony/chrony.conf`

1. **Open the Chrony Configuration File**

   ```bash
   sudo nano /etc/chrony/chrony.conf
   ```

2. **Update the Configuration**

   Replace or update the server (XXX.XXX) entries to include your local GPS PPS server and Google NTP servers with specified polling intervals:

   ```plaintext
   # Welcome to the chrony configuration file. See chrony.conf(5) for more
   # information about usable directives.

   # Include configuration files found in /etc/chrony/conf.d.

   confdir /etc/chrony/conf.d

   # Use Local PPS GPS Server - REPLACE XXX.XXX BELOW
   server 192.168.XXX.XXX iburst prefer minpoll 4 maxpoll 4

   # Google NTP servers
   server time.google.com iburst minpoll 4 maxpoll 6
   server time2.google.com iburst minpoll 4 maxpoll 6
   server time3.google.com iburst minpoll 4 maxpoll 6
   server time4.google.com iburst minpoll 4 maxpoll 6

   # Use time sources from DHCP.
   sourcedir /run/chrony-dhcp

   # Use NTP sources found in /etc/chrony/sources.d.
   sourcedir /etc/chrony/sources.d

   # This directive specify the location of the file containing ID/key pairs for
   # NTP authentication.
   keyfile /etc/chrony/chrony.keys

   # This directive specify the file into which chronyd will store the rate
   # information.
   driftfile /var/lib/chrony/chrony.drift

   # Save NTS keys and cookies.
   ntsdumpdir /var/lib/chrony

   # Uncomment the following line to turn logging on.
   #log tracking measurements statistics

   # Log files location.
   logdir /var/log/chrony

   # Stop bad estimates upsetting machine clock.
   maxupdateskew 100.0

   # This directive enables kernel synchronisation (every 11 minutes) of the
   # real-time clock. Note that it can't be used along with the 'rtcfile' directive.
   rtcsync

   # Step the system clock instead of slewing it if the adjustment is larger than
   # one second, but only in the first three clock updates.
   makestep 1 3

   # Get TAI-UTC offset and leap seconds from the system tz database.
   # This directive must be commented out when using time sources serving
   # leap-smeared time.
   leapsectz right/UTC
   ```

   **Explanation of Changes:**

   - **Local GPS PPS Server:**
     - `server 192.168.68.126 iburst prefer minpoll 4 maxpoll 4`
       - **`192.168.68.126`**: Replace with the IP address of your local GPS PPS server.
       - **`iburst`**: Allows faster initial synchronization.
       - **`prefer`**: Marks this server as the preferred time source.
       - **`minpoll 4 maxpoll 4`**: Sets the polling interval to 16 seconds (2^4 seconds).

   - **Google NTP Servers:**
     - Added Google NTP servers as fallback options with polling intervals between 16 and 64 seconds.
     - **`minpoll 4 maxpoll 6`**: Allows polling intervals between 16 seconds (2^4) and 64 seconds (2^6).

3. **Save and Exit**

   - Press `Ctrl+O` to save the file.
   - Press `Ctrl+X` to exit the editor.

4. **Restart Chrony Service**

   ```bash
   sudo systemctl restart chrony
   ```

5. **Verify Chrony Sources**

   ```bash
   chronyc sources -v
   ```

   - Ensure that the local GPS PPS server is listed and marked as the preferred source (`^*`).

---

## Usage

### Importing the `TimeSync` Class

The `TimeSync` class is located in the `timesync.py` module within the repository. You can import it into your application as follows:

```python
from timesync import TimeSync
```

### Integrating `TimeSync` in Your Application

To use the `TimeSync` class within your application (e.g., a `CameraDataCollector` class), you should:

1. **Initialize the `TimeSync` Object** during the initialization of your application class.
2. **Pass Necessary Parameters** such as `sbc_id`, `central_server_url`, and `polling_interval`.
3. **Ensure Threading** is properly handled to allow `TimeSync` to run concurrently.

#### Example Initialization

```python
class CameraDataCollector:
    def __init__(self, ...):
        # Your existing initialization code

        # Initialize TimeSync object
        self.time_sync = TimeSync(
            sbc_id=self.sbc_id,
            central_server_url=self.central_server_url,
            polling_interval=15  # Set polling interval to 15 seconds
        )
```

### Starting the Time Synchronization Thread

Start the `TimeSync` thread within a method of your application class, such as a `start()` method. This allows the time synchronization to run in the background.

#### Example Start Method

```python
    def start(self):
        """Start the camera data collection and time synchronization."""
        if self.camera_index is None:
            print("No camera available to start.")
            return

        # Start the camera data collection thread
        self.camera_thread.start()

        # Start the time synchronization thread
        self.time_sync.start()

        print("Camera Data Collector started.")
```

- **Note**: Ensure that your main application continues running to keep the threads alive.

---

## Example Code

Below is a complete example demonstrating how to use the `TimeSync` class within a `CameraDataCollector` class, including threading to send timestamps and Chrony tracking data to the central server for synchronization analysis.

```python
import threading
import cv2
from timesync import TimeSync

class CameraDataCollector:
    def __init__(self, stop_event, sbc_id="SBC001", central_server_url='http://192.168.68.130:5000/receive_data',
                 data_collection_interval=10, data_directory='data', video_filename='video.avi',
                 delayed_start_timestamp=None, duration=None, camera_index=None):
        # Configuration
        self.sbc_id = sbc_id
        print(f"CameraDataCollector initialized with SBC ID: {self.sbc_id}")
        self.central_server_url = central_server_url
        self.data_collection_interval = data_collection_interval
        self.data_directory = data_directory
        self.video_filename = video_filename
        self.delayed_start_timestamp = delayed_start_timestamp
        self.duration = duration
        self.camera_index = camera_index
        self.stop_event = stop_event  # For graceful shutdown

        # Find a working camera index if none is provided
        if self.camera_index is None:
            self.camera_index = self.find_working_camera()
            if self.camera_index is None:
                print("No camera found. Exiting initialization.")
                return

        # Initialize TimeSync object
        self.time_sync = TimeSync(
            sbc_id=self.sbc_id,
            central_server_url=self.central_server_url,
            polling_interval=15  # Set polling interval to 15 seconds
        )

        # Camera data collection thread
        self.camera_thread = threading.Thread(target=self.collect_camera_data, daemon=True)

    def find_working_camera(self):
        """Tries different camera indices until it finds one that works, or returns None if none found."""
        for idx in range(0, 10):  # Adjust the range based on your devices
            cap = cv2.VideoCapture(idx)
            if cap.isOpened():
                print(f"Camera found at index {idx}.")
                cap.release()
                return idx
            cap.release()
        print("No camera found.")
        return None

    def collect_camera_data(self):
        """Collects camera data and processes it."""
        cap = cv2.VideoCapture(self.camera_index)
        while not self.stop_event.is_set():
            ret, frame = cap.read()
            if ret:
                # Process the frame or save it
                pass
            else:
                print("Failed to read frame from camera.")
                break
        cap.release()

    def start(self):
        """Start the camera data collection and time synchronization."""
        if self.camera_index is None:
            print("No camera available to start.")
            return

        # Start the camera data collection thread
        self.camera_thread.start()

        # Start the time synchronization thread
        self.time_sync.start()

        print("Camera Data Collector started.")

    def stop(self):
        """Stop the camera data collection and time synchronization."""
        self.stop_event.set()
        self.camera_thread.join()
        self.time_sync.stop()
        print("Camera Data Collector stopped.")

# Usage Example
if __name__ == "__main__":
    stop_event = threading.Event()
    camera_collector = CameraDataCollector(stop_event)
    camera_collector.start()

    try:
        while True:
            # Main application loop
            pass
    except KeyboardInterrupt:
        print("Stopping Camera Data Collector...")
        camera_collector.stop()
```

**Explanation:**

- **Initialization (`__init__`):**
  - The `TimeSync` object is initialized with the necessary parameters, including the `polling_interval` set to 15 seconds.
  - A camera data collection thread is prepared but not started yet.

- **`start()` Method:**
  - Starts both the camera data collection thread and the time synchronization thread.
  - Ensures that both processes run concurrently without blocking each other.

- **`stop()` Method:**
  - Gracefully stops the camera data collection and time synchronization by setting the `stop_event` and joining the threads.

- **Main Application Loop:**
  - Keeps the application running until a KeyboardInterrupt (Ctrl+C) is received.
  - Upon interruption, it calls the `stop()` method to clean up.

**Notes:**

- **Threading:**
  - The camera data collection and time synchronization run in separate threads.
  - The `daemon=True` parameter ensures that threads exit when the main program exits.

- **Graceful Shutdown:**
  - The `stop_event` is used to signal threads to stop, allowing for a clean shutdown.

- **Adjust Parameters:**
  - Replace `'http://192.168.68.130:5000/receive_data'` with the actual URL of your central server.
  - Modify `data_collection_interval` and other parameters as needed.

---

## Additional Notes

- **Central Server Setup:**
  - Ensure that the central server is running and configured to receive and process the timestamps and data sent by the `TimeSync` class and your application.
  - The server should have an endpoint matching the `central_server_url` provided.

- **Error Handling:**
  - Implement additional error handling as needed, especially for network operations and camera interactions.

- **Testing:**
  - Test the application in a controlled environment before deploying it in a production setting.

- **Dependencies:**
  - Make sure all dependencies are installed, including `opencv-python` if you're using camera functionalities.

- **Chrony Monitoring:**
  - Use `chronyc tracking` and `chronyc sources` to monitor Chrony's synchronization status.
  - Ensure that your device's time is accurately synchronized with the GPS PPS server.

---

## License

This project is licensed under the [MIT License](LICENSE).

---

**Disclaimer:** This example provides a basic structure for integrating time synchronization into your application using threading. You may need to adjust the code to fit your specific use case and handle any exceptions or edge cases relevant to your environment.

---

**Note:** The changes to the `/etc/chrony/chrony.conf` file are crucial for ensuring accurate time synchronization across your devices. By configuring Chrony to use a local GPS PPS server and reliable NTP servers, you enhance the precision of your time-sensitive applications.

---
