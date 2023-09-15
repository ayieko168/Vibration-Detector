import socket
from threading import Thread
from time import strftime, gmtime
import time
import struct
import binascii
from pprint import pprint
import json
from datetime import datetime
from decoder import CodecKT1Decoder
import requests

# Global Variables
host = '0.0.0.0'
port = 6500
url = "http://aws-demo.razorinformatics.co.ke/iot/v1/sensors"
headers = {
  'Accept': 'application/json'
}


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
        self.server_id = "3xVGBzjEJIR9xAj7"

    def run(self):
        print(f"[SERVER] [{datetime.now()}] NEW CONNECTION from: {self.addr[0]}")
        client = self.conn
        if client:
            self.identifier = self.addr
            
            try:
                decoder = CodecKT1Decoder()
                buff = self.conn.recv(8192)
                received = buff.decode('utf-8').strip()
                received_sepd = ' '.join(received[i:i+2] for i in range(0, len(received), 2))
                
                print(f"\n[DEBUG] [{self.addr[0]}] [{datetime.now()}]: DATA RECIEVED RAW: {buff}")
                # print(f"\n[DEBUG] [{self.addr[0]}] [{datetime.now()}]: DATA RECIEVED: {received_sepd}")
                
                ## Split the data packet according to the packet format.
                packet_structure = decoder.decode_packet_structure(received)
                print(json.dumps(packet_structure, indent=2))
                
                ## Verify the structure of the packet
                if packet_structure.get('start_bit') == 'EEEE' and packet_structure.get('stop_bit') == 'AAAA':
                    ## Found a login packet, create acknowledgement packet and send to device.
                    print(f"[DEBUG] [{self.addr[0]}] [{datetime.now()}]: Found a valid packet, running conditional protocol according to protocol number...")
                    
                    ## Login Message
                    if packet_structure.get('protocol_number') == '01':
                        
                        devide_data = decoder.device_data_decoder(received, print_bits=True)
                        imei = devide_data.get('imei')
                        print(json.dumps(devide_data, indent=2))
                        
                        print(f"[DEBUG] [{self.addr[0]}] [{datetime.now()}] Protocol Number: {packet_structure.get('protocol_number')} from IMEI: {imei}")
                        
                        # Achknowledge login and send responce to device
                        response_packet = str(decoder.calc_crc(packet_structure['information_bits'])).encode('utf-8')
                        response_packet += b"\n\r"
                        self.conn.send(response_packet)
                        # self.conn.send(f"FROM SERVER!!\n\r".encode('utf-8'))
                        print(f'[DEBUG] [{self.addr[0]}] [{datetime.now()}]: Sent response: {response_packet}')
                        
                        # Save the imei number
                        self.imei = imei
                        
                        # Send the data to server
                        response = requests.request("POST", url, headers=headers, data=devide_data)
                        while not response.ok:
                            print(f'[WARNING] [{self.addr[0]}] [{datetime.now()}]: failed to send the data to server CODE: {response.status_code}, trying again..')
                            response = requests.request("POST", url, headers=headers, data=devide_data)
                            time.sleep(2)
                        
                        print(f'[DEBUG] [{self.addr[0]}] [{datetime.now()}]: Successfully sent the data packet.')
                    
                else:
                    print(f"[DEBUG] [{self.addr[0]}] [{datetime.now()}]: Found a INVALID packet, Exiting.")
                    self.conn.close()

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
        
