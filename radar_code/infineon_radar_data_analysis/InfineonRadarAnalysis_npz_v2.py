import json
import os
import glob
import numpy as np
import matplotlib.pyplot as plt
from scipy import constants
from pymkv import MKVFile, MKVTrack
from FFT_spectrum import *  # Assuming these are necessary and available
from RangeAlgorithmv1 import *
import imageio

# -------------------------------------------------
# Helper Functions
# -------------------------------------------------

def circle_fit_by_taubin(XY):
    """
    Circle fit by Taubin method.
    Input:  XY(n, 2) - array of coordinates of n points x(i) = XY[i, 0], y(i) = XY[i, 1]
    Output: [a, b, R] - fitting circle center (a, b) and radius R
    """
    n = XY.shape[0]
    centroid = np.mean(XY, axis=0)

    # Centering data
    Xi = XY[:, 0] - centroid[0]
    Yi = XY[:, 1] - centroid[1]
    Zi = Xi**2 + Yi**2

    Mxy = np.mean(Xi * Yi)
    Mxx = np.mean(Xi**2)
    Myy = np.mean(Yi**2)
    Mxz = np.mean(Xi * Zi)
    Myz = np.mean(Yi * Zi)
    Mzz = np.mean(Zi**2)

    # Coefficients of the characteristic polynomial
    Mz = Mxx + Myy
    Cov_xy = Mxx * Myy - Mxy**2
    A3 = 4 * Mz
    A2 = -3 * Mz**2 - Mzz
    A1 = Mzz * Mz + 4 * Cov_xy * Mz - Mxz**2 - Myz**2 - Mz**3
    A0 = Mxz**2 * Myy + Myz**2 * Mxx - Mzz * Cov_xy - 2 * Mxz * Myz * Mxy + Mz**2 * Cov_xy
    A22 = A2 + A2
    A33 = A3 + A3 + A3

    # Newton's method starting at x = 0
    xnew = 0
    ynew = 1e+20
    epsilon = 1e-12
    IterMax = 20

    for _ in range(IterMax):
        yold = ynew
        ynew = A0 + xnew * (A1 + xnew * (A2 + xnew * A3))

        if abs(ynew) > abs(yold):
            print('Newton-Taubin goes wrong direction: |ynew| > |yold|')
            xnew = 0
            break

        Dy = A1 + xnew * (A22 + xnew * A33)
        xold = xnew
        xnew = xold - ynew / Dy

        if abs((xnew - xold) / xnew) < epsilon:
            break

        if _ >= IterMax - 1:
            print('Newton-Taubin will not converge')
            xnew = 0

        if xnew < 0:
            print(f'Newton-Taubin negative root: x = {xnew}')
            xnew = 0

    # Computing the circle parameters
    DET = xnew**2 - xnew * Mz + Cov_xy
    Center = [(Mxz * (Myy - xnew) - Myz * Mxy) / DET / 2,
              (Myz * (Mxx - xnew) - Mxz * Mxy) / DET / 2]
    a, b = Center + centroid
    R = np.sqrt(Center[0]**2 + Center[1]**2 + Mz)

    return [a, b, R]


def load_data_and_config(folder_path):
    """
    Load radar data and device configuration.
    """
    # Load .npz files
    npz_files = [file for file in os.listdir(folder_path) if file.endswith('.npz')]
    print("Data files:", npz_files)
    data_list = []
    timestamps_list = []
    
    for npz_file in npz_files:
        file_path = os.path.join(folder_path, npz_file)
        try:
            # Enable pickle to load object arrays
            npz_data = np.load(file_path, allow_pickle=True)
            print("Keys in npz file:", list(npz_data.keys()))  # Debugging

            # Access data and timestamps
            data_part = npz_data['data']
            timestamps_part = npz_data['frame_timestamps_list']  # Update key if necessary

            data_list.append(data_part)
            timestamps_list.append(timestamps_part)
        except Exception as e:
            print(f"Error loading file {npz_file}: {e}")

    data = np.concatenate(data_list, axis=0)
    timestamps = np.concatenate(timestamps_list, axis=0)

    # Load config.json
    config_file = os.path.join(folder_path, 'config.json')
    if not os.path.exists(config_file):
        raise FileNotFoundError(f"Configuration file not found: {config_file}")
    with open(config_file, 'r') as f:
        config = json.load(f)
        print("Loaded configuration:")
        print(json.dumps(config, indent=4))

    return data, timestamps, config



# def save_to_mkv(frames, file_path, fps=12):
#     """
#     Save radar data frames to an MKV file using imageio.

#     :param frames: List of numpy arrays representing frames.
#     :param file_path: Path to save the MKV file.
#     :param fps: Frames per second.
#     """
#     import numpy as np  # Ensure NumPy is imported
#     # Normalize frames and convert to uint8 images
#     images = []
#     for frame in frames:
#         frame_min = frame.min()
#         frame_range = np.ptp(frame)
#         if frame_range == 0:
#             # Avoid division by zero if frame has constant values
#             frame_normalized = np.zeros_like(frame, dtype=np.uint8)
#         else:
#             frame_normalized = np.uint8(255 * (frame - frame_min) / frame_range)
#         images.append(frame_normalized)

#     # Save frames as video
#     imageio.mimwrite(file_path, images, fps=fps, codec='libx264', format='FFMPEG')
#     print(f"Saved MKV file: {file_path}")


def save_to_mkv(frames, file_path, fps=1.2):
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
def main():
    GUI_on = False  # Recording was made using Radar Fusion GUI or Raspberry Pi
    Adult_on = True  # Recording was made on an adult or infant

    # Folder containing radar data
    folder_path = "/home/dcope/labx_master/radar_code/data/test_capture_V91"

    # Load data and configuration
    data, timestamps, config = load_data_and_config(folder_path)

    # Extract parameters from config
    chirp_config = config['chirp']
    frame_repetition_time_s = config['frame_repetition_time_s']
    chirp_repetition_time_s = config['chirp_repetition_time_s']
    start_frequency_Hz = chirp_config['start_frequency_Hz']
    end_frequency_Hz = chirp_config['end_frequency_Hz']
    sample_rate_Hz = chirp_config['sample_rate_Hz']
    num_chirps_per_frame = config['num_chirps']
    num_samples_per_chirp = chirp_config['num_samples']
    num_rx_antennas = bin(chirp_config['rx_mask']).count('1')

    # Get data dimensions
    num_frames = data.shape[0]
    num_rx_antennas_in_data = data.shape[1]
    if num_rx_antennas != num_rx_antennas_in_data:
        print(f"Warning: Number of RX antennas in config ({num_rx_antennas}) does not match data ({num_rx_antennas_in_data})")
        num_rx_antennas = num_rx_antennas_in_data

    FMCW_Bandwidth = end_frequency_Hz - start_frequency_Hz
    range_resolution = constants.c / (2 * FMCW_Bandwidth)
    range_bins = np.linspace(0, range_resolution * num_samples_per_chirp / 2, num_samples_per_chirp, endpoint=False)

    # Filter range bins based on min and max distance
    min_distance = 0.5
    max_distance = 1.5
    min_bin_index = np.searchsorted(range_bins, min_distance)
    max_bin_index = np.searchsorted(range_bins, max_distance)
    filtered_range_bins = range_bins[min_bin_index:max_bin_index]

    # Threshold for standard deviation to identify motion frames
    motion_threshold = 100

    # Initialize bandpass frequencies
    if Adult_on:
        lowcut_bp = 42 / 60  # Convert bpm to Hz
        highcut_bp = 180 / 60
    else:
        lowcut_bp = 85 / 60
        highcut_bp = 220 / 60

    # Initialize window
    window_in_sec = 10
    frames_in_window = int(window_in_sec / frame_repetition_time_s)
    num_segments = num_frames - frames_in_window

    # Time vector
    ts = chirp_repetition_time_s
    fs = 1 / ts
    time = np.arange(0, ts * frames_in_window * num_chirps_per_frame, ts)

    # Create directory for MKV files if not exists
    mkv_output_dir = f"{folder_path}/radar_mkvs_v2/"
    os.makedirs(mkv_output_dir, exist_ok=True)

    # Reshape and prepare data for range-Doppler processing
    # Expected data shape: (n_channels, n_frames, n_bins, n_doppler)

    # Transpose and reshape data
    # Original data shape: (num_frames, num_rx_antennas, num_chirps_per_frame, num_samples_per_chirp)
    # We need to rearrange it to (num_rx_antennas, num_frames, num_samples_per_chirp, num_chirps_per_frame)

    dataCubes = np.transpose(data, (1, 0, 3, 2))  # Now shape is (num_rx_antennas, num_frames, num_samples_per_chirp, num_chirps_per_frame)

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