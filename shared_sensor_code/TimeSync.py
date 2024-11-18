import time
import subprocess
import requests
from threading import Thread, Event
from datetime import datetime
import logging

class TimeSync:
    def __init__(self, deployed_sensor_id, central_server_url, sync_polling_interval=10):
        self.deployed_sensor_id = deployed_sensor_id
        self.central_server_url = central_server_url
        self.sync_polling_interval = sync_polling_interval
        self.stop_event = Event()
        self.last_tracking_output = None  # Store the last tracking output

        # Log initialization
        logging.info(f"TimeSync initialized for sensor ID {self.deployed_sensor_id}")

    def collect_and_send_time_sync_data(self):
        """Collects Chrony data and sends it to the server at regular intervals."""
        logging.info(f"TimeSync polling started with interval {self.sync_polling_interval} seconds.")
        while not self.stop_event.is_set():
            try:
                logging.info("Fetching tracking information from Chrony...")
                # Fetch tracking information
                tracking_result = subprocess.run(['chronyc', 'tracking'], stdout=subprocess.PIPE, text=True)
                tracking_output = tracking_result.stdout
                logging.info(f"Tracking information fetched.")

                # Get current time
                current_time = time.time()

                # Prepare payload
                payload = {
                    'deployed_sensor_id': self.deployed_sensor_id,
                    'data': {
                        'timestamp': current_time,
                        'chronyc_output': tracking_output
                    }
                }
                logging.info(f"Sending payload: {payload}")

                try:
                    response = requests.post(self.central_server_url, json=payload, timeout=5)
                    if response.status_code == 200:
                        logging.info(f"Successfully sent Chrony data to server at {datetime.fromtimestamp(current_time)}")
                    else:
                        logging.warning(f"Failed to send data: HTTP {response.status_code} - {response.text}")
                except requests.exceptions.RequestException as e:
                    logging.error(f"Exception during HTTP POST to {self.central_server_url}: {e}")
            except Exception as e:
                logging.error(f"Error collecting or sending Chrony data: {e}")

            # Wait for the specified interval or until stop_event is set
            logging.info(f"Waiting for {self.sync_polling_interval} seconds before next sync...")
            if self.stop_event.wait(self.sync_polling_interval):
                logging.info("Stop event set, exiting TimeSync polling loop.")
                break


    def start(self):
        """Start the time sync data collection."""
        self.thread = Thread(target=self.collect_and_send_time_sync_data)
        self.thread.start()
        logging.info("TimeSync thread started.")

    def stop(self):
        """Stop the time sync data collection."""
        self.stop_event.set()
        self.thread.join()
        logging.info("TimeSync thread stopped.")
