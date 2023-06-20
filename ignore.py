import libscrc
import binascii

def crc_check(string_crc, error_check):
    crc16 = libscrc.x25(string_crc)
    var=0
    final_crc=""
    for j in hex(crc16):
        if var>1:
            final_crc=final_crc+j
        var+=1
    if len(final_crc)==3:
        final_crc="0"+final_crc
    elif len(final_crc)==2:
        final_crc="00"+final_crc
    elif len(final_crc)==1:
        final_crc="000"+final_crc
    print(f"Generated crc: {final_crc}")
    print(f"sent by terminal: {error_check}")
    if final_crc==error_check:
        return True
    else:
        return False
    
    
def calculate_error_check(packet):
    # Convert the packet to bytes
    packet_bytes = bytes.fromhex(packet)

    # Perform XOR operation on all bytes except start and stop bits
    error_check = 0
    for byte in packet_bytes:
        error_check ^= byte

    # Return the error check value as a hex string
    return hex(error_check)[2:].zfill(2).upper()


def crc16x_25_bit(data, crc=0xFFFF):
    if data is None:
        return 0
    crc = ~crc
    crc &= 0xffff
    for byte in data:
        crc ^= byte
        for k in range(8):
            crc = (crc >> 1) ^ 0x8408 if crc & 1 else crc >> 1
    crc ^= 0xffff
    return crc





# string_crc = bytes.fromhex() #+ b'0123456789012345' +code+time_zone+snol
# error_check = bytes.fromhex('8CDD')

# print(string_crc)

bollen = crc16x_25_bit(b'0D0101234567890123450001')
print(hex(libscrc.x25(bytes.fromhex('0D0101234567890123450001')))[2:].zfill(4).upper())
