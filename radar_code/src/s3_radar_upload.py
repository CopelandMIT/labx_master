import argparse
import pprint
import numpy as np
import signal
import sys
import time
import json
import os
import threading
from datetime import datetime, timedelta
import boto3
from botocore.exceptions import NoCredentialsError

from ifxradarsdk import get_version_full
from ifxradarsdk.fmcw import DeviceFmcw
from ifxradarsdk.fmcw.types import create_dict_from_sequence, FmcwSimpleSequenceConfig, FmcwMetrics, FmcwSequenceChirp

# Global duration for recording
RECORDING_DURATION = 20  # seconds
print(f"Recording for {RECORDING_DURATION} seconds in each thread.")

# AWS S3 bucket information
S3_BUCKET_NAME = 'test-lab-in-a-box-storage'

# Initialize AWS S3 client
s3_client = boto3.client('s3')

# Global event to signal threads to stop
stop_event = threading.Event()
# -------------------------------------------------
# Radar Helpers
# -------------------------------------------------

def upload_to_s3(file_path):
    """Upload the file to S3 and delete it locally after successful upload."""
    try:
        s3_client.upload_file(file_path, S3_BUCKET_NAME, os.path.basename(file_path))
        print(f"Uploaded {file_path} to S3 bucket {S3_BUCKET_NAME}.")
        os.remove(file_path)
        print(f"Local file {file_path} deleted after upload.")
    except NoCredentialsError:
        print("Credentials not available for S3 upload.")
    except Exception as e:
        print(f"Failed to upload {file_path} to S3: {e}")

def save_data_to_buffer(data, buffer):
    """Append data to the buffer."""
    buffer.append(data)

def save_buffer_to_file(buffer, folder_path):
    """Save buffer data to an .npy file."""
    filename = f"data_{datetime.now().strftime('%Y%m%d_%H%M%S%f')[:-3]}.npy"
    file_path = os.path.join(folder_path, filename)
    print(f"Shape of buffer: {np.concatenate(buffer, axis=0).shape}")
    np.save(file_path, np.concatenate(buffer, axis=0))
    print(f"Saved {len(buffer)} frames to {filename}")
    buffer.clear()
    return file_path



def radar_data_recording(thread_id):
    """Record radar data in 20-second intervals."""
    folder_name = f"DATA_{datetime.now().strftime('%Y%m%d_%H%M%S%f')[:-3]}_thread{thread_id}"
    full_folder_path = os.path.join('/home/dcope_rpi5_32bit/LabX/data/radar', folder_name)
    os.makedirs(full_folder_path, exist_ok=True)

    parser = argparse.ArgumentParser(description='''Derives raw data and saves to .npy file''')
    parser.add_argument('-f', '--frate', type=float, default=1/1.28, help="frame rate in Hz, default 5")
    args = parser.parse_args()

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
            rx_mask= (1 << 3) - 1,
            tx_mask=1,
            tx_power_level=31,
            lp_cutoff_Hz=500000,
            hp_cutoff_Hz=80000,
            if_gain_dB=45,
        )
    )

    buffer = []
    BUFFER_SIZE = int(np.round(20 / config.frame_repetition_time_s))  # 20 seconds of data

    with DeviceFmcw() as device:
        sequence = device.create_simple_sequence(config)
        device.set_acquisition_sequence(sequence)

        start_time = time.time()
        while not stop_event.is_set() and (time.time() - start_time) < RECORDING_DURATION:
            try:
                frame_contents = device.get_next_frame()
                save_data_to_buffer(frame_contents, buffer)
            except Exception as e:
                print(f"Error capturing frame: {e}")

            if len(buffer) >= BUFFER_SIZE:
                file_path = save_buffer_to_file(buffer, full_folder_path)
                upload_thread = threading.Thread(target=upload_to_s3, args=(file_path,))
                upload_thread.start()
                upload_thread.join()

        if buffer:
            file_path = save_buffer_to_file(buffer, full_folder_path)
            upload_thread = threading.Thread(target=upload_to_s3, args=(file_path,))
            upload_thread.start()
            upload_thread.join()

    print(f"Thread {thread_id} completed recording and uploading.")


def handle_termination(signum, frame):
    """Handle termination signals for graceful shutdown."""
    print("Received termination signal. Cleaning up...")
    stop_event.set()
    sys.exit(0)

# Register the signal handler for SIGTERM (termination signal) and SIGINT (interrupt signal)
signal.signal(signal.SIGTERM, handle_termination)
signal.signal(signal.SIGINT, handle_termination)

if __name__ == '__main__':
    thread_id = 0
    try:
        while True:
            if stop_event.is_set():
                break

            radar_thread = threading.Thread(target=radar_data_recording, args=(thread_id,))
            radar_thread.start()
            radar_thread.join()

            thread_id += 1

    except KeyboardInterrupt:
        print("Interrupted. Cleaning up...")
        stop_event.set()
    except Exception as e:
        print(f"An error occurred: {e}")
        stop_event.set()