import os
import sys
import json
import subprocess
import threading
import time
from datetime import datetime
import logging
import argparse  # Import argparse for command-line arguments

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

def parse_arguments():
    parser = argparse.ArgumentParser(description='ZED 2i Data Collector')
    parser.add_argument('--base_filename', type=str, default='zed_default_data', help='Base filename for the output data')
    parser.add_argument('--capture_duration', type=int, default=20, help='Capture duration in seconds')
    parser.add_argument('--central_server_url', type=str, default='http://192.168.68.130:5000/receive_data', help='URL of the central server to send data')
    parser.add_argument('--no-gui', default=True, action='store_true', help='Run the ZED executable in headless mode without GUI')
    return parser.parse_args()

class ZEDDataCollector:
    def __init__(
        self, 
        stop_event, 
        deployed_sensor_id="ZED001", 
        central_server_url='http://192.168.68.130:5000/receive_data',
        sync_polling_interval=10, 
        capture_duration=20, 
        base_filename='zed_default_data',
        no_gui=True  # Default to FTrue
    ):
        """
        Initialize the ZEDDataCollector.

        Args:
            stop_event (threading.Event): Event to signal stopping the collector.
            deployed_sensor_id (str): Identifier for the deployed sensor.
            central_server_url (str): URL of the central server to send data.
            sync_polling_interval (int): Interval in seconds for time synchronization polling.
            capture_duration (int): Duration in seconds for data capture.
            base_filename (str): Base filename for the output data.
            no_gui (bool): Flag to run the executable in headless mode without GUI.
        """
        # Configuration
        self.deployed_sensor_id = deployed_sensor_id
        logging.info(f"ZEDDataCollector initialized with SBC ID: {self.deployed_sensor_id}")
        self.central_server_url = central_server_url
        self.sync_polling_interval = sync_polling_interval
        self.capture_duration = capture_duration
        logging.info(f"Capture Duration: {self.capture_duration}")
        self.base_filename = base_filename
        self.stop_event = stop_event  # Store the stop_event for graceful shutdown
        self.no_gui = no_gui  # Store the no_gui flag

        # TimeSync object
        self.time_sync = TimeSync(
            deployed_sensor_id=self.deployed_sensor_id,
            central_server_url=self.central_server_url,
            sync_polling_interval=self.sync_polling_interval
        )

        # Data collection variables
        self.collected_data = []
        self.data_lock = threading.Lock()

        os.makedirs(SAVE_DIR, exist_ok=True)
        logging.info(f"Save directory created: {SAVE_DIR}")

    def start_time_sync(self):
        """Start the time synchronization."""
        logging.info("Starting time synchronization.")
        self.time_sync.start()
        logging.info("Time synchronization started.")

    def stop(self):
        """Stop time synchronization and any ongoing processes."""
        self.stop_event.set()
        self.time_sync.stop()
        logging.info("ZED Data Collector stopped.")

    def run_zed_executable(self):
        """
        Run the ZED C++ executable for body tracking data collection.

        Returns:
            str or None: Path to the output file if successful, else None.
        """
        try:
            timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
            output_file = os.path.join(
                SAVE_DIR, 
                f"{self.base_filename}_{timestamp}_{self.capture_duration}s.json"
            )
            logging.info(f"Running ZED executable: {ZED_EXECUTABLE_PATH}")

            # Construct the command with optional --no-gui flag
            command = [
                ZED_EXECUTABLE_PATH,
                output_file,
                str(self.capture_duration)
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

    def save_data_to_file(self):
        """Saves collected data to a file if needed."""
        with self.data_lock:
            data_to_save = self.collected_data.copy()
            self.collected_data.clear()

        if data_to_save:
            try:
                with open(self.base_filename, 'a') as f:
                    for entry in data_to_save:
                        f.write(json.dumps(entry) + '\n')
                logging.info(f"Data saved to {self.base_filename}.")
            except Exception as e:
                logging.error(f"Error saving data to file: {e}")

    def collect_data(self):
        """Main method to handle data collection and timing."""
        logging.info("Starting ZED data collection.")
        self.start_time_sync()

        # Start the C++ executable for ZED data collection
        output_file = self.run_zed_executable()

        # Load and process data from output_file if it exists
        if output_file:
            try:
                with open(output_file, 'r') as f:
                    data = json.load(f)
                logging.info(f"Data loaded from {output_file}.")
                with self.data_lock:
                    self.collected_data.extend(data)
                self.save_data_to_file()
            except Exception as e:
                logging.error(f"Error processing data from {output_file}: {e}")

        # Stop time sync after collection is complete
        self.stop()

def main():
    args = parse_arguments()
    stop_event = threading.Event()
    zed_collector = ZEDDataCollector(
        stop_event=stop_event,
        base_filename=args.base_filename,
        capture_duration=args.capture_duration,
        central_server_url=args.central_server_url,
        no_gui=args.no_gui  # Pass the no_gui argument
    )

    try:
        zed_collector.collect_data()
    except KeyboardInterrupt:
        logging.warning("Interrupted by user, stopping data collector.")
        stop_event.set()
        zed_collector.stop()

if __name__ == '__main__':
    main()
