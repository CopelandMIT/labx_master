import os
import json
import subprocess
import threading
import queue
import time
from datetime import datetime
import sys
sys.path.append('/home/daniel/labx_master')
from shared_sensor_code.TimeSync import TimeSync

# Constants
ZED_CAPTURE_LENGTH = 20
ZED_EXECUTABLE_PATH = "/home/daniel/labx_master/ZED2i_code/build/ZED_Bodies_JSON_Export"
SAVE_DIR = "/home/daniel/labx_master/ZED2i_code/data"

class ZEDDataCollector:
    def __init__(self, stop_event, sbc_id="ZED001", central_server_url='http://192.168.68.130:5000/receive_data',
                 polling_interval=10, data_file='zed_data.json'):
        # Configuration
        self.sbc_id = sbc_id
        self.central_server_url = central_server_url
        self.polling_interval = polling_interval
        self.data_file = data_file
        self.stop_event = stop_event  # Store the stop_event for graceful shutdown

        # TimeSync object
        self.time_sync = TimeSync(
            sbc_id=self.sbc_id,
            central_server_url=self.central_server_url,
            polling_interval=self.polling_interval
        )

        # Data collection variables
        self.collected_data = []
        self.data_lock = threading.Lock()

    def start_time_sync(self):
        """Start the time synchronization."""
        self.time_sync.start()
        print("Time synchronization started.")

    def stop(self):
        """Stop time synchronization and any ongoing processes."""
        self.stop_event.set()
        self.time_sync.stop()
        print("ZED Data Collector stopped.")

    def run_zed_executable(self):
        """Run the ZED C++ executable for body tracking data collection."""
        try:
            output_file = os.path.join(SAVE_DIR, f"ZED_data_{datetime.now().strftime('%Y%m%d_%H%M%S')}.json")
            result = subprocess.run(
                [ZED_EXECUTABLE_PATH, output_file, str(ZED_CAPTURE_LENGTH)],
                check=True,
                capture_output=True,
                text=True
            )
            print("ZED executable output:", result.stdout)
            return output_file
        except subprocess.CalledProcessError as e:
            print(f"Error running ZED executable: {e.stderr}")
            return None

    def save_data_to_file(self):
        """Saves collected data to a file if needed."""
        with self.data_lock:
            data_to_save = self.collected_data.copy()
            self.collected_data.clear()
        if data_to_save:
            try:
                with open(self.data_file, 'a') as f:
                    for entry in data_to_save:
                        f.write(json.dumps(entry) + '\n')
                print(f"Data saved to {self.data_file}.")
            except Exception as e:
                print(f"Error saving data to file: {e}")

    def collect_data(self):
        """Main method to handle data collection and timing."""
        self.start_time_sync()
        print("ZED data collection started.")
        
        # Start the C++ executable for ZED data collection
        output_file = self.run_zed_executable()

        # Load and process data from output_file if it exists
        if output_file:
            with open(output_file, 'r') as f:
                data = json.load(f)
            with self.data_lock:
                self.collected_data.extend(data)
            self.save_data_to_file()

        # Stop time sync after collection is complete
        self.stop()

# Example usage
def main():
    stop_event = threading.Event()
    zed_collector = ZEDDataCollector(stop_event=stop_event)

    try:
        zed_collector.collect_data()
    except KeyboardInterrupt:
        print("Interrupted, stopping data collector.")
        stop_event.set()
        zed_collector.stop()

if __name__ == '__main__':
    main()
