
import struct
import binascii
from pprint import pprint
import json
import libscrc

## TODO: Decode network operators code from: https://xphone24.com/operator-codes.php#:~:text=Operator%20code%20is%20a%20unique%20number%20assigned%20to,Network%20Code%20%28MNC%29%20and%20Mobile%20Country%20Code%20%28MCC%29.

class Decoder:
    
    def __init__(self):
        
        self.avl_ids = {}

        ## read the alv IDS dictionary
        with open('avl_ids.json') as f:
            self.avl_ids = json.load(f)
    
    def padd(self, data):
        if type(data) == bytes:
            data = data.decode('utf-8')
            
        return ' '.join(data[i:i+2] for i in range(0, len(data), 2))
    
    def extract_packet_data(self, avl_packet):

        # Breakdown the AVL Data Packet
        zero_bytes = avl_packet[:8]
        data_field_length = avl_packet[8:16]
        codec_id = avl_packet[16:18]
        num_data_records = avl_packet[18:20]

        offset = 20
        data_records = []

        for _ in range(int(num_data_records, 16)):
            record = {}

            # AVL Data
            timestamp = avl_packet[offset:offset+16]
            offset += 16
            priority = avl_packet[offset:offset+2]
            offset += 2
            longitude = avl_packet[offset:offset+8]
            offset += 8
            latitude = avl_packet[offset:offset+8]
            offset += 8
            altitude = avl_packet[offset:offset+4]
            offset += 4
            angle = avl_packet[offset:offset+4]
            offset += 4
            satellites = avl_packet[offset:offset+2]
            offset += 2
            speed = avl_packet[offset:offset+4]
            offset += 4

            # Event IO ID
            event_io_id = avl_packet[offset:offset+2]
            offset += 2

            # N of Total ID
            n_total_id = avl_packet[offset:offset+2]
            offset += 2

            # N1 of One Byte IO
            n_one_byte_io = avl_packet[offset:offset+2]
            offset += 2

            # One Byte IO
            one_byte_io = {}
            for _ in range(int(n_one_byte_io, 16)):
                io_id = avl_packet[offset:offset+2]
                offset += 2
                io_value = avl_packet[offset:offset+2]
                offset += 2
                one_byte_io[io_id] = io_value

            # N2 of Two Bytes IO
            n_two_bytes_io = avl_packet[offset:offset+2]
            offset += 2

            # Two Bytes IO
            two_bytes_io = {}
            for _ in range(int(n_two_bytes_io, 16)):
                io_id = avl_packet[offset:offset+2]
                offset += 2
                io_value = avl_packet[offset:offset+4]
                offset += 4
                two_bytes_io[io_id] = io_value

            # N4 of Four Bytes IO
            n_four_bytes_io = avl_packet[offset:offset+2]
            offset += 2

            # Four Bytes IO
            four_bytes_io = {}
            for _ in range(int(n_four_bytes_io, 16)):
                io_id = avl_packet[offset:offset+2]
                offset += 2
                io_value = avl_packet[offset:offset+8]
                offset += 8
                four_bytes_io[io_id] = io_value

            # N8 of Eight Bytes IO
            n_eight_bytes_io = avl_packet[offset:offset+2]
            offset += 2

            # Eight Bytes IO
            eight_bytes_io = {}
            for _ in range(int(n_eight_bytes_io, 16)):
                io_id = avl_packet[offset:offset+2]
                offset += 2
                io_value = avl_packet[offset:offset+16]
                offset += 16
                eight_bytes_io[io_id] = io_value

            record = {
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
        num_total_records = avl_packet[offset:offset+2]
        offset += 2

        # CRC-16
        crc16 = avl_packet[offset:]

        decoded_data = {
            'Zero Bytes': zero_bytes,
            'Data Field Length': data_field_length,
            'Codec ID': codec_id,
            'Number of Data 1 (Records)': num_data_records,
            'Data Records': data_records,
            'Number of Data 2 (Number of Total Records)': num_total_records,
            'CRC-16': crc16
        }

        # pprint(decoded_data)

        return decoded_data

    def decode_timestamp(self, timestamp_packet):
        return int(timestamp_packet, 16)
    
    def decode_priority(self, priority_packet):
        priority = int(priority_packet, 16)

        if priority == 0:
            return "LOW"
        elif priority == 1:
            return "HIGH"
        elif priority == 2:
            return "PANIC"

    def decode_io_elements(self, io_packet):

        ## Extract the id and value from the packet
        element_id, val = io_packet

        ## Convert the extracted values from hex to integers
        element_id = int(element_id, 16)
        val = int(val, 16)
        
        ## Get the io_element defination from the avl ids dictionary lookup
        # also add the sent io value to the dictionary.
        result_dict = self.avl_ids[str(element_id)]
        result_dict["Value"] = val

        ## Format the data in the format: Property Name, (Value * multimlier [units])
        # if Description has data as a key
        property_value = ""
        if result_dict['Description'].get('data') is not None:
            if result_dict['Multiplier'] != '-':
                property_value = f"{val * float(result_dict['Multiplier'])}"
            else:
                property_value = f"{val}"

            if result_dict['Units'] != '-':
                    property_value += f" [{result_dict['Units']}]"
        else:
            property_value = f"{result_dict['Description'][str(val)]}"
        
        ## Contstruct the data dictionary
        result = {
            "Property Name": result_dict['Property Name'],
            "Property Value": property_value
        }


        return result

    def decode_coordinate(self, coordinate_hex):
        
        # Convert the hex byte string to an integer
        byte_value = int(coordinate_hex, 16)
        
        # Check the sign bit (bit 31)
        if byte_value & 0x80000000:
            # Perform two's complement for negative values
            byte_value = byte_value - 0xFFFFFFFF - 1
        
        # Divide by 10000000 to get the decimal value
        coordinate_dec = byte_value / 10000000.0
        
        return coordinate_dec

    def decode_data(self, avl_packet):
        
        ## Dict to hold the decoded data
        data = {}

        ## Break down the packets into the individual bytes of data as per the Teltonika AVL Protocol
        unpacked_packet = self.extract_packet_data(avl_packet)
        # pprint(unpacked_packet)

        ## Verify the packet is correct
        preamble = int(unpacked_packet['Zero Bytes'], 16)
        if preamble != 0:
            print(f"[DECODER] [ERROR] Invalid packet - Zerro padding not accurate: {preamble}")
            return None
        
        num_of_data_1 = int(unpacked_packet["Number of Data 1 (Records)"], 16)
        num_of_data_2 = int(unpacked_packet["Number of Data 2 (Number of Total Records)"], 16)
        if num_of_data_1 != num_of_data_2:
            print(f"[DECODER] [ERROR] Invalid packet - The Number of Data 1 and 2 are not equal: DATA1={num_of_data_1}, DATA2={num_of_data_2}")
            return None
        
        if len(unpacked_packet['Data Records']) != num_of_data_1:
            print(f"[DECODER] [ERROR] Invalid packet - The Number of Records and Data 1 are equal: REC_LEN={len(unpacked_packet['Data Records'])}, DATA1={num_of_data_1}, DATA2={num_of_data_1}")
            return None
        
        ## Decode the data records
        avl_records = []
        for avl_data in unpacked_packet['Data Records']:
            avl_record = {}
            # Timestamp – a difference, in milliseconds, between the current time and midnight, January, 1970 UTC (UNIX time).
            avl_record['timestamp'] = self.decode_timestamp(avl_data['Timestamp'])

            # Priority – field which define AVL data priority.
            avl_record['priority'] = self.decode_priority(avl_data['Priority'])

            # GPS Element – location information of the AVL data.
            avl_record['longitude'] = self.decode_coordinate(avl_data['Longitude'])  # Longitude – east – west position.
            avl_record['latitude'] = self.decode_coordinate(avl_data['Latitude'])  # Latitude – north – south position.
            avl_record['altitude'] = int(avl_data['Altitude'], 16)  # Altitude – meters above sea level.
            avl_record['angle'] = int(avl_data['Angle'], 16)  # Angle – degrees from north pole.
            avl_record['satellites'] = int(avl_data['Satellites'], 16)  # Satellites – number of visible satellites.
            avl_record['speed'] = int(avl_data['Speed'], 16)  # Speed – speed calculated from satellites.

            ## IO Element – additional configurable information from device.
            io_elements = []
            for io_element in avl_data['One Byte IO'].items():
                io_elements.append(self.decode_io_elements(io_element))
            for io_element in avl_data['Two Bytes IO'].items():
                io_elements.append(self.decode_io_elements(io_element))
            for io_element in avl_data['Four Bytes IO'].items():
                io_elements.append(self.decode_io_elements(io_element))
            for io_element in avl_data['Eight Bytes IO'].items():
                io_elements.append(self.decode_io_elements(io_element))

            # TODO Add Events logic here
            # io_elements.append(self.decode_io_elements(avl_data['Event IO ID'].items()))

            avl_record['io_elements'] = io_elements


            avl_records.append(avl_record)
        
        data['data_length'] = int(unpacked_packet['Data Field Length'], 16)
        data['records_length'] = int(unpacked_packet['Number of Data 1 (Records)'], 16)
        data['avl_records'] = avl_records

        return data

    def get_imei(self, imei_data):

        # Extract IMEI length from the first two bytes
        imei_length = int(imei_data[:4], 16)

        # Extract IMEI bytes based on the length
        imei_bytes = imei_data[4:]

        # Decode IMEI as text
        imei = binascii.unhexlify(imei_bytes).decode('utf-8')

        # print(f"imei_length = {imei_length}")
        # print(f"imei_bytes = {imei_bytes}")
        # print(f"imei = {imei}")

        return imei


class ConcoxDecoder:

    def __init__(self) -> None:
        
        self.crctab16 = [
            0x0000, 0x1189, 0x2312, 0x329B, 0x4624, 0x57AD, 0x6536, 0x74BF,
            0x8C48, 0x9DC1, 0xAF5A, 0xBED3, 0xCA6C, 0xDBE5, 0xE97E, 0xF8F7,
            0x1081, 0x0108, 0x3393, 0x221A, 0x56A5, 0x472C, 0x75B7, 0x643E,
            0x9CC9, 0x8D40, 0xBFDB, 0xAE52, 0xDAED, 0xCB64, 0xF9FF, 0xE876,
            0x2102, 0x308B, 0x0210, 0x1399, 0x6726, 0x76AF, 0x4434, 0x55BD,
            0xAD4A, 0xBCC3, 0x8E58, 0x9FD1, 0xEB6E, 0xFAE7, 0xC87C, 0xD9F5,
            0x3183, 0x200A, 0x1291, 0x0318, 0x77A7, 0x662E, 0x54B5, 0x453C,
            0xBDCB, 0xAC42, 0x9ED9, 0x8F50, 0xFBEF, 0xEA66, 0xD8FD, 0xC974,
            0x4204, 0x538D, 0x6116, 0x709F, 0x0420, 0x15A9, 0x2732, 0x36BB,
            0xCE4C, 0xDFC5, 0xED5E, 0xFCD7, 0x8868, 0x99E1, 0xAB7A, 0xBAF3,
            0x5285, 0x430C, 0x7197, 0x601E, 0x14A1, 0x0528, 0x37B3, 0x263A,
            0xDECD, 0xCF44, 0xFDDF, 0xEC56, 0x98E9, 0x8960, 0xBBFB, 0xAA72,
            0x6306, 0x728F, 0x4014, 0x519D, 0x2522, 0x34AB, 0x0630, 0x17B9,
            0xEF4E, 0xFEC7, 0xCC5C, 0xDDD5, 0xA96A, 0xB8E3, 0x8A78, 0x9BF1,
            0x7387, 0x620E, 0x5095, 0x411C, 0x35A3, 0x242A, 0x16B1, 0x0738,
            0xFFCF, 0xEE46, 0xDCDD, 0xCD54, 0xB9EB, 0xA862, 0x9AF9, 0x8B70,
            0x8408, 0x9581, 0xA71A, 0xB693, 0xC22C, 0xD3A5, 0xE13E, 0xF0B7,
            0x0840, 0x19C9, 0x2B52, 0x3ADB, 0x4E64, 0x5FED, 0x6D76, 0x7CFF,
            0x9489, 0x8500, 0xB79B, 0xA612, 0xD2AD, 0xC324, 0xF1BF, 0xE036,
            0x18C1, 0x0948, 0x3BD3, 0x2A5A, 0x5EE5, 0x4F6C, 0x7DF7, 0x6C7E,
            0xA50A, 0xB483, 0x8618, 0x9791, 0xE32E, 0xF2A7, 0xC03C, 0xD1B5,
            0x2942, 0x38CB, 0x0A50, 0x1BD9, 0x6F66, 0x7EEF, 0x4C74, 0x5DFD,
            0xB58B, 0xA402, 0x9699, 0x8710, 0xF3AF, 0xE226, 0xD0BD, 0xC134,
            0x39C3, 0x284A, 0x1AD1, 0x0B58, 0x7FE7, 0x6E6E, 0x5CF5, 0x4D7C,
            0xC60C, 0xD785, 0xE51E, 0xF497, 0x8028, 0x91A1, 0xA33A, 0xB2B3,
            0x4A44, 0x5BCD, 0x6956, 0x78DF, 0x0C60, 0x1DE9, 0x2F72, 0x3EFB,
            0xD68D, 0xC704, 0xF59F, 0xE416, 0x90A9, 0x8120, 0xB3BB, 0xA232,
            0x5AC5, 0x4B4C, 0x79D7, 0x685E, 0x1CE1, 0x0D68, 0x3FF3, 0x2E7A,
            0xE70E, 0xF687, 0xC41C, 0xD595, 0xA12A, 0xB0A3, 0x8238, 0x93B1,
            0x6B46, 0x7ACF, 0x4854, 0x59DD, 0x2D62, 0x3CEB, 0x0E70, 0x1FF9,
            0xF78F, 0xE606, 0xD49D, 0xC514, 0xB1AB, 0xA022, 0x92B9, 0x8330,
            0x7BC7, 0x6A4E, 0x58D5, 0x495C, 0x3DE3, 0x2C6A, 0x1EF1, 0x0F78
            ]

    def decode_data(self, data_packet) -> dict:
        pass

    def decode_1st_hand_shake(self, byte_string):
        """
        The initial handshake sent by the device to start packet transmission to the server.
        """
        
        byte_string = byte_string.decode('utf-8').upper()
        byte_string = byte_string.strip().replace(' ', '')

        packet = {}
        packet['Start Bit'] = byte_string[:4]
        packet['Packet Length'] = byte_string[4:6]
        packet['Protocol Number'] = byte_string[6:8]
        packet['Terminal ID'] = byte_string[8:24]
        packet['Information Serial Number'] = byte_string[24:28]
        packet['Error Check'] = byte_string[28:32]
        packet['Stop Bit'] = byte_string[32:]

        return packet

    def construct_response(self, protocol_number, serial_number, error_check) -> bytes:
        start_bit = '7878'
        packet_length = '05'
        stop_bit = '0D0A'
        response_packet = start_bit + packet_length + protocol_number + serial_number + error_check + stop_bit
        return response_packet.encode('utf-8')

    def handshake_response(self, handshake_1st: dict) -> dict:
        
        # Start bit
        response_packet = "7878"

        # Packet length
        response_packet += str(hex(len(
            handshake_1st['Protocol Number'] + 
            handshake_1st['Terminal ID'] + 
            handshake_1st['Information Serial Number'] + 
            handshake_1st['Error Check']
            ))).replace('0x', '')
        
        # Protocol Number
        response_packet += str(handshake_1st['Protocol Number'])

        # Information Serial Number
        response_packet += str(handshake_1st['Information Serial Number'])

        # Error Check

        # Stop bit
        response_packet += '0D0A'

        response_packet = response_packet.upper()
        return response_packet
    
    def convert_to_hex_byte(self, s):
        return ' '.join([hex(ord(c)).replace('0x', '\\x') for c in s]).encode('utf-8')

    def crc_check(self, data):
        """Calculates the CRC error check value for the given data.

        Args:
            data: The data to calculate the CRC error check value for.

        Returns:
            The CRC error check value.
        """

        # Initialize the CRC register to 0.
        crc_register = 0

        # For each byte in the data, do the following:
        for byte in data:

            # XOR the byte with the CRC register.
            crc_register ^= byte

            # Divide the result by the CRC polynomial.
            crc_register = crc_register % 0x10000

        # The final value of the CRC register is the CRC error check value.
        return crc_register



    

if __name__ == '__main__':
    
    
    decoder = ConcoxDecoder()
    # print(decoder.handshake_response({'Start Bit': '7878', 'Packet Length': '0d', 'Protocol Number': '01', 'Terminal ID': '0358657103213430', 'Information Serial Number': '0004', 'Error Check': 'fe11', 'Stop Bit': '0d0a'}))
    # print(decoder.crc_itu(b'01'))

    crc = decoder.crc_check([0x01, 0x0001])
    print(crc)
    # print(decoder.decode_coordinate('209CCA80'))
    
    
    #############################  Teltonika Tests  ######################################
    tel_decoder = Decoder()
    z = tel_decoder.decode_data("000000000000004308020000016B40D57B480100000000000000000000000000000001010101000000000000016B40D5C198010000000000000000000000000000000101010101000000020000252C")
    print(json.dumps(z, indent=2))
    

    