import socket
import struct
import binascii

PORT = 6543


# Set up a TCP/IP server
tcp_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
tcp_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
# Bind the socket to server address and port 81
server_address = ('0.0.0.0', PORT)
tcp_socket.bind(server_address)

# Listen on port 81
tcp_socket.listen(1)

while True:
    print("Waiting for connection")
    connection, client = tcp_socket.accept()
    try:
        print('Connection from', client)
        while True:
            data = connection.recv(1280)
            data = binascii.hexlify(data)
            # data = data.decode("utf-8")
            print(f'Received : ', data)
            if data:
                # if data.strip().isdigit():
                #     device_id = int(data.strip())
                #     print("Device ID: ", device_id)
                connection.sendall("\01".encode('utf-8'))
            else:
                break
    finally:
        # Clean up the connection
        connection.close()


