#!/usr/bin/env python
#
# Objective:
# 	Fetch open data from Taiwan EPA, and publish the measurement data
#	to the MQTT of LASS
#
# Note:
#	This source code is for reference only. Please do NOT run it unless
#	you change the MQTT server settings (we don't want to have too many
#	duplicate messages on LASS server :p)
#
# Requirements:
# 	Paho: The Paho Python Client provides a client class with support 
#	      for both MQTT v3.1 and v3.1.1 on Python 2.7 or 3.x. It also 
#	      provides some helper functions to make publishing one off 
#	      messages to an MQTT server very straightforward.
#	      URL: https://eclipse.org/paho/clients/python/

import paho.mqtt.client as mqtt
import re
import json
import requests
import pytz, datetime
import sys

################################################################
# Please configure the following settings for your environment
#MQTT_SERVER = "your mqtt server"
MQTT_PORT = 1883
MQTT_ALIVE = 60
MQTT_TOPIC = "LASS/Test/OpenData"

URL_DATA = "http://opendata.epa.gov.tw/ws/Data/REWXQA/"
PARA_DATA = dict(
	orderby = 'SiteName',
	skip = '0',
	top = '1000',
	format = 'json'
)
URL_SITE = "http://opendata.epa.gov.tw/ws/Data/AQXSite/"
PARA_SITE = dict(
	orderby = 'SiteName',
	skip = '0',
	top = '1000',
	format = 'json'
)

################################################################

def on_connect(client, userdata, flags, rc):
    print("MQTT Connected with result code "+str(rc))

def on_publish(mosq, obj, mid):
    print("mid: " + str(mid))


mqtt_client = mqtt.Client()
mqtt_client.on_connect = on_connect
mqtt_client.on_publish = on_publish
mqtt_client.connect(MQTT_SERVER, MQTT_PORT, MQTT_ALIVE)

resp = requests.get(url=URL_SITE, params=PARA_SITE)
array_site = json.loads(resp.text)

resp = requests.get(url=URL_DATA, params=PARA_DATA)
array_data = json.loads(resp.text)

for item in array_data:
	lat = 0
	lon = 0
	msg = "|ver_format=3|fmt_opt=1|app=EPA_COPY"
	publish_time = item['PublishTime']
	local = pytz.timezone("Asia/Taipei")
	naive = datetime.datetime.strptime(publish_time, "%Y-%m-%d %H:%M")
	local_dt = local.localize(naive, is_dst=None)
	publish_time = local_dt.astimezone(pytz.utc).strftime("%Y-%m-%d %H:%M:%S")
	pairs = publish_time.split(" ")
	msg = msg + "|date=" + pairs[0]
	msg = msg + "|time=" + pairs[1]
	for site in array_site:
		if (item['SiteName']==site['SiteName']):
			lat = site['TWD97Lat']
			lon = site['TWD97Lon']
			item['gps_lat'] = site['TWD97Lat']
			item['gps_lon'] = site['TWD97Lon']
			item['SiteEngName'] = site['SiteEngName']
			item['SiteType'] = site['SiteType']
			break;
	for key in item.iterkeys():
		msg = msg + "|" + key.replace(".","_") + "=" + item[key]

	mqtt_client.publish(MQTT_TOPIC, msg)

mqtt_client.disconnect()	
	
