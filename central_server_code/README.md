# Lab in a Box - Central Server Code

## Overview
This repository contains the central server code for the "Lab in a Box" system. It is responsible for coordinating data collection from multiple sensors, monitoring time synchronization, and collecting recorded data after the capture process.

## Prerequisites
Before running the central server, ensure that:
1. All sensors are correctly configured and connected to the network. (Go to sensor subfolder if you need to set up a given sensor)
2. Each sensor has an assigned IP address and username for SSH access.
3. The necessary dependencies are installed using `requirements.txt`.

## Setup and Execution

**LabX Master Setup Guide**

This document provides step-by-step instructions to set up the LabXMaster project. Follow these instructions to clone the repository, configureSSH key-based authentication with all sensors (e.g., Raspberry Pi), and launchthe GUI control panel for data collection.

**1\. Clone the Repository**

Clone the repository using the command below:
```git clone https://github.com/CopelandMIT/labx_master.git```

**2\. Configure SSH and Copy Permission Keys**

Before running the project, ensure that you have exchanged SSH keyswith all of your sensors. This section explains how to generate an SSH key pairon the central server, copy your public key to a Raspberry Pi, and verify thesetup.

**2.1 Generate SSH Key Pair on the Central Server**

Create a new SSH key pair if one doesn’t already exist. This keypair is used for authenticating with the Raspberry Pi.

**Command:**

 ```ssh-keygen -t rsa -b 4096 -C "your_email@example.com"```

**Notes:**

• Replace "your\_email@example.com"with your actual email.

• Press Enter to accept thedefault file location.

• Choose whether to set apassphrase or leave it empty.

**2.2 Copy SSH Public Key to the Raspberry Pi**

You can copy your public key using one of the following methods:

**Using ssh-copy-id**

This automates copying the public key to the Raspberry Pi’s authorized\_keys file.

**Command:**

 ```ssh-copy-id pi@192.168.YYY.XXX ```

**Notes:**

• Replace piwith your Raspberry Pi’s username.

• Replace 192.168.YYY.XXXwith the IP address of your Raspberry Pi.

• You will be prompted for theRaspberry Pi user’s password once.

**Manual Method**

1\. **Displaythe Public Key on the Central Server:**

```cat ~/.ssh/id_rsa.pub ```

2\. **Copythe output.**

3\. **Onthe Raspberry Pi:**

• Create the .sshdirectory if it does not exist:

``` mkdir -p ~/.ssh ```

• Create or edit the authorized\_keys file:

```  nano ~/.ssh/authorized_keys   ```

• Paste the copied public keyinto the file and save your changes.

4\. **SetCorrect Permissions on the Raspberry Pi:**

```  chmod 700 ~/.ssh   ```
```chmod 600 ~/.ssh/authorized_keys   ```

**2.3 Check and Configure SSH Keys**

Verify that the SSH keys are set up correctly and that you canconnect without a password.

1\. **VerifySSH Key Permissions on the Raspberry Pi:**

``` chmod 700 ~/.ssh   ```

``` chmod 600 ~/.ssh/authorized_keys   ```

2\. **Testthe SSH Connection from the Central Server:**

```  ssh pi@192.168.YYY.XXX   ```

You should now connect without being prompted for a password.

3\. **Troubleshooting:**

• **Ensurethe Correct SSH Key Pair:**

Verify that the private key on the central server (~/.ssh/id\_rsa)matches the public key on the Raspberry Pi (~/.ssh/authorized\_keys).

• **CheckSSH Configuration on the Raspberry Pi:**

Open /etc/ssh/sshd\_config and ensure thesesettings are enabled:

```  PasswordAuthentication no   ```

```  PubkeyAuthentication yes   ```

These enforce key-based authentication.

• **Restartthe SSH Service:**

```  sudo systemctl restart ssh   ```

**3\. Start the GUI Control Panel**

Follow these steps to configure and launch the GUI for datacollection.

1\. **ChangeDirectory to the Central Server Source Code:**

```  cd labx_master/central_server_code/src   ```

2\. **Createand Activate a Python Virtual Environment:**

• Create the environment:

```  python3 -m venv env   ```

• Activate the environment:

```  source env/bin/activate   ```

3\. **InstallRequired Dependencies:**

```  pip install -r requirements.txt   ```

4\. **Runthe GUI Control Panel:**

```  python3 labx_gui_oop_multisensor.py   ```

You should now be ready to configure and start data collectionusing the LabX Master GUI control panel. If you encounter any issues, referback to the troubleshooting steps or consult the project documentation.

Make sure you copy the entire content above into your file toretain all markdown formatting.

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
Once the capture process is complete, click the 'Collect Data' Button or run the `sensor_data_collector.py` script to collect the recorded data from each sensor:
```bash
python3 sensor_data_collector.py
```
This script will:
- Connect to each sensor via SSH.
- Retrieve the recorded data.
- Store the data in the designated local directory (`DEST_ROOT`).

## File Structure
```
central_server_code/
│── archives/                   # Archived logs and previous runs
│── data/                       # Local storage for collected data
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

