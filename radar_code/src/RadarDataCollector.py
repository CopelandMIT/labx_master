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
    filename=os.path.join(LOG_DIR, f"sensor_output_{datetime.now().strftime('%Y%m%d_%H%M%S%f')[:-3]}.log"),
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
        logging.info(f"Initializing RadarDataCollector with ID {self.deployed_sensor_id}")
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
        logging.info("Radar data synchronization started.")

    def stop(self):
        """Stop the radar data collection and time synchronization."""
        self.stop_event.set()
        self.time_sync.stop()
        print("Radar Data Collector stopped.")   
        logging.info("Radar data synchronization stopped.") 

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
                logging.info(f"Data saved to {self.base_filename}.")
            except Exception as e:
                print(f"Error saving data to file: {e}")
                logging.info(f"Data saved to {self.base_filename}.")
        else:
            print("No Chrony data to save.")
            logging.info(f"Data saved to {self.base_filename}.")

# -------------------------------------------------
# Radar Data Collection Code
# -------------------------------------------------

def data_saving_thread(base_filename, data_queue, stop_event, data_output_directory):
    """Thread that saves data from the queue to disk."""
    while not stop_event.is_set() or not data_queue.empty():
        try:
            buffer, frame_timestamps_list, buffer_duration_ms = data_queue.get(timeout=1)
            if buffer:
                # Use duration directly from the queue
                batch_start_time = frame_timestamps_list[0].strftime('%Y%m%d_%H%M%S%f')[:-3]
                filename = f"{base_filename}_{batch_start_time}_{buffer_duration_ms}ms.npz"

                # Save data
                file_path = os.path.join(data_output_directory, filename)
                np.savez(file_path, data=np.concatenate(buffer, axis=0), frame_timestamps_list=np.array(frame_timestamps_list))
                logging.info(f"Saved {len(buffer)} frames to {filename}")
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
    parser.add_argument('--central_server_url', type=str, default="http://192.168.68.130:5000/receive_data", help="Central Server Url for time sync monitoring")
    args = parser.parse_args()

    logging.info("Starting RadarDataCollector main function.")

    if args.central_server_url:
        logging.info(f"Central server URL: {args.central_server_url}")
        # Add logic to send data to the central server if necessary

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
        central_server_url=args.central_server_url,
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

        last_frame_perf = None
        total_frame_gap_duration = 0.0
        expected_frame_interval = config.frame_repetition_time_s
        frame_gap_threshold = expected_frame_interval * 1.05  # Reduced threshold for higher sensitivity

        buffer_start_perf = None  # Initialize variable to track the first frame's perf_counter in a buffer

        try:
            while not stop_event.is_set():
                logging.info("Collecting data...")
                elapsed_time = time.time() - start_time
                if elapsed_time >= capture_duration:
                    print(f"Reached recording duration of {capture_duration} seconds.")
                    logging.info(f"Reached recording duration of {capture_duration} seconds.")
                    break  # Exit the loop when duration is reached

                # Use perf_counter for precise timing and datetime for human-readable timestamps
                frame_start_perf = time.perf_counter()  # Precise start time for calculations
                frame_start_datetime = datetime.now()  # Human-readable start time

                frame_contents = device.get_next_frame()

                frame_end_perf = time.perf_counter()  # Precise end time for calculations
                frame_end_datetime = datetime.now()  # Human-readable end time

                # Calculate processing duration using perf_counter
                frame_processing_duration = frame_end_perf - frame_start_perf
                print(f"Frame started at {frame_start_datetime}, ended at {frame_end_datetime}, "
                    f"duration: {frame_processing_duration:.6f} seconds")
                logging.debug(f"Frame started at {frame_start_datetime}, ended at {frame_end_datetime}, "
                            f"duration: {frame_processing_duration:.6f} seconds")

                # Use the human-readable start time as the frame's timestamp
                timestamp = frame_start_datetime
                buffer.append(frame_contents)
                frame_timestamps_list.append(timestamp)

                # Set buffer_start_perf for the first frame in the buffer
                if buffer_start_perf is None:
                    buffer_start_perf = frame_start_perf

                # Calculate time between frames using perf_counter for precision
                if last_frame_perf is not None:
                    time_gap_between_frames = frame_start_perf - last_frame_perf
                    if time_gap_between_frames > frame_gap_threshold:
                        gap_duration = time_gap_between_frames - expected_frame_interval
                        total_frame_gap_duration += gap_duration
                        print(f"Gap detected: {gap_duration:.6f} seconds "
                            f"(time_gap_between_frames = {time_gap_between_frames:.6f} seconds)")
                        logging.info(f"Gap detected: {gap_duration:.6f} seconds "
                                    f"(time_gap_between_frames = {time_gap_between_frames:.6f} seconds)")

                # Update last frame time for both perf_counter and datetime
                last_frame_perf = frame_end_perf  # Assign the end time of the current frame

                if len(buffer) >= MAX_BUFFER_FRAMES:
                    # Calculate the buffer duration using perf_counter
                    batch_end_perf = time.perf_counter()  # Precise end time for the batch
                    buffer_duration_ms = int((batch_end_perf - buffer_start_perf) * 1000)  # Duration in milliseconds

                    print(f"Buffer full with {len(buffer)} frames. Duration: {buffer_duration_ms} ms.")
                    logging.info(f"Buffer full with {len(buffer)} frames. Duration: {buffer_duration_ms} ms.")

                    # Pass buffer duration and timestamps to the data queue
                    data_queue.put((buffer.copy(), frame_timestamps_list.copy(), buffer_duration_ms))
                    buffer.clear()
                    frame_timestamps_list.clear()
                    buffer_start_perf = None  # Reset buffer_start_perf for the next batch

        except KeyboardInterrupt:
            print("Interrupted. Cleaning up...")
            logging.error("Interrupted. Cleaning up...")
            stop_event.set()
        except Exception as e:
            print(f"An error occurred: {e}")
            logging.error(f"An error occurred: {e}")
            stop_event.set()

        # Handle any residual buffer data
        if buffer:
            batch_end_perf = time.perf_counter()  # Use perf_counter for precise timing
            buffer_duration_ms = int((batch_end_perf - buffer_start_perf) * 1000)  # Duration in milliseconds

            print(f"Saving residual buffer with {len(buffer)} frames. Duration: {buffer_duration_ms} ms.")
            logging.info(f"Saving residual buffer with {len(buffer)} frames. Duration: {buffer_duration_ms} ms.")

            # Pass residual data to the data queue
            data_queue.put((buffer.copy(), frame_timestamps_list.copy(), buffer_duration_ms))
            buffer.clear()
            frame_timestamps_list.clear()
            buffer_start_perf = None  # Reset buffer_start_perf


    #Signal threads to stop
    stop_event.set()
    logging.info("RadarDataCollector main function completed.")
    
    # Wait for the data saving thread to finish
    data_queue.join()
    saving_thread.join()
    radar_data_collector.stop()
    print("Data capture and saving complete.")
    print(f"Total gap time where data was not captured: {total_frame_gap_duration:.6f} seconds")


if __name__ == '__main__':
    main()