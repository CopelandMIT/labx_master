# Shared Sensor Code

This repository contains code for synchronizing sensor data timestamps across multiple devices using a central server. The `TimeSync` class allows your application to send timestamps to the central server for synchronization analysis. By using threading, you can send timestamps in the background without blocking your main application logic.

This README provides instructions on how to import and use the `TimeSync` class within your application, specifically in the context of a `CameraDataCollector` class.

## Table of Contents

- [Prerequisites](#prerequisites)
- [Installation](#installation)
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
- **Required Python Packages** (listed in `requirements.txt`):
  - `requests`
  - `threading` (part of the standard library)
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

## Usage

### Importing the `TimeSync` Class

The `TimeSync` class is located in the `timesync.py` module within the repository. You can import it into your application as follows:

```python
from timesync import TimeSync
```

### Integrating `TimeSync` in Your Application

To use the `TimeSync` class within your application (e.g., a `CameraDataCollector` class), you should:

1. **Initialize the `TimeSync` Object** during the initialization of your application class.
2. **Pass Necessary Parameters** such as `sbc_id`, `central_server_url`, and `data_collection_interval`.
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
            data_collection_interval=self.data_collection_interval
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

Below is a complete example demonstrating how to use the `TimeSync` class within a `CameraDataCollector` class, including threading to send timestamps to the central server for synchronization analysis.

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
            data_collection_interval=self.data_collection_interval
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
  - The `TimeSync` object is initialized with the necessary parameters.
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

---

## License

This project is licensed under the [MIT License](LICENSE).

---

**Disclaimer:** This example provides a basic structure for integrating time synchronization into your application using threading. You may need to adjust the code to fit your specific use case and handle any exceptions or edge cases relevant to your environment.
