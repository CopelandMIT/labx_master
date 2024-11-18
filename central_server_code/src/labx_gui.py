import tkinter as tk
from tkinter import messagebox
import os
import subprocess
import paramiko
import socket
import signal
import sys
import threading
import logging
import time

LOG_DIR = "/home/daniel/labx_master/central_server_code/logs"
os.makedirs(LOG_DIR, exist_ok=True)

logging.basicConfig(
    filename=os.path.join(LOG_DIR, f"sensor_output_{time.time()}.log"),
    level=logging.INFO,
    format="%(asctime)s - %(levelname)s - %(message)s"
)

logging.info("Starting Radar Data Collector script.")

# Define constants
DB_PATH = "path/to/central_server_code/data/lab_in_a_box.db"
CENTRAL_SERVER_SCRIPT = "/home/daniel/labx_master/central_server_code/src/central_server_v3.py"
PI_USERNAME = "dcope"
PI_PASSWORD = "CopeRasp5!"  # Replace with actual password or use key-based authentication
PORT = 5000

# Sensor type choices
SENSOR_TYPES = ["camera", "body_tracking", "radar"]

# Function to check if port is in use
def is_port_in_use(port):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        return s.connect_ex(('localhost', port)) == 0

# Function to kill the process using a specific port
def kill_process_on_port(port):
    try:
        result = subprocess.run(["lsof", "-t", f"-i:{port}"], capture_output=True, text=True)
        pid = result.stdout.strip()
        if pid:
            os.kill(int(pid), signal.SIGKILL)
            print(f"Killed process on port {port}")
            return True
    except Exception as e:
        print(f"Error killing process on port {port}: {e}")
        return False

# Function to start the central server
def start_central_server():
    if is_port_in_use(PORT):
        if not kill_process_on_port(PORT):
            messagebox.showerror("Error", f"Failed to free up port {PORT}.")
            return
    try:
        subprocess.Popen(["python3", CENTRAL_SERVER_SCRIPT])
        messagebox.showinfo("Server Started", f"Central server started on port {PORT}.")
    except Exception as e:
        messagebox.showerror("Error", f"Failed to start central server: {e}")


def start_remote_capture_threaded(ip_address, sensor_type, base_filename, capture_duration):
    threading.Thread(
        target=start_remote_capture,
        args=(ip_address, sensor_type, base_filename, capture_duration),
        daemon=True
    ).start()

def start_remote_capture(ip_address, sensor_type, base_filename, capture_duration):
    logging.info(f"Starting remote capture: {ip_address}, {sensor_type}, {base_filename}, {capture_duration}")
    ssh = paramiko.SSHClient()
    ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    private_key_path = "/home/daniel/.ssh/id_rsa"  # Path to private key
    
    try:
        # Load private key
        private_key = paramiko.RSAKey.from_private_key_file(private_key_path)
        
        # Connect to the remote host
        ssh.connect(ip_address, username=PI_USERNAME, pkey=private_key)

        # Command based on sensor type
        if sensor_type == "camera":
            command = (
                f"/home/pi/labx_master/camera_code/labx_env/bin/python "
                f"/home/pi/labx_master/camera_code/src/CameraDataCollector.py "
                f"--base_filename {base_filename} --capture_duration {capture_duration}"
            )
        elif sensor_type == "radar":
            command = (
                f"/home/dcope/labx_master/radar_code/labx_env/bin/python "
                f"/home/dcope/labx_master/radar_code/src/RadarDataCollector.py "
                f"--base_filename {base_filename} --capture_duration {capture_duration}"
            )
        else:
            messagebox.showerror("Error", f"Unsupported sensor type: {sensor_type}")
            return

        # Execute the command
        stdin, stdout, stderr = ssh.exec_command(command)

        # Read output line by line
        for line in iter(stdout.readline, ""):
            print(f"STDOUT: {line.strip()}")
            logging.info(f"STDOUT: {line.strip()}")
        
        for line in iter(stderr.readline, ""):
            print(f"STDERR: {line.strip()}")
            logging.error(f"STDERR: {line.strip()}")

        messagebox.showinfo("Capture Started", f"Capture started on {ip_address} for {sensor_type} with base_filename '{base_filename}' and capture_duration {capture_duration} seconds.")
    except Exception as e:
        messagebox.showerror("Connection Error", f"Could not connect to {ip_address}: {e}")
    finally:
        ssh.close()

# # Function to ping the sensor to check connectivity
# def ping_sensor(ip_address):
#     response = os.system(f"ping -c 1 {ip_address}")
#     return response == 0

# Start Capture logic
def start_capture():
    ip_address = ip_entry.get()
    base_filename = base_filename_entry.get()
    capture_duration = capture_duration_entry.get()
    sensor_type = sensor_type_var.get()
    if not ip_address or not base_filename or not capture_duration.isdigit():
        messagebox.showerror("Input Error", "Please enter a valid IP address, base_filename, and capture_duration.")
        return
    if not sensor_type:
        messagebox.showerror("Input Error", "Please select a sensor type.")
        return
    # if not ping_sensor(ip_address):
    #     messagebox.showerror("Ping Error", f"Sensor at {ip_address} is unreachable.")
    #     return
    start_remote_capture_threaded(ip_address, sensor_type, base_filename, int(capture_duration))


# Function to handle termination signals and close the GUI
def handle_exit_signal(signum, frame):
    print("Termination signal received. Closing GUI...")
    root.quit()  # Stop the Tkinter main loop
    root.destroy()  # Close the window

# Set up the signal handlers
signal.signal(signal.SIGINT, handle_exit_signal)
signal.signal(signal.SIGTERM, handle_exit_signal)

# Initialize Tkinter
root = tk.Tk()
root.title("Lab in a Box - Control Panel")

# Central Server Controls
tk.Label(root, text="Central Server Controls:").grid(row=0, column=0, padx=10, pady=5)
server_button = tk.Button(root, text="Start Central Server", command=start_central_server)
server_button.grid(row=0, column=1, padx=10, pady=5)

# Create StringVar instances to hold default values
ip_default = tk.StringVar(value="192.168.68.1")
base_filename_default = tk.StringVar(value="test_capture_V")
capture_duration_default = tk.StringVar(value="10")

# Sensor Controls
tk.Label(root, text="Sensor IP Address:").grid(row=1, column=0, padx=10, pady=5)
ip_entry = tk.Entry(root, textvariable=ip_default)
ip_entry.grid(row=1, column=1, padx=10, pady=5)

tk.Label(root, text="Base Filename:").grid(row=2, column=0, padx=10, pady=5)
base_filename_entry = tk.Entry(root, textvariable=base_filename_default)
base_filename_entry.grid(row=2, column=1, padx=10, pady=5)

tk.Label(root, text="Capture Duration (seconds):").grid(row=3, column=0, padx=10, pady=5)
capture_duration_entry = tk.Entry(root, textvariable=capture_duration_default)
capture_duration_entry.grid(row=3, column=1, padx=10, pady=5)

# Sensor Type Selection
tk.Label(root, text="Sensor Type:").grid(row=4, column=0, padx=10, pady=5)
sensor_type_var = tk.StringVar(value=SENSOR_TYPES[0])  # Default to first option
sensor_buttons = [
    tk.Radiobutton(root, text=type_, variable=sensor_type_var, value=type_)
    for type_ in SENSOR_TYPES
]
for i, button in enumerate(sensor_buttons, start=1):
    button.grid(row=4, column=i, padx=5, pady=5)

# Start Capture Button
start_button = tk.Button(root, text="Start Capture", command=start_capture)
start_button.grid(row=5, column=0, columnspan=2, pady=10)

root.mainloop()
