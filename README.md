# LabX Master Project

Welcome to the **LabX Master** repository—the central hub for our synchronized distributed sensor network project. This initiative integrates a variety of sensors controlled by single-board computers (SBCs) to create a cohesive system for advanced research applications.

## Overview

The LabX Master project is designed to develop a synchronized, distributed network of multimodal sensors, including:

- **60GHz Infineon FMCW Radar**
- **USB Cameras**
- **GPS Modules**

By utilizing Network Time Protocol (NTP) and Pulse Per Second (PPS) GPS signals, we achieve high-precision time synchronization across the network. The MSI (Metadata Synchronization and Integration) server is pivotal in handling metadata, monitoring PPS GPS synchronization, and providing analytical tools for research purposes.

## Features

- **Distributed Sensor Network**: Deploy multiple sensors across various locations, all synchronized for unified data collection.
- **Multimodal Sensing**: Combine radar, visual, and geospatial data for comprehensive monitoring.
- **High-Precision Time Synchronization**: Achieve microsecond-level synchronization using NTP and PPS GPS.
- **Centralized Data Management**: The MSI server aggregates metadata and facilitates data analysis.
- **Scalable Architecture**: Easily add or remove sensors and SBCs to scale the network as needed.

## Components

### Sensors

- **60GHz Infineon FMCW Radar**: Offers high-resolution range and velocity data.
- **USB Cameras**: Captures visual information to complement other sensor data.
- **GPS Modules**: Provides precise location data and time synchronization via PPS signals.

### Single-Board Computers (SBCs)

Each sensor is controlled by an SBC, responsible for data acquisition, time sync with the GPS, initial processing, and communication with the central server.

### Time Synchronization

- **Network Time Protocol (NTP)**: Synchronizes device clocks over the network.
- **Pulse Per Second (PPS) GPS**: Provides precise time signals for synchronization.

### Central Server

The Central server is the core of the system:

- **Metadata Management**: Aggregates and organizes metadata from all sensors.
- **Synchronization Monitoring**: Ensures all devices maintain precise time alignment.
- **Data Analysis**: Offers tools and interfaces for research-oriented data analysis.

- **GUI: Inprogress...**

## Possible Use Cases

The distributed multimodal monitoring capabilities of the LabX Master project enable a wide range of applications:

- **Environmental Monitoring**: Observe weather patterns, wildlife movements, and ecological changes.
- **Traffic Analysis**: Monitor vehicle speeds, density, and flow for urban planning.
- **Security and Surveillance**: Enhance situational awareness in critical infrastructures.
- **Industrial Automation**: Oversee operations in large-scale or remote facilities.
- **Academic Research**: Support studies in robotics, computer vision, remote sensing, and more.

## Getting Started

To participate in the LabX Master project:

1. **Create a Central Server**: Follow the steps on the central server guide page. 
2. **Review Documentation**: Explore the component/sensor (camera, central server, GPS, radar) directories for hardware requirements and setup guides.
3. **Set Up Hardware**: Prepare your SBCs and sensors following the hardware setup guide. Example: Radar set up instructions are in radar_code's README.md
4. **Configure Time Sync**: Set up NTP and PPS GPS synchronization as per instructions.
5. **Configure SSH and Copy Permission Keys**

    1. **Generate SSH Key Pair on the Central Server**:  
       - **Purpose**: Creates a new SSH key pair if one doesn't already exist. This key pair will be used for authenticating with the Raspberry Pi.
       - **Command**:
         ```bash
         ssh-keygen -t rsa -b 4096 -C "your_email@example.com"
         ```
       - **Notes**: Replace `"your_email@example.com"` with your actual email. You can press **Enter** to accept the default file location and choose whether to set a passphrase.
    
    2. **Copy SSH Public Key to the Raspberry Pi**:  
       - **Using `ssh-copy-id`**:
         - **Purpose**: Automates the process of copying the public key to the Raspberry Pi’s `authorized_keys` file.
         - **Command**:
           ```bash
           ssh-copy-id pi@192.168.YYY.XXX
           ```
         - **Notes**: Replace `pi` with your Raspberry Pi’s username and `192.168.YYY.XXX` with its IP address. You’ll need to enter the Raspberry Pi user’s password once during this process.
    
       - **Manual Method**:
         - **Purpose**: Provides an alternative method to transfer the public key if `ssh-copy-id` is not available.
         - **Steps**:
           1. **Display the Public Key** on the central server:
              ```bash
              cat ~/.ssh/id_rsa.pub
              ```
           2. **Copy the Output**.
           3. **On the Raspberry Pi**, create the `.ssh` directory and `authorized_keys` file if they don't exist:
              ```bash
              mkdir -p ~/.ssh
              nano ~/.ssh/authorized_keys
              ```
           4. **Paste the Public Key** into the `authorized_keys` file.
           5. **Set Correct Permissions**:
              ```bash
              chmod 700 ~/.ssh
              chmod 600 ~/.ssh/authorized_keys
              ```
    
    3. **Check and Configure SSH Keys**:  
       - **Purpose**: Ensures that the SSH keys are correctly set up and that the connection works as intended.
       - **Steps**:
         - **Verify SSH Key Permissions** on the Raspberry Pi:
           ```bash
           chmod 700 ~/.ssh
           chmod 600 ~/.ssh/authorized_keys
           ```
         - **Test SSH Connection** from the Central Server:
           ```bash
           ssh pi@192.168.YYY.XXX
           ```
           - You should now connect without being prompted for a password.
         - **Troubleshooting**:
           - **Ensure Correct SSH Key Pair**: The private key on the central server (`~/.ssh/id_rsa`) should correspond to the public key on the Raspberry Pi (`~/.ssh/authorized_keys`).
           - **Check SSH Configuration**: On the Raspberry Pi, ensure that `/etc/ssh/sshd_config` has the following settings:
             ```
             PasswordAuthentication no
             PubkeyAuthentication yes
             ```
             - This enforces key-based authentication for enhanced security.
           - **Restart SSH Service** after any changes:
             ```bash
             sudo systemctl restart ssh
             ```
    
6. **Run the MSI Server**: Start the server to begin managing your sensor network.

### Additional Recommendations:

- **Replace Placeholders**: Ensure that placeholders like `"your_email@example.com"`, `pi`, and `192.168.YYY.XXX` are replaced with actual values relevant to your setup.
  
- **Security Considerations**:
  - **Protect Private Keys**: Advise users to keep their private keys (`~/.ssh/id_rsa`) secure and not share them.
  - **Use Strong Passphrases**: Encourage the use of strong passphrases for SSH keys to enhance security, especially if not using a passphrase is a security risk in your environment.
  
- **Verification Steps**:
  - After setting up SSH keys, performing a test SSH connection ensures that everything is configured correctly before proceeding with further setup steps.


## License

This project is licensed under the MIT License—see the [LICENSE](LICENSE) file for details.

## Contributing and Contact

For contribution requests, questions, or support, please open an issue or contact us at dcope_at_mit_dot_edu.

---

Feel free to explore, contribute, and utilize the LabX Master project for your research and development needs!
