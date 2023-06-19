# import libscrc
# import binascii

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

def decode_codec8_longitude(hex_byte):
    # Convert the hex byte string to an integer
    byte_value = int(hex_byte, 16)
    
    # Check the sign bit (bit 31)
    if byte_value & 0x80000000:
        # Perform two's complement for negative values
        byte_value = byte_value - 0xFFFFFFFF - 1
    
    # Divide by 10000000 to get the decimal value
    longitude = byte_value / 10000000.0
    
    return longitude

hex_byte = '15f27275'
longitude = decode_codec8_longitude(hex_byte)

print(longitude)



# string_crc = bytes.fromhex('0d0001') #+ b'0123456789012345' +code+time_zone+snol
# error_check = bytes.fromhex('8CDD')

# print(string_crc)
# bollen = crc16x_25_bit(string_crc)
# print(bollen)
