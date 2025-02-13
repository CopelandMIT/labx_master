import numpy as np
from matplotlib import pyplot as plt
from matplotlib import animation
import pandas as pd
from mpl_toolkits.mplot3d import Axes3D

# Specify the path to ffmpeg
plt.rcParams['animation.ffmpeg_path'] = '/usr/bin/ffmpeg'

# Load the CSV data
csv_file = "/home/daniel/labx_master/ZED2i_code/data/test_capture_V90/test_capture_V90_20241121_150932_30s.csv"
data = pd.read_csv(csv_file)

print(data.head)

# Set up the figure and 3D axis
fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')

# Set limits for the axes based on your expected keypoint coordinates
ax.set_xlim(-3000, 3000)
ax.set_ylim(-3000, 3000)
ax.set_zlim(-3000, 3000)

# Function to update the plot for each frame
def update(frame):
    # Clear the plot for the current frame
    ax.clear()

    # Extract the x, y, z coordinates of the keypoints for the current frame
    try:
        x_coords = data.iloc[frame, 2::3].values.astype(float)  # Extracting all x-coordinates
        y_coords = data.iloc[frame, 3::3].values.astype(float)  # Extracting all y-coordinates
        z_coords = data.iloc[frame, 4::3].values.astype(float)  # Extracting all z-coordinates
    except Exception as e:
        print(f"Error in frame {frame}: {e}")
        return

    # Plot the keypoints as a scatter plot in 3D
    if len(x_coords) > 0 and len(y_coords) > 0 and len(z_coords) > 0:
        ax.scatter(x_coords, y_coords, z_coords, s=100)

    # Optionally, set titles or other plot decorations here
    ax.set_title(f"Frame {frame + 1}")
    ax.set_xlim(-3000, 3000)
    ax.set_ylim(-3000, 3000)
    ax.set_zlim(-3000, 3000)

# Create the animation, ending two frames earlier
ani = animation.FuncAnimation(fig, update, frames=len(data) - 2, repeat=False)

# Use the ffmpeg writer to save the animation as a video file
writer = animation.FFMpegWriter(fps=60, codec='libx264')

# Save the animation as an MP4 file
ani.save('/home/daniel/labx_master/ZED2i_code/data/test_capture_V90/body_tracking_animation_3d_v2.mp4', writer=writer)

print('3D animation saved successfully!')
