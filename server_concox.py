import socket
from threading import Thread
from time import strftime, gmtime
import time
import struct
import binascii
from pprint import pprint
import json
from datetime import datetime

from decoder import *

# Global Variables
host = '0.0.0.0'
port = 6544



## EXAMPLE DATA
example_login_packet =    b"78 78 0d 01 03 58 65 71 03 21 34 30 00 06 dd 03 0d 0a"
example_location_packet = b"78 78 1f 12 17 06 09 0c 04 0c c6 00 23 15 ca 03 f3 54 34 0d 10 88 02 7f 02 07 f7 00 08 ad 00 07 57 25 0d 0a"
example_status_packet = b"78 78 0a 13 45 06 04 00 01 00 c1 0b 69 0d 0a"
## EXAMPLE DATA END


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
                    decoder = ConcoxDecoder()
                    buff = self.conn.recv(8192)
                    received = binascii.hexlify(buff)
                    received_sepd = binascii.hexlify(buff, " ")
                    
                    print(f"\n[DEBUG] [{self.addr[0]}] [{datetime.now()}]: DATA RECIEVED: {received_sepd}")
                    
                    ## Split the data packet according to the packet format.
                    packet_structure = decoder.decode_packet_format(received)
                    
                    ## Verify the structure of the packet
                    if packet_structure.get('start_bit') == '7878' and packet_structure.get('stop_bit') == '0D0A':
                        # Found a valid packet, create new response packet and send to device
                        print(f"[DEBUG] [{self.addr[0]}] [{datetime.now()}]: Found a valid packet, running conditional protocol according to protocol number...")
                        
                        ## Login Message
                        if packet_structure.get('protocol_number') == '01':
                            
                            print(f"[DEBUG] [{self.addr[0]}] [{datetime.now()}] [PROTOCOL NUMBER={packet_structure.get('protocol_number')}]: Is a login handshake.")
                            print(f"[SERVER] [{self.addr[0]}] [{datetime.now()}] Valid connection from IMEI: {packet_structure.get('information_content')}")
                            
                            # Achknowledge login and send responce to device
                            response_packet = decoder.construct_response(
                                packet_structure['protocol_number'], 
                                packet_structure['information_serial_number']
                                )
                            self.conn.send(response_packet)
                            print(f'[DEBUG] [{self.addr[0]}] [{datetime.now()}]: Sent response: {response_packet}')
                            
                            # Save the imei number
                            self.imei = packet_structure.get('information_content')
                        
                        ## Location Data 
                        elif packet_structure.get('protocol_number') == '12':
                            print(f"[DEBUG] [{self.addr[0]}] [{datetime.now()}] [PROTOCOL NUMBER={packet_structure.get('protocol_number')}]: Is a location message packet.")
                            print(f"[SERVER] [{self.addr[0]}] [{datetime.now()}] Valid connection from IMEI: {self.imei}")
                            
                            decoded_location_data = decoder.location_decoder(received)
                            print(f"[DEBUG] [{self.addr[0]}] [{datetime.now()}] [LOCATION DATA] :: {json.dumps(decoded_location_data)}")
                                                    
                        # ## Status Information
                        elif packet_structure.get('protocol_number') == '13':
                            print(f"[DEBUG] [{self.addr[0]}] [{datetime.now()}] [PROTOCOL NUMBER={packet_structure.get('protocol_number')}]: Is a status infromation packet.")
                            print(f"[SERVER] [{self.addr[0]}] [{datetime.now()}] Valid connection from IMEI: {self.imei}")

                            decoded_packet, response_packet = decoder.status_decoder(received)
                            
                            self.conn.send(response_packet)
                            print(f"[DEBUG] [{self.addr[0]}] [{datetime.now()}] [STATUS INFORMATION] :: {json.dumps(decoded_packet)}")
                            print(f'[DEBUG] [{self.addr[0]}] [{datetime.now()}]: Sent response: {response_packet}')
                            
                        else:
                            print(f"[DEBUG] [{self.addr[0]}] [{datetime.now()}] [UNKNOWN] [PROTOCOL NUMBER={packet_structure.get('protocol_number')}]: Is UNKNOWN.")
                            print(f"[SERVER] [{self.addr[0]}] [{datetime.now()}] Valid connection from IMEI: {self.imei}")
                            
                            with open('unkown_protocols.txt', 'a') as fo:
                                fo.write(f"[DEBUG] [{self.addr[0]}] [{datetime.now()}] [UNKNOWN] [PROTOCOL NUMBER={packet_structure.get('protocol_number')}]: Is UNKNOWN.\n")
                                fo.write(f"[DEBUG] [{self.addr[0]}] [{datetime.now()}] Valid connection from IMEI: {self.imei}\n")
                                fo.write(f"[DEBUG] [{self.addr[0]}] [{datetime.now()}] Recieved data packet: {received_sepd}\n")
                                fo.write('\n\n')
                                                    
                    else:
                        # Invalid packet
                        print(f"[ERROR] [{self.addr[0]}] [{datetime.now()}] : Invalid packet: {packet_structure}")
                        self.conn.send('\x00'.encode('utf-8'))
                        self.conn.close()

                except socket.error as err:
                    print(f"[SERVER] [ERROR] [{self.addr[0]}] [{datetime.now()}] Socket Error: {err}")
                    self.conn.close()
            
        else:
            print(f"[SERVER] [ERROR] [{self.addr[0]}] [{datetime.now()}] : Socket is null")
            self.conn.close()


if __name__ == "__main__":
    
    # exit(0)

    print(f"CONCOX GPS SERVER. {datetime.now()}")
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
        
