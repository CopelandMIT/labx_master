from flask import Flask, request, jsonify
import threading
import time
import math
import csv
import os
import logging
import argparse
from datetime import datetime

app = Flask(__name__)

# Data storage
offset_data = {}  # {sbc_id: (timestamp, data)}
data_lock = threading.Lock()

# Parse command-line arguments
parser = argparse.ArgumentParser(description="Central Server Script")
parser.add_argument("--base_filename", type=str, required=True, help="Base filename for the CSV")
parser.add_argument("--duration", type=int, required=True, help="Duration of the capture in ms")
parser.add_argument("--log_file", type=str, required=True, help="Log filepath for GUI")
parser.add_argument("--ip_address", type=str, required=True, help="IP address to host the central server")
parser.add_argument("--port", type=int, required=True, help="Port to host the central server")
parser.add_argument("--sync_metrics_dir", type=str, required=True, help="Port to host the central server")
args = parser.parse_args()

# Configure Flask app to use the specified IP and port
app_host = args.ip_address
app_port = args.port

# Set up logging using the passed log file
logging.basicConfig(
    filename=args.log_file,
    level=logging.INFO,
    format="%(asctime)s - %(levelname)s - %(message)s"
)
logging.info("Starting Central Server.")

# Generate a timestamp with microsecond accuracy
timestamp = datetime.now().strftime('%Y%m%d_%H%M%S%f')
duration_ms = 1000 * int(args.duration)

# Generate the CSV file path
CSV_FILE = f'/{args.sync_metrics_dir}/{args.base_filename}_{timestamp}_{duration_ms}ms.csv'

# Ensure the CSV file exists and has a header
if not os.path.isfile(CSV_FILE):
    os.makedirs(os.path.dirname(CSV_FILE), exist_ok=True)
    with open(CSV_FILE, 'w', newline='') as csvfile:
        fieldnames = ['timestamp', 'max_offset_ms', 'mean_offset_ms', 'jitter_ms', 'mean_root_dispersion_ms', 'max_root_dispersion_ms']
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
        writer.writeheader()

logging.info(f"Initialized CSV file: {CSV_FILE}")

@app.route('/receive_data', methods=['POST'])
def receive_data():
    """Receive and process data from sensors."""
    data = request.get_json()
    deployed_sensor_id = data.get('deployed_sensor_id')
    entry = data.get('data')  # Adjusted from 'entries' to 'entry'

    if deployed_sensor_id is None or entry is None:
        logging.error("Invalid data received: Missing 'deployed_sensor_id' or 'data'")
        return jsonify({'status': 'error', 'message': 'Invalid data'}), 400

    with data_lock:
        # Ensure that entry is a dictionary
        if not isinstance(entry, dict):
            logging.error("Invalid entry format received.")
            return jsonify({'status': 'error', 'message': 'Invalid entry format'}), 400

        timestamp = entry.get('timestamp')
        chronyc_output = entry.get('chronyc_output')
        if timestamp is None or chronyc_output is None:
            logging.error("Missing 'timestamp' or 'chronyc_output' in data.")
            return jsonify({'status': 'error', 'message': 'Missing timestamp or chronyc_output'}), 400

        # Parse metrics from chronyc_output
        chrony_data = parse_chronyc_output(chronyc_output)
        if not chrony_data:
            logging.error("Failed to parse chronyc output.")
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

        # Log the received data
        logging.info(f"Received data from sensor {deployed_sensor_id}: {chrony_data}")

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
                offset_str = line.split()[3]
                offset_value = ''.join(filter(lambda c: c.isdigit() or c == '.' or c == '-' or c == '+', offset_str))
                data['system_time_offset'] = float(offset_value)
            elif "Root dispersion" in line:
                disp_str = line.split()[3]
                disp_value = ''.join(filter(lambda c: c.isdigit() or c == '.' or c == '-' or c == '+', disp_str))
                data['root_dispersion'] = float(disp_value)
            elif "Root delay" in line:
                delay_str = line.split()[3]
                delay_value = ''.join(filter(lambda c: c.isdigit() or c == '.' or c == '-' or c == '+', delay_str))
                data['root_delay'] = float(delay_value)
            elif "Reference ID" in line:
                data['reference_id'] = line.split(":")[1].strip()
            elif "Stratum" in line:
                data['stratum'] = int(line.split(":")[1].strip())
    except Exception as e:
        logging.error(f"Error parsing chronyc output: {e}")
        return None
    return data

def calculate_synchronization_metrics():
    """Calculate and log synchronization metrics."""
    with data_lock:
        if len(offset_data) < 2:
            logging.warning("Not enough data to calculate synchronization metrics.")
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
                time_offsets.append(abs(offset_a - offset_b))

        if time_offsets:
            max_offset = max(time_offsets)
            mean_offset = sum(time_offsets) / len(time_offsets)
            stddev_offset = math.sqrt(sum((x - mean_offset) ** 2 for x in time_offsets) / len(time_offsets))

            max_offset_ms = max_offset * 1000
            mean_offset_ms = mean_offset * 1000
            jitter_ms = stddev_offset * 1000

            logging.info(f"Synchronization Metrics at {time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(current_time))}:")
            logging.info(f"  Max Offset: {max_offset_ms:.3f} ms, Mean Offset: {mean_offset_ms:.3f} ms, Jitter: {jitter_ms:.3f} ms")

            # Print synchronization metrics
            print(f"\nSynchronization Metrics at {time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(current_time))}:")
            print(f"  Max Offset Between SBCs: {max_offset_ms:.3f} ms")
            print(f"  Mean Offset Between SBCs: {mean_offset_ms:.3f} ms")
            print(f"  Jitter (Std Dev of Offsets): {jitter_ms:.3f} ms")

            root_dispersions = [metrics[deployed_sensor_id]['root_dispersion'] for deployed_sensor_id in deployed_sensor_ids]
            mean_root_dispersion = sum(root_dispersions) / len(root_dispersions)
            max_root_dispersion = max(root_dispersions)

            logging.info(f"  Root Dispersion Metrics: Mean: {mean_root_dispersion * 1000:.3f} ms, Max: {max_root_dispersion * 1000:.3f} ms")

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
                logging.error(f"Error saving data to CSV: {e}")
        else:
            logging.warning("No offsets to calculate metrics.")

if __name__ == '__main__':
    logging.info(f"Starting server at http://{app_host}:{app_port}/receive_data")
    app.run(host=app_host, port=app_port)
