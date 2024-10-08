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


def get_data(folder_path):

    # List all .npy files in the folder
    npy_files = [file for file in os.listdir(folder_path) if file.endswith('.npy')]
    # Initialize an empty list to store the data
    data_list = []

    # Iterate over each .npy file
    for npy_file in npy_files:
        print(npy_file)
        # Load data from the current file
        file_path = os.path.join(folder_path, npy_file)
        data = np.load(file_path)
        
        # Append the data to the list
        data_list.append(data)

    # Concatenate the data along the first axis to create a single array
    concatenated_data = np.concatenate(data_list, axis=0)

    # Print the shape of the concatenated data
    print("Shape of concatenated data:", concatenated_data.shape)
    
    # if file was recorded with GUI:
    # Read the JSON file
    with open(folder_path + 'config.json', 'r') as file:
        config = json.load(file)

    # Extract device_config parameters
    device_config = config['device_config']['fmcw_single_shape']

    return concatenated_data, device_config

def get_data_RP(folder_path):

    # List all .npy files in the folder
    npy_files = [file for file in os.listdir(folder_path) if file.endswith('.npy')]
    # Initialize an empty list to store the data
    data_list = []

    # Iterate over each .npy file
    for npy_file in npy_files:
        print(npy_file)
        # Load data from the current file
        file_path = os.path.join(folder_path, npy_file)
        data = np.load(file_path)
        
        # Append the data to the list
        data_list.append(data)

    # Concatenate the data along the first axis to create a single array
    concatenated_data = np.concatenate(data_list, axis=0)

    # Print the shape of the concatenated data
    print("Shape of concatenated data:", concatenated_data.shape)


    config_file = glob.glob(os.path.join(folder_path, '*.txt'))
    with open(config_file[0], 'r') as file:
        config = json.load(file)


    return concatenated_data, config

import matplotlib.pyplot as plt
import numpy as np
import os
from pymkv import MKVFile, MKVTrack

def save_to_mkv(frames, file_path, frame_duration=40000000):
    """
    Save radar data frames to an MKV file with custom headers for AWS Kinesis Video Streams.
    
    :param frames: List of complex radar data frames to be saved.
    :param file_path: Path to save the MKV file.
    :param frame_duration: Duration of each frame in nanoseconds (default is 25 FPS).
    """
    # Temporary directory to save frames as images
    temp_image_dir = 'temp_images'
    os.makedirs(temp_image_dir, exist_ok=True)

    # Save each frame as a grayscale image using matplotlib
    for i, frame in enumerate(frames):
        plt.imshow(np.abs(frame), cmap='gray')
        plt.axis('off')
        
        # Save image ensuring the dimensions are even for H.264 encoding
        plt.savefig(f"{temp_image_dir}/frame_{i:04d}.png", bbox_inches='tight', pad_inches=0)
        plt.close()

    # Use ffmpeg to convert the images to a video file with H.264 encoding
    temp_video_file = 'temp_video.mp4'
    ffmpeg_cmd = f"ffmpeg -y -framerate 12 -i {temp_image_dir}/frame_%04d.png -c:v libx264 -preset ultrafast -pix_fmt yuv420p {temp_video_file}"
    os.system(ffmpeg_cmd)

    # Create MKV file and add the video track
    mkv = MKVFile()
    track = MKVTrack(temp_video_file)
    mkv.add_track(track)

    # Write MKV file to disk
    mkv.mux(file_path)
    print(f"Saved MKV file: {file_path}")

    # Clean up temporary files
    os.remove(temp_video_file)
    for filename in os.listdir(temp_image_dir):
        file_path = os.path.join(temp_image_dir, filename)
        os.remove(file_path)
    os.rmdir(temp_image_dir)


# -------------------------------------------------
# Analyze
# -------------------------------------------------

GUI_on = 0 #Recording was made using Radar Fusion GUI or Raspberry Pi
Adult_on = 1 #Recording was made on an adult or infant

if GUI_on:
    # Folder containing .npy files for GUI recording
    folder_path = "/home/dcope_rpi5_32bit/LabX/data/radar_20240925_114942882/"
else:
    # Folder containing .npy files for Raspberry Pi recording
    folder_path = "/home/dcope_rpi5_32bit/LabX/data/radar_20240925_114942882/"

# Get all .npy files in the directory
npz_abs_file_paths = [os.path.join(folder_path,file) for file in os.listdir(folder_path) if file.endswith('.npz')]
print(npz_abs_file_paths)

# Iterate through each .npy file
for npz_abs_file_path in npz_abs_file_paths:
    # Extract base file name (without extension) for use in MKV filename
    base_filename = os.path.splitext(npz_abs_file_path)[0]

    print(npz_abs_file_path)
    data_and_timestamps_npz = np.load(npz_abs_file_path)
    npy_file = data_and_timestamps_npz['data']
    timestamps = data_and_timestamps_npz['timestamps']
    

    # Load data based on GUI_on flag
    file_path = os.path.join(folder_path, npy_file)
    if GUI_on:
        # Load data and device configuration for GUI-based recording
        data, device_config = get_data(folder_path)
        
        # Parameters
        num_frames, num_rx_antennas, num_chirps_per_frame, num_samples_per_chirp = data.shape
        frame_repetition_time_s = device_config['frame_repetition_time_s']
        chirp_repetition_time_s = device_config['chirp_repetition_time_s']
        start_frequency_Hz = device_config['start_frequency_Hz']
        end_frequency_Hz = device_config['end_frequency_Hz']
        sample_rate_Hz = device_config['sample_rate_Hz']
    else:
        # Load data and device configuration for Raspberry Pi-based recording
        data, device_config = get_data_RP(folder_path)
        
        num_frames, num_rx_antennas, num_chirps_per_frame, num_samples_per_chirp = data.shape
        chirp = device_config[0]['loop']['sub_sequence'][0]['loop']['sub_sequence'][0]['chirp']
        num_chirps_per_frame = device_config[0]['loop']['sub_sequence'][0]['loop']['num_repetitions']
        num_samples_per_chirp = chirp['num_samples']
        sample_rate_Hz = chirp['sample_rate_Hz']
        start_frequency_Hz = chirp['start_frequency_Hz']
        end_frequency_Hz = chirp['end_frequency_Hz']
        chirp_repetition_time_s = device_config[0]['loop']['sub_sequence'][0]['loop']['repetition_time_s']
        frame_repetition_time_s = device_config[0]['loop']['repetition_time_s'] 


FMCW_Bandwidth = end_frequency_Hz-start_frequency_Hz

range_resolution = constants.c/(2*(FMCW_Bandwidth))
range_bins = np.linspace(0, range_resolution * num_samples_per_chirp/2, num_samples_per_chirp, endpoint=False)


# Filter range bins based on min and max distance
# Define the distance range (in meters)
min_distance = 0.5
max_distance = 1.5
min_bin_index = np.searchsorted(range_bins, min_distance)
max_bin_index = np.searchsorted(range_bins, max_distance)
filtered_range_bins = range_bins[min_bin_index:max_bin_index]


# Threshold for standard deviation to identify motion frames
motion_threshold = 100

# Initialize bandpass frequencies
if Adult_on:
    lowcut_bp = 42/60
    highcut_bp = 180/60 
else:
    lowcut_bp = 85/60 
    highcut_bp = 220/60 

# -------------------------------------------------
# Range FFT
# -------------------------------------------------

# Initialize window
window_in_sec = 10
frames_in_window = int(window_in_sec/frame_repetition_time_s)
num_segments = num_frames-frames_in_window

# Initialize arrays
rr_bpm_all_ant = np.empty((0, num_segments), int)
hr_bpm_all_wo_adaptive_filter_ant = np.empty((0 , num_segments), int)

# Initialize range FFT
range_map = RangeAlgo(num_samples_per_chirp, frames_in_window * num_chirps_per_frame, start_frequency_Hz, end_frequency_Hz)

# time vector
ts = chirp_repetition_time_s
fs = 1/ts
time = np.arange(0,ts*frames_in_window*num_chirps_per_frame , ts)

# Updated loop for processing radar data and saving to MKV
for i_ant in range(0, num_rx_antennas):
    frames = []  # List to store all frames for this antenna
    
    # Process radar data into frames for each antenna
    num_segments = num_frames - frames_in_window
    for frame_number in range(0, num_segments):
        frame_contents = data[frame_number:(frame_number+frames_in_window), :, :, :]
        mat = frame_contents[:, i_ant, :, :]
        mat = mat.reshape(frames_in_window * num_chirps_per_frame, num_samples_per_chirp)

        range_fft, distance_data, distance_peak_idx, distance_peak_m = range_map.compute_range_map(mat, min_bin_index, max_bin_index)
        # Append the current range_fft_abs as a frame to the frames list
        frames.append(np.abs(range_fft))

    # Save the frames as an MKV file for this antenna appending the npy filename
    mkv_filename = f"/home/dcope_rpi5_32bit/LabX/data/radar_mkvs/{base_filename}_ant{i_ant}.mkv"
    save_to_mkv(frames, mkv_filename)
    print(f"Radar data saved to MKV file for antenna {i_ant} with filename {mkv_filename}.")


        # print(distance_peak_idx)
        # plt.figure()
        # plt.imshow(np.abs(range_fft), aspect='auto')
        # plt.show()

        # target_range_bin = range_fft[:, distance_peak_idx]
        
        # std_deviation = np.std(np.abs(target_range_bin))
        # # print(std_deviation)
        
        # # Check if the standard deviation exceeds the motion threshold
        # if std_deviation > motion_threshold:
        #     continue
        #     print(f"Frame {frame_number} discarded due to significant motion.")
        # # -------------------------------------------------
        # # Extracting I & Q signals
        # # -------------------------------------------------    

        # I = target_range_bin.real
        # Q = target_range_bin.imag
    

        # params = circle_fit_by_taubin(np.column_stack((I, Q)))

        # # lim = 0.05
        # # plt.figure()
        # # plt.plot(time, I)
        # # # plt.plot(time, I-params[0])
        # # plt.plot(time, Q)
        # # # plt.plot(time, Q-params[1])
        # # plt.legend(['I',  'Q'])
        # # # plt.legend(['I', 'I filt', 'Q', 'Q filt'])
        # # plt.xlabel('t[sec]')
        # # # plt.ylim((-lim, lim))
        # # plt.show()
        
        # # plt.figure()
        # # plt.scatter(I, Q)
        # # plt.plot(I,Q)
        # # # plt.scatter(I-params[0], Q-params[1])
        # # plt.xlabel('I')
        # # plt.ylabel('Q')
        # # # plt.ylim((-lim, lim))
        # # # plt.xlim((-lim, lim))
        # # # plt.legend(['IQ', 'IQ after circle fit'])
        # # plt.show()

    
        # I_lpf = I - params[0]  
        # Q_lpf = Q - params[1] 

        