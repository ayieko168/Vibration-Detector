import socket
from threading import Thread
from time import strftime, gmtime
import struct
import binascii
from pprint import pprint
import json

from decoder import Decoder

# Global Variables
host = '0.0.0.0'
port = 6543


## EXAMPLE DATA

example_imei = b"000F333536333037303432343431303133"
example_avl = b"00000000000004a7080c000001885bb720200015ffae49ff3df8a6064600ad0b0000001407ef01f0011504c80045010100b30009b50009b60006424c6b430fbf4400000900ae11ffd612000f13fc1102f10000f99e100000d15b020b0000000213fed40b0e00000001000dcf3e000001885bb6e5880015ffae6aff3df852064600ad0b0000001407ef01f0011503c80045010100b30009b5000fb60007424c7d430fc14400000900ae11ffd212001b13fc1802f10000f99e100000d15b020b0000000213fed40b0e00000001000dcf3e000001885bb6ddb80015ffaf32ff3df77a064700a80b000c001407ef01f0011503c80045010100b30009b5000fb60007424c24430fb04400000900ae11ffce12001b13fc1502f10000f99e100000d15b020b0000000213fed40b0e00000001000dcf3e000001885bb6d9d00015ffaf21ff3df7ee064800b30a000b001407ef01f0011503c80045010100b30009b50010b60007424c55430fbc4400000900ae11ffd612001713fc1502f10000f99e100000d15b020b0000000213fed40b0e00000001000dcf3e000001885bb6ca300015ffafc8ff3df884064800bd080007001407ef01f0011503c80045010100b30009b5000bb60009424c95430fc54400000900ae11ffd212001b13fc1802f10000f99e100000d155020b0000000213fed40b0e00000001000dcf3e000001885bb6aed80015ffb00bff3df7ff06480005080007001407ef01f0011503c80045010100b30009b5000bb60009424c57430fba4400000900ae11ffce12001b13fc0d02f10000f99e100000d155020b0000000213fed40b0e00000001000dcf3e000001885bb68f980015ffafa7ff3df6c206490015090000001407ef01f0011504c80045010100b30009b5000ab60007424c63430fbc4400000900ae11ffd212001b13fc1502f10000f99e100000d148020b0000000213fed40b0e00000001000dcf3e000001885bb655000015ffb080ff3df392064d00a80a0006001407ef01f0011504c80045010100b30009b50010b60007424c2e430fb34400000900ae11ffd212001b13fc0d02f10000f99e100000d148020b0000000213fed40b0e00000001000dcf3e000001885bb64d300015ffb090ff3df33e064d009d09000a001407ef01f0011504c80045010100b30009b5000ab60007424c74430fc14400000900ae11ffce12001f13fbf102f10000f99e100000d148020b0000000213fed40b0e00000001000dcf3e000001885bb635c00015ffaf75ff3df726064e00a909000e001407ef01f0011504c80045010100b30009b5000ab60007424c68430fbe4400000900ae11ffd212001f13fc1502f10000f99e100000d13e020b0000000213fed40b0e00000001000dcf3e000001885bb62df00015ffaf43ff3df8b6064e00ba090009001407ef01f0011504c80045010100b30009b50010b60008424c89430fc54400000900ae11ffd212001b13fc1502f10000f99e100000d138020b0000000213fed40b0e00000001000dcf3e000001885bb62a080015ffaf43ff3df92b064d00c6090007001407ef01f0011504c80045010100b30009b5000ab60007424c8e430fc44400000900ae11ffd212001713fc1502f10000f99e100000d138020b0000000213fed40b0e00000001000dcf3e0c00003767".strip()

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
        print(f"{self.logTime}\t{self.identifier}\t{msg}")
        pass

    def run(self):
        client = self.conn
        if client:
            self.identifier = self.addr[0]
            for _ in range(2):
                try:
                    decoder = Decoder()
                    buff = self.conn.recv(8192)
                    received = binascii.hexlify(buff)
                    received_sepd = binascii.hexlify(buff, " ")
                    
                    print(f"\n[DEBUG]: DATA RECIEVED: {buff}\n\n")
                    
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
        
