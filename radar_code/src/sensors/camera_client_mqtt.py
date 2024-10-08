import threading
import json
import os
import time
import cv2
import paho.mqtt.client as mqtt

class CameraClientMQTT:
    def __init__(self, client_id, broker_address = "192.168.68.125", port=1883) -> None:
        self.port = port
        self.broker_address = broker_address
        self.deployed_sensor_id = client_id
        self.client = None
        self.is_recording = False
        self.stop_event = threading.Event()
        self.status = "Not Connected"
        self.HEARTBEAT_INTERVAL = 10


# SENSOR MQTT SET UP

    def handle_mqtt_operations(self):
        self.client = mqtt.Client(client_id=self.deployed_sensor_id)  
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message

        try:
            self.client.connect(self.broker_address, self.port, 60)
            self.client.loop_start()   
            # mqtt thread loop    
            last_heartbeat_time = time.time()
            while not self.stop_event.is_set():
                
                current_time = time.time()
                if current_time - last_heartbeat_time >= self.HEARTBEAT_INTERVAL:
                    self.send_heartbeat_to_central_server()
                    last_heartbeat_time = time.time()
                time.sleep(1)
                
        except Exception as e:
            print(f"Failed to connect to Broker: {e}")
        finally:
            self.client.loop_stop()
            self.client.disconnect()
            print("Disconnected from broker.")        

    def on_connect(self, client, userdata, flags, rc):
        if rc == 0:
            print(f"{self.deployed_sensor_id} is connected to broker!")
            self.subscribe_to_topics(["to_sensor/global/commands/start_recording",
                                      "to_sensor/global/commands/stop_recording",
                                      f"to_sensor/{self.deployed_sensor_id}/commands/confirm_connection",
                                      f"to_sensor/{self.deployed_sensor_id}/commands/start_recording",
                                      f"to_sensor/{self.deployed_sensor_id}/commands/stop_recording",
                                      f"to_sensor/{self.deployed_sensor_id}/commands/status_update_request"])
        else:
            print(f"{self.client} unable to connect to broker with return code: {rc}")

    def subscribe_to_topics(self, topics):
        if isinstance(topics, str):
            topics = [topics]
        for topic in topics:
            self.client.subscribe(topic=topic)
            print(f"{self.deployed_sensor_id} is subscribed to {topic}")
    
    def on_message(self, client, userdata, msg):
        topic = msg.topic
        print(topic)
        data = json.loads(msg.payload)
        if topic.endswith("confirm_connection"):
            print(f"Received payload: {msg.payload.decode()}")  # Decode bytes to string for readability
            self.confirm_connection()
        elif topic.endswith("start_recording"):
            self.start_recording(data)
        elif topic.endswith("stop_recording"):
            self.stop_recording()
        elif topic.endswith("status_update_request"):
            self.send_status_update()

# CONFIRM CONNECTION WITH CENTRAL SERVER

    def confirm_connection(self):
        print(f"Central Server to {self.deployed_sensor_id} connection confrimed!")
        self.client.publish(
            f"to_central_server/{self.deployed_sensor_id}/status/confirm_connection", 
            json.dumps({"confrim connection": self.deployed_sensor_id}), 
            qos=2)
        self.status = "Ready"

# SEND HEARTBEAT STATUS UPDATE

    def send_heartbeat_to_central_server(self):
        self.client.publish(
            f"to_central_server/{self.deployed_sensor_id}/status/heartbeats_from_sensors",
            json.dumps({"heartbeat_time" : time.time()}), 
            qos=1)
        #print(f"Heartbeat sent from {self.deployed_sensor_id}")

# SEND REQUESTED STATUS UPDATE

    def send_status_update(self):
        # Send a regular status update about the recording status
        self.status = "Recording" if self.is_recording else "Ready"
        self.client.publish(
            f"to_central_server/{self.deployed_sensor_id}/status/updates_from_sensor",
            json.dumps({"status": self.status}), 
            qos=2)
        
# START CAMERA RECORDING

    def start_recording(self, message_data):
        threading.Thread(target=self.handle_recording, args=(message_data,)).start()

    def confirm_start_recording(self):
        self.client.publish(
            f"to_central_server/{self.deployed_sensor_id}/status/started_recording",\
            json.dumps({
                "deployed_sensor_id":self.deployed_sensor_id, 
                "message":"Recording Started"}),
            qos=2)

    def handle_recording(self, message_data):
        self.is_recording = True
        filepath = self._setup_camera(message_data['filename'])
        self._perform_recording(
            message_data['duration'], 
            message_data['delayed_start_timestamp'], 
            filepath)
        filesize = self._get_file_size(filepath)
        self._send_finished_recording_message_to_central_server(filename=message_data['filename'], filesize=filesize)
        self._cleanup_camera()

    def _setup_camera(self, filename):
        self.cap = cv2.VideoCapture(0)
        fourcc = cv2.VideoWriter_fourcc(*'XVID')
        data_directory = os.path.join(os.getcwd(), 'data')
        if not os.path.exists(data_directory):
            os.makedirs(data_directory)
        filepath = os.path.join(data_directory, f"{filename}.avi")
        self.out = cv2.VideoWriter(filepath, fourcc, 20.0, (640, 480))
        return filepath

    def _perform_recording(self, duration, delayed_start_timestamp, filepath):
        print("Waiting for delayed start time")
        while time.time() < delayed_start_timestamp:
            time.sleep(0.01)
        print("Recording started")
        while time.time() - delayed_start_timestamp < duration:
            ret, frame = self.cap.read()
            if ret:
                self.out.write(frame)
            else:
                break
        print(f"Data collection completed at {time.time()}")
        print(f"File saved: {filepath}")
        self.is_recording = False

    def _cleanup_camera(self):
        self.cap.release()
        self.out.release()
        
# SEND FINISHED RECORDING METADATA

    def _send_finished_recording_message_to_central_server(self, filename, filesize):
        self.client.publish(
            f"to_central_server/{self.deployed_sensor_id}/status/finished_recording",
            json.dumps({"deployed_sensor_id": self.deployed_sensor_id,
                "filename": filename,
                "file_size": filesize}), 
            qos=2)
        print("Sent file metadata to central server.")

    def _get_file_size(self, file_path):
        try:
            return os.path.getsize(file_path)
        except OSError as e:
            print(f"Error: {e}")
            return None

# CENTRAL SERVER COMMANDED STOP RECORDING

    def stop_recording(self):
        # Set a flag or a condition to stop the recording thread
        self.stop_event.set()
        self.client.publish(
            f"to_central_server/{self.deployed_sensor_id}/status/finished_recording", 
            "Recording Terminated by User",
            qos=1)

# SEND ALERTS TO CENTRAL SERVER
# TODO add alerts


#MAIN SENSOR TESTING

def main():
    stop_event = threading.Event()
    mqtt_client = CameraClientMQTT(client_id="0001")
    mqtt_client_thread = threading.Thread(target=mqtt_client.handle_mqtt_operations)
    mqtt_client_thread.start()

    try: 
        while True:
            time.sleep(10)
    except KeyboardInterrupt:
        print("Program interupted")
        stop_event.set()
        mqtt_client_thread.join()

if __name__ == "__main__":
    main()
    