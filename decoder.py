
import struct
import binascii
from pprint import pprint
import json

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
        # Convert hex string to integer
        coordinate_int = int(coordinate_hex, 16)

        # Apply the coordinate transformation formula
        coordinate_int = coordinate_int / 10000000
        # coordinate_int = coordinate_int + (coordinate_int /)

        return coordinate_int

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
        pass

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


if __name__ == '__main__':
    decoder = ConcoxDecoder()
    print(decoder.handshake_response({'Start Bit': '7878', 'Packet Length': '0d', 'Protocol Number': '01', 'Terminal ID': '0358657103213430', 'Information Serial Number': '0004', 'Error Check': 'fe11', 'Stop Bit': '0d0a'}))


    # print(decoder.decode_coordinate('209CCA80'))
