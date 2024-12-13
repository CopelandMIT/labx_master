import numpy as np
import os
import json
import matplotlib.pyplot as plt
from scipy import signal, constants, fft
from scipy.optimize import curve_fit
import math
# from statsmodels.graphics.tsaplots import plot_acf
import re

from Utils import *
from FFT_spectrum import *
from RangeAlgorithmv1 import *
#from NXC_Code_Functions import *


# -------------------------------------------------
# Get Data
# -------------------------------------------------

GUI_on = 1 #Recording was made using Radar Fusion GUI or Raspberry Pi
Adult_on = 1 #Recording was made on an adult or infant

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


if GUI_on:
    # Folder containing .npy files
    folder_path = r'/media/daniel/4TBLacie2/LabXData/Squat_tests_12112024/radar_28/squat_v2_45s'#'C:/Users/Inbar Chityat/Dropbox (MIT)/Radar_Experimental_Data/General Lab Experiments/BGT60TR13C_record_20240528-114743_15RR_65HR/RadarIfxAvian_00/'
    # folder_path = 'C:/Users/inbarc/Dropbox (MIT)/Radar_Experimental_Data/General Lab Experiments/BGT60TR13C_record_20240618-154745/RadarIfxAvian_00/'
    # data, device_config = get_data(folder_path)
    data, timestamps, device_config = load_data_and_config(folder_path, GUI_on)

    # Parameters
    num_frames, num_rx_antennas, num_chirps_per_frame, num_samples_per_chirp = data.shape
    frame_repetition_time_s = device_config['frame_repetition_time_s']
    chirp_repetition_time_s = device_config['chirp_repetition_time_s']
    start_frequency_Hz =  device_config['chirp']['start_frequency_Hz']
    end_frequency_Hz = device_config['chirp']['end_frequency_Hz']
    sample_rate_Hz = device_config['chirp']['sample_rate_Hz']

else:
    folder_path = 'C:/Users/Inbar Chityat/Dropbox (MIT)/Radar_Experimental_Data/ECG Experiment 6.24.24/DATA_20240624_152735_test1/'
    # folder_path = 'C:/Users/inbarc/Dropbox (MIT)/Radar_Experimental_Data/General Lab Experiments/No_Target/'
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

frame_gap = frame_repetition_time_s - chirp_repetition_time_s*num_chirps_per_frame
print( frame_gap )
# Use regular expressions to find the numbers before "RR" and "HR"
rr_match = re.search(r'(\d+)RR', folder_path)
hr_match = re.search(r'(\d+)HR', folder_path)

# Extract the numbers and store them in variables
GT_RR = int(rr_match.group(1)) if rr_match else 0
GT_HR = int(hr_match.group(1)) if hr_match else 0

FMCW_Bandwidth = end_frequency_Hz-start_frequency_Hz
FMCW_lambda = constants.c/6.05e9#start_frequency_Hz

range_resolution = constants.c/(2*(FMCW_Bandwidth))
range_bins = np.linspace(0, range_resolution * num_samples_per_chirp/2, num_samples_per_chirp, endpoint=False)


# Filter range bins based on min and max distance
# Define the distance range (in meters)
min_distance = 0.5
max_distance = 4
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
window_in_sec = 3
frames_in_window = int(window_in_sec/frame_repetition_time_s)
num_segments = num_frames-frames_in_window

# Initialize arrays
rr_bpm_all_ant = np.empty((0, num_segments), int)
hr_bpm_all_wo_adaptive_filter_ant = np.empty((0 , num_segments), int)
kalman_hr_all_ant = np.empty((0, num_segments) , int)

# Initialize range FFT
range_map = RangeAlgo(num_samples_per_chirp, frames_in_window * num_chirps_per_frame, start_frequency_Hz, end_frequency_Hz)

# time vector
ts = chirp_repetition_time_s
fs = 1/ts
time = np.arange(0,ts*frames_in_window*num_chirps_per_frame , ts)
segment_size = num_chirps_per_frame
delay = frame_gap
time_segments = (len(time) + segment_size - 1) // segment_size  # Ceiling division
delays = np.arange(time_segments) * delay
extended_delays = np.repeat(delays, segment_size)[:len(time)]
time = time + extended_delays
# plt.figure()
# plt.plot(time)
# plt.show()

for i_ant in range(1,num_rx_antennas-1):
    
    rr_bpm_all = []
    hr_bpm_all_wo_adaptive_filter = []
    kalman_hr_all = []

    print(i_ant)
    
    for frame_number in range(0,num_segments):  
        frame_contents = data[frame_number:(frame_number+frames_in_window),:,:,:]
        mat = frame_contents[:,i_ant, :, :]
        mat = mat.reshape(frames_in_window * num_chirps_per_frame, num_samples_per_chirp)

        range_fft, distance_data, distance_peak_idx, distance_peak_m = range_map.compute_range_map(mat, min_bin_index, max_bin_index)
        
        print(distance_peak_idx)
        plt.figure()
        plt.imshow(np.abs(range_fft), aspect='auto')
        plt.show()

        target_range_bin = range_fft[:, 93]#distance_peak_idx]
        
        std_deviation = np.std(np.abs(target_range_bin))
        # print(std_deviation)
        
        # Check if the standard deviation exceeds the motion threshold
        if std_deviation > motion_threshold:
            continue
            print(f"Frame {frame_number} discarded due to significant motion.")
        # -------------------------------------------------
        # Extracting I & Q signals
        # -------------------------------------------------    

        I = target_range_bin.real
        Q = target_range_bin.imag
    

        params = circle_fit_by_taubin(np.column_stack((I, Q)))

        # lim = 0.05
        # plt.figure()
        # plt.plot(time, I)
        # # plt.plot(time, I-params[0])
        # plt.plot(time, Q)
        # # plt.plot(time, Q-params[1])
        # plt.legend(['I',  'Q'])
        # # plt.legend(['I', 'I filt', 'Q', 'Q filt'])
        # plt.xlabel('t[sec]')
        # # plt.ylim((-lim, lim))
        # plt.show()
        
        # plt.figure()
        # plt.scatter(I, Q)
        # plt.plot(I,Q)
        # # plt.scatter(I-params[0], Q-params[1])
        # plt.xlabel('I')
        # plt.ylabel('Q')
        # # plt.ylim((-lim, lim))
        # # plt.xlim((-lim, lim))
        # # plt.legend(['IQ', 'IQ after circle fit'])
        # plt.show()
        
    
        I_lpf = I - params[0]  
        Q_lpf = Q - params[1] 
        
#         # -------------------------------------------------
#         # Ellipse Fitting
#         # -------------------------------------------------    
#         #### version 1 ####
#         coeffs = fit_ellipse(I_lpf,Q_lpf)
#         Amp_imbalance, Phase_imbalance = calc_imbalance(coeffs[2], coeffs[1])
#         adjusted_I_1, adjusted_Q_1 = compensate_imbalances(I_lpf, Q_lpf, Amp_imbalance, Phase_imbalance)

        
#         #### version 2 ####
#         coeffs2 = fit_ellipse2(I_lpf, Q_lpf)
#         Amp_imbalance2, Phase_imbalance2 = calc_imbalance(coeffs2[0], coeffs2[1])
#         adjusted_I_2, adjusted_Q_2 = compensate_imbalances(I_lpf, Q_lpf, Amp_imbalance2, Phase_imbalance2)
        
#         # # plot ellipse vs unit circle    
#         # plt.figure()
#         # plt.scatter(I_lpf, Q_lpf ,color='red',marker='o')
#         # plt.scatter(adjusted_I_1,adjusted_Q_1,color='blue', marker='*')
#         # plt.scatter(adjusted_I_2,adjusted_Q_2,color='green', marker='x')
#         # theta = np.linspace(0, 2*np.pi, 100)
#         # plt.plot(np.cos(theta), np.sin(theta))
#         # plt.legend(['orig','1','2','unit'])
#         # plt.show()
#         # plt.figure()
#         # plt.scatter(I_lpf,Q_lpf)
#         # plt.show()
    
    
        # -------------------------------------------------
        # Phase + Disp Extraction
        # -------------------------------------------------  

        I_final =    I_lpf #adjusted_I_1  #adjusted_I_2 #
        Q_final =     Q_lpf #adjusted_Q_1  #adjusted_Q_2 #
        
        phase_lpf =  (np.arctan(Q_final/I_final)) #(np.arctan2(Q_final, I_final))#
        phase_lpf_unwrap = np.unwrap(phase_lpf, period=np.pi)
        displacement = phase_lpf_unwrap * FMCW_lambda / (4*np.pi)
        displacement = np.sqrt(I_final**2 + Q_final**2)
        
        if Adult_on:
            displacement_bp_wo_adaptive_filter = butter_bandpass_filter(displacement, 42/60, 180/60, fs=1/ts, order=4)
        else:
            displacement_bp_wo_adaptive_filter = butter_bandpass_filter(displacement, 85/60, 220/60, fs=1/ts, order=4)

        displacement_bp = butter_bandpass_filter(displacement, lowcut_bp, highcut_bp, fs=1/ts, order=4)

        displacement_bp_wo_adaptive_filter = normalize_signal(displacement_bp_wo_adaptive_filter)
        displacement_bp = normalize_signal(displacement_bp)
         
        # -------------------------------------------------
        # HR Extraction Kalman
        # -------------------------------------------------    
        
        # Variables to adjust peak detection
        min_peak_height = -5000# -50  # Minimum peak height
        prominence_hr_peaks = 0.001
        time_between_peaks = (1/3) if Adult_on else (1/3.75)
        distance_hr_peaks = int((time_between_peaks)/ts)
        hr_hz = find_vs_peaks(displacement_bp_wo_adaptive_filter,distance_hr_peaks, min_peak_height, prominence_hr_peaks, ts, plot_on=0)
        hr_bpm_all_wo_adaptive_filter.append(hr_hz*60)
        
        # -------------------------------------------------
        # HR Extraction Kalman
        # -------------------------------------------------    
        
        hr_hz = find_vs_peaks(displacement_bp,distance_hr_peaks, min_peak_height, prominence_hr_peaks, ts, plot_on=0)
        if frame_number==0:
            #Initialize Kalman
            x_k = np.array([[hr_hz,0.01,0.01]]).T
            P_k = np.identity(3)
            M_no_update = 0
        
        x_k, P_k, no_update = Kalman_filter(ts, hr_hz, x_k, P_k) #need to check ro_a, R, gamma
        M_no_update = M_no_update + no_update if no_update else 0
        # print(M_no_update)
        if M_no_update==0 or M_no_update>=5:
            lowcut_bp = x_k[0] - np.sqrt(P_k[0,0])
            highcut_bp = x_k[0] + np.sqrt(P_k[0,0])
            # print(lowcut_bp)
            # print(highcut_bp)
            
        kalman_hr_all.append(60*x_k[0][0])

        # -------------------------------------------------
        # RR Extraction
        # -------------------------------------------------    
        
        phase_on = 1

        if phase_on:

            phase_lpf = np.unwrap(np.arctan2(Q_final,I_final))#, period=np.pi) #np.arctan2(Q_final,I_final) #

            plt.figure()
            plt.plot(phase_lpf)  
            plt.title('phase')
            plt.show()
            
            # phase_lpf = butter_bandpass_filter(phase_lpf,30/60, 140/60, fs=1/chirp_repetition_time_s, order=4)
            # min_peak_height = -5000# -50  # Minimum peak height
            # prominence_hr_peaks = 0.001
            # time_between_peaks = (1/2) if Adult_on else (1/3)
            # distance_hr_peaks = int((time_between_peaks)/chirp_repetition_time_s)
            # OutputRR = find_vs_peaks(phase_lpf,distance_hr_peaks, min_peak_height, prominence_hr_peaks, chirp_repetition_time_s, plot_on=0)
            
            # _, OutputRR = HR_RR_Extract_PSD(phase_lpf,ts, Adult_on, plot_on=False)
            
            # rr_bpm_all.append(OutputRR)
        else:
            amp_data_all_chirps = np.abs(target_range_bin)
            amp_cur = butter_bandpass_filter(amp_data_all_chirps, lowcut=0.01, highcut=2, fs=1/ts, order=5)

            # plt.figure()
            # plt.plot(amp_cur)
            # plt.show() 
            
            _, OutputRR = HR_RR_Extract_PSD(amp_cur,ts, Adult_on, plot_on=False)

            rr_bpm_all.append(OutputRR)


    rr_bpm_all_ant = np.vstack((rr_bpm_all_ant, np.array(rr_bpm_all)))
    hr_bpm_all_wo_adaptive_filter_ant = np.vstack((hr_bpm_all_wo_adaptive_filter_ant, np.array(hr_bpm_all_wo_adaptive_filter)))
    
    kalman_hr_all_ant = np.vstack((kalman_hr_all_ant, np.array(kalman_hr_all)))


rr_bpm_all_ant = np.mean(rr_bpm_all_ant, axis=0)
hr_bpm_all_wo_adaptive_filter_ant = np.mean(hr_bpm_all_wo_adaptive_filter_ant, axis=0)
kalman_hr_all_ant = np.mean(kalman_hr_all_ant, axis=0)
# -------------------------------------------------
# Compare to GT
# -------------------------------------------------    
# GT from ECG _Analysis code

# ground_truth_hr = [64.77294893152015, 64.44591861949026, 67.57794670410975, 67.6254489347403, 68.01532061501132, 67.25834560430157, 67.32539740210282, 65.83281816712635, 65.43349603405149, 66.65414850480119, 68.30507083483738] # ECG 5
# ground_truth_hr = [67.22803845089204, 63.196579813268265, 64.96201501299173, 64.9059300501402, 64.97243272748459, 65.05245821140677, 67.44760140376161, 67.0294142270703, 65.72917683517845, 65.93567992172672, 65.75560352548452, 65.9430323615615]  #ECG 3
# ground_truth_hr = [68, 67, 66 , 67, 68, 67] # RR15
# ground_truth_hr = [71, 70, 68 , 69, 71, 72] # RR20
# ground_truth_hr = [72, 70, 69 , 70, 72, 72] # RR12
# ground_truth_hr = [68, 71 , 68, 71, 73] # RR0
# ground_truth_hr = [99, 95 ,90,  82, 76, 69,62] # Jump

ground_truth_hr = np.ones(len(hr_bpm_all_wo_adaptive_filter_ant))*GT_HR
mae = np.mean(np.abs(ground_truth_hr-hr_bpm_all_wo_adaptive_filter_ant))
mae_kalman = np.mean(np.abs(ground_truth_hr-kalman_hr_all_ant))

time_plot = np.arange(0,frame_repetition_time_s*(len(ground_truth_hr)) , frame_repetition_time_s)

# plt.figure()
# plt.title('HR')
# plt.plot(time_plot, ground_truth_hr)
# plt.plot(time_plot, hr_bpm_all_wo_adaptive_filter_ant)
# plt.plot(time_plot, kalman_hr_all_ant)
# plt.plot(ground_truth_hr+2,'r--')
# plt.plot(ground_truth_hr-2,'r--')
# plt.legend(['GT','Estimation', 'Kalman']) 
# plt.title(f"MAE: {mae}, MAE Kalman: {mae_kalman}")
# plt.xlabel('time [s]')
# plt.ylabel('bpm')
# plt.ylim((42,180)) if Adult_on else plt.ylim((85,220))
# plt.show() 

# -------------------------------------------------
# Compare to GT
# -------------------------------------------------    
# GT from ECG_Analysis code

# ground_truth_rr = [14.724819837696746, 13.637635517590287, 13.337913267677006, 12.627012516397329] # ECG 5
# ground_truth_rr = [13.088919271034346, 12.012436835247764, 13.870565457124698, 13.527711100759257] # ECG 3
ground_truth_rr = np.ones(len(rr_bpm_all_ant))*GT_RR
mae = np.mean(np.abs(ground_truth_rr-rr_bpm_all_ant))

# plt.figure()
# plt.title('RR')
# plt.plot(time_plot, ground_truth_rr)
# plt.plot(time_plot, rr_bpm_all_ant)
# plt.plot(ground_truth_rr+2,'r--')
# plt.plot(ground_truth_rr-2,'r--')
# plt.xlabel('time [s]')
# plt.legend(['GT','Estimation'])
# plt.title(f"Mean Absolute Error: {mae}")
# plt.ylabel('bpm')
# plt.ylim((8,22)) if Adult_on else plt.ylim((30,140))
# plt.show() 