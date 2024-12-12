import json
import matplotlib.pyplot as plt
from scipy import signal, constants, fft
from scipy.optimize import curve_fit
from pymkv import MKVFile, MKVTrack  # Add this import for MKV handling
import math
import re
import glob
import os

from FFT_spectrum import *
from RangeAlgorithmv1 import *


import os
import json
import numpy as np
from scipy import constants
import imageio

# -------------------------------------------------
# Helper Functions
# -------------------------------------------------

def load_data_and_config(folder_path, GUI_on):
    """
    Load radar data and device configuration based on GUI_on flag.
    """
    # Load .npz files
    npz_files = [file for file in os.listdir(folder_path) if file.endswith('.npz')]
    npz_abs_file_paths = [os.path.join(folder_path, file) for file in npz_files]
    print("NPZ Files Found:", npz_abs_file_paths)
    data_list = []
    timestamps_list = []

    for npz_abs_file_path in npz_abs_file_paths:
        print(f"Loading file: {npz_abs_file_path}")
        npz_data = np.load(npz_abs_file_path, allow_pickle=True)
        print("Keys in npz file:", list(npz_data.keys()))  # Debugging
        data_part = npz_data['data']
        timestamps_part = npz_data['frame_timestamps_list']
        data_list.append(data_part)
        timestamps_list.append(timestamps_part)

    data = np.concatenate(data_list, axis=0)  # Concatenate along the frame axis
    timestamps = np.concatenate(timestamps_list, axis=0)

    if GUI_on:
        # Load device configuration for GUI-based recording
        config_file = os.path.join(folder_path, 'config.json')
        if not os.path.exists(config_file):
            raise FileNotFoundError(f"Configuration file not found: {config_file}")
        with open(config_file, 'r') as f:
            device_config = json.load(f)
    else:
        # Load device configuration for Raspberry Pi-based recording
        # Assuming the configuration is stored in 'device_config.json'
        config_file = os.path.join(folder_path, 'config.json')
        if not os.path.exists(config_file):
            raise FileNotFoundError(f"Configuration file not found: {config_file}")
        with open(config_file, 'r') as f:
            device_config = json.load(f)

    return data, timestamps, device_config


def save_to_mkv(frames, file_path, fps=1.2*4):
    """
    Save radar data frames to an MKV file using imageio.

    :param frames: List of numpy arrays representing frames.
    :param file_path: Path to save the MKV file.
    :param fps: Frames per second.
    """
    import numpy as np  # Ensure NumPy is imported
    # Calculate global min and max
    global_min = np.min([frame.min() for frame in frames])
    global_max = np.max([frame.max() for frame in frames])
    global_range = global_max - global_min

    # Normalize frames and convert to uint8 images
    images = []
    for frame in frames:
        if global_range == 0:
            # Avoid division by zero if frames have constant values
            frame_normalized = np.zeros_like(frame, dtype=np.uint8)
        else:
            frame_normalized = np.uint8(255 * (frame - global_min) / global_range)
        images.append(frame_normalized)

    # Save frames as video
    imageio.mimwrite(file_path, images, fps=fps, codec='libx264', format='FFMPEG')
    print(f"Saved MKV file: {file_path}")

def range_doppler_processing(dataCubes):
    """
    Processes each frame in the dataCube for each channel to generate Range-Doppler Maps.

    Args:
        dataCubes (np.ndarray): The raw data cubes to be processed.

    Returns:
        np.ndarray: Array of processed Range-Doppler Maps for each channel.
    """
    n_channels, n_frames, n_bins, n_doppler = dataCubes.shape
    rdm_all_channels = []

    # Define a window function for the range and Doppler dimensions
    range_window = np.hanning(n_bins)
    doppler_window = np.hanning(n_doppler)

    for channel_idx in range(n_channels):
        rdm_list = []
        for frame_idx in range(n_frames):
            # Extract current data for the frame and channel
            current_data = dataCubes[channel_idx, frame_idx, :, :]

            # Apply the Hanning window function
            windowed_data = np.outer(range_window, doppler_window) * current_data

            # Apply 2D FFT and shift
            rdm = np.fft.fft2(windowed_data)
            rdm = np.fft.fftshift(rdm, axes=1)  # Shift along the Doppler axis (second axis in Python)

            # Take the absolute value
            rdm = np.abs(rdm)
            
            # Normalize the data before logarithmic scaling
            rdm_max = np.max(rdm)
            rdm_min = np.min(rdm)
            rdm = (rdm - rdm_min) / (rdm_max - rdm_min + 1e-3)  # Avoid division by zero
            
            # Log scaling - apply log1p for numerical stability
            rdm = np.log1p(rdm)

            # Slice the RDM to remove the mirrored part (keep only one half)
            # Assuming the mirrored part is along the Range axis (axis 0)
            half_index = rdm.shape[0] // 2
            rdm = rdm[:half_index, :]

            # Append to the list for the current channel
            rdm_list.append(rdm)

        # Append the result for the current channel
        rdm_all_channels.append(rdm_list)

    return np.array(rdm_all_channels)


# -------------------------------------------------
# Main Processing
# -------------------------------------------------

# -------------------------------------------------
# Main Processing
# -------------------------------------------------
def main():
    GUI_on = False  # Recording was made using Radar Fusion GUI or Raspberry Pi
    Adult_on = True  # Recording was made on an adult or infant

    # Folder containing radar data
    folder_path = "/Volumes/4TBLacie2/LabXData/Squat_tests_12112024/radar_28/squat_v2_45s"

    # Load data and configuration
    data, timestamps, config = load_data_and_config(folder_path, GUI_on=GUI_on)

    # Get data dimensions
    num_frames, num_rx_antennas, num_chirps_per_frame, num_samples_per_chirp = data.shape
    print(f"Original data shape: {data.shape}")

    # Total number of chirps
    total_chirps = num_frames * num_chirps_per_frame * num_rx_antennas
    print(f"Total chirps: {total_chirps}")

    # Desired new number of chirps per frame
    new_num_chirps_per_frame = 64
    new_total_frames = total_chirps // (new_num_chirps_per_frame * num_rx_antennas)
    remaining_chirps = total_chirps % (new_num_chirps_per_frame * num_rx_antennas)

    if remaining_chirps != 0:
        print(f"Discarding {remaining_chirps} chirps to reshape data.")
        data = data.reshape(-1, num_samples_per_chirp)
        data = data[:-remaining_chirps, :]
    else:
        data = data.reshape(-1, num_samples_per_chirp)

    # Reshape into the new data shape
    data = data.reshape(new_total_frames, num_rx_antennas, new_num_chirps_per_frame, num_samples_per_chirp)
    print(f"Reshaped data shape: {data.shape}")

    # Update the number of frames and chirps per frame
    num_frames = new_total_frames
    num_chirps_per_frame = new_num_chirps_per_frame

    # Create directory for MKV files if not exists
    mkv_output_dir = f"{folder_path}/radar_mkvs_v4/"
    os.makedirs(mkv_output_dir, exist_ok=True)

    # Transpose and reshape data
    # New data shape after reshaping: (num_frames, num_rx_antennas, num_chirps_per_frame, num_samples_per_chirp)
    # We need to rearrange it to (num_rx_antennas, num_frames, num_samples_per_chirp, num_chirps_per_frame)
    dataCubes = np.transpose(data, (1, 0, 3, 2))  # Shape: (num_rx_antennas, num_frames, num_samples_per_chirp, num_chirps_per_frame)
    print(f"DataCubes shape after transpose: {dataCubes.shape}")

    # Perform range-Doppler processing
    rdm_all_channels = range_doppler_processing(dataCubes)

    # Save the Range-Doppler Maps as MKV files for each antenna
    num_channels = rdm_all_channels.shape[0]
    num_frames = rdm_all_channels.shape[1]

    for channel_idx in range(num_channels):
        frames = []
        for frame_idx in range(num_frames):
            rdm = rdm_all_channels[channel_idx, frame_idx, :, :]
            frames.append(rdm)

        # Save the frames as an MKV file for this antenna
        mkv_filename = os.path.join(mkv_output_dir, f"antenna_{channel_idx}.mkv")
        save_to_mkv(frames, mkv_filename)
        print(f"Radar data saved to MKV file for antenna {channel_idx} with filename {mkv_filename}.")


if __name__ == "__main__":
    main()