from flask import Flask, request, jsonify
import threading
import time
from collections import defaultdict
import math
import csv
import os

app = Flask(__name__)

# Data storage
offset_data = {}  # {sbc_id: (timestamp, data)}
data_lock = threading.Lock()

# CSV file for storing metrics
CSV_FILE = f'data/synchronization_metrics_1_camera_1_radar_{time.time()}.csv'

# Ensure the CSV file exists and has a header
if not os.path.isfile(CSV_FILE):
    with open(CSV_FILE, 'w', newline='') as csvfile:
        fieldnames = ['timestamp', 'max_offset_ms', 'mean_offset_ms', 'jitter_ms', 'mean_root_dispersion_ms', 'max_root_dispersion_ms']
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
        writer.writeheader()

@app.route('/receive_data', methods=['POST'])
def receive_data():
    data = request.get_json()
    sbc_id = data.get('sbc_id')
    entries = data.get('data')

    print(f"from {sbc_id}: {data}")

    if sbc_id is None or entries is None:
        return jsonify({'status': 'error', 'message': 'Invalid data'}), 400

    with data_lock:
        for entry in entries:
            timestamp = entry.get('timestamp')
            chronyc_output = entry.get('chronyc_output')
            if timestamp is None or chronyc_output is None:
                continue  # Skip invalid entries

            # Parse metrics from chronyc_output
            chrony_data = parse_chronyc_output(chronyc_output)
            if chrony_data:
                # Store the latest data
                offset_data[sbc_id] = (timestamp, chrony_data)

    return jsonify({'status': 'success'}), 200

def parse_chronyc_output(chronyc_output):
    """Extract required metrics from chronyc output."""
    data = {}
    try:
        for line in chronyc_output.strip().split('\n'):
            if "System time" in line:
                offset_str = line.split()[3]  # Fourth item is the value
                data['system_time_offset'] = float(offset_str)
            elif "Root dispersion" in line:
                disp_str = line.split()[3]  # Fourth item is the value
                data['root_dispersion'] = float(disp_str)
    except Exception as e:
        print(f"Error parsing chronyc output: {e}")
        return None
    return data

def calculate_synchronization():
    while True:
        time.sleep(60)  # Analysis interval
        with data_lock:
            current_time = time.time()
            # Filter out data older than 2 minutes
            recent_data = {sbc_id: (timestamp, data)
                           for sbc_id, (timestamp, data) in offset_data.items()
                           if current_time - timestamp <= 120}

            sbc_ids = list(recent_data.keys())
            if len(sbc_ids) < 2:
                print("Not enough recent data to calculate synchronization metrics.")
                continue

            # Collect the latest metrics from each SBC
            metrics = {sbc_id: data for sbc_id, (timestamp, data) in recent_data.items()}

            # Calculate time offsets between SBCs
            time_offsets = []
            for i in range(len(sbc_ids)):
                for j in range(i + 1, len(sbc_ids)):
                    sbc_a = sbc_ids[i]
                    sbc_b = sbc_ids[j]
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
                root_dispersions = [metrics[sbc_id]['root_dispersion'] for sbc_id in sbc_ids]
                mean_root_dispersion = sum(root_dispersions) / len(root_dispersions)
                max_root_dispersion = max(root_dispersions)

                print(f"Root Dispersion Metrics:")
                print(f"  Mean Root Dispersion: {mean_root_dispersion * 1000:.3f} ms")
                print(f"  Max Root Dispersion: {max_root_dispersion * 1000:.3f} ms")

                # Store metrics in CSV file
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
            else:
                print("No offsets to calculate metrics.")

if __name__ == '__main__':
    # Start the synchronization calculation thread
    print("Server Started")
    sync_thread = threading.Thread(target=calculate_synchronization, daemon=True)
    sync_thread.start()

    # Run the Flask app
    app.run(host='0.0.0.0', port=5000)
