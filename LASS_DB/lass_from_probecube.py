#!/usr/bin/python

import paho.mqtt.client as mqtt
import paho.mqtt.publish as publish
import json, requests, sys
import time

reload(sys)  
sys.setdefaultencoding('utf-8')

MQTT_SERVER = "gpssensor.ddns.net"
MQTT_PORT = 1883
MQTT_ALIVE = 60
MQTT_TOPIC = "LASS/Test/OpenData"

JSON_SOURCE = "http://g0vairmap.3203.info/Data/ProbeCube_last.json"


################################################################

def on_connect(client, userdata, flags, rc):
    print("MQTT Connected with result code "+str(rc))

def on_publish(mosq, obj, mid):
    print("mid: " + str(mid))

################################################################

mqtt_client = mqtt.Client()
mqtt_client.on_connect = on_connect
mqtt_client.on_publish = on_publish
mqtt_client.connect(MQTT_SERVER, MQTT_PORT, MQTT_ALIVE)

resp = requests.get(url=JSON_SOURCE)
data = json.loads(resp.text)

msgs = []

for items in data:
	msg = {}
	if "created_at" in items["RawData"]:
		timestamp = items["RawData"]["created_at"]
	else:
		continue;
	msg["date"] = timestamp[0:10]
	msg["time"] = timestamp[11:19]
	msg["app"] = "ProbeCube_COPY"
	msg["device_id"] = "PC_" + str(items["Channel_id"])
	msg["SiteName"] = items["SiteName"]
	msg["gps_lat"] = str(items["LatLng"]["lat"])
	msg["gps_lon"] = str(items["LatLng"]["lng"])
	msg["PM2_5"] = items["RawData"]["field5"]
	msg["Temperature"] = items["RawData"]["field1"]
	msg["Humidity"] = items["RawData"]["field2"]
	msg["s_0"] = str(items["RawData"]["entry_id"])
	try:
		s = "|ver_format=3|fmt_opt=1|app=ProbeCube_COPY"

		for key in msg.iterkeys():
			s = s + "|" + key + "=" + msg[key].encode('utf-8')

		#time.sleep(0.2)
		#mqtt_client.publish(MQTT_TOPIC, s)
        	msg2 = {}
        	msg2["topic"] = MQTT_TOPIC
        	msg2["payload"] = s
        	msg2["qos"] = 1
        	msgs.append(msg2)
	except:
		print "Unexpected error:", sys.exc_info()[0]


publish.multiple(msgs, MQTT_SERVER)
mqtt_client.disconnect()
