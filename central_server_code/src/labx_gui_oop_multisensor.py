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
from datetime import datetime
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import pandas as pd
import requests

class LabInABoxControlPanel:
    LOG_DIR = "/home/daniel/labx_master/central_server_code/logs"
    CENTRAL_SERVER_SCRIPT = "/home/daniel/labx_master/central_server_code/src/central_server_v3.py"
    SYNC_METRICS_DIR = "/home/daniel/labx_master/central_server_code/data/sync_metrics"
    PORT = 5000
    SENSOR_TYPES = ["camera", "body_tracking", "radar"]



    # --------------------------------------
    # Initialization and Setup
    # --------------------------------------

    def __init__(self):
        # Ensure the log directory exists
        os.makedirs(self.LOG_DIR, exist_ok=True)
        os.makedirs(self.SYNC_METRICS_DIR, exist_ok=True)

        # Generate the timestamp-based log file name
        timestamp = datetime.now().strftime('%Y%m%d_%H%M%S%f')
        self.log_file = os.path.join(self.LOG_DIR, f"labx_gui_log_{timestamp}.log")

        # Set up logging
        logging.basicConfig(
            filename=self.log_file,
            level=logging.INFO,
            format="%(asctime)s - %(levelname)s - %(message)s"
        )
        logging.info("Starting Lab In A Box Control Panel.")

        # Initialize Tkinter
        self.root = tk.Tk()
        self.root.title("Lab in a Box - Setup and Control Panel")

        # Configurations list for multiple RPis
        self.configurations = []

        self.max_offset_data = []
        self.timestamps = []
        self.plot_running = False

        # Setup GUI with plotting
        self.setup_gui_with_plot()

        self.central_server_process = None
        self.current_csv_file = None  # To track the latest CSV file

        # Set up signal handlers
        signal.signal(signal.SIGINT, self.handle_exit_signal)
        signal.signal(signal.SIGTERM, self.handle_exit_signal)

    def setup_gui_with_plot(self):
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
        tk.Label(self.root, text="Status:").grid(row=3, column=3, padx=5, pady=5)

        self.ip_entry = tk.Entry(self.root, textvariable=self.ip_default)
        self.ip_entry.grid(row=4, column=0, padx=5, pady=5)

        self.username_entry = tk.Entry(self.root)
        self.username_entry.grid(row=4, column=1, padx=5, pady=5)

        self.sensor_type_var = tk.StringVar(value=self.SENSOR_TYPES[0])  # Default to first option
        self.sensor_type_menu = tk.OptionMenu(self.root, self.sensor_type_var, *self.SENSOR_TYPES)
        self.sensor_type_menu.grid(row=4, column=2, padx=5, pady=5)

        self.status_canvas = tk.Canvas(self.root, width=20, height=20)
        self.status_canvas.grid(row=4, column=3, padx=5, pady=5)
        self.update_status_indicator("gray")  # Start with neutral (gray) status

        add_device_button = tk.Button(self.root, text="Add Device", command=self.add_device)
        add_device_button.grid(row=4, column=4, padx=5, pady=5)

        # Add live plot to the GUI
        self.fig, self.ax = plt.subplots()
        self.canvas = FigureCanvasTkAgg(self.fig, master=self.root)
        self.canvas_widget = self.canvas.get_tk_widget()
        self.canvas_widget.grid(row=9, column=0, columnspan=4, pady=10)


        # Device List
        tk.Label(self.root, text="Configured Devices:").grid(row=5, column=0, columnspan=4, pady=5)
        self.config_listbox = tk.Listbox(self.root, width=70, height=10, selectmode=tk.SINGLE)
        self.config_listbox.grid(row=6, column=0, columnspan=3, padx=10, pady=5)

        delete_device_button = tk.Button(self.root, text="Delete Selected Device", command=self.delete_device)
        delete_device_button.grid(row=6, column=3, padx=5, pady=5)

        # Indicator for recording status
        tk.Label(self.root, text="Recording Status:").grid(row=7, column=0, columnspan=2, pady=5)
        self.recording_status = tk.Canvas(self.root, width=20, height=20)
        self.recording_status.create_oval(2, 2, 18, 18, fill="grey")  # Neutral state
        self.recording_status.grid(row=7, column=2, columnspan=2, pady=5)

        # Start All Captures Button
        start_all_button = tk.Button(self.root, text="Start All Captures", command=self.start_all_captures)
        start_all_button.grid(row=8, column=0, columnspan=4, pady=10)


    def run(self):
        self.root.mainloop()


    # --------------------------------------
    # GUI Utility Functions
    # --------------------------------------

    def flash_status_indicator(self, color):
        """Flash the status indicator with a given color and reset to neutral."""
        self.update_status_indicator(color)
        self.root.after(500, lambda: self.update_status_indicator("gray"))

    def update_status_indicator(self, color):
        """Update the status indicator with a specified color."""
        self.status_canvas.delete("all")  # Clear the canvas
        if color == "green":
            self.status_canvas.create_oval(2, 2, 18, 18, fill="green")
        elif color == "red":
            self.status_canvas.create_oval(2, 2, 18, 18, fill="red")
        elif color == "gray":
            self.status_canvas.create_oval(2, 2, 18, 18, fill="gray")

    def update_recording_status(self, color):
        """Update the recording status indicator."""
        logging.debug(f"Updating recording status to {color}")
        self.recording_status.delete("all")
        self.recording_status.create_oval(2, 2, 18, 18, fill=color)
        self.root.update_idletasks()  # Ensure the UI updates immediately

    def handle_exit_signal(self, signum, frame):
        logging.info("Termination signal received. Closing GUI...")
        self.root.quit()
        self.root.destroy()

    # --------------------------------------
    # Device Management
    # --------------------------------------

    def add_device(self):
        """Add a device configuration to the list and flash status indicator."""
        ip_address = self.ip_entry.get()
        username = self.username_entry.get()
        sensor_type = self.sensor_type_var.get()
        if not ip_address or not username or not sensor_type:
            messagebox.showerror("Input Error", "Please enter a valid IP address, username, and select a sensor type.")
            return

        # Ping the device
        is_reachable = self.ping_device(ip_address)

        #TODO Add check on name and auth access

        # Flash the status indicator
        self.flash_status_indicator("green" if is_reachable else "red")

        # Add configuration to the list with status
        status = "reachable" if is_reachable else "unreachable"
        config = {"ip_address": ip_address, "username": username, "sensor_type": sensor_type, "status": status}
        self.configurations.append(config)
        self.config_listbox.insert(tk.END, f"IP: {ip_address}, Username: {username}, Sensor: {sensor_type}, Status: {status}")

        # Calculate next IP address
        octets = ip_address.split(".")
        if len(octets) == 4 and octets[3].isdigit():
            next_ip = f"{octets[0]}.{octets[1]}.{octets[2]}.{int(octets[3]) + 1}"
            self.ip_default.set(next_ip)  # Update default IP entry
        else:
            messagebox.showwarning("Invalid IP", "IP Address is invalid. Please enter a valid IP next time.")

        # Clear the username entry
        self.username_entry.delete(0, tk.END)


    def ping_device(self, ip_address):
        """Ping the device to check if it's reachable."""
        try:
            response = os.system(f"ping -c 1 -W 1 {ip_address} > /dev/null 2>&1")
            return response == 0
        except Exception as e:
            logging.error(f"Ping error for {ip_address}: {e}")
            return False

    def delete_device(self):
        """Delete the selected device configuration from the list."""
        selected_index = self.config_listbox.curselection()
        if not selected_index:
            messagebox.showerror("Selection Error", "Please select a device to delete.")
            return

        # Remove the selected device from the configurations list and the listbox
        index = selected_index[0]
        self.configurations.pop(index)
        self.config_listbox.delete(index)

        logging.info(f"Deleted device configuration at index {index}.")
        messagebox.showinfo("Device Deleted", "Selected device has been deleted.")


    # --------------------------------------
    # Central Server Management
    # --------------------------------------

    def start_central_server(self, base_filename, duration):
        """Launch the central server with command-line arguments."""
        if self.is_port_in_use(self.PORT):
            logging.warning(f"Port {self.PORT} is in use. Attempting to free it.")
            if not self.kill_process_on_port(self.PORT):
                messagebox.showerror("Error", f"Failed to free up port {self.PORT}.")
                return False
        try:
            logging.info("Starting the central server.")

            # Get the local IP address of the machine
            central_server_ip_address = self.get_lan_ip()

            # Pass filename and duration as arguments
            subprocess.Popen([
                "python3", self.CENTRAL_SERVER_SCRIPT,
                "--base_filename", base_filename,
                "--duration", str(duration),
                "--log_file", self.log_file,
                "--ip_address", central_server_ip_address,
                "--port", str(self.PORT),
                "--sync_metrics_dir", self.SYNC_METRICS_DIR
            ])
            logging.info(f"Central server launched at http://{central_server_ip_address}:{self.PORT}/receive_data")
            self.central_server_url = f"http://{central_server_ip_address}:{self.PORT}/receive_data"  # Store the URL for live data fetching
            logging.info(f"Central server url launched successfully at {self.central_server_url}.")
            time.sleep(0.5)  # Allow time for server initialization
            return True
        except Exception as e:
            logging.error(f"Failed to start central server: {e}")
            messagebox.showerror("Error", f"Failed to start central server: {e}")
            return False

    def get_lan_ip(self):
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        try:
            # Connect to an external server to determine the LAN interface
            s.connect(("8.8.8.8", 80))
            ip = s.getsockname()[0]
        finally:
            s.close()
        return ip

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



    # --------------------------------------
    # Remote Capture Management
    # --------------------------------------

    def start_all_captures(self):
        base_filename = self.base_filename_entry.get()
        capture_duration = self.capture_duration_entry.get()

        if not base_filename or not capture_duration.isdigit():
            messagebox.showerror("Input Error", "Please enter a valid base_filename and capture_duration.")
            return

        # Update recording indicator to green immediately
        self.update_recording_status("green")
        self.root.update_idletasks()  # Force immediate redraw

        # Start the central server with the base filename and duration
        if not self.start_central_server(base_filename, int(capture_duration)):
            self.update_recording_status("gray")  # Revert to gray if server fails
            return  # Exit if the server fails to start
        
        threads = []

        # Create and start a thread for each device
        for config in self.configurations:
            thread = threading.Thread(
                target=self.start_remote_capture,
                args=(config["ip_address"], config["username"], config["sensor_type"], base_filename, int(capture_duration)),
                daemon=True
            )
            threads.append(thread)
            thread.start()

        # Start live plotting in a separate thread
        if not self.plot_running:
            self.plot_running = True
            threading.Thread(target=self.start_plot, daemon=True).start()

        # Use after() to schedule completion handling instead of thread.join()
        self.root.after(int(capture_duration) * 1000, self.complete_all_captures)

    def complete_all_captures(self):
        """Handle the completion of all captures and stop live plotting."""
        self.plot_running = False
        # Update recording indicator to neutral
        self.update_recording_status("grey")
        # Show capture completed popup
        messagebox.showinfo("Capture Completed", "All captures have finished.")

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
                    f"--base_filename {base_filename} --capture_duration {capture_duration} "
                    f"--central_server_url {self.central_server_url}"
                )
            elif sensor_type == "radar":
                command = (
                    f"/home/{username}/labx_master/radar_code/labx_env/bin/python "
                    f"/home/{username}/labx_master/radar_code/src/RadarDataCollector.py "
                    f"--base_filename {base_filename} --capture_duration {capture_duration} "
                    f"--central_server_url {self.central_server_url}"
                )
            else:
                logging.error(f"Unsupported sensor type: {sensor_type}")
                return

            logging.info(f"Executing command on {ip_address}: {command}")

            # Start the command and handle timeout manually
            stdin, stdout, stderr = ssh.exec_command(command)
            start_time = time.time()

            while True:
                if stdout.channel.exit_status_ready():  # Check if command is done
                    break
                if time.time() - start_time > capture_duration + 2:  # Timeout logic
                    logging.error(f"Command timed out on {ip_address}.")
                    ssh.close()
                    return
                time.sleep(0.1)  # Avoid busy waiting

            # Log stdout and stderr
            for line in iter(stdout.readline, ""):
                logging.info(f"STDOUT: {line.strip()}")
            for line in iter(stderr.readline, ""):
                logging.error(f"STDERR: {line.strip()}")

            logging.info(f"Capture completed on {ip_address} for {sensor_type}")
        except Exception as e:
            logging.error(f"Connection Error: Could not connect to {ip_address}: {e}")
        finally:
            ssh.close()

    # --------------------------------------
    # Live Plotting
    # --------------------------------------

    def start_plot(self):
        """Launch the live plot."""
        threading.Thread(target=self.run_plot, daemon=True).start()

    def run_plot(self):
        """Live plot the data from the latest CSV file."""
        import pandas as pd
        import matplotlib.pyplot as plt
        from matplotlib.animation import FuncAnimation

        fig, ax = plt.subplots()
        times, offsets = [], []
        line, = ax.plot(times, offsets, label="Max Offset (ms)")
        ax.set_xlabel("Time (s)")
        ax.set_ylabel("Max Offset (ms)")
        ax.set_title("Live Max Offset")
        ax.legend()

        def update_plot(frame):
            self.current_csv_file = self.get_latest_csv_file()
            if not self.current_csv_file:
                return

            try:
                data = pd.read_csv(self.current_csv_file).tail(50)
                if not data.empty:
                    relative_times = (data['timestamp'] - data['timestamp'].iloc[0]) / 1000  # Assuming timestamp in ms
                    times[:] = relative_times.tolist()
                    offsets[:] = data['max_offset_ms'].tolist()
                    line.set_data(times, offsets)
                    ax.relim()
                    ax.autoscale_view()
            except Exception as e:
                logging.error(f"Error updating plot: {e}")

        ani = FuncAnimation(fig, update_plot, interval=1000)
        plt.show()

    def get_latest_csv_file(self):
        try:
            files = [os.path.join(self.SYNC_METRICS_DIR, f) for f in os.listdir(self.SYNC_METRICS_DIR) if f.endswith(".csv")]
            if not files:
                logging.warning("No CSV files found in the sync_metrics directory.")
                return None
            return max(files, key=os.path.getctime)
        except Exception as e:
            logging.error(f"Error fetching latest CSV file: {e}")
            return None




if __name__ == "__main__":
    app = LabInABoxControlPanel()
    app.run()
