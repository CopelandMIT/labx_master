import tkinter as tk
from tkinter import messagebox
import os
import subprocess
import paramiko
import socket
import signal
import threading
import logging
import time


class LabInABoxControlPanel:
    LOG_DIR = "/home/daniel/labx_master/central_server_code/logs"
    DB_PATH = "path/to/central_server_code/data/lab_in_a_box.db"
    CENTRAL_SERVER_SCRIPT = "/home/daniel/labx_master/central_server_code/src/central_server_v3.py"
    PI_USERNAME = "dcope"
    PORT = 5000
    SENSOR_TYPES = ["camera", "body_tracking", "radar"]

    def __init__(self):
        # Set up logging
        os.makedirs(self.LOG_DIR, exist_ok=True)
        logging.basicConfig(
            filename=os.path.join(self.LOG_DIR, f"sensor_output_{time.time()}.log"),
            level=logging.INFO,
            format="%(asctime)s - %(levelname)s - %(message)s"
        )
        logging.info("Starting Lab In A Box Control Panel.")

        # Initialize Tkinter
        self.root = tk.Tk()
        self.root.title("Lab in a Box - Control Panel")

        # Set up the GUI components
        self.setup_gui()

        # Set up signal handlers
        signal.signal(signal.SIGINT, self.handle_exit_signal)
        signal.signal(signal.SIGTERM, self.handle_exit_signal)

    def setup_gui(self):
        # Central Server Controls
        tk.Label(self.root, text="Central Server Controls:").grid(row=0, column=0, padx=10, pady=5)
        server_button = tk.Button(self.root, text="Start Central Server", command=self.start_central_server)
        server_button.grid(row=0, column=1, padx=10, pady=5)

        # Create StringVar instances for inputs
        self.ip_default = tk.StringVar(value="192.168.68.1")
        self.base_filename_default = tk.StringVar(value="test_capture_V")
        self.capture_duration_default = tk.StringVar(value="10")
        self.sensor_type_var = tk.StringVar(value=self.SENSOR_TYPES[0])  # Default to first option

        # Sensor Controls
        tk.Label(self.root, text="Sensor IP Address:").grid(row=1, column=0, padx=10, pady=5)
        self.ip_entry = tk.Entry(self.root, textvariable=self.ip_default)
        self.ip_entry.grid(row=1, column=1, padx=10, pady=5)

        tk.Label(self.root, text="Base Filename:").grid(row=2, column=0, padx=10, pady=5)
        self.base_filename_entry = tk.Entry(self.root, textvariable=self.base_filename_default)
        self.base_filename_entry.grid(row=2, column=1, padx=10, pady=5)

        tk.Label(self.root, text="Capture Duration (seconds):").grid(row=3, column=0, padx=10, pady=5)
        self.capture_duration_entry = tk.Entry(self.root, textvariable=self.capture_duration_default)
        self.capture_duration_entry.grid(row=3, column=1, padx=10, pady=5)

        # Sensor Type Selection
        tk.Label(self.root, text="Sensor Type:").grid(row=4, column=0, padx=10, pady=5)
        for i, type_ in enumerate(self.SENSOR_TYPES, start=1):
            tk.Radiobutton(self.root, text=type_, variable=self.sensor_type_var, value=type_).grid(row=4, column=i, padx=5, pady=5)

        # Start Capture Button
        start_button = tk.Button(self.root, text="Start Capture", command=self.start_capture)
        start_button.grid(row=5, column=0, columnspan=2, pady=10)

    def is_port_in_use(self, port):
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            return s.connect_ex(('localhost', port)) == 0

    def kill_process_on_port(self, port):
        try:
            result = subprocess.run(["lsof", "-t", f"-i:{port}"], capture_output=True, text=True)
            pid = result.stdout.strip()
            if pid:
                os.kill(int(pid), signal.SIGKILL)
                logging.info(f"Killed process on port {port}")
                return True
        except Exception as e:
            logging.error(f"Error killing process on port {port}: {e}")
        return False

    def start_central_server(self):
        if self.is_port_in_use(self.PORT):
            if not self.kill_process_on_port(self.PORT):
                messagebox.showerror("Error", f"Failed to free up port {self.PORT}.")
                return
        try:
            subprocess.Popen(["python3", self.CENTRAL_SERVER_SCRIPT])
            messagebox.showinfo("Server Started", f"Central server started on port {self.PORT}.")
        except Exception as e:
            logging.error(f"Failed to start central server: {e}")
            messagebox.showerror("Error", f"Failed to start central server: {e}")

    def start_remote_capture(self, ip_address, sensor_type, base_filename, capture_duration):
        logging.info(f"Starting remote capture: {ip_address}, {sensor_type}, {base_filename}, {capture_duration}")
        ssh = paramiko.SSHClient()
        ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        private_key_path = "/home/daniel/.ssh/id_rsa"  # Path to private key

        try:
            private_key = paramiko.RSAKey.from_private_key_file(private_key_path)
            ssh.connect(ip_address, username=self.PI_USERNAME, pkey=private_key)

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

            stdin, stdout, stderr = ssh.exec_command(command)

            for line in iter(stdout.readline, ""):
                logging.info(f"STDOUT: {line.strip()}")
            for line in iter(stderr.readline, ""):
                logging.error(f"STDERR: {line.strip()}")

            messagebox.showinfo("Capture Started", f"Capture started on {ip_address} for {sensor_type} with base_filename '{base_filename}' and capture_duration {capture_duration} seconds.")
        except Exception as e:
            logging.error(f"Connection Error: Could not connect to {ip_address}: {e}")
            messagebox.showerror("Connection Error", f"Could not connect to {ip_address}: {e}")
        finally:
            ssh.close()

    def start_remote_capture_threaded(self, ip_address, sensor_type, base_filename, capture_duration):
        threading.Thread(
            target=self.start_remote_capture,
            args=(ip_address, sensor_type, base_filename, capture_duration),
            daemon=True
        ).start()

    def start_capture(self):
        ip_address = self.ip_entry.get()
        base_filename = self.base_filename_entry.get()
        capture_duration = self.capture_duration_entry.get()
        sensor_type = self.sensor_type_var.get()

        if not ip_address or not base_filename or not capture_duration.isdigit():
            messagebox.showerror("Input Error", "Please enter a valid IP address, base_filename, and capture_duration.")
            return
        if not sensor_type:
            messagebox.showerror("Input Error", "Please select a sensor type.")
            return
        self.start_remote_capture_threaded(ip_address, sensor_type, base_filename, int(capture_duration))

    def handle_exit_signal(self, signum, frame):
        logging.info("Termination signal received. Closing GUI...")
        self.root.quit()
        self.root.destroy()

    def run(self):
        self.root.mainloop()


if __name__ == "__main__":
    app = LabInABoxControlPanel()
    app.run()
