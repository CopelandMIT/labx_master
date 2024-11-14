import json
import csv

# Load the JSON data from a file
input_json_file = '/usr/local/zed/samples/body tracking/export/JSON export/cpp/build/detected_bodies_18.json'
output_csv_file = 'tracked_bodies_18.csv'

with open(input_json_file, 'r') as f:
    data = json.load(f)

# Open CSV file for writing
with open(output_csv_file, 'w', newline='') as csvfile:
    csv_writer = csv.writer(csvfile)

    # Write the header (time, frame, and the 3D coordinates for each keypoint)
    # Assuming keypoints are tracked in 3D (x, y, z)
    num_keypoints = len(data[next(iter(data))]['body_list'][0]['keypoint']) if 'body_list' in data[next(iter(data))] else 0
    header = ['Time', 'Frame'] + [f'Keypoint_{i}_{axis}' for i in range(num_keypoints) for axis in ['x', 'y', 'z']]
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
