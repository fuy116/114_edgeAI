# python_client.py
import socket
import time

class Socket:
    def __init__(self):
        # Create socket
        self.client_socket = None

    def conn(self, address='127.0.0.1', port=4500):
        self.client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_address = (address, port)
        while True:
            try:
                self.client_socket.connect((address, port))
                print(f"Connected to {address} on port {port}")
                return

            except socket.error as e:
                print(f"Connection failed: {e}. Retrying in 2 seconds...")
                time.sleep(2)  
            except socket.timeout:
                print(f"Connection timed out. Retrying in 2 seconds...")
                time.sleep(2) 

    def send(self, data):
        try:
            self.client_socket.sendall(data.encode())
            # receive from server
            #response = self.client_socket.recv(1024)
            #print(f"Received from server: {response.decode()}")
        except socket.error as e:
            print(f"Send data failed: {e}")

    def close(self):
        if self.client_socket:
            try:
                self.client_socket.close()
                print("Socket closed")
            except socket.error as e:
                print(f"Error closing socket: {e}")
            finally:
                self.client_socket = None

    def __del__(self):
        self.close()
