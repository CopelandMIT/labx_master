import os
import numpy as np
import matplotlib.pyplot as plt
import imageio
from scipy import constants
import json

# -------------------------------------------------
# Helper Functions
# -------------------------------------------------

def load_radar_data(folder_path):
    """
    Load radar data from .npz files in the specified folder.

    Args:
        folder_path (str): Path to the folder containing .npz files.

    Returns:
        data (np.ndarray): Radar data array.
        timestamps (np.ndarray): Timestamps array.
        device_config (dict): Configuration dictionary.
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
        print("Keys in npz file:", list(npz_data.keys()))
        data_part = npz_data['data']
        timestamps_part = npz_data['frame_timestamps_list']
        data_list.append(data_part)
        timestamps_list.append(timestamps_part)

    data = np.concatenate(data_list, axis=0)
    timestamps = np.concatenate(timestamps_list, axis=0)

    # Load device configuration
    config_file = os.path.join(folder_path, 'config.json')
    if not os.path.exists(config_file):
        raise FileNotFoundError(f"Configuration file not found: {config_file}")
    with open(config_file, 'r') as f:
        device_config = json.load(f)

    return data, timestamps, device_config

def save_phase_video(frames, timestamps, file_path, fps=12):
    """
    Save phase data frames as an MKV video file using imageio.

    Args:
        frames (List[np.ndarray]): List of phase data frames (2D arrays).
        timestamps (np.ndarray): Timestamps corresponding to each frame.
        file_path (str): Path to save the MKV file.
        fps (int): Frames per second for the video.
    """
    import numpy as np

    # Normalize frames and convert to uint8 images
    images = []
    global_min = np.min([frame.min() for frame in frames])
    global_max = np.max([frame.max() for frame in frames])
    global_range = global_max - global_min

    for frame in frames:
        if global_range == 0:
            frame_normalized = np.zeros_like(frame, dtype=np.uint8)
        else:
            frame_normalized = np.uint8(255 * (frame - global_min) / global_range)
        images.append(frame_normalized)

    # Save frames as video
    writer = imageio.get_writer(file_path, fps=fps, codec='libx264', format='FFMPEG')

    for idx, image in enumerate(images):
        # Optionally, you can add timestamps to the frames
        plt.figure(figsize=(8, 6))
        plt.imshow(image, cmap='jet', aspect='auto')
        plt.title(f'Time: {timestamps[idx]:.2f} s')
        plt.axis('off')

        # Convert Matplotlib figure to image array
        plt.tight_layout()
        plt.draw()
        fig_image = np.frombuffer(plt.gcf().canvas.tostring_rgb(), dtype=np.uint8)
        fig_image = fig_image.reshape(plt.gcf().canvas.get_width_height()[::-1] + (3,))
        plt.close()

        writer.append_data(fig_image)

    writer.close()
    print(f"Saved MKV file: {file_path}")

# -------------------------------------------------
# Main Processing
# -------------------------------------------------

def main():
    # Folder containing radar data
    folder_path = "/home/dcope/labx_master/radar_code/data/test_capture_V91"

    # Load data and device configuration
    data, timestamps, device_config = load_radar_data(folder_path)

    # Data dimensions
    num_frames, num_rx_antennas, num_chirps_per_frame, num_samples_per_chirp = data.shape
    print(f"Data shape: {data.shape}")
    print(f"Timestamps shape: {timestamps.shape}")

    # Extract parameters from device configuration
    start_frequency_Hz = device_config['chirp']['start_frequency_Hz']
    end_frequency_Hz = device_config['chirp']['end_frequency_Hz']
    sample_rate_Hz = device_config['chirp']['sample_rate_Hz']
    chirp_repetition_time_s = device_config['chirp_repetition_time_s']
    frame_repetition_time_s = device_config['frame_repetition_time_s']
    num_samples_per_chirp = device_config['chirp']['num_samples']
    num_chirps_per_frame = device_config['num_chirps']
    num_rx_antennas = bin(device_config['chirp']['rx_mask']).count('1')

    # Constants
    c = constants.c  # Speed of light in m/s

    # Compute range resolution and bins
    bandwidth_Hz = end_frequency_Hz - start_frequency_Hz
    range_resolution = c / (2 * bandwidth_Hz)
    max_range = range_resolution * num_samples_per_chirp
    range_bins = np.linspace(0, max_range, num_samples_per_chirp, endpoint=False)

    # Assume 0.005 m length between chirps
    # This is unusual but we'll adjust the sample rate to reflect this assumption
    # Typically, the sample rate is determined by the ADC and hardware setup
    # For the purpose of this script, we'll proceed with the given assumption

    # Find indices for 1 to 3 meters
    min_distance = 1.0  # in meters
    max_distance = 3.0
    min_bin_index = np.searchsorted(range_bins, min_distance)
    max_bin_index = np.searchsorted(range_bins, max_distance)
    selected_range_bins = range_bins[min_bin_index:max_bin_index]

    # After computing range_bins
    print(f"Range bins: {range_bins}")

    # Print min_bin_index and max_bin_index
    print(f"min_bin_index: {min_bin_index}, max_bin_index: {max_bin_index}")

    # Compute number of selected bins
    num_selected_bins = max_bin_index - min_bin_index
    print(f"Number of selected range bins: {num_selected_bins}")


    # Initialize list to store frames for video
    video_frames = []

    # Process data for each frame
    for frame_idx in range(num_frames):
        # Data for the current frame
        frame_data = data[frame_idx, :, :, :]  # Shape: (num_rx_antennas, num_chirps_per_frame, num_samples_per_chirp)

        # Perform Range FFT along the samples per chirp axis
        range_fft = np.fft.fft(frame_data, axis=2)  # Shape: (num_rx_antennas, num_chirps_per_frame, num_samples_per_chirp)

        # Select range bins corresponding to 1-3 meters
        selected_bins = range_fft[:, :, min_bin_index:max_bin_index]  # Shape: (num_rx_antennas, num_chirps_per_frame, num_selected_bins)

        # Extract phase information
        phase_data = np.angle(selected_bins)  # Shape: (num_rx_antennas, num_chirps_per_frame, num_selected_bins)

        # Unwrap phase along chirps
        unwrapped_phase = np.unwrap(phase_data, axis=1)  # Unwrap along chirps

        # Average phase over chirps for each antenna and range bin
        phase_over_chirps = np.mean(unwrapped_phase, axis=1)  # Shape: (num_rx_antennas, num_selected_bins)

        # For visualization, you can average over antennas or select a specific antenna
        # Here, we'll average over antennas
        phase_frame = np.mean(phase_over_chirps, axis=0)  # Shape: (num_selected_bins,)

        # Store the phase frame
        video_frames.append(phase_frame)

    # Convert list to array for easier handling
    video_frames = np.array(video_frames)  # Shape: (num_frames, num_selected_bins)

    # For visualization, we can create a 2D image with time on one axis and range bins on the other
    # Since we have phase data over time and range bins, we'll create frames for the video

    # Create directory for output video if it doesn't exist
    output_dir = os.path.join(folder_path, "phase_video")
    os.makedirs(output_dir, exist_ok=True)

    # File path for the output MKV video
    mkv_filename = os.path.join(output_dir, "phase_over_time.mkv")

    # Prepare frames for the video
    frames_for_video = []
    for idx in range(video_frames.shape[0]):
        # Create an image for each frame
        phase_frame = video_frames[idx, :]  # Shape: (num_selected_bins,)

        # Plot the phase data
        plt.figure(figsize=(8, 6))
        plt.plot(selected_range_bins, phase_frame)
        plt.title(f'Phase Data at Time {timestamps[idx]:.2f} s')
        plt.xlabel('Distance (m)')
        plt.ylabel('Phase (radians)')
        plt.grid(True)
        plt.ylim([-np.pi, np.pi])

        # Convert Matplotlib figure to image array
        plt.tight_layout()
        plt.draw()
        fig_image = np.frombuffer(plt.gcf().canvas.tostring_rgb(), dtype=np.uint8)
        fig_image = fig_image.reshape(plt.gcf().canvas.get_width_height()[::-1] + (3,))
        plt.close()

        frames_for_video.append(fig_image)

    # Save the phase data as an MKV video file
    save_phase_video(frames_for_video, timestamps, mkv_filename, fps=12)

if __name__ == "__main__":
    main()