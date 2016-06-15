#!/usr/bin/env python
#
# Version: 0.2.0
#
# Objctive: This program will do the followings:
#	1. work as a MQTT subscriber for the PM25 version of LASS
#	2. submit the MQTT content to ThingSpeak.com
#
# To Run:
#	chmod 755 lass_pm25_to_thingspeak.py
#	./lass_pm25_to_thingspeak.py LASS_DEVICE_ID ThingSpeak_API_KEY
#
#   where LASS_DEVICE_ID is the device_id of the LASS node, and
#         ThingSpeak_API_Key is the API Key of the ThingSpeak channel
#
# Input Format:
#	LASS version 0.7.5 (LASS data format version 3.0)
#
# ThingSpeak.com Setting:
#      1. create a channel
#      2. configure field1 for PM25
#      3. configure field2 for temperature
#      4. configure field3 for humidity
#      5. configure field4 for battery level
#      6. c/p the API key for this program
#
# Python Module Requirements:
# 	Paho: The Paho Python Client provides a client class with support 
#	      for both MQTT v3.1 and v3.1.1 on Python 2.7 or 3.x. It also 
#	      provides some helper functions to make publishing one off 
#	      messages to an MQTT server very straightforward.
#	      URL: https://eclipse.org/paho/clients/python/
#	To install Paho, simply type 'pip install paho-mqtt' on your machine
#
# Note:
#     1. To subscribe a specific topic, you can change "MQTT_TOPIC" in the following settings
#

import paho.mqtt.client as mqtt
import re
import httplib, urllib
import socket
import sys
import time

################################################################
# Please configure the following settings for your environment

MQTT_SERVER = "gpssensor.ddns.net"
MQTT_PORT = 1883
MQTT_ALIVE = 60
MQTT_TOPIC = "LASS/Test/#"
################################################################

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("MQTT Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe(MQTT_TOPIC)

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    #print("mqtt payload=%s" %(msg.payload))
    items = re.split('\|',str(msg.payload))
    for item in items:
        if item == '':
            continue 
        pairs = re.split('=',item)
        if (len(items)==1):
            continue
        if (pairs[0] == "device_id"):
            value_devId = pairs[1]
        elif (pairs[0] == "s_d0"):
            value_dust = pairs[1]
        elif (pairs[0] == "s_t0"):
            value_temperature = pairs[1]
        elif (pairs[0] == "s_h0"):
            value_humidity = pairs[1]
        elif (pairs[0] == "s_1"):
            value_battery = pairs[1]

    try:
        if (value_devId == LASS_DEVICE_ID):
            #print "Got the data from %s" % LASS_DEVICE_ID
            params = urllib.urlencode({'field1': value_dust, 'field2': value_temperature, 
                               'field3': value_humidity, 'field4': value_battery, 
                               'key': ThingSpeak_API})
            post_to_thingspeak(params)
    except:
         return

# Post the data to ThingSpeak
def post_to_thingspeak(payload):
    headers = {"Content-type": "application/x-www-form-urlencoded","Accept": "text/plain"}
    not_connected = 1
    while (not_connected):
        try:
            conn = httplib.HTTPConnection("api.thingspeak.com:80")
            conn.connect()
            not_connected = 0
        except (httplib.HTTPException, socket.error) as ex:
            print "Error: %s" % ex
            time.sleep(10)  # sleep 10 seconds

    conn.request("POST", "/update", payload, headers)
    response = conn.getresponse()
    print( response.status, response.reason, payload, time.strftime("%c"))
    data = response.read()
    conn.close()

if len(sys.argv) != 3:
    sys.exit('Usage: %s LASS_DEVICE_ID ThingSpeak_API_Key' % sys.argv[0])

LASS_DEVICE_ID = sys.argv[1]
ThingSpeak_API = sys.argv[2]

mqtt_client = mqtt.Client()
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message

mqtt_client.connect(MQTT_SERVER, MQTT_PORT, MQTT_ALIVE)

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
mqtt_client.loop_forever()
