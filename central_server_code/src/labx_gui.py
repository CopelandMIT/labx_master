import tkinter as tk
from tkinter import messagebox
import os
import subprocess
import paramiko
import socket
import signal

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

# Function to run the appropriate capture script remotely
def start_remote_capture(ip_address, sensor_type, filename, duration):
    ssh = paramiko.SSHClient()
    ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    try:
        ssh.connect(ip_address, username=PI_USERNAME, password=PI_PASSWORD)

        # Command varies based on sensor type
        if sensor_type == "camera":
            command = (
                f"/home/pi/labx_master/camera_code/labx_env/bin/python "
                f"/home/pi/labx_master/camera_code/src/CameraDataCollector.py "
                f"--video_filename {filename} --duration {duration}"
            )
        elif sensor_type == "radar":
            command = (
                f"/home/dcope/labx_master/radar_code/labx_env/bin/python "
                f"/home/dcope/labx_master/radar_code/src/RadarDataCollector.py "
                f"--data_file {filename} --duration {duration}"
            )
        else:
            messagebox.showerror("Error", f"Unsupported sensor type: {sensor_type}")
            return

        # Execute the command
        stdin, stdout, stderr = ssh.exec_command(command)
        output = stdout.read().decode()
        error = stderr.read().decode()

        print("STDOUT:", output)
        print("STDERR:", error)

        if error:
            messagebox.showerror("Error", f"Failed to start capture: {error}")
        else:
            messagebox.showinfo("Capture Started", f"Capture started on {ip_address} for {sensor_type} with filename '{filename}' and duration {duration} seconds.")
    except Exception as e:
        messagebox.showerror("Connection Error", f"Could not connect to {ip_address}: {e}")
    finally:
        ssh.close()

# Function to ping the sensor to check connectivity
def ping_sensor(ip_address):
    response = os.system(f"ping -c 1 {ip_address}")
    return response == 0

# Start Capture logic
def start_capture():
    ip_address = ip_entry.get()
    filename = filename_entry.get()
    duration = duration_entry.get()
    sensor_type = sensor_type_var.get()
    if not ip_address or not filename or not duration.isdigit():
        messagebox.showerror("Input Error", "Please enter a valid IP address, filename, and duration.")
        return
    if not sensor_type:
        messagebox.showerror("Input Error", "Please select a sensor type.")
        return
    if not ping_sensor(ip_address):
        messagebox.showerror("Ping Error", f"Sensor at {ip_address} is unreachable.")
        return
    start_remote_capture(ip_address, sensor_type, filename, int(duration))

# Initialize Tkinter
root = tk.Tk()
root.title("Lab in a Box - Control Panel")

# Central Server Controls
tk.Label(root, text="Central Server Controls:").grid(row=0, column=0, padx=10, pady=5)
server_button = tk.Button(root, text="Start Central Server", command=start_central_server)
server_button.grid(row=0, column=1, padx=10, pady=5)

# Sensor Controls
tk.Label(root, text="Sensor IP Address:").grid(row=1, column=0, padx=10, pady=5)
ip_entry = tk.Entry(root)
ip_entry.grid(row=1, column=1, padx=10, pady=5)

tk.Label(root, text="Filename:").grid(row=2, column=0, padx=10, pady=5)
filename_entry = tk.Entry(root)
filename_entry.grid(row=2, column=1, padx=10, pady=5)

tk.Label(root, text="Capture Duration (seconds):").grid(row=3, column=0, padx=10, pady=5)
duration_entry = tk.Entry(root)
duration_entry.grid(row=3, column=1, padx=10, pady=5)

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
