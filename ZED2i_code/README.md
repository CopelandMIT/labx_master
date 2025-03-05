# Lab in a Box - ZED2i Markerless Motion Capture Setup

This guide explains how to set up the **ZED2i camera** with the **ZED SDK** for markerless motion capture as part of the **Lab in a Box** system.

---

## ✅ System Requirements

- Computer with a compatible **NVIDIA GPU** ([check compatibility](https://www.stereolabs.com/docs/requirements/))
- **Ubuntu 20.04/22.04** or **Windows**
- **PPS GPS Clock** on the same network
- **Chrony** installed
- **Lab in a Box Central Server** with GUI running

---

## ⚡ Setup Instructions

### 1. Install the Correct NVIDIA Driver

1. Identify your GPU:
   ```bash
   nvidia-smi
   ```

	2.	Download and install the appropriate NVIDIA driver from the NVIDIA Driver Downloads.
	3.	Reboot after installation.

2. Download and Install the ZED SDK
	1.	Download the latest ZED SDK from StereoLabs ZED SDK.
	2.	Follow installation instructions from the website for your OS.
	3.	Verify installation:
 ```bash
   zed_info
   ```

3. Configure Time Synchronization with PPS GPS Clock
	1.	Install Chrony:

sudo apt update
sudo apt install chrony


	2.	Edit your Chrony configuration:

sudo nano /etc/chrony/chrony.conf


	3.	Add your PPS GPS clock’s IP address (replace <GPS_IP>):

server <GPS_IP> prefer iburst


	4.	Restart Chrony:

sudo systemctl restart chrony


	5.	Verify synchronization:

chronyc tracking

4. Share SSH Keys

Ensure passwordless SSH access between the ZED2i computer and the Central Server.

On the ZED2i computer:

ssh-keygen
ssh-copy-id <username>@<central_server_ip>

On the Central Server:

ssh-copy-id <username>@<zed2i_computer_ip>

🖥️ Running the Markerless Motion Capture
	1.	Open the Lab in a Box GUI on the Central Server.
	2.	In the GUI:
	•	Enter the IP address of the ZED2i computer.
	•	Enter the username for SSH login.
	•	Select Sensor Type: Motion Capture.
	3.	Press Run.

✅ Confirm Successful Operation:
	•	The GUI will indicate a successful connection.
	•	ZED2i data will stream and record synchronized with the rest of the Lab in a Box system.
	•	Logs and data will be saved locally and/or uploaded to your designated storage (e.g., S3).

🚀 Troubleshooting:
	•	Verify GPU compatibility and driver installation using nvidia-smi.
	•	Check ZED SDK installation using zed_info.
	•	Confirm time sync status with chronyc tracking.
	•	Test SSH connectivity with:

ssh <username>@<zed2i_computer_ip>


	•	Ensure the PPS GPS clock is online and serving NTP.

📂 Folder Structure Example:

labx_master/
├── zed2i/
│   ├── data/
│   ├── src/
│   └── README.md

📧 Contact

For support, please contact Dan Copeland at dcope@mit.edu.


# JSON Bodies export sample

This sample shows how to export bodies detected with the SDK in a JSON format.

The structure will be like this :

```
{
    "0": {
        "is_new": false,
        "is_tracked": false,
        "timestamp": 0
    },
    "1680100601285": {
        "body_list": [
            {
                "action_state": 2,
                "bounding_box": [
                    [
                        -0.19150683283805847,
                        -0.37149742245674133,
                        0.3122797906398773
                    ],
                    [
                        -0.1915380209684372,
                        -0.3712289333343506,
                        0.8308528065681458
                    ],
                    [
                        0.32703498005867004,
                        -0.37111806869506836,
                        0.8308839201927185
                    ],
                    [
                        0.32706621289253235,
                        -0.3713865578174591,
                        0.31231093406677246
                    ],
                    [
                        -0.1916736662387848,
                        0.40883582830429077,
                        0.3118757903575897
                    ],
                    [
                        -0.19170483946800232,
                        0.4091043174266815,
                        0.8304488062858582
                    ],
                    [
                        0.3268681764602661,
                        0.40921521186828613,
                        0.8304799199104309
                    ],
                    [
                        0.32689934968948364,
                        0.408946692943573,
                        0.31190693378448486
                    ]
                ],
                "bounding_box_2d": [
                    [
                        0,
                        0
                    ],
                    [
                        0,
                        0
                    ],
                    [
                        0,
                        0
                    ],
                    [
                        0,
                        0
                    ]
                ],
                "confidence": 76.05413055419922,
                "dimensions": [
                    0.0,
                    0.0,
                    0.0
                ],
                "global_root_orientation": [
                    0.0,
                    0.0,
                    0.0,
                    1.0
                ],
                "head_position": [
                    0.0,
                    0.0,
                    0.0
                ],
                "id": 0,
                "keypoint": [
                    [
                        0.0,
                        -0.550000011920929,
                        0.0
                    ],
                    [
                        0.0,
                        -0.3666685223579407,
                        0.0
                    ],
                    [
                        0.0,
                        -0.1833370178937912,
                        0.0
                    ],
                    [
                        0.0,
                        -1.4901161193847656e-08,
                        0.0
                    ],
                    [
                        -0.05009057745337486,
                        0.0009963065385818481,
                        0.0
                    ],
                    [
                        -0.20009058713912964,
                        0.0009963065385818481,
                        0.0
                    ],
                    [
                        -0.20009058713912964,
                        -0.2590036988258362,
                        -0.0
                    ],
                    [
                        -0.20009058713912964,
                        -0.5090036988258362,
                        -0.0
                    ],
                    [
                        -0.20009058713912964,
                        -0.5590037107467651,
                        -0.0
                    ],
                    [
                        -0.20009058713912964,
                        -0.659003734588623,
                        -0.0
                    ],
                    [
                        -0.14009058475494385,
                        -0.6090037226676941,
                        -0.0
                    ],
                    [
                        0.05009057745337486,
                        0.0009963065385818481,
                        0.0
                    ],
                    [
                        0.20009058713912964,
                        0.0009963065385818481,
                        0.0
                    ],
                    [
                        0.20009058713912964,
                        -0.2590036988258362,
                        0.0
                    ],
                    [
                        0.20009058713912964,
                        -0.5090036988258362,
                        0.0
                    ],
                    [
                        0.20009058713912964,
                        -0.5590037107467651,
                        0.0
                    ],
                    [
                        0.20009058713912964,
                        -0.659003734588623,
                        0.0
                    ],
                    [
                        0.14009058475494385,
                        -0.6090037226676941,
                        0.0
                    ],
                    [
                        -0.10000000149011612,
                        -0.550000011920929,
                        -0.0
                    ],
                    [
                        -0.10000000149011612,
                        -1.0,
                        -0.0
                    ],
                    [
                        -0.10000000149011612,
                        -1.399999976158142,
                        -0.0
                    ],
                    [
                        -0.10000000149011612,
                        -1.5,
                        -0.11999999731779099
                    ],
                    [
                        0.10000000149011612,
                        -0.550000011920929,
                        0.0
                    ],
                    [
                        0.10000000149011612,
                        -1.0,
                        0.0
                    ],
                    [
                        0.10000000149011612,
                        -1.399999976158142,
                        0.0
                    ],
                    [
                        0.10000000149011612,
                        -1.5,
                        -0.11999999731779099
                    ],
                    [
                        0.0,
                        0.1359274536371231,
                        -0.06343282014131546
                    ],
                    [
                        0.0,
                        0.1812366098165512,
                        -0.06343282014131546
                    ],
                    [
                        -0.02718549221754074,
                        0.21295303106307983,
                        -0.03171641007065773
                    ],
                    [
                        -0.07702556997537613,
                        0.1921108067035675,
                        0.05437098443508148
                    ],
                    [
                        0.02718549221754074,
                        0.21295303106307983,
                        -0.03171641007065773
                    ],
                    [
                        0.07702556997537613,
                        0.1921108067035675,
                        0.05437098443508148
                    ],
                    [
                        -0.10000000149011612,
                        -1.5,
                        0.03999999910593033
                    ],
                    [
                        0.10000000149011612,
                        -1.5,
                        0.03999999910593033
                    ]
                ],
                "keypoint_2d": [
                    [
                        0.0,
                        0.0
                    ],
                    [
                        0.0,
                        0.0
                    ],
                    [
                        0.0,
                        0.0
                    ],
                    [
                        0.0,
                        0.0
                    ],
                    [
                        0.0,
                        0.0
                    ],
                    [
                        0.0,
                        0.0
                    ],
                    [
                        0.0,
                        0.0
                    ],
                    [
                        0.0,
                        0.0
                    ],
                    [
                        0.0,
                        0.0
                    ],
                    [
                        0.0,
                        0.0
                    ],
                    [
                        0.0,
                        0.0
                    ],
                    [
                        0.0,
                        0.0
                    ],
                    [
                        0.0,
                        0.0
                    ],
                    [
                        0.0,
                        0.0
                    ],
                    [
                        0.0,
                        0.0
                    ],
                    [
                        0.0,
                        0.0
                    ],
                    [
                        0.0,
                        0.0
                    ],
                    [
                        0.0,
                        0.0
                    ],
                    [
                        0.0,
                        0.0
                    ],
                    [
                        0.0,
                        0.0
                    ],
                    [
                        0.0,
                        0.0
                    ],
                    [
                        0.0,
                        0.0
                    ],
                    [
                        0.0,
                        0.0
                    ],
                    [
                        0.0,
                        0.0
                    ],
                    [
                        0.0,
                        0.0
                    ],
                    [
                        0.0,
                        0.0
                    ],
                    [
                        0.0,
                        0.0
                    ],
                    [
                        0.0,
                        0.0
                    ],
                    [
                        0.0,
                        0.0
                    ],
                    [
                        0.0,
                        0.0
                    ],
                    [
                        0.0,
                        0.0
                    ],
                    [
                        0.0,
                        0.0
                    ],
                    [
                        0.0,
                        0.0
                    ],
                    [
                        0.0,
                        0.0
                    ]
                ],
                "keypoint_confidence": [
                    67.47969055175781,
                    63.501380920410156,
                    63.501380920410156,
                    55.54474639892578,
                    42.91035842895508,
                    30.275968551635742,
                    27.52467918395996,
                    48.211692810058594,
                    48.211692810058594,
                    48.211692810058594,
                    48.211692810058594,
                    55.54474639892578,
                    55.54474639892578,
                    77.03418731689453,
                    53.65507888793945,
                    53.65507888793945,
                    53.65507888793945,
                    53.65507888793945,
                    86.74769592285156,
                    96.46119689941406,
                    93.28395080566406,
                    93.28395080566406,
                    48.211692810058594,
                    48.211692810058594,
                    48.211692810058594,
                    48.211692810058594,
                    34.05530548095703,
                    34.05530548095703,
                    72.93236541748047,
                    72.93236541748047,
                    72.93236541748047,
                    72.93236541748047,
                    93.28395080566406,
                    48.211692810058594
                ],
                "keypoint_cov": [
                    [
                        null,
                        null,
                        null,
                        null,
                        null,
                        null
                    ],
                    [
                        null,
                        null,
                        null,
                        null,
                        null,
                        null
                    ],
                    [
                        null,
                        null,
                        null,
                        null,
                        null,
                        null
                    ],
                    [
                        null,
                        null,
                        null,
                        null,
                        null,
                        null
                    ],
                    [
                        null,
                        null,
                        null,
                        null,
                        null,
                        null
                    ],
                    [
                        null,
                        null,
                        null,
                        null,
                        null,
                        null
                    ],
                    [
                        null,
                        null,
                        null,
                        null,
                        null,
                        null
                    ],
                    [
                        null,
                        null,
                        null,
                        null,
                        null,
                        null
                    ],
                    [
                        null,
                        null,
                        null,
                        null,
                        null,
                        null
                    ],
                    [
                        null,
                        null,
                        null,
                        null,
                        null,
                        null
                    ],
                    [
                        null,
                        null,
                        null,
                        null,
                        null,
                        null
                    ],
                    [
                        null,
                        null,
                        null,
                        null,
                        null,
                        null
                    ],
                    [
                        null,
                        null,
                        null,
                        null,
                        null,
                        null
                    ],
                    [
                        null,
                        null,
                        null,
                        null,
                        null,
                        null
                    ],
                    [
                        null,
                        null,
                        null,
                        null,
                        null,
                        null
                    ],
                    [
                        null,
                        null,
                        null,
                        null,
                        null,
                        null
                    ],
                    [
                        null,
                        null,
                        null,
                        null,
                        null,
                        null
                    ],
                    [
                        null,
                        null,
                        null,
                        null,
                        null,
                        null
                    ],
                    [
                        null,
                        null,
                        null,
                        null,
                        null,
                        null
                    ],
                    [
                        null,
                        null,
                        null,
                        null,
                        null,
                        null
                    ],
                    [
                        null,
                        null,
                        null,
                        null,
                        null,
                        null
                    ],
                    [
                        null,
                        null,
                        null,
                        null,
                        null,
                        null
                    ],
                    [
                        null,
                        null,
                        null,
                        null,
                        null,
                        null
                    ],
                    [
                        null,
                        null,
                        null,
                        null,
                        null,
                        null
                    ],
                    [
                        null,
                        null,
                        null,
                        null,
                        null,
                        null
                    ],
                    [
                        null,
                        null,
                        null,
                        null,
                        null,
                        null
                    ],
                    [
                        null,
                        null,
                        null,
                        null,
                        null,
                        null
                    ],
                    [
                        null,
                        null,
                        null,
                        null,
                        null,
                        null
                    ],
                    [
                        null,
                        null,
                        null,
                        null,
                        null,
                        null
                    ],
                    [
                        null,
                        null,
                        null,
                        null,
                        null,
                        null
                    ],
                    [
                        null,
                        null,
                        null,
                        null,
                        null,
                        null
                    ],
                    [
                        null,
                        null,
                        null,
                        null,
                        null,
                        null
                    ],
                    [
                        null,
                        null,
                        null,
                        null,
                        null,
                        null
                    ],
                    [
                        null,
                        null,
                        null,
                        null,
                        null,
                        null
                    ]
                ],
                "local_orientation_per_joint": [
                    [
                        0.0,
                        0.0,
                        0.0,
                        1.0
                    ],
                    [
                        0.0,
                        0.0,
                        0.0,
                        1.0
                    ],
                    [
                        0.0,
                        0.0,
                        0.0,
                        1.0
                    ],
                    [
                        0.0,
                        0.0,
                        0.0,
                        1.0
                    ],
                    [
                        0.0,
                        0.0,
                        0.0,
                        1.0
                    ],
                    [
                        0.0,
                        0.0,
                        0.7071067094802856,
                        0.7071067690849304
                    ],
                    [
                        0.0,
                        0.0,
                        0.0,
                        1.0
                    ],
                    [
                        0.0,
                        0.0,
                        0.0,
                        1.0
                    ],
                    [
                        0.0,
                        0.0,
                        0.0,
                        1.0
                    ],
                    [
                        0.0,
                        0.0,
                        0.0,
                        1.0
                    ],
                    [
                        0.0,
                        0.0,
                        0.0,
                        1.0
                    ],
                    [
                        0.0,
                        0.0,
                        0.0,
                        1.0
                    ],
                    [
                        0.0,
                        0.0,
                        -0.7071067094802856,
                        0.7071067690849304
                    ],
                    [
                        0.0,
                        0.0,
                        0.0,
                        1.0
                    ],
                    [
                        0.0,
                        0.0,
                        0.0,
                        1.0
                    ],
                    [
                        0.0,
                        0.0,
                        0.0,
                        1.0
                    ],
                    [
                        0.0,
                        0.0,
                        0.0,
                        1.0
                    ],
                    [
                        0.0,
                        0.0,
                        0.0,
                        1.0
                    ],
                    [
                        0.0,
                        0.0,
                        0.0,
                        1.0
                    ],
                    [
                        0.0,
                        0.0,
                        0.0,
                        1.0
                    ],
                    [
                        0.0,
                        0.0,
                        0.0,
                        1.0
                    ],
                    [
                        0.0,
                        0.0,
                        0.0,
                        1.0
                    ],
                    [
                        0.0,
                        0.0,
                        0.0,
                        1.0
                    ],
                    [
                        0.0,
                        0.0,
                        0.0,
                        1.0
                    ],
                    [
                        0.0,
                        0.0,
                        0.0,
                        1.0
                    ],
                    [
                        0.0,
                        0.0,
                        0.0,
                        1.0
                    ],
                    [
                        0.0,
                        0.0,
                        0.0,
                        1.0
                    ],
                    [
                        0.0,
                        0.0,
                        0.0,
                        1.0
                    ],
                    [
                        0.0,
                        0.0,
                        0.0,
                        1.0
                    ],
                    [
                        0.0,
                        0.0,
                        0.0,
                        1.0
                    ],
                    [
                        0.0,
                        0.0,
                        0.0,
                        1.0
                    ],
                    [
                        0.0,
                        0.0,
                        0.0,
                        1.0
                    ],
                    [
                        0.0,
                        0.0,
                        0.0,
                        1.0
                    ],
                    [
                        0.0,
                        0.0,
                        0.0,
                        1.0
                    ]
                ],
                "local_position_per_joint": [
                    [
                        0.0,
                        0.0,
                        0.0
                    ],
                    [
                        0.0,
                        0.18333150446414948,
                        0.0
                    ],
                    [
                        0.0,
                        0.18333150446414948,
                        0.0
                    ],
                    [
                        0.0,
                        0.18333700299263,
                        0.0
                    ],
                    [
                        -0.05009057745337486,
                        0.18433332443237305,
                        0.0
                    ],
                    [
                        -0.15000000596046448,
                        0.0,
                        0.0
                    ],
                    [
                        -0.25999999046325684,
                        0.0,
                        0.0
                    ],
                    [
                        -0.25,
                        0.0,
                        0.0
                    ],
                    [
                        -0.05000000074505806,
                        0.0,
                        0.0
                    ],
                    [
                        -0.10000000149011612,
                        0.0,
                        0.0
                    ],
                    [
                        -0.10000000149011612,
                        -0.05999999865889549,
                        0.0
                    ],
                    [
                        0.05009057745337486,
                        0.18433332443237305,
                        0.0
                    ],
                    [
                        0.15000000596046448,
                        0.0,
                        0.0
                    ],
                    [
                        0.25999999046325684,
                        0.0,
                        0.0
                    ],
                    [
                        0.25,
                        0.0,
                        0.0
                    ],
                    [
                        0.05000000074505806,
                        0.0,
                        0.0
                    ],
                    [
                        0.10000000149011612,
                        0.0,
                        0.0
                    ],
                    [
                        0.10000000149011612,
                        -0.05999999865889549,
                        0.0
                    ],
                    [
                        -0.10000000149011612,
                        0.0,
                        0.0
                    ],
                    [
                        0.0,
                        -0.44999998807907104,
                        0.0
                    ],
                    [
                        0.0,
                        -0.4000000059604645,
                        0.0
                    ],
                    [
                        0.0,
                        -0.10000000149011612,
                        -0.11999999731779099
                    ],
                    [
                        0.10000000149011612,
                        0.0,
                        0.0
                    ],
                    [
                        0.0,
                        -0.44999998807907104,
                        0.0
                    ],
                    [
                        0.0,
                        -0.4000000059604645,
                        0.0
                    ],
                    [
                        0.0,
                        -0.10000000149011612,
                        -0.11999999731779099
                    ],
                    [
                        0.0,
                        0.1359274685382843,
                        -0.06343282014131546
                    ],
                    [
                        0.0,
                        0.0453091561794281,
                        0.0
                    ],
                    [
                        -0.02718549221754074,
                        0.07702556997537613,
                        0.03171641007065773
                    ],
                    [
                        -0.07702556997537613,
                        0.0561833530664444,
                        0.11780380457639694
                    ],
                    [
                        0.02718549221754074,
                        0.07702556997537613,
                        0.03171641007065773
                    ],
                    [
                        0.07702556997537613,
                        0.0561833530664444,
                        0.11780380457639694
                    ],
                    [
                        0.0,
                        -0.10000000149011612,
                        0.03999999910593033
                    ],
                    [
                        0.0,
                        -0.10000000149011612,
                        0.03999999910593033
                    ]
                ],
                "position": [
                    0.06768068671226501,
                    -0.018858879804611206,
                    -0.5713798999786377
                ],
                "position_covariance": [
                    0.0,
                    0.0,
                    0.0,
                    1.0,
                    0.0,
                    0.4000000059604645
                ],
                "tracking_state": 0,
                "unique_object_id": "0b3a1b48-5316-4fa1-aa47-7c8844c68e5b",
                "velocity": [
                    0.0,
                    0.0,
                    0.0
                ]
            }
        ],
        "is_new": true,
        "is_tracked": false,
        "timestamp": 1680100601285169401
    },
    "1680100601351": {
        // another body...
    }
}
```
