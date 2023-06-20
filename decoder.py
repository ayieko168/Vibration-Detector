
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
        pass
    
    def location_decoder(self, data_packet: bytes) -> dict:
        """
        Decode infromation from the location packet.
        
        Returns: Dictionary with decoded infromation.
        
        Args: data_packet: The hexbyte data packet containing the location infromed fro the protocol number: 12
        """
        
        ## Clean the data packet
        # data_packet = data_packet.decode('utf-8').upper()
        # data_packet = data_packet.strip().replace(' ', '')
        
        ## Deconstruct the data packet to its component bytes
        packet_structure = self.decode_packet_format(data_packet)
        # print(json.dumps(packet_structure, indent=2))
        
        ### Split the infromation packet to extract the data it contains
        
        ## GSP Infromation
        gps_packet = packet_structure.get('information_content')[:36]
        # Date Time
        date_time = self.decode_date_time(gps_packet[:12])
        # Satelites Count
        satelites = int(gps_packet[12:14][1])
        # Latitude
        latitude = self.decode_coordinate(gps_packet[14:22])
        # Longitude
        longitude = self.decode_coordinate(gps_packet[22:30])
        # Speed
        speed = int(gps_packet[30:32], 16)
        # Direction Course and GPS Status
        course, status = self.decode_course_status(gps_packet[32:36])
        
        ## LBS Information
        lbs_packet = packet_structure.get('information_content')[36:]
        # Mobile Country Code (MCC) - 2 bytes
        mobile_country_code = int(lbs_packet[:4], 16)
        # Mobile Network Code (MNC) - 1 byte
        mobile_network_code = int(lbs_packet[4:6], 16)
        # Location Area Code (LAC) - 2 bytes
        location_area_code = int(lbs_packet[6:10], 16)
        # Cell Tower ID (Cell ID) - 3 bytes
        cell_tower_id = int(lbs_packet[10:], 16)
        
        decoded_data = {
            'protocol_number': packet_structure.get('protocol_number'),
            'date_time' : date_time,
            'satelites' : satelites,
            'latitude' : latitude,
            'longitude' : longitude,
            'speed' : speed,
            'course' : course,
            'status' : status,
            'mobile_country_code' : mobile_country_code,
            'mobile_network_code' : mobile_network_code,
            'location_area_code' : location_area_code,
            'cell_tower_id' : cell_tower_id,
        }
        
        return decoded_data
        

    def decode_packet_format(self, packet: bytes) -> dict:
        """
        Function to split the data packet sent by the device for easy parsing.
        Splits the data usingnthe general format of sent data packet.
        """
        
        # print(packet)
        
        packet = packet.decode('utf-8').upper()
        hexbytes = packet.strip().replace(' ', '')
        
        split_dict = {
            "start_bit": hexbytes[:4].upper(),
            "packet_length": hexbytes[4:6].upper(),
            "protocol_number": hexbytes[6:8].upper(),
            "information_content": hexbytes[8:-12].upper(),
            "information_serial_number": hexbytes[-12:-8].upper(),
            "error_check": hexbytes[-8:-4].upper(),
            "stop_bit": hexbytes[-4:].upper()
        }

        return split_dict

    def construct_response(self, protocol_number, serial_number) -> bytes:
        start_bit = '7878'
        packet_length = '05'
        stop_bit = '0D0A'
        error_check = self.calc_crc(packet_length+protocol_number+serial_number)
        response_packet = start_bit + packet_length + protocol_number + serial_number + error_check + stop_bit
        return bytes.fromhex(response_packet)

    def calc_crc(self, data):
        """Calculates the CRC 16 error check value for the given data. Uses CRC 16 x25

        Args:
            data: The data to calculate the CRC error check value for. hex string
            EXAMPLE: '0D0101234567890123450001'
        Returns:
            The CRC error check value.
            EXAMPLE: '8CDD'
        """

        return hex(libscrc.x25(bytes.fromhex(data)))[2:].zfill(4).upper()

    def decode_date_time(self, date_time_packet: str) -> str:
        """
        Convert a data packet containing the date and time to human readable format.
        Output is a date-time string in the format:: YYYY-MM-DD HH:mm:ss
        """
        
        year = int(date_time_packet[0:2], 16) + 2000
        month = int(date_time_packet[2:4], 16)
        day = int(date_time_packet[4:6], 16)
        hour = int(date_time_packet[6:8], 16)
        minute = int(date_time_packet[8:10], 16)
        second = int(date_time_packet[10:12], 16)

        datetime_str = f"{year:04d}-{month:02d}-{day:02d} {hour:02d}:{minute:02d}:{second:02d}"
        return datetime_str

    def decode_coordinate(self, coordinate_hex_packet: str):
        
        decimal_value = int(coordinate_hex_packet, 16)
        decimal_seconds = decimal_value / 30000
        degrees = int(decimal_seconds // 60)
        minutes = decimal_seconds % 60
        return degrees + (minutes / 60)
    
    def decode_course_status(self, hex_value):
        
        binary_value = bin(int(hex_value, 16))[2:].zfill(16)
                
        course_binary = binary_value[6:]
        course = int(course_binary, 2)
        
        print(course)

        status = {
            'real_time_GPS': bool(int(binary_value[2])),
            'GPS_tracking': bool(int(binary_value[3])),
            'longitude_direction': '+' if int(binary_value[4]) == 0 else '-',
            'latitude_direction': '+' if int(binary_value[5]) == 1 else '-',
        }

        return course, status

        

if __name__ == '__main__':
    
    
    decoder = ConcoxDecoder()

    crc = decoder.location_decoder(b'78 78 1f 12 17 06 14 12 12 1c c6 00 22 ed 0b 03 f5 b5 a0 00 10 d3 02 7f 02 0f a8 00 0c e9 00 27 8a 30 0d 0a')
    # crc = decoder.decode_course_status(b'154c')
    # print(crc)
    # print(decoder.decode_coordinate('209CCA80'))
    
    
    #############################  Teltonika Tests  ######################################
    # tel_decoder = Decoder()
    # z = tel_decoder.decode_data("000000000000004308020000016B40D57B480100000000000000000000000000000001010101000000000000016B40D5C198010000000000000000000000000000000101010101000000020000252C")
    # print(json.dumps(z, indent=2))
    

    