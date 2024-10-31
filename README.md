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

1. **Clone the Repository**:  
   ```bash
   git clone https://github.com/yourusername/LabX-Master.git
   ```
2. **Review Documentation**: Explore the component (camera, cetnral server, GPS, radar) directories for setup guides.
3. **Set Up Hardware**: Prepare your SBCs and sensors following the hardware setup guide.
4. **Configure Time Sync**: Set up NTP and PPS GPS synchronization as per instructions.
5. **Run the MSI Server**: Start the server to begin managing your sensor network.


## License

This project is licensed under the MIT License—see the [LICENSE](LICENSE) file for details.

## Contributing and Contact

For contribution requests, questions, or support, please open an issue or contact us at dcope_at_mit_dot_edu.

---

Feel free to explore, contribute, and utilize the LabX Master project for your research and development needs!
