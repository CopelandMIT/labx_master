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
import subprocess
import requests

from ifxradarsdk import get_version_full
from ifxradarsdk.fmcw import DeviceFmcw
from ifxradarsdk.fmcw.types import FmcwSimpleSequenceConfig, FmcwSequenceChirp

# Add the parent directory to sys.path to import shared modules
current_dir = os.path.dirname(os.path.abspath(__file__))
parent_dir = os.path.abspath(os.path.join(current_dir, '..', '..'))
sys.path.append(parent_dir)

from shared_sensor_code.TimeSync import TimeSync


# -------------------------------------------------
# Radar Data Collector Class with Chrony
# -------------------------------------------------

RADAR_CAPTURE_LENGTH = 300

class RadarDataCollector:
    def __init__(self, stop_event, sbc_id="SBC002", central_server_url='http://192.168.68.130:5000/receive_data',
                 polling_interval=10, data_file='radar_data.json'):
        # Configuration
        self.sbc_id = sbc_id
        print(f"RadarDataCollector initialized with SBC ID: {self.sbc_id}")
        self.central_server_url = central_server_url
        self.polling_interval = polling_interval
        self.data_file = data_file
        self.stop_event = stop_event  # Store the stop_event for graceful shutdown
        # Data storage
        self.collected_data = []
        self.data_lock = threading.Lock()

        # TimeSync object
        self.time_sync = TimeSync(
            sbc_id=self.sbc_id,
            central_server_url=self.central_server_url,
            polling_interval=self.polling_interval
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
                with open(self.data_file, 'a') as f:
                    for entry in data_to_save:
                        f.write(json.dumps(entry) + '\n')
                print(f"Data saved to {self.data_file}.")
            except Exception as e:
                print(f"Error saving data to file: {e}")
        else:
            print("No Chrony data to save.")

# -------------------------------------------------
# Radar Data Collection Code
# -------------------------------------------------

def data_saving_thread(data_queue, stop_event, SAVE_DIR):
    """Thread that saves data from the queue to disk."""
    while not stop_event.is_set() or not data_queue.empty():
        try:
            buffer, timestamps = data_queue.get(timeout=1)
            if buffer:
                filename = f"data_{datetime.now().strftime('%Y%m%d_%H%M%S%f')[:-3]}.npz"
                file_path = os.path.join(SAVE_DIR, filename)
                np.savez(file_path, data=np.concatenate(buffer, axis=0), timestamps=np.array(timestamps))
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
    parser.add_argument('--sbc_id', type=str, default='SBC002', help="Single Board Computer ID")
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
    SAVE_DIR = f'{parent_dir}/radar_code/data/radar_{current_time}'
    print(f"save dir: {SAVE_DIR}")
    os.makedirs(SAVE_DIR, exist_ok=True)

    # Extract configuration parameters into a dictionary
    config_dict = {
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

    # Save the configuration dictionary to a JSON file in SAVE_DIR
    config_file_path = os.path.join(SAVE_DIR, 'config.json')
    with open(config_file_path, 'w') as f:
        json.dump(config_dict, f, indent=4)
    print(f"Configuration saved to {config_file_path}")

    BUFFER_DURATION = 20  # seconds
    BUFFER_SIZE = int(np.round(BUFFER_DURATION / config.frame_repetition_time_s))
    buffer = []
    timestamps = []

    RECORDING_DURATION = RADAR_CAPTURE_LENGTH  # Record for X seconds
    start_time = time.time()

    data_queue = queue.Queue()
    saving_thread = threading.Thread(target=data_saving_thread, args=(data_queue, stop_event, SAVE_DIR))
    saving_thread.start()

    # Initialize Chrony Data Collector with stop_event
    radar_data_collector = RadarDataCollector(
        stop_event=stop_event,
        sbc_id=args.sbc_id,
        central_server_url='http://192.168.68.130:5000/receive_data',
        polling_interval=10
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
        total_gap_time = 0.0
        expected_frame_interval = config.frame_repetition_time_s
        gap_threshold = expected_frame_interval * 1.05  # Reduced threshold for higher sensitivity

        try:
            while not stop_event.is_set():
                elapsed_time = time.time() - start_time
                if elapsed_time >= RECORDING_DURATION:
                    print(f"Reached recording duration of {RECORDING_DURATION} seconds.")
                    break  # Exit the loop when duration is reached

                frame_start_time = time.perf_counter()
                frame_contents = device.get_next_frame()
                frame_end_time = time.perf_counter()
                timestamp = frame_end_time
                buffer.append(frame_contents)
                timestamps.append(timestamp)

                # Calculate time between frames
                if last_frame_time is not None:
                    delta_time = timestamp - last_frame_time
                    if delta_time > gap_threshold:
                        gap_duration = delta_time - expected_frame_interval
                        total_gap_time += gap_duration
                        print(f"Gap detected: {gap_duration:.6f} seconds (delta_time = {delta_time:.6f} seconds)")
                last_frame_time = timestamp

                if len(buffer) >= BUFFER_SIZE:
                    # Transfer buffer to data queue for saving
                    print(f"Buffer full with {len(buffer)} frames. Sending to saving thread.")
                    data_queue.put((buffer.copy(), timestamps.copy()))
                    buffer.clear()
                    timestamps.clear()

            # Save any remaining data
            if buffer:
                print(f"Saving remaining {len(buffer)} frames.")
                data_queue.put((buffer, timestamps))

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
    print(f"Total gap time where data was not captured: {total_gap_time:.6f} seconds")


if __name__ == '__main__':
    main()