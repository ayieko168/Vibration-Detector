import json
import libscrc
import binascii

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
   
   def device_data_decoder(self, packet: str) -> None:
      ## Get the structure of the data packet
      packet_structure = self.decode_packet_structure(packet)
      
      ## Decode the data according to the protocol
      information_bits = packet_structure['information_bits']
      # device_information_packets = {
      #    'longitude' : information_bits[:8],
      #    'latitude' : information_bits[8:16],
      #    'timestamp' : information_bits[16:32],
      #    'satellites' : information_bits[32:34],
      #    'state' : information_bits[34:36],
      #    'batt_voltage' : information_bits[36:40],
      #    'acceleration' : information_bits[40:]
      # }
      
      device_information_packets = {
         "longitude": binascii.hexlify(binascii.unhexlify(information_bits[:8])[::-1]).decode(),
         "latitude": binascii.hexlify(binascii.unhexlify(information_bits[8:16])[::-1]).decode(),
         "timestamp": binascii.hexlify(binascii.unhexlify(information_bits[16:32])[::-1]).decode(),
         "satellites": information_bits[32:34],
         "state": information_bits[34:36],
         "batt_voltage": binascii.hexlify(binascii.unhexlify(information_bits[36:40])[::-1]).decode(),
         "acceleration": binascii.hexlify(binascii.unhexlify(information_bits[40:])[::-1]).decode()
      }
      
      
      print(json.dumps(device_information_packets, indent=2))
      
      ## Convert the packets to actual values
      # device_information = {}
      # for k, v in device_information_packets.items():
      #    device_information[k] = 
   
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








if __name__ == '__main__':
   
   decoder = Decoder()
   
   packet = "eeee17022067ebfffe3bef01941eda60000000000a00c801407e2a38aaaa"
   
   print(
      json.dumps(
         decoder.decode_packet_structure(
            packet, True, True
         ),
         indent=2
      )
   )
   
   print(
      decoder.device_data_decoder(packet)
   )
   
   # print(
   #    decoder.calc_crc(
   #       '39 01 30 33 35 36 33 30 37 30 34 32 34 34 31 30 31 33 67 42 68 6D 53 62 4A 6C 6D 49 48 75 52 62 76 67 78 63 52 61 6A 4A 54 72 51 53 47 6F 5A 6F 5A 71 4A 5A 44 45 50 4E 44 4A'
   #    )
   # )
   
   print(
      decoder.calc_crc(
         decoder.decode_packet_structure(packet).get('information_bits')
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
   
      
      
      