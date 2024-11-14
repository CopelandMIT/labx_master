import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import random

# Filename
csv_file = '/usr/local/zed/samples/body\ tracking/export/JSON\ export/cpp/data/P001_QLS_SQUAT_FR_T01.json'

# Load the CSV data into a pandas DataFrame
data = pd.read_csv(csv_file)

# List of keypoints (this should match the column names in your CSV)
keypoints_columns = [
    'nose_x', 'nose_y', 'nose_z', 'neck_x', 'neck_y', 'neck_z', 'right_shoulder_x', 'right_shoulder_y', 'right_shoulder_z',
    'left_shoulder_x', 'left_shoulder_y', 'left_shoulder_z', 'right_elbow_x', 'right_elbow_y', 'right_elbow_z', 
    'left_elbow_x', 'left_elbow_y', 'left_elbow_z', 'right_wrist_x', 'right_wrist_y', 'right_wrist_z',
    'left_wrist_x', 'left_wrist_y', 'left_wrist_z', 'right_hip_x', 'right_hip_y', 'right_hip_z', 
    'left_hip_x', 'left_hip_y', 'left_hip_z', 'right_knee_x', 'right_knee_y', 'right_knee_z',
    'left_knee_x', 'left_knee_y', 'left_knee_z', 'right_ankle_x', 'right_ankle_y', 'right_ankle_z', 
    'left_ankle_x', 'left_ankle_y', 'left_ankle_z'
]

# Pick a random row (observation) from the dataset
random_row = random.randint(0, len(data) - 2)

# Extract the 3D keypoints for the selected observation
keypoints_data = data.iloc[random_row][keypoints_columns].values

# Ensure that the keypoints_data is a NumPy array and reshape it to 3D
keypoints_data = np.array(keypoints_data, dtype=float).reshape(-1, 3)

print(keypoints_data)

# Create a 3D plot
fig = plt.figure()
ax = fig.add_subplot(projection='3d')

# Plot the keypoints in 3D
xs = keypoints_data[:, 0]  # X coordinates
ys = keypoints_data[:, 1]  # Y coordinates
zs = keypoints_data[:, 2]  # Z coordinates

ax.scatter(xs, ys, zs, c='b', marker='o')

# Set axis labels
ax.set_xlabel('X Axis')
ax.set_ylabel('Y Axis')
ax.set_zlabel('Z Axis')

# Ensure that all axes have the same scale
max_range = np.array([xs.max() - xs.min(), ys.max() - ys.min(), zs.max() - zs.min()]).max() / 2.0

mid_x = (xs.max() + xs.min()) * 0.5
mid_y = (ys.max() + ys.min()) * 0.5
mid_z = (zs.max() + zs.min()) * 0.5

ax.set_xlim(mid_x - max_range, mid_x + max_range)
ax.set_ylim(mid_y - max_range, mid_y + max_range)
ax.set_zlim(mid_z - max_range, mid_z + max_range)

# Show the plot
plt.show()
