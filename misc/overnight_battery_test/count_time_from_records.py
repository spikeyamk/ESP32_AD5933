import os
import json

os.chdir('records')
list = os.listdir()
time_s = 0

for record in list:
    with open(record) as file:
        parsed_content = json.load(file)
        zfreq_corrected_gon = parsed_content.get('measurement_periodic_corrected_gon').get('zfreq_corrected_gon')
        corrected_gon_data = zfreq_corrected_gon[0].get('corrected_gon_data').get('corrected_gon_data')
        first = corrected_gon_data[0]
        last = corrected_gon_data[-1]
        time_s += last.get('unix_timestamp') - first.get('unix_timestamp')

print("time_s ", time_s)
print("time_min ", time_s / 60)
print("time_h ", time_s / (3600))