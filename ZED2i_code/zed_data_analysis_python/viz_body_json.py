import json
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import numpy as np

def load_json_data(json_file_path):
    with open(json_file_path, 'r') as f:
        data = json.load(f)
    return data

def parse_body_data(json_data):
    frames = []
    for timestamp_str, frame_data in json_data.items():
        timestamp_ms = int(timestamp_str)
        if 'body_list' in frame_data:
            bodies = frame_data['body_list']
            frames.append({'timestamp': timestamp_ms, 'bodies': bodies})
        else:
            frames.append({'timestamp': timestamp_ms, 'bodies': []})
    frames.sort(key=lambda x: x['timestamp'])
    return frames

def visualize_body_tracking(frames, output_filename='output.mp4', fps=15):
    import copy

    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')

    # Set labels
    ax.set_xlabel('X')
    ax.set_ylabel('Z')  # Swapped axes
    ax.set_zlabel('Y')

    # Updated skeleton connections based on your tracker order
    skeleton_connections = [
        (0, 1),  # PELVIS to SPINE_1
        (1, 2),  # SPINE_1 to SPINE_2
        (2, 3),  # SPINE_2 to SPINE_3
        (3, 4),  # SPINE_3 to NECK
        (4, 5),  # NECK to NOSE
        (5, 6),  # NOSE to LEFT_EYE
        (6, 8),  # LEFT_EYE to LEFT_EAR
        (5, 7),  # NOSE to RIGHT_EYE
        (7, 9),  # RIGHT_EYE to RIGHT_EAR
        (4, 10), # NECK to LEFT_CLAVICLE
        (10, 12),# LEFT_CLAVICLE to LEFT_SHOULDER
        (12, 14),# LEFT_SHOULDER to LEFT_ELBOW
        (14, 16),# LEFT_ELBOW to LEFT_WRIST
        (16, 30),# LEFT_WRIST to LEFT_HAND_THUMB_4
        (16, 32),# LEFT_WRIST to LEFT_HAND_INDEX_1
        (16, 34),# LEFT_WRIST to LEFT_HAND_MIDDLE_4
        (16, 36),# LEFT_WRIST to LEFT_HAND_PINKY_1
        (4, 11), # NECK to RIGHT_CLAVICLE
        (11, 13),# RIGHT_CLAVICLE to RIGHT_SHOULDER
        (13, 15),# RIGHT_SHOULDER to RIGHT_ELBOW
        (15, 17),# RIGHT_ELBOW to RIGHT_WRIST
        (17, 31),# RIGHT_WRIST to RIGHT_HAND_THUMB_4
        (17, 33),# RIGHT_WRIST to RIGHT_HAND_INDEX_1
        (17, 35),# RIGHT_WRIST to RIGHT_HAND_MIDDLE_4
        (17, 37),# RIGHT_WRIST to RIGHT_HAND_PINKY_1
        (0, 18), # PELVIS to LEFT_HIP
        (18, 20),# LEFT_HIP to LEFT_KNEE
        (20, 22),# LEFT_KNEE to LEFT_ANKLE
        (22, 24),# LEFT_ANKLE to LEFT_BIG_TOE
        (22, 26),# LEFT_ANKLE to LEFT_SMALL_TOE
        (22, 28),# LEFT_ANKLE to LEFT_HEEL
        (0, 19), # PELVIS to RIGHT_HIP
        (19, 21),# RIGHT_HIP to RIGHT_KNEE
        (21, 23),# RIGHT_KNEE to RIGHT_ANKLE
        (23, 25),# RIGHT_ANKLE to RIGHT_BIG_TOE
        (23, 27),# RIGHT_ANKLE to RIGHT_SMALL_TOE
        (23, 29) # RIGHT_ANKLE to RIGHT_HEEL
    ]

    # Determine the initial offsets
    initial_offset_x = 0
    initial_offset_y = 0

    # Find the first frame with body data
    for frame in frames:
        bodies = frame['bodies']
        if bodies:
            body = bodies[0]  # Assuming we're focusing on the first detected body
            keypoints = body['keypoint']
            valid_coords = [(kp[0], kp[1]) for kp in keypoints if kp[0] is not None and kp[1] is not None]
            if valid_coords:
                xs, ys = zip(*valid_coords)
                initial_offset_x = np.mean(xs)
                initial_offset_y = np.mean(ys)
                break

    # Adjust the data by subtracting the initial offsets
    adjusted_frames = copy.deepcopy(frames)
    for frame in adjusted_frames:
        bodies = frame['bodies']
        for body in bodies:
            keypoints = body['keypoint']
            for idx, kp in enumerate(keypoints):
                if kp[0] is not None and kp[1] is not None:
                    # Subtract offsets from X and Y
                    keypoints[idx][0] -= initial_offset_x
                    keypoints[idx][1] -= initial_offset_y
                    # Z remains the same
                else:
                    continue

    def update(num):
        ax.clear()
        frame = adjusted_frames[num]
        bodies = frame['bodies']

        # Adjust axes limits and labels inside the update function
        ax.set_xlabel('X')
        ax.set_ylabel('Z')  # Swapped
        ax.set_zlabel('Y')  # Swapped

        # Set consistent axes limits
        ax.set_xlim([-1000, 1000])
        ax.set_ylim([-1000, 1000])
        ax.set_zlim([-1000, 1000])

        for body in bodies:
            keypoints = body['keypoint']
            # Filter out valid keypoints
            valid_indices = [idx for idx, kp in enumerate(keypoints) if kp[0] is not None and kp[1] is not None and kp[2] is not None]
            if not valid_indices:
                continue
            xs = [keypoints[idx][0] for idx in valid_indices]
            ys = [keypoints[idx][1] for idx in valid_indices]
            zs = [keypoints[idx][2] for idx in valid_indices]

            # Swap Y and Z coordinates
            ys, zs = zs, ys

            # Scatter plot for keypoints
            ax.scatter(xs, ys, zs, color='blue')

            # Draw skeleton connections
            for joint_a, joint_b in skeleton_connections:
                if joint_a in valid_indices and joint_b in valid_indices:
                    x_vals = [keypoints[joint_a][0], keypoints[joint_b][0]]
                    y_vals = [keypoints[joint_a][1], keypoints[joint_b][1]]
                    z_vals = [keypoints[joint_a][2], keypoints[joint_b][2]]

                    # Swap Y and Z values for the lines
                    y_vals, z_vals = z_vals, y_vals

                    ax.plot(x_vals, y_vals, z_vals, zorder=1, color='red')

        ax.set_title(f"Frame at timestamp {frame['timestamp']} ms")

    # Create the animation
    ani = animation.FuncAnimation(fig, update, frames=len(adjusted_frames), interval=1000/fps, repeat=False)

    # Save the animation as an MP4 file
    Writer = animation.writers['ffmpeg']
    writer = Writer(fps=fps, metadata=dict(artist='ZED Visualization'), bitrate=1800)
    ani.save(output_filename, writer=writer)

    print(f"Animation saved as {output_filename}")

# Load and visualize data
data_folder = "/home/daniel/labx_master/ZED2i_code/data/test_capture_V91"
json_file_path = f'{data_folder}/test_capture_V91_20241121_155631_30s.json'
json_data = load_json_data(json_file_path)
frames = parse_body_data(json_data)
visualize_body_tracking(frames, output_filename=f'{data_folder}/body_tracking_v3.mp4', fps=60)
