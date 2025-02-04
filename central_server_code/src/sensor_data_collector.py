#!/usr/bin/env python3
import os
import sys
import paramiko

# ------------------------------------------------------------------------------
# SETTINGS
# ------------------------------------------------------------------------------
# 1) List of RPi hosts, each with:
#    - host (IP or hostname)
#    - username
#    - password (or None if you use private key)
#    - base_remote_path (the path that precedes the folder you want to copy)
rpi_hosts = [
    {
        'host': '192.168.0.70',
        'username': 'dcope',
        'password': '',  # or None if using keys
        'base_remote_path': '/home/dcope/labx_master/radar_code/data'
    },
    {
        'host': '192.168.0.28',
        'username': 'dcope',
        'password': '',
        'base_remote_path': '/home/dcope/labx_master/radar_code/data'
    },
      {
        'host': '192.168.0.121',
        'username': 'pi',
        'password': '',
        'base_remote_path': '/home/pi/labx_master/camera_code/data'
    },
    # ... add more as needed
]

# 2) Local root folder to which you want to copy the folder from each RPi.
DEST_ROOT = r'/media/daniel/FourTBLaCie/LabXData/pig_heart_test_v12'


# ------------------------------------------------------------------------------
# HELPER FUNCTION: Recursively copy a remote directory via SFTP
# ------------------------------------------------------------------------------
def copy_directory(sftp, remote_path, local_path):
    """
    Recursively copy the contents of remote_path to local_path using the given sftp client.
    """
    # Ensure the local path exists
    os.makedirs(local_path, exist_ok=True)

    # List items in the remote directory
    for item in sftp.listdir(remote_path):
        remote_item_path = os.path.join(remote_path, item)
        local_item_path = os.path.join(local_path, item)

        # Use lstat to determine if it's a directory or file
        attr = sftp.lstat(remote_item_path)
        if is_directory(attr):
            # Recursively copy the sub-directory
            copy_directory(sftp, remote_item_path, local_item_path)
        else:
            # It's a file, copy it
            print(f"Copying file: {remote_item_path} -> {local_item_path}")
            sftp.get(remote_item_path, local_item_path)


def is_directory(sftp_attr):
    """
    Return True if the SFTPAttributes object indicates a directory.
    On many systems, paramiko uses stat S_IFDIR for directories.
    """
    import stat
    return stat.S_ISDIR(sftp_attr.st_mode)


# ------------------------------------------------------------------------------
# MAIN SCRIPT
# ------------------------------------------------------------------------------
def main():
    # ------------------------------------------------------------------------------
    # 1) Get the folder name to copy from the command line or input prompt
    # ------------------------------------------------------------------------------
    if len(sys.argv) > 1:
        folder_name = sys.argv[1]
    else:
        folder_name = input("Enter the folder name to copy from each RPi: ").strip()

    # ------------------------------------------------------------------------------
    # 2) Iterate over the RPi hosts and copy the folder from each
    # ------------------------------------------------------------------------------
    for rpi in rpi_hosts:
        host = rpi['host']
        username = rpi['username']
        password = rpi['password']
        base_remote_path = rpi['base_remote_path']

        # Build the FULL remote folder path
        remote_folder_path = os.path.join(base_remote_path, folder_name)

        # Create a subfolder named after the host (or any unique identifier)
        local_subfolder = os.path.join(DEST_ROOT, f"{host}_{folder_name}")

        print(f"\nConnecting to {host} to copy: {remote_folder_path}")
        try:
            # ------------------------------------------------------------------------------
            # 3) Set up SSH and SFTP
            # ------------------------------------------------------------------------------
            ssh = paramiko.SSHClient()
            # Automatically add host keys from known_hosts
            ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
            ssh.connect(hostname=host, username=username, password=password)

            sftp = ssh.open_sftp()

            # ------------------------------------------------------------------------------
            # 4) Recursively copy the folder
            # ------------------------------------------------------------------------------
            print(f"Copying {remote_folder_path} -> {local_subfolder}")
            copy_directory(sftp, remote_folder_path, local_subfolder)

            # ------------------------------------------------------------------------------
            # 5) Close SFTP and SSH connections
            # ------------------------------------------------------------------------------
            sftp.close()
            ssh.close()
            print(f"Finished copying from {host}.\n")

        except Exception as e:
            print(f"ERROR copying from {host}: {e}")
            continue


if __name__ == "__main__":
    main()
