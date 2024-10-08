import zmq
import time
import cv2
import json
import os

class CameraNode:
    def __init__(self, central_server_ip, 
                 central_server_sub_port=5555, 
                 central_server_router_port=5566):
        self.central_server_ip = central_server_ip
        self.sub_port = central_server_sub_port
        self.router_port = central_server_router_port

        self.sub_address = f"tcp://{self.central_server_ip}:{self.sub_port}"
        self.router_address = f"tcp://{self.central_server_ip}:{self.router_port}"
        
        self.context = zmq.Context()
        
        self.sub_socket = self.context.socket(zmq.SUB)
        self.sub_socket.connect(self.sub_address)
        self.sub_socket.setsockopt_string(zmq.SUBSCRIBE, '')

        self.router_socket = self.context.socket(zmq.DEALER)
        self.router_socket.setsockopt(zmq.IDENTITY, b"001")
        self.router_socket.connect(self.router_address)

        self.running = True

    def listen_for_commands(self):
        print(f"Listening for commands on {self.sub_address}")
        while self.running:
            message = self.sub_socket.recv_string()
            message_data = json.loads(message)
            print(f"Recieved messgae data: {message_data}")
            command = message_data["command"]
            print(f"Handling command: {command}")

            if message_data["command"] == "CONFIRM_CONNECTION":
                print("Central Server Connection Confirmed")
                self.send_status("Ready")
            elif message_data["command"] == "START_RECORDING":
                # self.acknowledge_command(message_data)
                self.start_data_collection(
                    delayed_start_timestamp=message_data["delayed_start_timestamp"], 
                    sensor_deployment_id=message_data['sensor_deployment_id'],
                    duration=message_data['duration'],
                    filename=message_data['filename'],
                    additional_info=message_data['additional_info']
                )
            elif message_data["command"] == "STATUS_UPDATE":
                self.send_status("Recording")
            elif message_data["command"] == "STOP_RECORDING":
                self.send_status("Completed")
                self.shutdown()
                self.running = False

    def send_status(self, status):
        try:
            self.router_socket.send_json({
                "status": status
            })
            print(f"Sent status: {status}")
        except Exception as e:
            print(f"Failed to send status: {e}")

    def acknowledge_command(self, message_data):
        try:
            self.router_socket.send_json({
                "status": "ACK",
                "sensor_deployment_id": message_data['sensor_deployment_id'],
                "message": "Command received and processed."
            })
            response = self.router_socket.recv_json()
            print(f"Acknowledgment sent and confirmed by server: {response['status']}")
        except Exception as e:
            print(f"Failed to send acknowledgment: {e}")

    def start_data_collection(self, delayed_start_timestamp, sensor_deployment_id, duration, filename, additional_info):
        cap = cv2.VideoCapture(0)
        fourcc = cv2.VideoWriter_fourcc(*'XVID')
        data_directory = os.path.join(os.getcwd(),'data')
        print(filename)
        filepath = os.path.join(data_directory, filename)+".avi"
        print(filepath)
        out = cv2.VideoWriter(filepath, fourcc, 20.0, (640, 480))

        # Wait until the specified timestamp to start data collection
        while time.time() < delayed_start_timestamp:
            time.sleep(0.01)

        start_time = time.time()
        print(f"Camera {sensor_deployment_id} Started")
        self.send_status("Recording")
        while time.time() - start_time < duration:
            ret, frame = cap.read()
            if ret:
                out.write(frame)
            else:
                break

        cap.release()
        out.release()
        self.send_status("Completed")
        print(f"Data collection completed at {time.time()}")
        print(f"File saved: {filepath}.avi")

    def shutdown(self):
        print("Shutting down the camera node...")
        self.sub_socket.close()
        self.router_socket.close()
        self.context.term()
        print("Resources have been cleanly terminated.")