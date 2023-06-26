
import json

location_data =  """{"protocol_number": "12", "date_time": "2023-06-21 13:48:46", "satelites": 6, "latitude": 1.276905, "longitude": 36.82160666666667, "speed": 0, "course": 133, "status": {"real_time_GPS": false, "GPS_tracking": true, "longitude_direction": "+", "latitude_direction": "-"}, "mobile_country_code": 639, "mobile_network_code": 2, "location_area_code": 2039, "cell_tower_id": 2220}"""
status_data = """{"terminal_info_content": {"Oil and Electricity": "Connected", "GPS Tracking": "On", "Alarm": "Normal", "Charge": "On", "ACC": "Low", "Defense": "Activated"}, "voltage_level": "Very High", "gsm_signal_strength": "Strong Signal", "alarm_language": "No Alarm and Language is Chinese"}"""


print(json.dumps(json.loads(location_data), indent=2))

