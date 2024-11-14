import json
import csv

# Load the JSON data from a file
input_json_file = '/usr/local/zed/samples/body tracking/export/JSON export/cpp/build/data/detected_bodies_18.json'
output_csv_file = '/data/tracked_bodies_18.csv'

# Define the keypoint names in the correct order
keypoint_names = [
    'nose', 'neck', 'right_shoulder', 'right_elbow', 'right_wrist', 'left_shoulder', 'left_elbow',
    'left_wrist', 'right_hip', 'right_knee', 'right_ankle', 'left_hip', 'left_knee', 'left_ankle',
    'right_eye', 'left_eye', 'right_ear', 'left_ear'
]

with open(input_json_file, 'r') as f:
    data = json.load(f)

# Open CSV file for writing
with open(output_csv_file, 'w', newline='') as csvfile:
    csv_writer = csv.writer(csvfile)

    # Write the header (time, frame, and the 3D coordinates for each keypoint)
    # Assuming keypoints are tracked in 3D (x, y, z)
    header = ['Time', 'Frame'] + [f'{name}_{axis}' for name in keypoint_names for axis in ['x', 'y', 'z']]
    csv_writer.writerow(header)

    frame_number = 1

    # Loop through the JSON data
    for timestamp, body_data in data.items():
        if 'body_list' in body_data:
            for body in body_data['body_list']:
                keypoints = body.get('keypoint', [])
                # Flatten the 3D keypoints for each body
                keypoints_flat = [coord for point in keypoints for coord in point]
                # Write the row: timestamp, frame number, keypoints (x, y, z)
                csv_writer.writerow([timestamp, frame_number] + keypoints_flat)
                frame_number += 1

print(f'Data has been written to {output_csv_file}')
