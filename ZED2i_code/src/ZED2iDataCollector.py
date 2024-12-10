import os
import sys
import json
import subprocess
import threading
import time
from datetime import datetime
import logging
import argparse
import socket

# Add the parent directory to sys.path
current_dir = os.path.dirname(os.path.abspath(__file__))
parent_dir = os.path.abspath(os.path.join(current_dir, '..', '..'))
sys.path.append(parent_dir)

from shared_sensor_code.TimeSync import TimeSync

# Constants
ZED_EXECUTABLE_PATH = "/home/daniel/labx_master/ZED2i_code/build/ZED_Bodies_JSON_Export"
SAVE_DIR = "/home/daniel/labx_master/ZED2i_code/data"

# Set up logging
LOG_DIR = "/home/daniel/labx_master/ZED2i_code/logs"
os.makedirs(LOG_DIR, exist_ok=True)

logging.basicConfig(
    filename=os.path.join(LOG_DIR, f"zed_collector_log_{datetime.now().strftime('%Y%m%d_%H%M%S%f')[:-3]}.log"),
    level=logging.INFO,
    format="%(asctime)s - %(levelname)s - %(message)s"
)

logging.info("Starting ZED Data Collector script.")

def get_local_ip():
    """Attempt to retrieve the local machine's IP address in a reliable manner."""
    try:
        # This approach attempts to connect to a known public address and then retrieves the local IP used for that route.
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
            s.connect(("8.8.8.8", 80))
            return s.getsockname()[0]
    except Exception as e:
        logging.warning(f"Could not determine local IP using UDP method: {e}. Falling back to hostname method.")
        # Fallback method
        try:
            return socket.gethostbyname(socket.gethostname())
        except Exception as e2:
            logging.error(f"Could not determine local IP using hostname: {e2}. Defaulting to localhost.")
            return "127.0.0.1"

def parse_arguments():
    parser = argparse.ArgumentParser(description='ZED 2i Data Collector')
    local_ip = get_local_ip()
    default_central_server_url = f'http://{local_ip}:5000/receive_data'
    parser.add_argument('--base_filename', type=str, default='zed_default_data', help='Base filename for the output data')
    parser.add_argument('--capture_duration', type=int, default=None, help='Total capture duration in seconds')
    parser.add_argument('--central_server_url', type=str, default=default_central_server_url, help='URL of the central server to send data')
    parser.add_argument('--no-gui', default=True, action='store_true', help='Run the ZED executable in headless mode without GUI')
    parser.add_argument('--batch_duration', type=int, default=300, help='Duration of each data batch in seconds')
    return parser.parse_args()

class ZEDDataCollector:
    def __init__(
        self, 
        stop_event, 
        deployed_sensor_id="ZED001", 
        central_server_url='http://127.0.0.1:5000/receive_data',
        sync_polling_interval=10, 
        capture_duration=None,
        base_filename='zed_default_data',
        no_gui=True,
        batch_duration=300
    ):
        # Configuration
        self.deployed_sensor_id = deployed_sensor_id
        logging.info(f"ZEDDataCollector initialized with SBC ID: {self.deployed_sensor_id}")
        self.central_server_url = central_server_url
        self.sync_polling_interval = sync_polling_interval
        self.capture_duration = capture_duration
        logging.info(f"Total Capture Duration: {self.capture_duration}")
        self.base_filename = base_filename
        self.stop_event = stop_event
        self.no_gui = no_gui
        self.batch_duration = batch_duration

        # TimeSync object
        self.time_sync = TimeSync(
            deployed_sensor_id=self.deployed_sensor_id,
            central_server_url=self.central_server_url,
            sync_polling_interval=self.sync_polling_interval
        )

        # Thread for data collection
        self.collect_thread = threading.Thread(target=self.collect_zed_data, daemon=True)

        # Create data output directory
        self.data_output_directory = os.path.join(SAVE_DIR, self.base_filename)
        os.makedirs(self.data_output_directory, exist_ok=True)
        logging.info(f"Data output directory created: {self.data_output_directory}")

    def start(self):
        """Start data collection thread."""
        self.collect_thread.start()
        self.time_sync.start()
        logging.info("Time synchronization started.")
        logging.info("ZED Data Collector started.")

    def collect_zed_data(self):
        """Collect data from the ZED camera in batches."""
        logging.info("Starting ZED data collection.")
        capture_start_time = time.time()

        while not self.stop_event.is_set():
            # Calculate remaining capture time
            if self.capture_duration:
                elapsed_capture_time = time.time() - capture_start_time
                remaining_capture_time = self.capture_duration - elapsed_capture_time
                if remaining_capture_time <= 0:
                    logging.info(f"Reached total capture duration of {self.capture_duration} seconds.")
                    break
                current_batch_duration = min(self.batch_duration, remaining_capture_time)
            else:
                current_batch_duration = self.batch_duration

            # Run ZED executable for current_batch_duration seconds
            output_file = self.run_zed_executable(int(current_batch_duration))

            if output_file:
                logging.info(f"Data saved to {output_file}")
                print(f"Data saved to {output_file}")

            if self.capture_duration and (time.time() - capture_start_time) >= self.capture_duration:
                logging.info(f"Reached total capture duration of {self.capture_duration} seconds.")
                break

        logging.info("ZED data collection stopped.")

    def run_zed_executable(self, duration):
        """
        Run the ZED C++ executable for body tracking data collection.

        Args:
            duration (int): Duration in seconds for which to run the ZED executable.

        Returns:
            str or None: Path to the output file if successful, else None.
        """
        try:
            timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
            output_file = os.path.join(
                self.data_output_directory,
                f"{self.base_filename}_{timestamp}_{duration}s.json"
            )
            logging.info(f"Running ZED executable: {ZED_EXECUTABLE_PATH}")

            # Construct the command with optional --no-gui flag
            command = [
                ZED_EXECUTABLE_PATH,
                output_file,
                str(duration)
            ]

            if self.no_gui:
                command.insert(1, "--no-gui")  # Insert after executable path

            logging.info(f"Executing command: {' '.join(command)}")

            result = subprocess.run(
                command,
                check=True,
                capture_output=True,
                text=True
            )
            logging.info("ZED executable completed successfully.")
            logging.info(f"ZED executable output: {result.stdout}")
            return output_file
        except subprocess.CalledProcessError as e:
            logging.error(f"Error running ZED executable: {e.stderr}")
            return None
        except FileNotFoundError:
            logging.error(f"ZED executable not found at path: {ZED_EXECUTABLE_PATH}")
            return None
        except Exception as e:
            logging.error(f"Unexpected error running ZED executable: {str(e)}")
            return None

    def stop(self):
        """Stop time synchronization and any ongoing processes."""
        self.stop_event.set()
        self.collect_thread.join()
        self.time_sync.stop()
        logging.info("ZED Data Collector stopped.")

def main():
    args = parse_arguments()
    stop_event = threading.Event()
    zed_collector = ZEDDataCollector(
        stop_event=stop_event,
        base_filename=args.base_filename,
        capture_duration=args.capture_duration,
        central_server_url=args.central_server_url,
        no_gui=args.no_gui,
        batch_duration=args.batch_duration
    )

    zed_collector.start()

    try:
        while zed_collector.collect_thread.is_alive():
            zed_collector.collect_thread.join(timeout=1)
    except KeyboardInterrupt:
        logging.warning("Interrupted by user, stopping data collector.")
        stop_event.set()
        zed_collector.stop()
    finally:
        # Ensure that everything is stopped
        stop_event.set()
        zed_collector.stop()

if __name__ == '__main__':
    main()
