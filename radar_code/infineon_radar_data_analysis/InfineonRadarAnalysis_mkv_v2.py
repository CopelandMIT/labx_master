import json
import matplotlib.pyplot as plt
from scipy import signal, constants, fft
from scipy.optimize import curve_fit
#from pymkv import MKVFile, MKVTrack  # Add this import for MKV handling
import math
import re
import glob
import os
from time import time
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


def save_to_mkv(frames, file_path, fps=1.28, min_range_bin = None, max_range_bin = None):
    """
    Save radar data frames to an MKV file using imageio with Jet coloring.

    :param frames: List of numpy arrays representing frames.
    :param file_path: Path to save the MKV file.
    :param fps: Frames per second.
    """
    import numpy as np
    import matplotlib.pyplot as plt
    from matplotlib.cm import get_cmap

    if max_range_bin is not None and isinstance(max_range_bin, int): 
        # Focus on the first 50 range bins
        frames = [frame[:max_range_bin, :] for frame in frames]
        
    if min_range_bin is not None and isinstance(min_range_bin, int): 
        # Focus on the first 50 range bins
        frames = [frame[min_range_bin:, :] for frame in frames]

    # Get the jet colormap
    cmap = get_cmap("jet")

    # Normalize frames and apply colormap
    images = []
    for frame in frames:
        # Normalize the frame to the range [0, 1]
        frame_min = frame.min()
        frame_max = frame.max()
        frame_range = frame_max - frame_min
        if frame_range == 0:
            frame_normalized = np.zeros_like(frame, dtype=np.float32)
        else:
            frame_normalized = (frame - frame_min) / frame_range

        # Apply the colormap (jet)
        colored_frame = cmap(frame_normalized)[:, :, :3]  # Drop the alpha channel
        colored_frame = (colored_frame * 255).astype(np.uint8)  # Convert to uint8
        images.append(colored_frame)

    # Save frames as video
    imageio.mimwrite(file_path, images, fps=fps, codec="libx264", format="FFMPEG")
    print(f"Saved MKV file with jet coloring: {file_path}")


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
from time import time

def main():
    GUI_on = False  # Recording was made using Radar Fusion GUI or Raspberry Pi
    Adult_on = True  # Recording was made on an adult or infant

    # Folder containing radar data
    folder_path = "/Volumes/4TBLacie2/LabXData/Squat_tests_12112024/radar_28/squat_v2_45s"

    # Load data and configuration
    data, timestamps, config = load_data_and_config(folder_path, GUI_on=GUI_on)
    
    print(config)

    # Get data dimensions
    num_frames, num_rx_antennas, num_chirps_per_frame, num_samples_per_chirp = data.shape
    print(f"Original data shape: {data.shape}")

    # Total number of chirps
    total_chirps = num_frames * num_chirps_per_frame     
    print(f"Total chirps: {total_chirps} across {num_rx_antennas} antenna(s)")

    # Desired new number of chirps per frame
    new_num_chirps_per_frame = 32
    new_total_frames = total_chirps // new_num_chirps_per_frame
    remaining_chirps = total_chirps % new_num_chirps_per_frame

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

    # Create directory for MKV files if not exists
    mkv_output_dir = f"{folder_path}/radar_mkvs_{int(time())}/"
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
    
    MIN_RANGE_BIN = 10
    MAX_RANGE_BIN = 100
    fps = ((1/config['frame_repetition_time_s']) * config['num_chirps']/new_num_chirps_per_frame)
    print(f"Calculated FPS: {fps:.2f}")

    # Calculate and print expected video duration
    video_duration_seconds = num_frames / fps
    print(f"Expected video duration: {video_duration_seconds:.2f} seconds")

    for channel_idx in range(num_channels):
        frames = []
        for frame_idx in range(num_frames):
            rdm = rdm_all_channels[channel_idx, frame_idx, :, :]
            frames.append(rdm)

        # Save the frames as an MKV file for this antenna
        mkv_filename = os.path.join(mkv_output_dir, f"antenna_{channel_idx}.mkv")
        save_to_mkv(frames, mkv_filename, fps=fps, min_range_bin=MIN_RANGE_BIN, max_range_bin=MAX_RANGE_BIN)
        print(f"Radar data saved to MKV file for antenna {channel_idx} with filename {mkv_filename}.")
        
if __name__ == "__main__":
    main()