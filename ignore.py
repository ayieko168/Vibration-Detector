from pprint import pprint
from decoder import Decoder
import json

decoder = Decoder()

# Example 1
packet1 = '00000000000004a7080c000001885bb720200015ffae49ff3df8a6064600ad0b0000001407ef01f0011504c80045010100b30009b50009b60006424c6b430fbf4400000900ae11ffd612000f13fc1102f10000f99e100000d15b020b0000000213fed40b0e00000001000dcf3e000001885bb6e5880015ffae6aff3df852064600ad0b0000001407ef01f0011503c80045010100b30009b5000fb60007424c7d430fc14400000900ae11ffd212001b13fc1802f10000f99e100000d15b020b0000000213fed40b0e00000001000dcf3e000001885bb6ddb80015ffaf32ff3df77a064700a80b000c001407ef01f0011503c80045010100b30009b5000fb60007424c24430fb04400000900ae11ffce12001b13fc1502f10000f99e100000d15b020b0000000213fed40b0e00000001000dcf3e000001885bb6d9d00015ffaf21ff3df7ee064800b30a000b001407ef01f0011503c80045010100b30009b50010b60007424c55430fbc4400000900ae11ffd612001713fc1502f10000f99e100000d15b020b0000000213fed40b0e00000001000dcf3e000001885bb6ca300015ffafc8ff3df884064800bd080007001407ef01f0011503c80045010100b30009b5000bb60009424c95430fc54400000900ae11ffd212001b13fc1802f10000f99e100000d155020b0000000213fed40b0e00000001000dcf3e000001885bb6aed80015ffb00bff3df7ff06480005080007001407ef01f0011503c80045010100b30009b5000bb60009424c57430fba4400000900ae11ffce12001b13fc0d02f10000f99e100000d155020b0000000213fed40b0e00000001000dcf3e000001885bb68f980015ffafa7ff3df6c206490015090000001407ef01f0011504c80045010100b30009b5000ab60007424c63430fbc4400000900ae11ffd212001b13fc1502f10000f99e100000d148020b0000000213fed40b0e00000001000dcf3e000001885bb655000015ffb080ff3df392064d00a80a0006001407ef01f0011504c80045010100b30009b50010b60007424c2e430fb34400000900ae11ffd212001b13fc0d02f10000f99e100000d148020b0000000213fed40b0e00000001000dcf3e000001885bb64d300015ffb090ff3df33e064d009d09000a001407ef01f0011504c80045010100b30009b5000ab60007424c74430fc14400000900ae11ffce12001f13fbf102f10000f99e100000d148020b0000000213fed40b0e00000001000dcf3e000001885bb635c00015ffaf75ff3df726064e00a909000e001407ef01f0011504c80045010100b30009b5000ab60007424c68430fbe4400000900ae11ffd212001f13fc1502f10000f99e100000d13e020b0000000213fed40b0e00000001000dcf3e000001885bb62df00015ffaf43ff3df8b6064e00ba090009001407ef01f0011504c80045010100b30009b50010b60008424c89430fc54400000900ae11ffd212001b13fc1502f10000f99e100000d138020b0000000213fed40b0e00000001000dcf3e000001885bb62a080015ffaf43ff3df92b064d00c6090007001407ef01f0011504c80045010100b30009b5000ab60007424c8e430fc44400000900ae11ffd212001713fc1502f10000f99e100000d138020b0000000213fed40b0e00000001000dcf3e0c00003767'
longitude = b'ff3df852'
# pprint()
print(json.dumps(decoder.decode_data(packet1), indent=2))

with open('example_out.json', 'w') as f:
    avl_ids = json.load(f)

#     for 


