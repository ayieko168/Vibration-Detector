import socket
from threading import Thread
from time import strftime, gmtime
import time
import struct
import binascii
from pprint import pprint
import json
from datetime import datetime


# Global Variables
host = '0.0.0.0'
port = 6500




class ClientThread(Thread):
    def __init__(self, _socket) -> None:
        Thread.__init__(self)
        self.conn = _socket[0]
        self.addr = _socket[1]
        self.identifier = "None"
        self.logTime = datetime.now()
        self.step = 1
        self.imei = "unknown"
        self.start_time = time.time()

    def run(self):
        print(f"[SERVER] NEW CONNECTION from: {self.addr[0]}")
        client = self.conn
        if client:
            self.identifier = self.addr
            for _ in range(5):
                try:
                    
                    buff = self.conn.recv(8192)
                    received = binascii.hexlify(buff)
                    received_sepd = binascii.hexlify(buff, " ")
                    
                    print(f"\n[DEBUG] [{self.addr[0]}] [{datetime.now()}]: DATA RECIEVED RAW: {buff}")
                    # print(f"\n[DEBUG] [{self.addr[0]}] [{datetime.now()}]: DATA RECIEVED: {received_sepd}")

                    self.conn.send(f"FROM SERVER: I recieved :: {buff}".encode("utf-8"))
                    
        
                except socket.error as err:
                    print(f"[SERVER] [ERROR] [{self.addr[0]}] [{datetime.now()}] Socket Error: {err}")
                    self.conn.close()
            
        else:
            print(f"[SERVER] [ERROR] [{self.addr[0]}] [{datetime.now()}] : Socket is null")
            self.conn.close()


if __name__ == "__main__":
    
    # exit(0)

    print(f"GENERAL TCP SERVER. {datetime.now()}")
    print(f"Server Started at port: {port}")
    server = None
    try:
        server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    except socket.error as e:
        print("[SERVER] [ERROR] Socket Creation Error: {}".format(e))
        exit(0)

    try:
        server.bind((host, port))
    except socket.error as e:
        print("[SERVER] [ERROR] Socket Binding Error: {}".format(e))
        exit(0)

    try:
        server.listen(5)
    except socket.error as e:
        print("[SERVER] [ERROR] Socket Listening Error: {}".format(e))
        exit(0)

    while True:
        ClientThread(server.accept()).start()
        
