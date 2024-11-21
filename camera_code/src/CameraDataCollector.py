import sys
import os
import threading
import time
import cv2
import signal
import argparse
from datetime import datetime
from queue import Queue, Empty


# Add the parent directory to sys.path
current_dir = os.path.dirname(os.path.abspath(__file__))
parent_dir = os.path.abspath(os.path.join(current_dir, '..', '..'))
sys.path.append(parent_dir)

from shared_sensor_code.TimeSync import TimeSync

import logging

LOG_DIR = "/home/pi/labx_master/camera_code/logs"
os.makedirs(LOG_DIR, exist_ok=True)

logging.basicConfig(
    filename=os.path.join(LOG_DIR, f"camera_collector_log_{datetime.now().strftime('%Y%m%d_%H%M%S%f')[:-3]}.log"),
    level=logging.INFO,
    format="%(asctime)s - %(levelname)s - %(message)s"
)

logging.info("Starting Camera Data Collector script.")


# -------------------------------------------------
# Camera Data Collector Class
# -------------------------------------------------

VIDEO_CAPTURE_LENGTH = 30  # Default capture length for the whole session
FRAME_RATE = 30

class CameraDataCollector:
    def __init__(self, stop_event, deployed_sensor_id="CAM001", central_server_url='http://192.168.68.130:5000/receive_data',
                 sync_polling_interval=10, base_filename='default_name_video',
                 delayed_start_timestamp=None, capture_duration=None, camera_index=None,
                 batch_duration=10, disable_data_sync=False):
        # Configuration
        self.deployed_sensor_id = deployed_sensor_id
        logging.info(f"CameraDataCollector initialized with SBC ID: {self.deployed_sensor_id}")
        self.central_server_url = central_server_url
        self.sync_polling_interval = sync_polling_interval
        self.base_filename = base_filename
        self.delayed_start_timestamp = delayed_start_timestamp
        self.capture_duration = capture_duration
        self.camera_index = camera_index
        self.batch_duration = batch_duration
        self.stop_event = stop_event
        self.disable_data_sync = disable_data_sync

        self.data_output_directory = os.path.expanduser(f'~/labx_master/camera_code/data/{self.base_filename}')
        os.makedirs(self.data_output_directory, exist_ok=True)

        self.data_queue = Queue()  # Queue for producer-consumer model

        if self.camera_index is None:
            self.camera_index = self.find_working_camera()
            if self.camera_index is None:
                logging.error("No camera found. Exiting initialization.")
                return

        if not self.disable_data_sync:
            self.time_sync = TimeSync(
                deployed_sensor_id=self.deployed_sensor_id,
                central_server_url=self.central_server_url,
                sync_polling_interval=self.sync_polling_interval
            )

        self.camera_thread = threading.Thread(target=self.collect_camera_data, daemon=True)
        self.save_thread = threading.Thread(target=self.save_buffered_data, daemon=True)


    def find_working_camera(self):
        for idx in range(0, 38):  # Adjust based on expected camera range
            cap = cv2.VideoCapture(idx)
            if cap.isOpened():
                logging.info(f"Camera found at index {idx}.")
                cap.release()
                return idx
            cap.release()
        logging.error("No working camera found.")
        return None

    def start(self):
        if self.camera_index is None:
            logging.error("No camera available to start.")
            return

        self.camera_thread.start()
        self.save_thread.start()

        if not self.disable_data_sync:
            self.time_sync.start()
            logging.info("Time synchronization started.")

        logging.info("Camera Data Collector started.")

    def collect_camera_data(self):
        cap = cv2.VideoCapture(self.camera_index, cv2.CAP_V4L2)
        if not cap.isOpened():
            logging.error(f"Cannot open camera at index {self.camera_index}")
            return

        # Handle delayed start if specified
        if self.delayed_start_timestamp is not None:
            while time.time() < self.delayed_start_timestamp and not self.stop_event.is_set():
                time.sleep(0.01)

        capture_start_time = time.time()
        batch_start_time = capture_start_time
        batch_start_datetime = datetime.now()
        logging.info(f"Camera data collection started at {batch_start_datetime}")

        frame_buffer = []  # Local buffer for this batch

        while not self.stop_event.is_set():
            elapsed_capture_time = time.time() - capture_start_time
            if self.capture_duration and elapsed_capture_time >= self.capture_duration:
                logging.info(f"Reached total capture duration of {self.capture_duration} seconds.")
                break

            ret, frame = cap.read()
            if ret:
                frame_buffer.append(frame)
                elapsed_batch_time = time.time() - batch_start_time
                if elapsed_batch_time >= self.batch_duration:
                    logging.info(f"Batch duration of {self.batch_duration} seconds reached. Sending batch to queue.")
                    self.data_queue.put((frame_buffer.copy(), batch_start_datetime))  # Add to queue
                    frame_buffer.clear()  # Clear local buffer
                    batch_start_time = time.time()
                    batch_start_datetime = datetime.now()
            else:
                logging.error("Error reading frame from camera.")
                break

        cap.release()
        logging.info("Camera data collection stopped.")

        # Enqueue remaining frames
        if frame_buffer:
            logging.info("Enqueuing remaining frames.")
            self.data_queue.put((frame_buffer.copy(), batch_start_datetime))

    def save_buffered_data(self):
        while not self.stop_event.is_set() or not self.data_queue.empty():
            try:
                frame_buffer, batch_start_datetime = self.data_queue.get(timeout=1)
                if frame_buffer:
                    self._save_to_file(frame_buffer, batch_start_datetime)
                    self.data_queue.task_done()
            except Empty:
                continue

    def _save_to_file(self, frame_buffer, batch_start_datetime):
        # Format batch start time and calculate duration
        batch_start_timestamp = batch_start_datetime.strftime('%Y%m%d_%H%M%S%f')[:-3]
        duration_ms = len(frame_buffer) * (1000 // FRAME_RATE)

        # Format filename
        output_filename = f'{self.base_filename}_{batch_start_timestamp}_{duration_ms}ms.avi'
        video_path = os.path.join(self.data_output_directory, output_filename)

        # Save video
        fourcc = cv2.VideoWriter_fourcc(*'XVID')
        out = cv2.VideoWriter(video_path, fourcc, FRAME_RATE, (640, 480))
        for frame in frame_buffer:
            out.write(frame)
        out.release()

        logging.info(f"Video batch saved to {video_path}")
        print(f"Video batch saved to {video_path}")

    def stop(self):
        self.stop_event.set()
        self.camera_thread.join()
        self.save_thread.join()

        # Save any remaining frames in the queue
        while not self.data_queue.empty():
            frame_buffer, batch_start_datetime = self.data_queue.get()
            self._save_to_file(frame_buffer, batch_start_datetime)
            self.data_queue.task_done()

        if not self.disable_data_sync:
            self.time_sync.stop()

        logging.info("Camera Data Collector stopped.")


# -------------------------------------------------
# Main Function
# -------------------------------------------------

def main():
    # Global event to signal threads to stop
    stop_event = threading.Event()

      # Argument parser setup
    parser = argparse.ArgumentParser(description='Camera data collection with optional NTP chrony metadata')
    parser.add_argument('--deployed_sensor_id', type=str, default='CAM001', help="Deployed sensor ID")
    parser.add_argument('--capture_duration', type=int, default=VIDEO_CAPTURE_LENGTH, help="Recording capture_duration in seconds")
    parser.add_argument('--base_filename', type=str, default='default_name_video', help="Base filename for the video data")
    parser.add_argument('--delayed_start_timestamp', type=float, default=None, help="Timestamp to delay start until")
    parser.add_argument('--sync_polling_interval', type=int, default=10, help="Interval to send Chrony data (seconds)")
    parser.add_argument('--camera_index', type=int, default=None, help="Camera index to use")
    parser.add_argument('--batch_duration', type=int, default=10, help="Duration of each video batch in seconds")
    parser.add_argument('--disable_data_sync', action='store_true', help="Disable data synchronization with central server, but allow capture to occur")
    parser.add_argument('--central_server_url', type=str, required=False , help="Central Server Url for time sync monitoring")
    args = parser.parse_args()

    if args.central_server_url:
        logging.info(f"Central server URL: {args.central_server_url}")
        # Add logic to send data to the central server if necessary

    if args.capture_duration <= 0:
        raise ValueError("Duration must be greater than 0 seconds.")

    # Initialize Camera Data Collector with stop_event
    camera_collector = CameraDataCollector(
        stop_event=stop_event,
        deployed_sensor_id=args.deployed_sensor_id,
        central_server_url=args.central_server_url,
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