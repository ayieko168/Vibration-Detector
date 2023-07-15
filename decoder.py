import json
import libscrc
import binascii
import struct

class Decoder:
   
   def __init__(self) -> None:
      """
      CodecKT1 Decoder
      """
   
   
   def decode_packet_structure(self, packet: str, padded: bool = False, print_length: bool = False) -> dict:
      """Decode the structure of a packet according to the CodecKT1 protocol.
      The protocol is as follows:
      - Start Bit: 2.0 bytes.
      - Packet Length: 1.0 bytes.
      - Protocol Number: 1.0 bytes.
      - Information Bits: 56.0 bytes.
      - Error Ckeck: 2.0 bytes.
      - Stop Bit: 2.0 bytes.

      Args:
         packet (str): A string with the hex packet value to be decoded.
         padded (bool): If true, will add a whitespace between each byte.

      Returns:
         dict: A dictiobnary with the deconstructed packet structure. The keys are the structure part, the value is the part of the packet of the structure.
      """
      
      ## Clean the packet
      packet = packet.upper().strip().replace(' ', '')
      
      ## The packet structure
      structure = {
         'start_bit': packet[:4],
         'packet_length': packet[4:6],
         'protocol_number': packet[6:8],
         'information_bits': packet[8:-8],
         'error_ckeck': packet[-8:-4],
         'stop_bit': packet[-4:]
      }
      
      ## Print the length if requested
      if print_length:
         for k, v in structure.items():
            print(f"- {k.title().replace('_', ' ')}: {len(v)/2} bytes.") 
      
      ## Add padding if needed
      if padded:
         for k, v in structure.items():
            new_value = ' '.join(v[i:i+2] for i in range(0, len(v), 2))
            structure[k] = new_value
      
      ## return the results
      return structure
   
   def login_packet_decoder(self, packet: str) -> list[bytes]:
      
      ## Get the structure of the data packet
      packet_structure = self.decode_packet_structure(packet)
      
      ## Decode the data according to the protocol
      device_imei_packet = packet_structure['information_bits'][:32]
      device_id_packet = packet_structure['information_bits'][32:]
      
      ## Decode the hex to ascii
      device_imei = b"".fromhex(device_imei_packet)
      device_id = b"".fromhex(device_id_packet)
      
      # print(device_imei, device_id)
      return device_imei, device_id
   
   def device_data_decoder(self, packet: str, print_bits: bool = False) -> dict:
      ## Get the structure of the data packet
      packet_structure = self.decode_packet_structure(packet)
      
      ## Decode the data according to the protocol
      information_bits = packet_structure['information_bits']
      device_information_packets = {
         'longitude' : information_bits[:8],
         'latitude' : information_bits[8:16],
         'timestamp' : information_bits[16:32],
         'satellites' : information_bits[32:34],
         'state' : information_bits[34:36],
         'batt_voltage' : information_bits[36:40],
         'acceleration' : information_bits[40:]
      }
      
      if print_bits:
         print(json.dumps(device_information_packets, indent=2))
      
      ## Decode to decimal
      device_information = {}
      for k, v in device_information_packets.items():
         if k == 'longitude' or k == 'latitude':
            device_information[k] = struct.unpack('!f', bytes.fromhex(v))[0]
         elif k == 'timestamp':
            device_information[k] = struct.unpack('!Q', bytes.fromhex(v))[0]
         elif k == 'satellites' or k == 'state':
            device_information[k] = struct.unpack('!B', bytes.fromhex(v))[0]
         elif k == 'batt_voltage' or k == 'acceleration':
            device_information[k] = struct.unpack('!H', bytes.fromhex(v))[0]
         else:
            device_information[k] = int(v)
      
      return device_information

   def calc_crc(self, data: str, padded: bool = False) -> str:
      """Calculates the CRC 16 error check value for the given data. Uses CRC 16 x25

      Args:
         data: The data to calculate the CRC error check value for. hex string
         EXAMPLE: '0D0101234567890123450001'
      Returns:
         The CRC error check value.
         EXAMPLE: '8CDD'
      """
      
      ## Clean the packet
      data = data.upper().strip().replace(' ', '')
      
      if padded:
         v = hex(libscrc.x25(bytes.fromhex(data)))[2:].zfill(4).upper()
         return ' '.join(v[i:i+2] for i in range(0, len(v), 2))

      return hex(libscrc.x25(bytes.fromhex(data)))[2:].zfill(4).upper()

   def createDeviceDataPacket(self, longitude: float, latitude: float, timestamp: int, satellites: int, acceleration: int, state: int, battVoltage: int):
      
      packet_hex_string = b""
      
      ## Start bit
      packet_hex_string += b"eeee"
      
      ## Packet Lenght
      packet_hex_string += b"ZZ"
      
      ## Protocol Number
      packet_hex_string += b"02"
      
      ## Information bit
      # Longitude
      packet_hex_string += struct.pack('!f', longitude).hex().upper().encode('utf-8')
      # Latitude
      packet_hex_string += struct.pack('!f', latitude).hex().upper().encode('utf-8')
      # Timestamp
      packet_hex_string += struct.pack('!Q', timestamp).hex().upper().encode('utf-8')
      # Satelittes
      packet_hex_string += struct.pack('!B', satellites).hex().upper().encode('utf-8')
      # State
      packet_hex_string += struct.pack('!B', state).hex().upper().encode('utf-8')
      # Battery Voltage
      packet_hex_string += struct.pack('!H', battVoltage).hex().upper().encode('utf-8')
      # Acceleration
      packet_hex_string += struct.pack('!H', acceleration).hex().upper().encode('utf-8')
      
      ## Recalculate the Packet Lenght
      info_packet = packet_hex_string[8:].decode('utf-8')
      packet_hex_string = packet_hex_string.replace(b'ZZ', struct.pack('!B', int(len(info_packet)/2)).hex().upper().encode('utf-8'))
      ## Error Check
      packet_hex_string += self.calc_crc(info_packet).encode('utf-8')
      
      ## Stop bit
      packet_hex_string += b"aaaa"


      return packet_hex_string.upper()

   


if __name__ == '__main__':
   
   decoder = Decoder()
   
   packet = "EEEE400137323336353538313437313338343800667A7863305747306C56416A3641546A6A58324139564E4673664A3471484C6F474275664252424801E6AAAA"
      
   # print(decoder.createDeviceDataPacket(-1.349856, 32.455678, 1689359519, 10, 512, 1, 3600))
   # print(decoder.createDeviceDataPacket(-1.34987856, 45.78, 1689359675, 1, 3781, 76, 12))
   
   print(
      json.dumps(
         decoder.decode_packet_structure(
            packet, True, True
         ),
         indent=2
      )
   )
   
   # print(
   #    json.dumps(
   #       decoder.device_data_decoder(packet),
   #       indent=2
   #    )
   # )
   
   # print(
   #    decoder.calc_crc(
   #       '32323431303137373738373938303434506564524D444C6E79686D31374743557A62676E46456A4F474F564B4A33636D48354E4977513667'
   #    )
   # )
   
   
   print(
      decoder.calc_crc(
         decoder.decode_packet_structure(packet).get('information_bits')
      )
   )
   
   # print(
   #    decoder.login_packet_decoder(
   #       'eeee3901303335363330373034323434313031336742686d53624a6c6d49487552627667786652616a4a54725153476f5a6f5a714a5a4445504e5a68b89eaaaa'
   #    )
   # )
   
   print(
      decoder.login_packet_decoder(
         packet
      )
   )
      
   # import random, string
   # values = ["".join(random.choices(string.hexdigits, k=random.randrange(30, 70, 2))).upper() for _ in range(10)]
   
   # for i in  values:
   #    print(f"\"{i}\",")
   
   # print("\n\n\n")
   
   # for i in values:
   #    print(f"{i} ==> {decoder.calc_crc(i)}")
   
   # print(
   #    decoder.calc_crc(
   #       '097a98'
   #    )
   # )
   
      
      
      