import socket
from threading import Thread
from time import strftime, gmtime
import struct
import binascii
from pprint import pprint
import json

from decoder import *

# Global Variables
host = '0.0.0.0'
port = 6544


## EXAMPLE DATA
example_login_packet =    b"78 78 0d 01 03 58 65 71 03 21 34 30 00 06 dd 03 0d 0a"
example_location_packet = b"78 78 1f 12 17 06 09 0c 04 0c c6 00 23 15 ca 03 f3 54 34 0d 10 88 02 7f 02 07 f7 00 08 ad 00 07 57 25 0d 0a"
## EXAMPLE DATA END


class ClientThread(Thread):
    def __init__(self, _socket) -> None:
        Thread.__init__(self)
        self.conn = _socket[0]
        self.addr = _socket[1]
        self.identifier = "None"
        self.logTime = strftime('%d %b %H:%M:%S', gmtime())
        self.step = 1
        self.imei = "unknown"

    def log(self, msg):
        print(f"[SERVER]: {self.logTime}\t{self.identifier}\t{msg}")
        pass

    def run(self):
        self.log(f"NEW CONNECTION ({self.identifier})")
        client = self.conn
        if client:
            self.identifier = self.addr[0]
            for _ in range(2):
                try:
                    decoder = ConcoxDecoder()
                    buff = self.conn.recv(8192)
                    received = binascii.hexlify(buff)
                    received_sepd = binascii.hexlify(buff, " ")
                    
                    print(f"\n[DEBUG]: DATA RECIEVED: {received_sepd}\n\n")
                    
                    ## Split the data packet according to the packet format.
                    packet_structure = decoder.decode_packet_format(received)
                    
                    ## Verify the structure of the packet
                    if packet_structure.get('start_bit') == '7878' and packet_structure.get('stop_bit') == '0D0A':
                        # Found a valid packet, create new response packet and send to device
                        print("[DEBUG]: Found a valid packet, running conditional protocol according to protocol number...")
                        
                        ## Login Message
                        if packet_structure.get('protocol_number') == '01':
                            
                            print(f"[DEBUG] [PROTOCOL NUMBER={packet_structure.get('protocol_number')}]: Is a login handshake.")
                            print(f"[VALID CONNECTION] Valid connection from IMEI: {packet_structure.get('information_content')}")
                            
                            # Achknowledge login and send responce to device
                            response_packet = decoder.construct_response(
                                packet_structure['protocol_number'], 
                                packet_structure['information_serial_number']
                                )
                            self.conn.send(response_packet)
                            print(f'[DEBUG]: Sent response: {response_packet}')
                            
                            # Save the imei number
                            self.imei = packet_structure.get('information_content')
                        
                        ## Location Data 
                        elif packet_structure.get('protocol_number') == '12':
                            print(f"[DEBUG] [PROTOCOL NUMBER={packet_structure.get('protocol_number')}]: Is a location message packet.")
                            print(f"[VALID CONNECTION] Valid connection from IMEI: {self.imei}")
                            
                            decoded_location_data = decoder.location_decoder(received)
                            print(f"[DEBUG] [LOCATION DATA] :: {json.dumps(decoded_location_data)}")
                        
                        else:
                            print(f"[DEBUG] [PROTOCOL NUMBER={packet_structure.get('protocol_number')}]: Is UNKNOWN.")
                            print(f"[VALID CONNECTION] Valid connection from IMEI: {self.imei}")
                                
                    else:
                        # Invalid packet
                        print(f"[ERROR]: Invalid packet: {packet_structure}")
                        self.conn.send('\x00'.encode('utf-8'))

                    # if len(received) > 2:
                    #     if self.step == 1:
                    #         self.step = 2
                    #         self.imei = decoder.get_imei(received) 
                    #         self.log("Device Authenticated | IMEI: {}".format(self.imei))
                    #         self.conn.send('\x01'.encode('utf-8'))
                    #     elif self.step == 2:
                    #         decoder = Decoder()
                    #         data = decoder.decode_data(received)
                    #         len_records = data['records_length']
                    #         if len_records == 0:
                    #             self.conn.send('\x00'.encode('utf-8'))
                    #             self.conn.close()
                    #         else:
                    #             self.conn.send(struct.pack("!L", len_records))
                    #             self.log("Done! Closing Connection")
                    #             self.conn.close()
                            
                    #         print(f"[DEBUG] DATA: \n{json.dumps(data, indent=2)}")
                    # else:
                    #     self.conn.send('\x00'.encode('utf-8'))
                except socket.error as err:
                    print(f"[+] Socket Error: {err}")
        else:
            self.log('Socket is null')


if __name__ == "__main__":
    
    # exit(0)

    print(f"SERVER. {strftime('%d %b %H:%M:%S', gmtime())}")
    print(f"Server Started at port: {port}")
    server = None
    try:
        server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    except socket.error as e:
        print("[X] Socket Creation Error: {}".format(e))
        exit(0)

    try:
        server.bind((host, port))
    except socket.error as e:
        print("[X] Socket Binding Error: {}".format(e))
        exit(0)

    try:
        server.listen(5)
    except socket.error as e:
        print("[X] Socket Listening Error: {}".format(e))
        exit(0)

    while True:
        ClientThread(server.accept()).start()
        
