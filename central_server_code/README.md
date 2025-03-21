# Lab in a Box - Central Server Code

## Overview
This repository contains the central server code for the "Lab in a Box" system. It is responsible for coordinating data collection from multiple sensors, monitoring time synchronization, and collecting recorded data after the capture process.
## Central Server Setup

### 1. Clone the Repository

Clone the repository using the command below:

```bash
git clone https://github.com/CopelandMIT/labx_master.git
```

---

### 2. Configure SSH and Copy Permission Keys

Before running the project, ensure that you have exchanged SSH keys with all of your sensors. This section explains how to generate an SSH key pair on the central server, copy your public key to a Raspberry Pi, and verify the setup.

#### 2.1 Generate SSH Key Pair on the Central Server

Create a new SSH key pair if one doesn't already exist. This key pair is used for authenticating with the Raspberry Pi.

**Command:**

```bash
ssh-keygen -t rsa -b 4096 -C "your_email@example.com"
```

**Notes:**
- Replace `"your_email@example.com"` with your actual email.
- Press Enter to accept the default file location.
- Choose whether to set a passphrase or leave it empty.

#### 2.2 Copy SSH Public Key to the Raspberry Pi

You can copy your public key using one of the following methods:

**Using `ssh-copy-id`**

This automates copying the public key to the Raspberry Pi’s `authorized_keys` file.

**Command:**

```bash
ssh-copy-id pi@192.168.YYY.XXX
```

**Notes:**
- Replace `pi` with your Raspberry Pi’s username.
- Replace `192.168.YYY.XXX` with the IP address of your Raspberry Pi.
- You will be prompted for the Raspberry Pi user’s password once.

**Manual Method**

1. **Display the Public Key on the Central Server:**

    ```bash
    cat ~/.ssh/id_rsa.pub
    ```

2. **Copy the output.**

3. **On the Raspberry Pi:**
    - Create the `.ssh` directory if it does not exist:
      ```bash
      mkdir -p ~/.ssh
      ```
    - Create or edit the `authorized_keys` file:
      ```bash
      nano ~/.ssh/authorized_keys
      ```
    - Paste the copied public key into the file and save your changes.

4. **Set Correct Permissions on the Raspberry Pi:**

    ```bash
    chmod 700 ~/.ssh
    chmod 600 ~/.ssh/authorized_keys
    ```

#### 2.3 Check and Configure SSH Keys

Verify that the SSH keys are set up correctly and that you can connect without a password.

1. **Verify SSH Key Permissions on the Raspberry Pi:**

    ```bash
    chmod 700 ~/.ssh
    chmod 600 ~/.ssh/authorized_keys
    ```

2. **Test the SSH Connection from the Central Server:**

    ```bash
    ssh pi@192.168.YYY.XXX
    ```

    You should now connect without being prompted for a password.

3. **Troubleshooting:**
    - **Ensure the Correct SSH Key Pair:**  
      Verify that the private key on the central server (`~/.ssh/id_rsa`) matches the public key on the Raspberry Pi (`~/.ssh/authorized_keys`).
    - **Check SSH Configuration on the Raspberry Pi:**  
      Open `/etc/ssh/sshd_config` and ensure these settings are enabled:
      ```
      PasswordAuthentication no
      PubkeyAuthentication yes
      ```
      These enforce key-based authentication.
    - **Restart the SSH Service:**
      ```bash
      sudo systemctl restart ssh
      ```

---

### 3. Start the GUI Control Panel

Follow these steps to configure and launch the GUI for data collection.

1. **Change Directory to the Central Server Source Code:**

    ```bash
    cd labx_master/central_server_code/src
    ```

2. **Create and Activate a Python Virtual Environment:**
    - Create the environment:
      ```bash
      python3 -m venv env
      ```
    - Activate the environment:
      ```bash
      source env/bin/activate
      ```

3. **Install Required Dependencies:**

    ```bash
    pip install -r requirements.txt
    ```

4. **Run the GUI Control Panel:**

    ```bash
    python3 labx_gui_oop_multisensor.py
    ```

You should now be ready to configure and start data collection using the LabX Master GUI control panel. If you encounter any issues, refer back to the troubleshooting steps or consult the project documentation.

### 2. Configure Devices and Capture Settings
- Enter the **Base Filename** for captured data.
- Set the **Capture Duration** (in seconds).
- Specify the **IP Address** and **Username** for each sensor.
- Select the **Sensor Type** (e.g., camera, radar, etc.).
- Click **Add Device** to register the device.
- Repeat this process for all devices.

### 3. Start the Capture Process
- Click **Start All Captures** to initiate data collection across all connected devices.
- Monitor the **Live Max Offset** plot to track time synchronization accuracy.

### 4. Collect Data After Capture
Once the capture process is complete, click the 'Collect Data'.

This will:
- Connect to each sensor via SSH.
- Retrieve the recorded data from the last or specified capture.
- Store the data in the designated local directory (`pulled_data`).

## File Structure
```
central_server_code/
│── archives/                   # Archived logs and previous runs
│── pulled_data/                # Local storage for collected data
│── labx_env/                   # Virtual environment (if applicable)
│── logs/                       # Log files for debugging
│── src/                        # Source code
│   │── central_server_v3.py    # Main central server script
│   │── create_database.py      # Database initialization
│   │── labx_gui_oop_multisensor.py # GUI for setup and monitoring
│   │── plot_multi_rpi_sync_data.py # Script for analyzing time sync data
│   │── requirements.txt         # Dependencies
│   │── sensor_data_collector.py # Script to retrieve data from sensors
│── README.md                   # Documentation
```

## Notes
- Ensure SSH access to all sensors is configured correctly.
- Time synchronization is crucial for accurate data collection.
- If any sensor fails to sync properly, restart the process and verify the network connection.

## Contact
For issues or questions, reach out to the development team or refer to the project's documentation.

