# auto_data_collector.py
import os
import paramiko
import getpass
import logging

CENTRAL_USERNAME = getpass.getuser()
LOG_FILE = f"/home/{CENTRAL_USERNAME}/labx_master/central_server_code/logs/auto_data_collector.log"

logging.basicConfig(
    filename=LOG_FILE,
    level=logging.INFO,
    format="%(asctime)s - %(levelname)s - %(message)s"
)

class AutoDataCollector:
    def __init__(self):
        self.dest_root = f"/home/{CENTRAL_USERNAME}/labx_master/pulled_data"
        os.makedirs(self.dest_root, exist_ok=True)

    def pull_previous_data(self, devices, folder_name="previous_capture"):
        """
        Pull data from a 'previous_capture' folder on each remote device.
        'devices' is a list of dicts, each containing 'ip_address', 'username', 'sensor_type', etc.
        """
        for device in devices:
            host = device["ip_address"]
            username = device["username"]
            sensor_type = device["sensor_type"]
            
            if sensor_type == 'body_tracking':
                sensor_type = 'ZED2i'

            # Construct remote folder path
            base_remote_path = f"/home/{username}/labx_master/{sensor_type}_code/data"
            remote_folder_path = os.path.join(base_remote_path,folder_name)

            # Make local subfolder
            local_subfolder = os.path.join(self.dest_root,folder_name)
            os.makedirs(local_subfolder, exist_ok=True)

            try:
                # Connect via SSH & SFTP
                ssh = paramiko.SSHClient()
                ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
                ssh.connect(hostname=host, username=username)
                sftp = ssh.open_sftp()

                # Copy files
                for item in sftp.listdir(remote_folder_path):
                    remote_file = os.path.join(remote_folder_path, item)
                    local_file = os.path.join(local_subfolder, item)
                    sftp.get(remote_file, local_file)

                sftp.close()
                ssh.close()

                logging.info(f"Pulled data from {host} into {local_subfolder}")
                print(f"Data pulled from {host} -> {local_subfolder}")

            except Exception as e:
                logging.error(f"Error pulling data from {host}: {e}")
                print(f"ERROR: Could not pull data from {host}. Reason: {e}")