import sys
import os
import threading
import time
import cv2
import subprocess
from datetime import datetime
import signal
import argparse 

# Add the parent directory to sys.path
current_dir = os.path.dirname(os.path.abspath(__file__))
parent_dir = os.path.abspath(os.path.join(current_dir, '..', '..'))
sys.path.append(parent_dir)

from shared_sensor_code.TimeSync import TimeSync


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

        # TimeSync object
        self.time_sync = TimeSync(
            sbc_id=self.sbc_id,
            central_server_url=self.central_server_url,
            data_collection_interval=self.data_collection_interval
        )

        # Camera thread
        self.camera_thread = threading.Thread(target=self.collect_camera_data, daemon=True)

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
        """Start the camera data collection and time synchronization."""
        if self.camera_index is None:
            print("No camera available to start.")
            return

        self.camera_thread.start()
        self.time_sync.start()
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

    def stop(self):
        """Stop the camera data collection and time synchronization."""
        self.stop_event.set()
        self.camera_thread.join()
        self.time_sync.stop()

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