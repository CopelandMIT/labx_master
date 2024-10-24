import time
import subprocess
import requests
from threading import Thread, Event
from datetime import datetime

class TimeSync:
    def __init__(self, sbc_id, central_server_url, polling_interval=15):
        self.sbc_id = sbc_id
        self.central_server_url = central_server_url
        self.polling_interval = polling_interval
        self.stop_event = Event()
        self.last_tracking_output = None  # Store the last tracking output

    def collect_and_send_chrony_data(self):
        """Collects Chrony data and sends it to the server when new data is available."""
        while not self.stop_event.is_set():
            try:
                # Fetch tracking information
                tracking_result = subprocess.run(['chronyc', 'tracking'], stdout=subprocess.PIPE, text=True)
                tracking_output = tracking_result.stdout

                # Check if tracking data has changed
                if tracking_output != self.last_tracking_output:
                    self.last_tracking_output = tracking_output  # Update the last tracking output

                    # Get current time
                    current_time = time.time()

                    # Prepare payload
                    payload = {
                        'sbc_id': self.sbc_id,
                        'data': {
                            'timestamp': current_time,
                            'chronyc_output': tracking_output
                        }
                    }

                    # Send data to the central server
                    response = requests.post(self.central_server_url, json=payload)
                    if response.status_code != 200:
                        print(f"Failed to send data: {response.text}")
                    else:
                        print(f"Successfully sent Chrony data to server at {datetime.fromtimestamp(current_time)}")
                else:
                    print("No new tracking data available.")

            except Exception as e:
                print(f"Error collecting or sending Chrony data: {e}")

            # Wait for the specified interval or until stop_event is set
            if self.stop_event.wait(self.polling_interval):
                break  # Exit if stop_event is set

    def start(self):
        """Start the time sync data collection."""
        self.thread = Thread(target=self.collect_and_send_chrony_data)
        self.thread.start()

    def stop(self):
        """Stop the time sync data collection."""
        self.stop_event.set()
        self.thread.join()
