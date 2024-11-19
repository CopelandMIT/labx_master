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
    CENTRAL_SERVER_SCRIPT = "/home/daniel/labx_master/central_server_code/src/central_server_v3.py"
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
        self.root.title("Lab in a Box - Setup and Control Panel")

        # Configurations list for multiple RPis
        self.configurations = []

        # Set up the GUI components
        self.setup_gui()

        # Set up signal handlers
        signal.signal(signal.SIGINT, self.handle_exit_signal)
        signal.signal(signal.SIGTERM, self.handle_exit_signal)

    def setup_gui(self):
        # Configuration setup
        tk.Label(self.root, text="Experiment Setup:").grid(row=0, column=0, columnspan=4, pady=5)

        # Create StringVar instances for default values
        self.ip_default = tk.StringVar(value="192.168.68.1")
        base_filename_default = tk.StringVar(value="test_capture_V")
        capture_duration_default = tk.StringVar(value="30")

        # Base Filename and Capture Duration
        tk.Label(self.root, text="Base Filename:").grid(row=1, column=0, padx=5, pady=5)
        self.base_filename_entry = tk.Entry(self.root, textvariable=base_filename_default)
        self.base_filename_entry.grid(row=1, column=1, columnspan=3, pady=5)

        tk.Label(self.root, text="Capture Duration (seconds):").grid(row=2, column=0, padx=5, pady=5)
        self.capture_duration_entry = tk.Entry(self.root, textvariable=capture_duration_default)
        self.capture_duration_entry.grid(row=2, column=1, columnspan=3, pady=5)

        # Add Devices
        tk.Label(self.root, text="IP Address:").grid(row=3, column=0, padx=5, pady=5)
        tk.Label(self.root, text="Username:").grid(row=3, column=1, padx=5, pady=5)
        tk.Label(self.root, text="Sensor Type:").grid(row=3, column=2, padx=5, pady=5)

        self.ip_entry = tk.Entry(self.root, textvariable=self.ip_default)
        self.ip_entry.grid(row=4, column=0, padx=5, pady=5)

        self.username_entry = tk.Entry(self.root)
        self.username_entry.grid(row=4, column=1, padx=5, pady=5)

        self.sensor_type_var = tk.StringVar(value=self.SENSOR_TYPES[0])  # Default to first option
        self.sensor_type_menu = tk.OptionMenu(self.root, self.sensor_type_var, *self.SENSOR_TYPES)
        self.sensor_type_menu.grid(row=4, column=2, padx=5, pady=5)

        add_device_button = tk.Button(self.root, text="Add Device", command=self.add_device)
        add_device_button.grid(row=4, column=3, padx=5, pady=5)

        # Device List
        tk.Label(self.root, text="Configured Devices:").grid(row=5, column=0, columnspan=4, pady=5)
        self.config_listbox = tk.Listbox(self.root, width=70, height=10)
        self.config_listbox.grid(row=6, column=0, columnspan=4, padx=10, pady=5)

        # Start All Captures Button
        start_all_button = tk.Button(self.root, text="Start All Captures", command=self.start_all_captures)
        start_all_button.grid(row=7, column=0, columnspan=4, pady=10)

    def add_device(self):
        """Add a device configuration to the list."""
        ip_address = self.ip_entry.get()
        username = self.username_entry.get()
        sensor_type = self.sensor_type_var.get()
        if not ip_address or not username or not sensor_type:
            messagebox.showerror("Input Error", "Please enter a valid IP address, username, and select a sensor type.")
            return

        # Add configuration to the list
        config = {"ip_address": ip_address, "username": username, "sensor_type": sensor_type}
        self.configurations.append(config)
        self.config_listbox.insert(tk.END, f"IP: {ip_address}, Username: {username}, Sensor: {sensor_type}")

        # Calculate next IP address
        octets = ip_address.split(".")
        if len(octets) == 4 and octets[3].isdigit():
            next_ip = f"{octets[0]}.{octets[1]}.{octets[2]}.{int(octets[3]) + 1}"
            self.ip_default.set(next_ip)  # Update default IP entry
        else:
            messagebox.showwarning("Invalid IP", "IP Address is invalid. Please enter a valid IP next time.")

        # Clear the username entry
        self.username_entry.delete(0, tk.END)

    def is_port_in_use(self, port):
        """Check if a given port is in use."""
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            return s.connect_ex(('localhost', port)) == 0

    def kill_process_on_port(self, port):
        """Kill the process using the specified port."""
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
        """Launch the central server, ensuring the port is available."""
        if self.is_port_in_use(self.PORT):
            logging.warning(f"Port {self.PORT} is in use. Attempting to free it.")
            if not self.kill_process_on_port(self.PORT):
                messagebox.showerror("Error", f"Failed to free up port {self.PORT}.")
                return False
        try:
            logging.info("Starting the central server.")
            subprocess.Popen(["python3", self.CENTRAL_SERVER_SCRIPT])
            logging.info("Central server launched successfully.")
            time.sleep(2)  # Allow time for server initialization
            return True
        except Exception as e:
            logging.error(f"Failed to start central server: {e}")
            messagebox.showerror("Error", f"Failed to start central server: {e}")
            return False

    def start_all_captures(self):
        """Start captures for all configured devices and launch the central server."""
        base_filename = self.base_filename_entry.get()
        capture_duration = self.capture_duration_entry.get()

        if not base_filename or not capture_duration.isdigit():
            messagebox.showerror("Input Error", "Please enter a valid base_filename and capture_duration.")
            return

        # Start the central server
        if not self.start_central_server():
            return  # Exit if the server fails to start

        threads = []
        for config in self.configurations:
            thread = threading.Thread(
                target=self.start_remote_capture,
                args=(config["ip_address"], config["username"], config["sensor_type"], base_filename, int(capture_duration)),
                daemon=True
            )
            threads.append(thread)
            thread.start()

        for thread in threads:
            thread.join()

        messagebox.showinfo("Capture Started", "All captures have been started.")



    def start_remote_capture(self, ip_address, username, sensor_type, base_filename, capture_duration):
        """Execute the remote capture command on an RPi."""
        logging.info(f"Starting remote capture for IP: {ip_address}, Username: {username}, Sensor Type: {sensor_type}")
        ssh = paramiko.SSHClient()
        ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        private_key_path = "/home/daniel/.ssh/id_rsa"

        try:
            private_key = paramiko.RSAKey.from_private_key_file(private_key_path)
            ssh.connect(ip_address, username=username, pkey=private_key)

            # Map sensor type to the correct command
            if sensor_type == "camera":
                command = (
                    f"/home/{username}/labx_master/camera_code/labx_env/bin/python "
                    f"/home/{username}/labx_master/camera_code/src/CameraDataCollector.py "
                    f"--base_filename {base_filename} --capture_duration {capture_duration}"
                )
            elif sensor_type == "radar":
                command = (
                    f"/home/{username}/labx_master/radar_code/labx_env/bin/python "
                    f"/home/{username}/labx_master/radar_code/src/RadarDataCollector.py "
                    f"--base_filename {base_filename} --capture_duration {capture_duration}"
                )
            else:
                logging.error(f"Unsupported sensor type: {sensor_type}")
                return

            logging.info(f"Executing command on {ip_address}: {command}")
            stdin, stdout, stderr = ssh.exec_command(command)
            for line in iter(stdout.readline, ""):
                logging.info(f"STDOUT: {line.strip()}")
            for line in iter(stderr.readline, ""):
                logging.error(f"STDERR: {line.strip()}")

            logging.info(f"Capture started on {ip_address} for {sensor_type}")
        except Exception as e:
            logging.error(f"Connection Error: Could not connect to {ip_address}: {e}")
        finally:
            ssh.close()

    def handle_exit_signal(self, signum, frame):
        logging.info("Termination signal received. Closing GUI...")
        self.root.quit()
        self.root.destroy()

    def run(self):
        self.root.mainloop()


if __name__ == "__main__":
    app = LabInABoxControlPanel()
    app.run()
