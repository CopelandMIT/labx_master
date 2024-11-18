import argparse
import numpy as np
import signal
import sys
import time
import os
import threading
import queue
import json
from datetime import datetime
# import subprocess
# import requests
import logging

from ifxradarsdk import get_version_full
from ifxradarsdk.fmcw import DeviceFmcw
from ifxradarsdk.fmcw.types import FmcwSimpleSequenceConfig, FmcwSequenceChirp

# Add the parent directory to sys.path to import shared modules
current_dir = os.path.dirname(os.path.abspath(__file__))
parent_dir = os.path.abspath(os.path.join(current_dir, '..', '..'))
sys.path.append(parent_dir)

from shared_sensor_code.TimeSync import TimeSync

# -------------------------------------------------
# Logging Setup
# -------------------------------------------------
# Ensure the logs directory exists
LOG_DIR = "/home/dcope/labx_master/radar_code/logs"
os.makedirs(LOG_DIR, exist_ok=True)

logging.basicConfig(
    filename=os.path.join(LOG_DIR, "sensor_output.log"),
    level=logging.INFO,
    format="%(asctime)s - %(levelname)s - %(message)s"
)

logging.info("Starting Radar Data Collector script.")

# -------------------------------------------------
# Radar Data Collector Class with Chrony
# -------------------------------------------------


class RadarDataCollector:
    def __init__(self, stop_event, deployed_sensor_id="RAD001", central_server_url='http://192.168.68.130:5000/receive_data',
                 sync_polling_interval=10, base_filename='default_radar_data'):
        # Configuration
        self.deployed_sensor_id = deployed_sensor_id
        print(f"RadarDataCollector initialized with SBC ID: {self.deployed_sensor_id}")
        self.central_server_url = central_server_url
        self.sync_polling_interval = sync_polling_interval
        self.base_filename = base_filename
        self.stop_event = stop_event  # Store the stop_event for graceful shutdown
        # Data storage
        self.collected_data = []
        self.data_lock = threading.Lock()

        # TimeSync object
        self.time_sync = TimeSync(
            deployed_sensor_id=self.deployed_sensor_id,
            central_server_url=self.central_server_url,
            sync_polling_interval=self.sync_polling_interval
        )

    def start_time_sync(self):
        """Start the radar data collection and time synchronization."""
        self.time_sync.start()
        print("Radar Data Collector started.")

    def stop(self):
        """Stop the radar data collection and time synchronization."""
        self.stop_event.set()
        self.time_sync.stop()
        print("Radar Data Collector stopped.")    

    def save_data_to_file(self):
        """Saves collected data to a file (optional, if needed)."""
        with self.data_lock:
            data_to_save = self.collected_data.copy()
            self.collected_data.clear()
        if data_to_save:
            try:
                with open(self.base_filename, 'a') as f:
                    for entry in data_to_save:
                        f.write(json.dumps(entry) + '\n')
                print(f"Data saved to {self.base_filename}.")
            except Exception as e:
                print(f"Error saving data to file: {e}")
        else:
            print("No Chrony data to save.")

# -------------------------------------------------
# Radar Data Collection Code
# -------------------------------------------------

def data_saving_thread(base_filename, data_queue, stop_event, data_output_directory):
    """Thread that saves data from the queue to disk."""
    while not stop_event.is_set() or not data_queue.empty():
        try:
            buffer, frame_timestamps_list = data_queue.get(timeout=1)
            if buffer:
                filename = f"{base_filename}_{datetime.now().strftime('%Y%m%d_%H%M%S%f')[:-3]}.npz"
                file_path = os.path.join(data_output_directory, filename)
                np.savez(file_path, data=np.concatenate(buffer, axis=0), frame_timestamps_list=np.array(frame_timestamps_list))
                print(f"Saved {len(buffer)} frames to {filename}")
            data_queue.task_done()
        except queue.Empty:
            continue


def main():
    # Global event to signal threads to stop
    stop_event = threading.Event()

    # Argument parser setup
    parser = argparse.ArgumentParser(description='Derives raw data and saves to .npz file')
    parser.add_argument('-f', '--frate', type=float, default=1/1.28, help="Frame rate in Hz, default 5")
    parser.add_argument('--deployed_sensor_id', type=str, default='RAD001', help="Single Board Computer ID")
    parser.add_argument('--base_filename', type=str, default='radar_data.json', help="File to save radar data")
    parser.add_argument('--capture_duration', type=int, default=600, help="Duration for data capture in seconds")
    args = parser.parse_args()

    # Radar configuration setup
    config = FmcwSimpleSequenceConfig(
        frame_repetition_time_s=1 / args.frate,
        chirp_repetition_time_s=0.005,
        num_chirps=256,
        tdm_mimo=True,
        chirp=FmcwSequenceChirp(
            start_frequency_Hz=58_000_000_000,
            end_frequency_Hz=60_000_000_000,
            sample_rate_Hz=2e6,
            num_samples=1024,
            rx_mask=(1 << 3) - 1,  # Activates RX antennas 0 to 2
            tx_mask=1,             # Activates TX antenna 0
            tx_power_level=31,
            lp_cutoff_Hz=500000,
            hp_cutoff_Hz=80000,
            if_gain_dB=45,
        )
    )

    # Directory for saving radar data
    current_time = datetime.now().strftime('%Y%m%d_%H%M%S%f')[:-3]
    current_dir = os.path.dirname(os.path.abspath(__file__))
    parent_dir = os.path.abspath(os.path.join(current_dir, '..', '..'))
    data_output_directory = f'{parent_dir}/radar_code/data/radar_{current_time}'
    print(f"Data output directory: {data_output_directory}")
    os.makedirs(data_output_directory, exist_ok=True)

    # Extract configuration parameters into a dictionary
    sensor_config_dict = {
        'frame_repetition_time_s': config.frame_repetition_time_s,
        'chirp_repetition_time_s': config.chirp_repetition_time_s,
        'num_chirps': config.num_chirps,
        'tdm_mimo': config.tdm_mimo,
        'chirp': {
            'start_frequency_Hz': config.chirp.start_frequency_Hz,
            'end_frequency_Hz': config.chirp.end_frequency_Hz,
            'sample_rate_Hz': config.chirp.sample_rate_Hz,
            'num_samples': config.chirp.num_samples,
            'rx_mask': config.chirp.rx_mask,
            'tx_mask': config.chirp.tx_mask,
            'tx_power_level': config.chirp.tx_power_level,
            'lp_cutoff_Hz': config.chirp.lp_cutoff_Hz,
            'hp_cutoff_Hz': config.chirp.hp_cutoff_Hz,
            'if_gain_dB': config.chirp.if_gain_dB,
        }
    }

    # Save the configuration dictionary to a JSON file in data_output_directory
    config_file_path = os.path.join(data_output_directory, 'config.json')
    with open(config_file_path, 'w') as f:
        json.dump(sensor_config_dict, f, indent=4)
    print(f"Configuration saved to {config_file_path}")

    BUFFER_DURATION = 20  # seconds
    MAX_BUFFER_FRAMES = int(np.round(BUFFER_DURATION / config.frame_repetition_time_s))
    buffer = []
    frame_timestamps_list = []

    capture_duration = args.capture_duration  # Record for X seconds
    start_time = time.time()

    data_queue = queue.Queue()
    saving_thread = threading.Thread(target=data_saving_thread, args=(args.base_filename, data_queue, stop_event, data_output_directory))
    saving_thread.start()

    # Initialize Chrony Data Collector with stop_event
    radar_data_collector = RadarDataCollector(
        stop_event=stop_event,
        deployed_sensor_id=args.deployed_sensor_id,
        central_server_url='http://192.168.68.130:5000/receive_data',
        sync_polling_interval=10
    )

    radar_data_collector.start_time_sync()

    # Register the signal handler for SIGTERM and SIGINT
    def handle_termination(signum, frame):
        print("Received termination signal. Cleaning up...")
        stop_event.set()
        saving_thread.join()
        radar_data_collector.collection_thread.join()

    signal.signal(signal.SIGTERM, handle_termination)
    signal.signal(signal.SIGINT, handle_termination)

    with DeviceFmcw() as device:
        sequence = device.create_simple_sequence(config)
        device.set_acquisition_sequence(sequence)

        last_frame_time = None
        total_frame_gap_duration = 0.0
        expected_frame_interval = config.frame_repetition_time_s
        frame_gap_threshold = expected_frame_interval * 1.05  # Reduced threshold for higher sensitivity

        try:
            while not stop_event.is_set():
                elapsed_time = time.time() - start_time
                if elapsed_time >= capture_duration:
                    print(f"Reached recording duration of {capture_duration} seconds.")
                    break  # Exit the loop when duration is reached

                frame_start_time = time.perf_counter()
                frame_contents = device.get_next_frame()
                frame_end_time = time.perf_counter()
                timestamp = frame_end_time
                buffer.append(frame_contents)
                frame_timestamps_list.append(timestamp)

                # Calculate time between frames
                if last_frame_time is not None:
                    time_gap_between_frames = timestamp - last_frame_time
                    if time_gap_between_frames > frame_gap_threshold:
                        gap_duration = time_gap_between_frames - expected_frame_interval
                        total_frame_gap_duration += gap_duration
                        print(f"Gap detected: {gap_duration:.6f} seconds (time_gap_between_frames = {time_gap_between_frames:.6f} seconds)")
                last_frame_time = timestamp

                if len(buffer) >= MAX_BUFFER_FRAMES:
                    # Transfer buffer to data queue for saving
                    print(f"Buffer full with {len(buffer)} frames. Sending to saving thread.")
                    data_queue.put((buffer.copy(), frame_timestamps_list.copy()))
                    buffer.clear()
                    frame_timestamps_list.clear()

            # Save any remaining data
            if buffer:
                print(f"Saving remaining {len(buffer)} frames.")
                data_queue.put((buffer, frame_timestamps_list))

        except KeyboardInterrupt:
            print("Interrupted. Cleaning up...")
            stop_event.set()
        except Exception as e:
            print(f"An error occurred: {e}")
            stop_event.set()

    #Signal threads to stop
    stop_event.set()
    
    # Wait for the data saving thread to finish
    data_queue.join()
    saving_thread.join()
    radar_data_collector.stop()
    print("Data capture and saving complete.")
    print(f"Total gap time where data was not captured: {total_frame_gap_duration:.6f} seconds")


if __name__ == '__main__':
    main()