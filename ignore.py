from pprint import pprint


def decode_avl_data_packet(packet):
    # Breakdown the AVL Data Packet
    zero_bytes = packet[:8]
    data_field_length = packet[8:16]
    codec_id = packet[16:18]
    num_data_records = packet[18:20]

    offset = 20
    data_records = []

    for _ in range(int(num_data_records, 16)):
        record = {}

        # AVL Data
        timestamp = packet[offset:offset+16]
        offset += 16
        priority = packet[offset:offset+2]
        offset += 2
        longitude = packet[offset:offset+8]
        offset += 8
        latitude = packet[offset:offset+8]
        offset += 8
        altitude = packet[offset:offset+4]
        offset += 4
        angle = packet[offset:offset+4]
        offset += 4
        satellites = packet[offset:offset+2]
        offset += 2
        speed = packet[offset:offset+4]
        offset += 4

        # Event IO ID
        event_io_id = packet[offset:offset+2]
        offset += 2

        # N of Total ID
        n_total_id = packet[offset:offset+2]
        offset += 2

        # N1 of One Byte IO
        n_one_byte_io = packet[offset:offset+2]
        offset += 2

        # One Byte IO
        one_byte_io = {}
        for _ in range(int(n_one_byte_io, 16)):
            io_id = packet[offset:offset+2]
            offset += 2
            io_value = packet[offset:offset+2]
            offset += 2
            one_byte_io[io_id] = io_value

        # N2 of Two Bytes IO
        n_two_bytes_io = packet[offset:offset+2]
        offset += 2

        # Two Bytes IO
        two_bytes_io = {}
        for _ in range(int(n_two_bytes_io, 16)):
            io_id = packet[offset:offset+2]
            offset += 2
            io_value = packet[offset:offset+4]
            offset += 4
            two_bytes_io[io_id] = io_value

        # N4 of Four Bytes IO
        n_four_bytes_io = packet[offset:offset+2]
        offset += 2

        # Four Bytes IO
        four_bytes_io = {}
        for _ in range(int(n_four_bytes_io, 16)):
            io_id = packet[offset:offset+2]
            offset += 2
            io_value = packet[offset:offset+8]
            offset += 8
            four_bytes_io[io_id] = io_value

        # N8 of Eight Bytes IO
        n_eight_bytes_io = packet[offset:offset+2]
        offset += 2

        # Eight Bytes IO
        eight_bytes_io = {}
        for _ in range(int(n_eight_bytes_io, 16)):
            io_id = packet[offset:offset+2]
            offset += 2
            io_value = packet[offset:offset+16]
            offset += 16
            eight_bytes_io[io_id] = io_value

        record['AVL Data'] = {
            'Timestamp': timestamp,
            'Priority': priority,
            'Longitude': longitude,
            'Latitude': latitude,
            'Altitude': altitude,
            'Angle': angle,
            'Satellites': satellites,
            'Speed': speed,
            'Event IO ID': event_io_id,
            'N of Total ID': n_total_id,
            'N1 of One Byte IO': n_one_byte_io,
            'One Byte IO': one_byte_io,
            'N2 of Two Bytes IO': n_two_bytes_io,
            'Two Bytes IO': two_bytes_io,
            'N4 of Four Bytes IO': n_four_bytes_io,
            'Four Bytes IO': four_bytes_io,
            'N8 of Eight Bytes IO': n_eight_bytes_io,
            'Eight Bytes IO': eight_bytes_io
        }

        data_records.append(record)

    # Number of Data 2 (Number of Total Records)
    num_total_records = packet[offset:offset+2]
    offset += 2

    # CRC-16
    crc16 = packet[offset:]

    decoded_data = {
        'Zero Bytes': zero_bytes,
        'Data Field Length': data_field_length,
        'Codec ID': codec_id,
        'Number of Data 1 (Records)': num_data_records,
        'Data Records': data_records,
        'Number of Data 2 (Number of Total Records)': num_total_records,
        'CRC-16': crc16
    }

    pprint(decoded_data)


def decode_coordinate(coordinate_hex):
    # Convert hex string to integer
    coordinate_int = int(coordinate_hex, 16)

    # Check if the coordinate is negative
    if coordinate_int & (1 << 31):
        # If negative, convert to positive by flipping the sign bit
        coordinate_int = coordinate_int ^ 0xFFFFFFFF

    # Extract individual components
    degrees = coordinate_int // 10000000
    minutes = (coordinate_int % 10000000) // 100000
    seconds = (coordinate_int % 100000) // 1000
    milliseconds = coordinate_int % 1000

    # Calculate the decimal value of the coordinate
    decimal_coordinate = (degrees + minutes / 60 + seconds / 3600 + milliseconds / 3600000) * 10000000

    # Adjust the sign if longitude is in west or latitude in south
    if coordinate_int & (1 << 31):
        decimal_coordinate *= -1

    return decimal_coordinate



# Example usage


# Example 1
packet1 = '00000000000004a7080c000001885bb720200015ffae49ff3df8a6064600ad0b0000001407ef01f0011504c80045010100b30009b50009b60006424c6b430fbf4400000900ae11ffd612000f13fc1102f10000f99e100000d15b020b0000000213fed40b0e00000001000dcf3e000001885bb6e5880015ffae6aff3df852064600ad0b0000001407ef01f0011503c80045010100b30009b5000fb60007424c7d430fc14400000900ae11ffd212001b13fc1802f10000f99e100000d15b020b0000000213fed40b0e00000001000dcf3e000001885bb6ddb80015ffaf32ff3df77a064700a80b000c001407ef01f0011503c80045010100b30009b5000fb60007424c24430fb04400000900ae11ffce12001b13fc1502f10000f99e100000d15b020b0000000213fed40b0e00000001000dcf3e000001885bb6d9d00015ffaf21ff3df7ee064800b30a000b001407ef01f0011503c80045010100b30009b50010b60007424c55430fbc4400000900ae11ffd612001713fc1502f10000f99e100000d15b020b0000000213fed40b0e00000001000dcf3e000001885bb6ca300015ffafc8ff3df884064800bd080007001407ef01f0011503c80045010100b30009b5000bb60009424c95430fc54400000900ae11ffd212001b13fc1802f10000f99e100000d155020b0000000213fed40b0e00000001000dcf3e000001885bb6aed80015ffb00bff3df7ff06480005080007001407ef01f0011503c80045010100b30009b5000bb60009424c57430fba4400000900ae11ffce12001b13fc0d02f10000f99e100000d155020b0000000213fed40b0e00000001000dcf3e000001885bb68f980015ffafa7ff3df6c206490015090000001407ef01f0011504c80045010100b30009b5000ab60007424c63430fbc4400000900ae11ffd212001b13fc1502f10000f99e100000d148020b0000000213fed40b0e00000001000dcf3e000001885bb655000015ffb080ff3df392064d00a80a0006001407ef01f0011504c80045010100b30009b50010b60007424c2e430fb34400000900ae11ffd212001b13fc0d02f10000f99e100000d148020b0000000213fed40b0e00000001000dcf3e000001885bb64d300015ffb090ff3df33e064d009d09000a001407ef01f0011504c80045010100b30009b5000ab60007424c74430fc14400000900ae11ffce12001f13fbf102f10000f99e100000d148020b0000000213fed40b0e00000001000dcf3e000001885bb635c00015ffaf75ff3df726064e00a909000e001407ef01f0011504c80045010100b30009b5000ab60007424c68430fbe4400000900ae11ffd212001f13fc1502f10000f99e100000d13e020b0000000213fed40b0e00000001000dcf3e000001885bb62df00015ffaf43ff3df8b6064e00ba090009001407ef01f0011504c80045010100b30009b50010b60008424c89430fc54400000900ae11ffd212001b13fc1502f10000f99e100000d138020b0000000213fed40b0e00000001000dcf3e000001885bb62a080015ffaf43ff3df92b064d00c6090007001407ef01f0011504c80045010100b30009b5000ab60007424c8e430fc44400000900ae11ffd212001713fc1502f10000f99e100000d138020b0000000213fed40b0e00000001000dcf3e0c00003767'
longitude = b'ff3df852'
# pprint()
pprint(decode_avl_data_packet(packet1))

