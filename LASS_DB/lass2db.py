#!/usr/bin/env python
#
# Version: 0.1.0
#
# Objctive: This program will do the followings:
#	1. work as a MQTT subscriber of LASS
#	2. convert the MQTT messages into BSON format and insert into MongoDB
#	3. convert the MQTT messages into BSON format and insert into Couchbase
#
# Parameters to change:
#	MongoDB_SERVER: the host address of your own MongoDB server
#	MongoDB_PORT: the port number of your own MongoDB server
#	Couchbase_SERVER: te host address of your own Couchbase server
#	Couchbase_PORT: the port number of your own Couchbase server
#
# Input Format:
#	LASS version 0.7.1+ (LASS data format version 2.0)
#
# Requirements:
# 	Paho: The Paho Python Client provides a client class with support 
#	      for both MQTT v3.1 and v3.1.1 on Python 2.7 or 3.x. It also 
#	      provides some helper functions to make publishing one off 
#	      messages to an MQTT server very straightforward.
#	      URL: https://eclipse.org/paho/clients/python/
#
#	PyMongo: PyMongo is a Python distribution containing tools for 
#                working with MongoDB, and is the recommended way to work 
#                with MongoDB from Python. 
#	         URL: https://api.mongodb.org/python/current/

import paho.mqtt.client as mqtt
import pymongo
import re
import json

MQTT_SERVER = "gpssensor.ddns.net"
MQTT_PORT = 1883
MQTT_ALIVE = 60

MongoDB_SERVER = "localhost"
MongoDB_PORT = 27017
MongoDB_DB = "LASS"

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe("LASS/Test/+")

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    #print(msg.topic+" "+str(msg.payload))
    items = re.split('\|',msg.payload)
    lat = "000.000"
    lon = "000.000"
    db_msg = "{"
    for item in items:
        if item == '':
            continue 
        pairs = re.split('=',item)
	if (pairs[0] == "gps-lat"):
	    lat = pairs[1]
	elif (pairs[0] == "gps-lon"):
	    lon = pairs[1]
	else:
            if (num_re_pattern.match(pairs[1])):
                db_msg = db_msg + "\"" + pairs[0] + "\":" + pairs[1] + ",\n"
            else:
                db_msg = db_msg + "\"" + pairs[0] + "\":\"" + pairs[1] + "\",\n"
    db_msg = db_msg + "\"loc\":{\"type\":\"Point\",\"coordinates\":["+ lat + "," + lon + "]}}"
    print(db_msg)
    mongodb_posts = mongodb_db.posts
    db_msg = json.loads(db_msg)
    db_result = mongodb_posts.insert_one(db_msg)

mongodb_client = pymongo.MongoClient(MongoDB_SERVER, MongoDB_PORT)
mongodb_db = mongodb_client[MongoDB_DB]

num_re_pattern = re.compile("^-?\d+\.\d+$|^-?\d+$")

mqtt_client = mqtt.Client()
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message

mqtt_client.connect(MQTT_SERVER, MQTT_PORT, MQTT_ALIVE)

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
mqtt_client.loop_forever()
