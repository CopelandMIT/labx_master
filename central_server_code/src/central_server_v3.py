from flask import Flask, request, jsonify
import threading
import time
import math
import csv
import os
import json

app = Flask(__name__)

# Data storage
offset_data = {}  # {sbc_id: (timestamp, data)}
data_lock = threading.Lock()

import argparse
from datetime import datetime

# Parse command-line arguments
parser = argparse.ArgumentParser(description="Central Server Script")
parser.add_argument("--base_filename", type=str, required=True, help="Base filename for the CSV")
parser.add_argument("--duration", type=int, required=True, help="Duration of the capture in ms")
parser.add_argument("--log_file", type=str, required=True, help="Log filepath for GUI")
args = parser.parse_args()

# Generate a timestamp with microsecond accuracy
timestamp = datetime.now().strftime('%Y%m%d_%H%M%S%f')
duration_ms = 1000 * int(args.duration)
# Generate the CSV file path
CSV_FILE = f'/home/daniel/labx_master/central_server_code/data/sync_metrics/{args.base_filename}_{timestamp}_{duration_ms}ms.csv'

# Ensure the CSV file exists and has a header
if not os.path.isfile(CSV_FILE):
    os.makedirs(os.path.dirname(CSV_FILE), exist_ok=True)
    with open(CSV_FILE, 'w', newline='') as csvfile:
        fieldnames = ['timestamp', 'max_offset_ms', 'mean_offset_ms', 'jitter_ms', 'mean_root_dispersion_ms', 'max_root_dispersion_ms']
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
        writer.writeheader()

@app.route('/receive_data', methods=['POST'])
def receive_data():
    data = request.get_json()
    deployed_sensor_id = data.get('deployed_sensor_id')
    entry = data.get('data')  # Adjusted from 'entries' to 'entry'

    if deployed_sensor_id is None or entry is None:
        return jsonify({'status': 'error', 'message': 'Invalid data'}), 400

    with data_lock:
        # Ensure that entry is a dictionary
        if not isinstance(entry, dict):
            return jsonify({'status': 'error', 'message': 'Invalid entry format'}), 400

        timestamp = entry.get('timestamp')
        chronyc_output = entry.get('chronyc_output')
        if timestamp is None or chronyc_output is None:
            return jsonify({'status': 'error', 'message': 'Missing timestamp or chronyc_output'}), 400

            # Parse metrics from chronyc_output
        chrony_data = parse_chronyc_output(chronyc_output)
        if not chrony_data:
            return jsonify({'status': 'error', 'message': 'Failed to parse chronyc output'}), 400

        # Print the parsed values in a structured format
        print("\n--- Received Data ---")
        print(f"Deployed Sensor ID: {deployed_sensor_id}")
        print(f"Timestamp: {timestamp}")
        print(f"Reference ID: {chrony_data.get('reference_id', 'N/A')}")
        print(f"Stratum: {chrony_data.get('stratum', 'N/A')}")
        print(f"System Time Offset: {chrony_data.get('system_time_offset', 'N/A')} seconds")
        print(f"Root Dispersion: {chrony_data.get('root_dispersion', 'N/A')} seconds")
        print(f"Root Delay: {chrony_data.get('root_delay', 'N/A')} seconds")
        print("--- End of Data ---\n")

        # Store the parsed data
        offset_data[deployed_sensor_id] = (timestamp, chrony_data)

    # After receiving new data, calculate synchronization metrics and save them immediately
    calculate_synchronization_metrics()

    return jsonify({'status': 'success'}), 200

def parse_chronyc_output(chronyc_output):
    """Extract required metrics from chronyc output."""
    data = {}
    try:
        for line in chronyc_output.strip().split('\n'):
            if "System time" in line:
                offset_str = line.split()[3]  # Fourth item is the value
                # Remove any non-numeric characters (e.g., 'seconds')
                offset_value = ''.join(filter(lambda c: c.isdigit() or c == '.' or c == '-' or c == '+', offset_str))
                data['system_time_offset'] = float(offset_value)
            elif "Root dispersion" in line:
                disp_str = line.split()[3]  # Fourth item is the value
                disp_value = ''.join(filter(lambda c: c.isdigit() or c == '.' or c == '-' or c == '+', disp_str))
                data['root_dispersion'] = float(disp_value)
            elif "Root delay" in line:
                delay_str = line.split()[3]  # Fourth item is the value
                delay_value = ''.join(filter(lambda c: c.isdigit() or c == '.' or c == '-' or c == '+', delay_str))
                data['root_delay'] = float(delay_value)
            elif "Reference ID" in line:
                data['reference_id'] = line.split(":")[1].strip()
            elif "Stratum" in line:
                data['stratum'] = int(line.split(":")[1].strip())
    except Exception as e:
        print(f"Error parsing chronyc output: {e}")
        return None
    return data

def calculate_synchronization_metrics():
    with data_lock:
        if len(offset_data) < 2:
            print("Not enough data to calculate synchronization metrics.")
            return

        # Collect the latest metrics from each SBC
        metrics = {sbc_id: data for sbc_id, (timestamp, data) in offset_data.items()}

        deployed_sensor_ids = list(metrics.keys())
        current_time = time.time()

        # Calculate time offsets between SBCs
        time_offsets = []
        for i in range(len(deployed_sensor_ids)):
            for j in range(i + 1, len(deployed_sensor_ids)):
                sbc_a = deployed_sensor_ids[i]
                sbc_b = deployed_sensor_ids[j]
                offset_a = metrics[sbc_a]['system_time_offset']
                offset_b = metrics[sbc_b]['system_time_offset']
                time_offset = abs(offset_a - offset_b)
                time_offsets.append(time_offset)

        # Compute synchronization metrics
        if time_offsets:
            max_offset = max(time_offsets)
            mean_offset = sum(time_offsets) / len(time_offsets)
            stddev_offset = math.sqrt(sum((x - mean_offset) ** 2 for x in time_offsets) / len(time_offsets))

            # Convert offsets to milliseconds for readability
            max_offset_ms = max_offset * 1000
            mean_offset_ms = mean_offset * 1000
            jitter_ms = stddev_offset * 1000  # Jitter is the standard deviation

            print(f"\nSynchronization Metrics at {time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(current_time))}:")
            print(f"  Max Offset Between SBCs: {max_offset_ms:.3f} ms")
            print(f"  Mean Offset Between SBCs: {mean_offset_ms:.3f} ms")
            print(f"  Jitter (Std Dev of Offsets): {jitter_ms:.3f} ms")

            # Root Dispersion Analysis
            root_dispersions = [metrics[deployed_sensor_id]['root_dispersion'] for deployed_sensor_id in deployed_sensor_ids]
            mean_root_dispersion = sum(root_dispersions) / len(root_dispersions)
            max_root_dispersion = max(root_dispersions)

            print(f"Root Dispersion Metrics:")
            print(f"  Mean Root Dispersion: {mean_root_dispersion * 1000:.3f} ms")
            print(f"  Max Root Dispersion: {max_root_dispersion * 1000:.3f} ms")

            # Save metrics immediately to the CSV file to avoid data loss
            try:
                with open(CSV_FILE, 'a', newline='') as csvfile:
                    fieldnames = ['timestamp', 'max_offset_ms', 'mean_offset_ms', 'jitter_ms', 'mean_root_dispersion_ms', 'max_root_dispersion_ms']
                    writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
                    writer.writerow({
                        'timestamp': current_time,
                        'max_offset_ms': max_offset_ms,
                        'mean_offset_ms': mean_offset_ms,
                        'jitter_ms': jitter_ms,
                        'mean_root_dispersion_ms': mean_root_dispersion * 1000,
                        'max_root_dispersion_ms': max_root_dispersion * 1000
                    })
            except IOError as e:
                print(f"Error saving data to CSV: {e}")
        else:
            print("No offsets to calculate metrics.")

if __name__ == '__main__':
    # Run the Flask app
    print("Server Started")
    app.run(host='0.0.0.0', port=5000)
