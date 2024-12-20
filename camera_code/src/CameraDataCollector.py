import sys
import os
import threading
import time
import cv2
import signal
import argparse
from datetime import datetime
import contextlib
import io

# Add the parent directory to sys.path
current_dir = os.path.dirname(os.path.abspath(__file__))
parent_dir = os.path.abspath(os.path.join(current_dir, '..', '..'))
sys.path.append(parent_dir)

from shared_sensor_code.TimeSync import TimeSync

# -------------------------------------------------
# Camera Data Collector Class
# -------------------------------------------------

VIDEO_CAPTURE_LENGTH = 15  # Default capture length for the whole session

class CameraDataCollector:
    def __init__(self, stop_event, deployed_sensor_id="CAM001", central_server_url='http://192.168.68.130:5000/receive_data',
                 sync_polling_interval=10, base_filename='default_name_video',
                 delayed_start_timestamp=None, capture_duration=None, camera_index=None,
                 batch_duration=10, disable_data_sync=False):
        # Configuration
        self.deployed_sensor_id = deployed_sensor_id
        print(f"CameraDataCollector initialized with SBC ID: {self.deployed_sensor_id}")
        self.central_server_url = central_server_url
        self.sync_polling_interval = sync_polling_interval
        self.base_filename = base_filename
        self.delayed_start_timestamp = delayed_start_timestamp
        self.capture_duration = capture_duration  # Total duration of the entire capture
        self.camera_index = camera_index
        self.batch_duration = batch_duration  # Duration of each batch (in seconds)
        self.stop_event = stop_event  # Store the stop_event for graceful shutdown
        self.disable_data_sync = disable_data_sync  # Flag to control data sync

        # Define the absolute path to the data directory
        self.data_output_directory = os.path.expanduser('~/labx_master/camera_code/data')

        # Create the data directory if it doesn't exist
        if not os.path.exists(self.data_output_directory):
            os.makedirs(self.data_output_directory)

        # Buffer for storing frames before saving
        self.frame_buffer = []
        self.buffer_lock = threading.Lock()

        # Find a working camera index if none is provided
        if self.camera_index is None:
            self.camera_index = self.find_working_camera()
            if self.camera_index is None:
                print("No camera found. Exiting initialization.")
                return

        # Only initialize TimeSync if data sync is not disabled
        if not self.disable_data_sync:
            self.time_sync = TimeSync(
                deployed_sensor_id=self.deployed_sensor_id,
                central_server_url=self.central_server_url,
                sync_polling_interval=self.sync_polling_interval
            )

        # Camera capture thread
        self.camera_thread = threading.Thread(target=self.collect_camera_data, daemon=True)
        # Saving thread
        self.save_thread = threading.Thread(target=self.save_buffered_data, daemon=True)

    def find_working_camera(self):
        """Tries different camera indices until it finds one that works, or returns None if none found."""
        for idx in range(0, 38):  # Adjust the range based on your devices
            # Suppress stdout and stderr temporarily
            with io.StringIO() as buf, contextlib.redirect_stdout(buf), contextlib.redirect_stderr(buf):
                cap = cv2.VideoCapture(idx)
                if cap.isOpened():
                    print(f"Camera found at index {idx}.")
                    cap.release()
                    return idx
                cap.release()
        print("No camera found.")
        return None

    def start(self):
        """Start the camera data collection and time synchronization (if enabled)."""
        if self.camera_index is None:
            print("No camera available to start.")
            return

        self.camera_thread.start()
        self.save_thread.start()

        # Start time sync only if it's not disabled
        if not self.disable_data_sync:
            self.time_sync.start()
            print("Time synchronization started.")

        print("Camera Data Collector started.")

    def collect_camera_data(self):
        """Collects camera video data and saves it to a buffer."""
        cap = cv2.VideoCapture(self.camera_index, cv2.CAP_V4L2)
        if not cap.isOpened():
            print(f"Cannot open camera at index {self.camera_index}")
            return

        # Wait until the specified timestamp to start data collection
        if self.delayed_start_timestamp is not None:
            while time.time() < self.delayed_start_timestamp and not self.stop_event.is_set():
                time.sleep(0.01)

        start_time = time.time()
        batch_start_time = start_time
        print(f"Camera data collection started at {start_time}")
        
        while not self.stop_event.is_set():
            ret, frame = cap.read()
            if ret:
                # Add frame to buffer
                with self.buffer_lock:
                    self.frame_buffer.append(frame)

                # Check if the batch duration has been reached
                current_time = time.time()
                if (current_time - batch_start_time) >= self.batch_duration:
                    print(f"Batch duration of {self.batch_duration} seconds reached. Saving batch.")
                    self.save_buffered_data(batch_start_time)
                    batch_start_time = current_time  # Reset the batch timer
            else:
                print("Error reading frame from camera.")
                break  # Error reading frame, exit loop

            # Check if total capture_duration has been reached
            if self.capture_duration is not None and (time.time() - start_time) >= self.capture_duration:
                print(f"Reached total recording capture_duration of {self.capture_duration} seconds.")
                break

        cap.release()
        print(f"Camera data collection stopped.")
        # Save any remaining frames in the buffer
        self.save_buffered_data(batch_start_time)
        print("Saved remaining frames.")

    def save_buffered_data(self, batch_start_time=None):
        """Saves the buffered frames to disk."""
        with self.buffer_lock:
            if len(self.frame_buffer) == 0:
                # Buffer is empty, nothing to save
                return

            # Create a new video file for the batch
            timestamp = datetime.now().strftime('%Y%m%d_%H%M%S%f')[:-3]
            output_filename = f'video_{timestamp}.avi'
            video_path = os.path.join(self.data_output_directory, output_filename)

            # Video writer setup
            fourcc = cv2.VideoWriter_fourcc(*'XVID')
            out = cv2.VideoWriter(video_path, fourcc, 30.0, (640, 480))  # Ensure correct FPS is used here

            # Write all frames from the buffer
            for frame in self.frame_buffer:
                out.write(frame)

            # Clear the buffer after saving
            self.frame_buffer.clear()
            out.release()
            print(f"Video batch saved to {video_path}")

    def stop(self):
        """Stop the camera data collection and time synchronization (if enabled)."""
        self.stop_event.set()
        self.camera_thread.join()
        self.save_thread.join()

        # Save any remaining frames in the buffer with the current timestamp as the batch_start_time
        current_time = time.time()
        self.save_buffered_data(current_time)

        if not self.disable_data_sync:
            self.time_sync.stop()
        print("Camera Data Collector stopped.")



# -------------------------------------------------
# Main Function
# -------------------------------------------------

def main():
    # Global event to signal threads to stop
    stop_event = threading.Event()

    # Argument parser setup
    parser = argparse.ArgumentParser(description='Camera data collection with optional NTP chrony metadata')
    parser.add_argument('--deployed_sensor_id', type=str, default='SBC001', help="Deployed sensor ID")
    parser.add_argument('--capture_duration', type=int, default=VIDEO_CAPTURE_LENGTH, help="Recording capture_duration in seconds")
    parser.add_argument('--base_filename', type=str, default='default_name_video', help="Base filename for the video data")
    parser.add_argument('--delayed_start_timestamp', type=float, default=None, help="Timestamp to delay start until")
    parser.add_argument('--sync_polling_interval', type=int, default=10, help="Interval to send Chrony data (seconds)")
    parser.add_argument('--camera_index', type=int, default=None, help="Camera index to use")
    parser.add_argument('--batch_duration', type=int, default=10, help="Duration of each video batch in seconds")
    parser.add_argument('--disable_data_sync', action='store_true', help="Disable data synchronization with central server, but allow capture to occur")
    args = parser.parse_args()

    if args._capture_duration <= 0:
        raise ValueError("Duration must be greater than 0 seconds.")

    # Initialize Camera Data Collector with stop_event
    camera_collector = CameraDataCollector(
        stop_event=stop_event,
        deployed_sensor_id=args.deployed_sensor_id,
        central_server_url='http://192.168.68.130:5000/receive_data',
        sync_polling_interval=args.sync_polling_interval,
        base_filename=args.base_filename,
        delayed_start_timestamp=args.delayed_start_timestamp,
        capture_duration=args.capture_duration,
        camera_index=args.camera_index,
        batch_duration=args.batch_duration,  # Use batch duration in seconds
        disable_data_sync=args.disable_data_sync  # Pass the flag for disabling data sync
    )

    camera_collector.start()

    # Register the signal handler for SIGTERM and SIGINT
    def handle_termination(signum, frame):
        print("Received termination signal. Cleaning up...")
        stop_event.set()
        camera_collector.camera_thread.join()
        camera_collector.save_thread.join()
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
    camera_collector.stop()  # Call stop to ensure graceful shutdown

    # Wait for threads to finish
    print("Data collection and saving complete.")

if __name__ == '__main__':
    main()