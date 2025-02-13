import numpy as np
import os
import json
import matplotlib.pyplot as plt
from scipy import signal, constants
from scipy.optimize import curve_fit
import glob


# -------------------------------------------------
# Get Data
# -------------------------------------------------

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


# -------------------------------------------------
# Filters
# -------------------------------------------------

def butter_bandpass(lowcut, highcut, fs, order=5):
    nyq = 0.5 * fs
    low = lowcut / nyq
    high = highcut / nyq
    sos = signal.butter(order, [low, high], analog=False, btype='band', output='sos')
    return sos

def butter_bandpass_filter(data, lowcut, highcut, fs, order=5):
    sos = butter_bandpass(lowcut, highcut, fs, order=order)
    y = signal.sosfiltfilt(sos, data)
    return y

def correct_phase_ambiguity(phase):
    d_phase = phase[1:] - phase[:-1]
    ofs = np.zeros_like(d_phase)
    idx = np.where(d_phase > np.pi)
    ofs[idx] = -2*np.pi
    idx = np.where(d_phase < -np.pi)
    ofs[idx] = 2*np.pi
    # ofs = np.cumsum(ofs)
    tmp = np.zeros_like(phase)
    tmp[1:] = ofs
    ofs = tmp
    return ofs + phase

# -------------------------------------------------
# HR & RR Extract
# -------------------------------------------------

def find_vs_peaks(data,  min_distance, min_height, prominence, ts, plot_on):
    peaks, properties  = signal.find_peaks(data, distance=min_distance ,prominence=prominence) #height=min_height) #
    time = np.arange(0,ts*(len(data)) , ts)
    peak_times = time[peaks]
    peak_intervals = np.mean(np.diff(peak_times))
    hr_hz = 1/peak_intervals
    
    if plot_on:
        # Plot filtered signal with detected peaks
        plt.figure(figsize=(12, 6))
        plt.plot(time, data, label='Filtered Signal')
        plt.plot(time[peaks], data[peaks], "x", label='Peaks')
        plt.xlabel('Time (s)')
        plt.ylabel('Amplitude')
        plt.title(f'Filtered Distance over Time with Detected Peaks ')
        plt.legend()
        plt.grid(True)
        plt.show()
        
    return hr_hz

def HR_RR_Extract_PSD(x, ts, Adult_on, plot_on):
    
    freq, PP = signal.welch(x, fs=1/(ts), nperseg=len(x), detrend=False)

    if plot_on:
        plt.plot(freq, PP)
        plt.show()
        
    if Adult_on:
        HR_searchrange = [50, 120]
        RR_searchrange = [8, 22]
    else:
        HR_searchrange = [85, 220]
        RR_searchrange = [30, 140]

    indHRUP = np.where(freq > HR_searchrange[0] / 60)[0]
    indHRDW = np.where(freq < HR_searchrange[1] / 60)[0]
    iindsU = np.intersect1d(indHRUP, indHRDW)

    # Find the maximum value and corresponding frequency
    index_4HR = np.argmax(PP[iindsU])
    mxvalFreqP_4HR = freq[iindsU][index_4HR]

    OutputHR = mxvalFreqP_4HR * 60
    # print('HR:', OutputHR)
    
    indHRUP = np.where(freq > RR_searchrange[0] / 60)[0]
    indHRDW = np.where(freq < RR_searchrange[1] / 60)[0]
    iindsU = np.intersect1d(indHRUP, indHRDW)

    # Find the maximum value and corresponding frequency
    index_4RR = np.argmax(PP[iindsU])
    mxvalFreqP_4RR = freq[iindsU][index_4RR]

    OutputRR = mxvalFreqP_4RR * 60
    # print('RR:', OutputRR, '\n')
    return OutputHR, OutputRR

# -------------------------------------------------
# Ellipse Fitting
# -------------------------------------------------

def normalize_complex_vector(vector):
    magnitudes = np.abs(vector)
    normalized_vector = vector / magnitudes
    return normalized_vector

def normalize_vector(vector):
    magnitude = np.linalg.norm(vector)  # Calculate the magnitude of the vector
    if magnitude == 0:
        return vector  # Avoid division by zero
    normalized_vector = vector / magnitude  # Normalize the vector
    return normalized_vector

def normalize_signal(signal):
    min_val = np.min(signal)
    max_val = np.max(signal)
    normalized_signal = (signal - min_val) / (max_val - min_val)
    return normalized_signal


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



def fit_ellipse2(I,Q):
    
    """
    According to paper" Data-Based Quadrature Imbalance Compensation for a CW Doppler Radar
    System"
    AQ^2 + BIQ + CI + DQ + E = -I^2
    Amplitude imbalance = sqrt(1/A)
    Phase Imbalance = arcsin(B/2sqrt(A))
    """
    
    M = np.stack((Q**2, I*Q, I, Q, np.ones(len(I)))).T
    b = -(I**2)
    
    coeffs = np.linalg.inv((M.T @ M)) @ M.T @ b
    
    return coeffs

def calc_imbalance(A, B):
    
    A_imbalance = np.sqrt(1/A)
    Phase_imbalance = np.arcsin(B/(2*np.sqrt(A)))
    return A_imbalance, Phase_imbalance

def windowOnSignal(signal, window_size):
    num_windows = int(len(signal) // window_size)
    return [signal[i*window_size:(i+1)*window_size] for i in range(num_windows)]


def compensate_imbalances(I, Q, amplitude, phase_imbalance):

    IQ_combined = np.stack((I, Q))
    adjusted_IQ = np.array([[1, 0],[-np.tan(phase_imbalance), 1/(amplitude*np.cos(phase_imbalance))]])
    result = np.dot(adjusted_IQ, IQ_combined)

    return result[0], result[1]


def fit_ellipse(x, y):
    """
    Fit the coefficients a,b,c,d,e,f, representing an ellipse described by
    the formula F(x,y) = ax^2 + bxy + cy^2 + dx + ey + f = 0 to the provided
    arrays of data points x=[x1, x2, ..., xn] and y=[y1, y2, ..., yn].

    Based on the algorithm of Halir and Flusser, "Numerically stable direct
    least squares fitting of ellipses'.
    """

    D1 = np.vstack([x**2, x*y, y**2]).T
    D2 = np.vstack([x, y, np.ones(len(x))]).T
    S1 = D1.T @ D1
    S2 = D1.T @ D2
    S3 = D2.T @ D2
    T = -np.linalg.inv(S3) @ S2.T
    M = S1 + S2 @ T
    C = np.array(((0, 0, 2), (0, -1, 0), (2, 0, 0)), dtype=float)
    M = np.linalg.inv(C) @ M
    eigval, eigvec = np.linalg.eig(M)
    con = 4 * eigvec[0]* eigvec[2] - eigvec[1]**2
    ak = eigvec[:, np.nonzero(con > 0)[0]]
    coeffs = np.concatenate((ak, T @ ak)).ravel()
    coeffs = coeffs/coeffs[0]
    return coeffs


def cart_to_pol(coeffs):
    """
    Convert the cartesian conic coefficients, (a, b, c, d, e, f), to the
    ellipse parameters, where F(x, y) = ax^2 + bxy + cy^2 + dx + ey + f = 0.
    The returned parameters are x0, y0, ap, bp, e, phi, where (x0, y0) is the
    ellipse centre; (ap, bp) are the semi-major and semi-minor axes,
    respectively; e is the eccentricity; and phi is the rotation of the semi-
    major axis from the x-axis.
    """

    # We use the formulas from https://mathworld.wolfram.com/Ellipse.html
    # which assumes a cartesian form ax^2 + 2bxy + cy^2 + 2dx + 2fy + g = 0.
    # Therefore, rename and scale b, d and f appropriately.
    a = coeffs[0]
    b = coeffs[1] / 2
    c = coeffs[2]
    d = coeffs[3] / 2
    f = coeffs[4] / 2
    g = coeffs[5]

    den = b**2 - a*c
    if den > 0:
        raise ValueError('coeffs do not represent an ellipse: b^2 - 4ac must'
                         ' be negative!')

    # The location of the ellipse centre.
    x0, y0 = (c*d - b*f) / den, (a*f - b*d) / den

    num = 2 * (a*f**2 + c*d**2 + g*b**2 - 2*b*d*f - a*c*g)
    fac = np.sqrt((a - c)**2 + 4*b**2)
    # The semi-major and semi-minor axis lengths (these are not sorted).
    ap = np.sqrt(num / den / (fac - a - c))
    bp = np.sqrt(num / den / (-fac - a - c))

    # Sort the semi-major and semi-minor axis lengths but keep track of
    # the original relative magnitudes of width and height.
    width_gt_height = True
    if ap < bp:
        width_gt_height = False
        ap, bp = bp, ap

    # The eccentricity.
    r = (bp/ap)**2
    if r > 1:
        r = 1/r
    e = np.sqrt(1 - r)

    # The angle of anticlockwise rotation of the major-axis from x-axis.
    if b == 0:
        phi = 0 if a < c else np.pi/2
    else:
        phi = np.arctan((2.*b) / (a - c)) / 2
        if a > c:
            phi += np.pi/2
    if not width_gt_height:
        # Ensure that phi is the angle to rotate to the semi-major axis.
        phi += np.pi/2
    phi = phi % np.pi

    return x0, y0, ap, bp, e, phi


def get_ellipse_pts(params, npts=100, tmin=0, tmax=2*np.pi):
    """
    Return npts points on the ellipse described by the params = x0, y0, ap,
    bp, e, phi for values of the parametric variable t between tmin and tmax.

    """

    x0, y0, ap, bp, e, phi = params
    # A grid of the parametric variable, t.
    t = np.linspace(tmin, tmax, npts)
    x = x0 + ap * np.cos(t) * np.cos(phi) - bp * np.sin(t) * np.sin(phi)
    y = y0 + ap * np.cos(t) * np.sin(phi) + bp * np.sin(t) * np.cos(phi)
    return x, y

def fit_ellipse_to_circle(coeffs):
    [a, b, c, d, f, g] = coeffs
    # Calculate center of the ellipse
    center_x = (c*d - b*f) / (b**2 - a*c)
    center_y = (a*f - b*d) / (b**2 - a*c)
    
    # Calculate semi-major and semi-minor axes
    discriminant = np.sqrt((a - c)**2 + 4*b**2)

    if discriminant == 0:  # Circle
        semi_major = np.sqrt(-g / a)
        semi_minor = semi_major
        angle = 0
    else:
        semi_major = np.sqrt(np.abs(2*(a*f**2 + c*d**2 + g*b**2 - 2*b*d*f - a*c*g) / ((b**2 - a*c)*(discriminant - (a + c)))))
        semi_minor = np.sqrt(np.abs(2*(a*f**2 + c*d**2 + g*b**2 - 2*b*d*f - a*c*g) / ((b**2 - a*c)*(-discriminant - (a + c)))))
        
        # Calculate rotation angle
        angle = 0.5 * np.arctan2(2*b, a - c)
    
    return center_x, center_y, semi_major, semi_minor, angle

def transform_to_unit_circle(I, Q, center_x, center_y, semi_major, semi_minor, angle):
    # Scaling factors
    sx = 1 / semi_major
    sy = 1 / semi_minor
    
    # Rotation matrix
    rot_matrix = np.array([[np.cos(angle), -np.sin(angle)],
                           [np.sin(angle), np.cos(angle)]])
    
    transformed_points = []
    for i, q in zip(I, Q):
        # Translate to origin
        x = i - center_x
        y = q - center_y
        
        # Scale
        x_scaled = x * sx
        y_scaled = y * sy
        
        # Rotate
        x_rotated, y_rotated = np.dot(rot_matrix, [x_scaled, y_scaled])
        
        transformed_points.append((x_rotated, y_rotated))
    
    return transformed_points


# -------------------------------------------------
# Kalman filter
# -------------------------------------------------

def Kalman_filter(ts, z_kp1, x_k_k, P_k_k):

    #state prediction
    A = np.array([[1, ts, 0.5*ts**2],[0, 1, ts],[0, 0, 1]])
    H = np.array([[1, 0, 0]])
    G = np.array([[0.5*ts**2, ts, 1]]).T
    ro_a = 0.01 #acceleration process noise
    Q = G @ G.T * (ro_a**2)
    R = 0.001 # expected square of estimation error

    x_kp1_k = A @ x_k_k
    P_kp1_k = A @ P_k_k @ A.T + Q

    # Ellipse gating
    y_kp1 = H @ x_kp1_k
    S = H @ P_kp1_k @ H.T + R
    gamma = 0.01

    d = (z_kp1 - y_kp1) * np.linalg.inv(S) * (z_kp1 - y_kp1)
    # print(d)
    if d>gamma:
        no_update = 1
        return x_kp1_k, P_kp1_k, no_update
    else:
        no_update = 0
        # state update
        K_kp1 = P_kp1_k @ H.T @ np.linalg.inv(H @ P_kp1_k @ H.T + R)
        x_kp1_kp1 = x_kp1_k + K_kp1*(z_kp1 - y_kp1)
        P_kp1_kp1 = (np.identity(3) - K_kp1 @ H) @ P_kp1_k
        return x_kp1_kp1, P_kp1_kp1, no_update
