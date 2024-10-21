import threading
import time
import os
import cv2
import requests
import subprocess
import argparse
from datetime import datetime
import signal

# -------------------------------------------------
# Camera Data Collector Class
# -------------------------------------------------

VIDEO_CAPTURE_LENGTH = 3600

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
        self.stop_event = stop_event  # Store the stop_event for graceful shutdown

        # Find a working camera index if none is provided
        if self.camera_index is None:
            self.camera_index = self.find_working_camera()
            if self.camera_index is None:
                print("No camera found. Exiting initialization.")
                return
        
        # Threads
        self.camera_thread = threading.Thread(target=self.collect_camera_data, daemon=True)
        self.chrony_collection_thread = threading.Thread(target=self.collect_chrony_data, daemon=True)

    def find_working_camera(self):
        """Tries different camera indices until it finds one that works, or returns None if none found."""
        for idx in range(0, 38):  # Adjust the range based on your devices
            cap = cv2.VideoCapture(idx)
            if cap.isOpened():
                print(f"Camera found at index {idx}.")
                cap.release()
                return idx
            cap.release()
        print("No camera found.")
        return None

    def start(self):
        """Start the camera data collection and chrony data collection threads."""
        if self.camera_index is None:
            print("No camera available to start.")
            return
        
        self.camera_thread.start()
        self.chrony_collection_thread.start()
        print("Camera Data Collector started.")

    def collect_camera_data(self):
        """Collects camera video data and saves it."""
        cap = cv2.VideoCapture(self.camera_index)
        if not cap.isOpened():
            print(f"Cannot open camera at index {self.camera_index}")
            return
        fourcc = cv2.VideoWriter_fourcc(*'XVID')
        if not os.path.exists(self.data_directory):
            os.makedirs(self.data_directory)
        video_path = os.path.join(self.data_directory, self.video_filename)
        out = cv2.VideoWriter(video_path, fourcc, 20.0, (640, 480))

        # Wait until the specified timestamp to start data collection
        if self.delayed_start_timestamp is not None:
            while time.time() < self.delayed_start_timestamp and not self.stop_event.is_set():
                time.sleep(0.01)

        start_time = time.time()
        print(f"Camera data collection started at {start_time}")
        while not self.stop_event.is_set():
            # Check if duration has been reached
            if self.duration is not None and (time.time() - start_time) >= self.duration:
                print(f"Reached recording duration of {self.duration} seconds.")
                break

            ret, frame = cap.read()
            if ret:
                out.write(frame)
            else:
                print("Error reading frame from camera.")
                break  # Error reading frame, exit loop

        cap.release()
        out.release()
        print(f"Camera data collection stopped. Video saved to {video_path}")

    def collect_chrony_data(self):
        """Collects Chrony data at specified intervals and sends each entry to the server."""
        while not self.stop_event.is_set():
            try:
                # Fetch tracking information
                tracking_result = subprocess.run(['chronyc', 'tracking'], stdout=subprocess.PIPE, text=True)
                tracking_output = tracking_result.stdout

                # Get current time
                current_time = time.time()

                # Prepare payload
                payload = {
                    'sbc_id': self.sbc_id,
                    'data': {
                        'timestamp': current_time,
                        'chronyc_output': tracking_output
                    }
                }

                # Send data to server
                response = requests.post(self.central_server_url, json=payload)
                if response.status_code != 200:
                    print(f"Failed to send data: {response.text}")
                else:
                    print(f"Successfully sent Chrony data to server at {datetime.fromtimestamp(current_time)}")
            except Exception as e:
                print(f"Error collecting or sending Chrony data: {e}")

            # Wait for the specified interval or until stop_event is set
            if self.stop_event.wait(self.data_collection_interval):
                break  # Exit if stop_event is set

# -------------------------------------------------
# Main Function
# -------------------------------------------------

def main():
    # Global event to signal threads to stop
    stop_event = threading.Event()

    # Argument parser setup
    parser = argparse.ArgumentParser(description='Camera data collection with NTP chrony metadata')
    parser.add_argument('--sbc_id', type=str, default='SBC001', help="Single Board Computer ID")
    parser.add_argument('--duration', type=int, default=VIDEO_CAPTURE_LENGTH, help="Recording duration in seconds")
    parser.add_argument('--data_directory', type=str, default='data', help="Directory to save video data")
    parser.add_argument('--video_filename', type=str, default=None, help="Filename for the video data")
    parser.add_argument('--delayed_start_timestamp', type=float, default=None, help="Timestamp to delay start until")
    parser.add_argument('--chrony_interval', type=int, default=10, help="Interval to send Chrony data (seconds)")
    parser.add_argument('--camera_index', type=int, default=None, help="Camera index to use")
    args = parser.parse_args()

    # If video_filename is not provided, create one based on current time
    if args.video_filename is None:
        current_time = datetime.now().strftime('%Y%m%d_%H%M%S%f')[:-3]
        video_filename = f'video_{current_time}.avi'
    else:
        video_filename = args.video_filename

    # Initialize Camera Data Collector with stop_event
    camera_collector = CameraDataCollector(
        stop_event=stop_event,
        sbc_id=args.sbc_id,
        central_server_url='http://192.168.68.130:5000/receive_data',
        data_collection_interval=args.chrony_interval,
        data_directory=args.data_directory,
        video_filename=video_filename,
        delayed_start_timestamp=args.delayed_start_timestamp,
        duration=args.duration,
        camera_index=args.camera_index
    )

    camera_collector.start()

    # Register the signal handler for SIGTERM and SIGINT
    def handle_termination(signum, frame):
        print("Received termination signal. Cleaning up...")
        stop_event.set()
        camera_collector.camera_thread.join()
        camera_collector.chrony_collection_thread.join()
        print("All threads have been terminated.")

    signal.signal(signal.SIGTERM, handle_termination)
    signal.signal(signal.SIGINT, handle_termination)

    # Wait for the camera thread to finish
    try:
        camera_collector.camera_thread.join()
    except KeyboardInterrupt:
        print("Interrupted by user. Cleaning up...")
        stop_event.set()

    # Signal threads to stop
    stop_event.set()

    # Wait for threads to finish
    camera_collector.chrony_collection_thread.join()
    print("Data collection and saving complete.")

if __name__ == '__main__':
    main()