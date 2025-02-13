import os
import json
import numpy as np
from scipy import constants
import imageio
from PIL import Image, ImageDraw, ImageFont
import datetime

# -------------------------------------------------
# Helper Functions
# -------------------------------------------------

def load_data_and_config(folder_path, GUI_on):
    """
    Load radar data and device configuration based on GUI_on flag.
    """
    import os
    import numpy as np
    import datetime
    import re  # For regular expression parsing

    # Load .npz files
    npz_files = [file for file in os.listdir(folder_path) if file.endswith('.npz')]

    # Extract timestamps from filenames and sort the files
    file_timestamp_pairs = []
    for file in npz_files:
        # Extract timestamp using regular expressions
        match = re.search(r'_(\d{8}_\d{9})_', file)
        if match:
            timestamp_str = match.group(1)
            # Parse the timestamp string into a datetime object
            timestamp = datetime.datetime.strptime(timestamp_str, '%Y%m%d_%H%M%S%f')
            file_timestamp_pairs.append((file, timestamp))
        else:
            print(f"Warning: Timestamp not found in filename {file}")
            # You can choose to skip the file or handle it differently
            continue

    # Sort the files based on the extracted timestamps
    sorted_files = [pair[0] for pair in sorted(file_timestamp_pairs, key=lambda x: x[1])]

    npz_abs_file_paths = [os.path.join(folder_path, file) for file in sorted_files]
    print("NPZ Files Found (sorted by timestamp):", npz_abs_file_paths)

    data_list = []
    timestamps_list = []

    for npz_abs_file_path in npz_abs_file_paths:
        print(f"Loading file: {npz_abs_file_path}")
        npz_data = np.load(npz_abs_file_path, allow_pickle=True)
        print("Keys in npz file:", list(npz_data.keys()))  # Debugging
        data_part = npz_data['data']
        timestamps_part = npz_data['frame_timestamps_list']

        # Ensure that timestamps are datetime objects
        if not isinstance(timestamps_part[0], datetime.datetime):
            timestamps_part = np.array([datetime.datetime.fromtimestamp(ts) for ts in timestamps_part])

        # Check if data is in reverse order and reverse if necessary
        if timestamps_part[0] > timestamps_part[-1]:
            data_part = data_part[::-1]
            timestamps_part = timestamps_part[::-1]

        data_list.append(data_part)
        timestamps_list.append(timestamps_part)

    # Concatenate data and timestamps
    data = np.concatenate(data_list, axis=0)
    timestamps = np.concatenate(timestamps_list, axis=0)

    # As an extra precaution, sort the concatenated data based on timestamps
    timestamps_numeric = np.array([ts.timestamp() for ts in timestamps])
    sort_indices = np.argsort(timestamps_numeric)
    data = data[sort_indices]
    timestamps = timestamps[sort_indices]

    # Load configuration
    config_file = os.path.join(folder_path, 'config.json')
    if not os.path.exists(config_file):
        raise FileNotFoundError(f"Configuration file not found: {config_file}")
    with open(config_file, 'r') as f:
        device_config = json.load(f)

    return data, timestamps, device_config


def save_to_mkv(frames, frame_timestamps, file_path, fps=1.2 * 4):
    import numpy as np  # Ensure NumPy is imported
    from PIL import Image, ImageDraw, ImageFont  # For overlaying text
    import matplotlib.cm as cm  # Import the colormap module
    start_time = frame_timestamps[0]
    print(frame_timestamps)
    #focus near radar
    frames_close_range = [frame[10:122, :] for frame in frames]

    # Calculate global min and max
    local_min = np.min([frame.min() for frame in frames_close_range])
    local_max = np.max([frame.max() for frame in frames_close_range])
    local_range = local_max - local_min

    # Normalize frames and apply colormap
    images = []
    for idx, frame in enumerate(frames_close_range):
        if local_range == 0:
            # Avoid division by zero if frames have constant values
            frame_normalized = np.zeros_like(frame, dtype=np.float32)
        else:
            # Normalize frame to range [0, 1]
            frame_normalized = (frame - local_min) / local_range

        # Apply the 'jet' colormap
        colored_frame = cm.jet(frame_normalized)  # Returns an RGBA image

        # Convert to uint8 and discard alpha channel
        colored_frame = (colored_frame[:, :, :3] * 255).astype(np.uint8)

        # Convert colored_frame to PIL Image
        img = Image.fromarray(colored_frame)

        # Create a draw object
        draw = ImageDraw.Draw(img)

        # Calculate elapsed time since start
        elapsed_time = (frame_timestamps[idx]- start_time).total_seconds()
        # Format timestamp in seconds down to milliseconds
        timestamp_text = f"{elapsed_time:.3f} s"

        # Get text size using textbbox
        bbox = draw.textbbox((0, 0), timestamp_text)
        text_width = bbox[2] - bbox[0]
        text_height = bbox[3] - bbox[1]
        img_width, img_height = img.size
        x = (img_width - text_width) / 2
        y = img_height - text_height - 10  # 10 pixels from bottom

        # Draw text on image
        draw.text((x, y), timestamp_text, fill=(255, 255, 255))

        # Append image to images list
        images.append(np.array(img))

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
    data, timestamps, config = load_data_and_config(folder_path, GUI_on=GUI_on)

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
    num_frames, num_rx_antennas_in_data, num_chirps_per_frame_in_data, num_samples_per_chirp_in_data = data.shape
    if num_rx_antennas != num_rx_antennas_in_data:
        print(f"Warning: Number of RX antennas in config ({num_rx_antennas}) does not match data ({num_rx_antennas_in_data})")
        num_rx_antennas = num_rx_antennas_in_data

    if num_chirps_per_frame != num_chirps_per_frame_in_data:
        print(f"Warning: Number of chirps per frame in config ({num_chirps_per_frame}) does not match data ({num_chirps_per_frame_in_data})")
        num_chirps_per_frame = num_chirps_per_frame_in_data

    if num_samples_per_chirp != num_samples_per_chirp_in_data:
        print(f"Warning: Number of samples per chirp in config ({num_samples_per_chirp}) does not match data ({num_samples_per_chirp_in_data})")
        num_samples_per_chirp = num_samples_per_chirp_in_data

    FMCW_Bandwidth = end_frequency_Hz - start_frequency_Hz
    range_resolution = constants.c / (2 * FMCW_Bandwidth)
    range_bins = np.linspace(0, range_resolution * num_samples_per_chirp / 2, num_samples_per_chirp, endpoint=False)

    print(f"Original data shape: {data.shape}")

    # Create directory for MKV files if not exists
    mkv_output_dir = f"{folder_path}/radar_mkvs_v7/"
    os.makedirs(mkv_output_dir, exist_ok=True)

    # Process data per antenna
    for ant_idx in range(num_rx_antennas):
        print(f"Processing antenna {ant_idx}")
        antenna_data = data[:, ant_idx, :, :]  # Shape: (num_frames, num_chirps_per_frame, num_samples_per_chirp)

        # Generate chirp timestamps
        chirp_timestamps = []
        for frame_idx in range(num_frames):
            t_frame = timestamps[frame_idx]
            for chirp_idx in range(num_chirps_per_frame):
                t_chirp = t_frame + datetime.timedelta(seconds=chirp_idx * chirp_repetition_time_s)
                chirp_timestamps.append(t_chirp)
        chirp_timestamps = np.array(chirp_timestamps)
        total_chirps = len(chirp_timestamps)
        print(f"Total chirps for antenna {ant_idx}: {total_chirps}")

        # Flatten data and timestamps
        antenna_data = antenna_data.reshape(-1, num_samples_per_chirp)  # Shape: (total_chirps, num_samples_per_chirp)
        chirp_timestamps = chirp_timestamps.reshape(-1)  # Flatten the chirp timestamps

        # Desired new number of chirps per frame
        new_num_chirps_per_frame = 64
        new_total_frames = total_chirps // new_num_chirps_per_frame
        remaining_chirps = total_chirps % new_num_chirps_per_frame

        if remaining_chirps != 0:
            print(f"Discarding {remaining_chirps} chirps to reshape data for antenna {ant_idx}.")
            antenna_data = antenna_data[:-remaining_chirps, :]
            chirp_timestamps = chirp_timestamps[:-remaining_chirps]

        # Reshape into the new data shape
        antenna_data = antenna_data.reshape(new_total_frames, new_num_chirps_per_frame, num_samples_per_chirp)
        chirp_timestamps = chirp_timestamps.reshape(new_total_frames, new_num_chirps_per_frame)
        print(f"Reshaped data shape for antenna {ant_idx}: {antenna_data.shape}")

        # Compute frame timestamps as the timestamp of the first chirp in each new frame
        frame_timestamps = chirp_timestamps[:, 0]

        # Transpose data to match expected shape for processing
        # From (new_total_frames, new_num_chirps_per_frame, num_samples_per_chirp) to (new_total_frames, num_samples_per_chirp, new_num_chirps_per_frame)
        antenna_data = antenna_data.transpose(0, 2, 1)  # Shape: (new_total_frames, num_samples_per_chirp, new_num_chirps_per_frame)

        # Prepare dataCubes for processing: (1, new_total_frames, num_samples_per_chirp, new_num_chirps_per_frame)
        dataCubes = antenna_data[np.newaxis, ...]

        # Perform range-Doppler processing
        rdm_all_channels = range_doppler_processing(dataCubes)  # Shape: (1, new_total_frames, ..., ...)

        # Extract the processed frames
        frames = []
        for frame_idx in range(new_total_frames):
            rdm = rdm_all_channels[0, frame_idx, :, :]
            frames.append(rdm)

        # Save the frames as an MKV file for this antenna
        mkv_filename = os.path.join(mkv_output_dir, f"antenna_{ant_idx}.mkv")
        save_to_mkv(frames, frame_timestamps, mkv_filename)
        print(f"Radar data saved to MKV file for antenna {ant_idx} with filename {mkv_filename}.")

if __name__ == "__main__":
    main()