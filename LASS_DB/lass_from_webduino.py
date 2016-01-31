#!/usr/bin/env python
#
# Objective:
# 	to fetch the latest measurement from http://marty5499.github.io/air-schools/, and upload to LASS MQTT server
#
# Packages Required:
#	sudo pip install requests==1.1.0
#	sudo pip install python-firebase
#	sudo pip install pytz
#	sudo pip install paho-mqtt


from firebase import firebase
import paho.mqtt.client as mqtt
import re
import pytz, datetime

################################################################
# Please configure the following settings for your environment
MQTT_SERVER = "your mqtt server"
MQTT_PORT = 1883
MQTT_ALIVE = 60
MQTT_TOPIC = "LASS/Test/OpenData"

firebase = firebase.FirebaseApplication('https://air-schools.firebaseio.com/', None)
results = firebase.get('/list', None)
devices = results['devices']

################################################################

def on_connect(client, userdata, flags, rc):
    print("MQTT Connected with result code "+str(rc))

def on_publish(mosq, obj, mid):
    print("mid: " + str(mid))

def dd2dms(dd):
    dd = float(dd)
    negative = dd < 0
    dd = abs(dd)
    minutes,seconds = divmod(dd*3600,60)
    degrees,minutes = divmod(minutes,60)
    if negative:
        if degrees > 0:
            degrees = -degrees
        elif minutes > 0:
            minutes = -minutes
        else:
            seconds = -seconds
    return str(degrees + 0.01 * minutes + 0.0001 * seconds)


mqtt_client = mqtt.Client()
mqtt_client.on_connect = on_connect
mqtt_client.on_publish = on_publish
mqtt_client.connect(MQTT_SERVER, MQTT_PORT, MQTT_ALIVE)


for key in devices.iterkeys():

	if 'icon' in devices[key]:
		if (bool(re.search('online',devices[key]['icon']))):
			online = 1
		else:
			online = 0
	else:
		online = 0
	if online==0:
		continue

	msg = "|ver_format=3|fmt_opt=1|app=WEBDUINO_COPY"

	if 'pm10' in devices[key]:
		PM10 = devices[key]['pm10']
	else:
		PM10 = -1
	if 'pm25' in devices[key]:
		PM25 = devices[key]['pm25']
	else:
		PM25 = -1
	if 'lat' in devices[key]:
		gps_lat = str(devices[key]['lat'])
	else:
		gps_lat = "0"
	if 'lng' in devices[key]:
		gps_lon = str(devices[key]['lng'])
	else:
		gps_lon = "0"
	if 'title' in devices[key]:
		#SiteName = devices[key]['title'].encode('utf-8')
		SiteName = devices[key]['title']
	else:
		SiteName = "Null"
	if 'time' in devices[key]:
		publish_time = devices[key]['time']
	else:
		publish_time = "2080-01-01 00:00:00"

	local = pytz.timezone("Asia/Taipei")
	naive = datetime.datetime.strptime(publish_time, "%Y-%m-%d %H:%M:%S")
	local_dt = local.localize(naive, is_dst=None)
	publish_time = local_dt.astimezone(pytz.utc).strftime("%Y-%m-%d %H:%M:%S")
	pairs = publish_time.split(" ")
	msg = msg + "|date=" + pairs[0]
	msg = msg + "|time=" + pairs[1]
	msg = msg + "|PM10=" + PM10
	msg = msg + "|PM2_5=" + PM25
	msg = msg + "|SiteName=" + repr(SiteName)
	msg = msg + "|SiteID=" + key
	msg = msg + "|gps_lat=" + dd2dms(gps_lat)
	msg = msg + "|gps_lon=" + dd2dms(gps_lon)
	
	#msg =  repr(SiteName) + " " + timestamp + " " + PM25 + " " + PM10 + " " + gps_lat + " " + gps_lon + " " + online
	print msg
	mqtt_client.publish(MQTT_TOPIC, msg)

mqtt_client.disconnect()
