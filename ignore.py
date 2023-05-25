import struct

def extract_data(byte_string):
    # Unpack the first byte to determine the number of bytes to read
    num_bytes = struct.unpack('!B', byte_string[:1])[0]

    # Pad the byte string with zero bytes if necessary
    padded_byte_string = byte_string[1:num_bytes+1].rjust(8, b'\x00')

    # Extract the desired number of bytes from the padded byte string
    data = struct.unpack('!Q', padded_byte_string)[0]

    return data

byte_string = b'\x00\x0f359633107797107'
result = extract_data(byte_string)
print(result)
