#!/usr/bin/env python
#
# Version: 0.2.2
#
# Objctive: This program will do the followings:
#	1. work as a MQTT subscriber for the PM25 version of LASS
#	2. submit the MQTT content to ThingSpeak.com
#   3. a optional method to re-send the MQTT payload to serial for lower application
#
# To Run:
#	chmod 755 lass_pm25_to_thingspeak.py
#	./lass_pm25_to_thingspeak.py LASS_DEVICE_ID ThingSpeak_API_KEY MODE
#
#   where LASS_DEVICE_ID is the device_id of the LASS node, and
#         ThingSpeak_API_Key is the API Key of the ThingSpeak channel
#         MODE 0: Thingspeak Only.  MODE 1:Serial Only  MODE 2:both
# Input Format:
#	LASS version 0.7.5 (LASS data format version 3.0)
#
# ThingSpeak.com Setting:
#      1. create a channel
#      2. configure field1 for PM25
#      3. configure field2 for temperature
#      4. configure field3 for humidity
#      5. configure field4 for PM10
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
import sys
from datetime import timedelta
from datetime import datetime
################################################################
# Please configure the following settings for your environment

MQTT_SERVER = "gpssensor.ddns.net"
MQTT_PORT = 1883
MQTT_ALIVE = 90
MQTT_TOPIC = "LASS/Test/#"
SERIALPORT="/dev/ttyS0"
BUADRATE=57600
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
    flag = 0
    global value_dust
    global value_pm10
    global value_humidity
    global value_temperature
    global LASS_DEVICE_ID
    global str_date
    global str_time
    for item in items:
        if item == '':
            continue 
        pairs = re.split('=',item)
        if (len(items)==1):
            continue
        flag = 1
        if (pairs[0] == "device_id"):
            if (pairs[1] != LASS_DEVICE_ID):
                return
            else:
                print("Got your MQTT channel")
                print(msg.topic +msg.payload+'\r\n')
                #if USE_FRIEND:
                #    s.write(msg.topic +msg.payload+'\n')
        elif (pairs[0] == "s_d0"):
            value_dust = pairs[1]
        elif (pairs[0] == "s_t0"):
            value_temperature = pairs[1]
        elif (pairs[0] == "s_h0"):
            value_humidity = pairs[1]
        elif (pairs[0] == "s_d1"):
            value_pm10 = pairs[1]
        elif (pairs[0] == "date"):
            str_date = pairs[1]
        elif (pairs[0] == "time"):
            str_time = pairs[1]
    if (flag==0):
        return
    if USE_FRIEND:
        strt="date="+str_date+'|time='+str_time
        dt2=datetime.strptime(strt, "date=%Y-%m-%d|time=%H:%M:%S")
        dt2=dt2+timedelta(hours=8, minutes=0)
        dt3s =dt2.strftime("date=%Y-%m-%d|time=%H:%M:%S")
        msg.payload=msg.payload.replace(strt,dt3s)
        s.write(msg.topic +msg.payload+'\n')
        print msg.topic +msg.payload+'\n'
    if USE_TS:
        import httplib, urllib
        import socket
        import time
        
        params = urllib.urlencode({'field1': value_dust, 'field3': value_temperature, 
                                   'field4': value_humidity, 'field2': value_pm10, 
                                   'key': ThingSpeak_API})
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

        conn.request("POST", "/update", params, headers)
        response = conn.getresponse()
        print( response.status, response.reason, params, time.strftime("%c"))
        data = response.read()
        conn.close()

if len(sys.argv) != 4:
    sys.exit('Usage: %s LASS_DEVICE_ID ThingSpeak_API_Key Mode(0:TS/1:Friend/2:Both)' % sys.argv[0])


def main():
    global mqtt_notconnected
    global LASS_DEVICE_ID
    global ThingSpeak_API
    global USE_FRIEND
    global USE_TS
    global s
    from time import sleep
    LASS_DEVICE_ID = sys.argv[1]
    ThingSpeak_API = sys.argv[2]
    ##########MODE SELECT###################
    USE_FRIEND=False
    USE_TS=False
    if sys.argv[3]=='0':
        USE_FRIEND = False
        USE_TS=True
    elif sys.argv[3]=='1':
        USE_FRIEND = True
        USE_TS=False
    elif sys.argv[3]=='2':
        USE_FRIEND=True
        USE_TS=True
    s=None
    if USE_FRIEND:
        import serial
        s = serial.Serial(SERIALPORT,BUADRATE)
    ######END MODE SELECT#######################
    mqtt_client = mqtt.Client()
    mqtt_client.on_connect = on_connect
    mqtt_client.on_message = on_message
    mqtt_notconnected=1
    while(True):
        try:
            if mqtt_notconnected:
                    print "Trying To Connect:"+MQTT_SERVER
                    mqtt_client.connect(MQTT_SERVER, MQTT_PORT, MQTT_ALIVE)
            mqtt_notconnected=0
            mqtt_client.loop_forever()
        except:
           print "Network not exist..wait 5 second"
           sleep(5)

if __name__ == '__main__':
    main()
