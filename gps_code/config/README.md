# PPS GPS Raspberry Pi Setup

This guide outlines the steps to set up the PPS GPS Raspberry Pi, including the hardware connection of the GPS module/antenna to the Raspberry Pi and the system configuration to accept and publish the GPS signal to other devices.

## Components Used

- **Raspberry Pi 4 or 5** 
- **GPS Module**: [NEO-6M GPS Module](https://www.amazon.com/dp/B07P8YMVNT?ref=ppx_yo2ov_dt_b_fed_asin_title)
- **GPS Antenna**: [GPS Active Antenna](https://www.amazon.com/gp/product/B083D59N55/ref=ppx_yo_dt_b_asin_title_o01_s00?ie=UTF8&psc=1)

> **Note**: Soldering is required to connect the antenna to the GPS module. Follow the steps below to properly set up the hardware and configure the Raspberry Pi.

---

## Step 1: Soldering the GPS Antenna to the GPS Module

The GPS module needs to be soldered to ensure a proper connection with the active antenna.

### Soldering Steps:

1. **Prepare the soldering iron** and make sure the GPS module pins are clean.
2. **Connect the antenna** leads to the appropriate solder points on the GPS module.
3. Ensure there are no loose connections, and double-check the solder joints.

![Soldering the GPS Module](./images/soldering_gps_module.jpg)

---

## Step 2: Connect the GPS Module to the Raspberry Pi

After soldering the antenna to the GPS module, the next step is to connect the GPS module to the Raspberry Pi's GPIO pins.

### Pin Connections:

- **3.3V/5V**: Connect the GPS module to the GPIO pin 4 on the Raspberry Pi (depending on your GPS module's power requirements).
- **GND (Ground)**: Connect the GND pin from the GPS module to a GND pin on the Raspberry Pi (GPIO pin 6).
- **TX Pin**: Connect the TX pin from the GPS module to the Raspberry Pi GPIO 14 (UART, pin 8).
- **PPS Pin**: Connect the PPS pin from the GPS module to the Raspberry Pi GPIO 18 (pin 10).

![Raspberry Pi Pin Connections](./images/rpi_pin_connections.jpg)

---

## Step 3: Install and Set Up the GPS Antenna

Place the GPS antenna in a location with a clear view of the sky to ensure good satellite reception. Use the provided cable to connect the antenna to the GPS module's antenna port.

### Antenna Placement Tips:

- Make sure the antenna is placed **outdoors** or near a **window** for optimal satellite reception.
- The antenna should be **fixed** securely to avoid any movement that could interrupt the signal.

![GPS Antenna Setup](./images/gps_antenna_setup.jpg)

---

## Step 4: Enable UART and PPS on the Raspberry Pi

1. Open the Raspberry Pi configuration tool:

   ```bash
   sudo raspi-config
   ```

2. Navigate to **Interfacing Options > Serial**, disable the console, but enable the serial hardware.

3. Install necessary software:

   ```bash
   sudo apt update
   sudo apt install gpsd gpsd-clients chrony pps-tools
   ```

4. Add the following lines to `/boot/config.txt`:

   ```conf
   dtoverlay=pps-gpio,gpiopin=18
   enable_uart=1
   ```

5. Reboot the Raspberry Pi:

   ```bash
   sudo reboot
   ```

---

## Step 5: Verify GPS and PPS Signals

Check that the GPS module is recognized:

```bash
cgps -s
```

Verify that the PPS signal is detected:

```bash
sudo ppstest /dev/pps0
```

---

## Step 6: Configure Chrony to Use PPS GPS Signal

Edit `/etc/chrony/chrony.conf` to add the following lines:

```conf
refclock PPS /dev/pps0 poll 2 refid PPS
refclock SHM 0 poll 3 refid GPS
```

Restart Chrony:

```bash
sudo systemctl restart chrony
```

---

## Step 7: Publish the GPS Time to Other RPi Nodes

To publish the GPS time to other Raspberry Pis on the local network, set up the PPS GPS Pi as an NTP server.

Add the following lines to `/etc/chrony/chrony.conf`:

```conf
allow 192.168.68.0/24
local stratum 1
```

Restart Chrony:

```bash
sudo systemctl restart chrony
```

The PPS GPS Raspberry Pi is now configured to act as a Stratum 1 NTP server for your local network.

---

## Step 8: Configure Other Raspberry Pis to Use the GPS PPS Signal

On the other Raspberry Pis, you'll need to configure them to prefer the PPS GPS time signal from the central Raspberry Pi. In the `/etc/chrony/chrony.conf` file on each RPi, add the following lines:

Add the IP address of the PPS GPS RPi as the preferred time source:

```conf
server 192.168.68.XXX iburst prefer
```

Replace `192.168.68.XXX` with the actual IP address of the PPS GPS Raspberry Pi.

Add backup NTP servers (e.g., Google NTP servers):
```conf
server time1.google.com iburst
server time2.google.com iburst
server time3.google.com iburst
server time4.google.com iburst
```

Restart Chrony on each RPi:

```bash
sudo systemctl restart chrony
```

---

## Conclusion

By following this guide, you have successfully set up the PPS GPS Raspberry Pi to act as a Stratum 1 NTP server, and configured the other Raspberry Pis to synchronize their time using this GPS source with Google NTP servers as a backup.


