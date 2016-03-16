#!/usr/bin/env python
#
# Version: 0.3.0
#
# Objctive: This program will do the followings:
#	1. work as a MQTT subscriber of LASS
#	2. convert the MQTT messages into BSON format and insert into MongoDB
#	3. convert the MQTT messages into BSON format and insert into Couchbase
#
# Parameters to change:
#	MongoDB_SERVER: the host address of your own MongoDB server
#	MongoDB_PORT: the port number of your own MongoDB server
#	Couchbase_SERVER: the host address and the data bucket name of 
#                         your own Couchbase DB
#
# Input Format:
#	LASS version 0.7.1+ (LASS data format version 2.0+)
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
#
#	Couchbase: Python client for Couchbase
#	           URL: https://pypi.python.org/pypi/couchbase

import paho.mqtt.client as mqtt
import pymongo
import re
import json
import sys
import math
from couchbase.bucket import Bucket

################################################################
# Please configure the following settings for your environment
USE_MongoDB = 1
USE_CouchbaseDB = 0

MQTT_SERVER = "gpssensor.ddns.net"
MQTT_PORT = 1883
MQTT_ALIVE = 60
MQTT_TOPIC = "LASS/AirBox/#"

AirBox_SiteName = "./AirBox/AirBox-SiteName.txt"

MongoDB_SERVER = "localhost"
MongoDB_PORT = 27017
MongoDB_DB = "LASS"

Couchbase_SERVER = "couchbase://localhost/LASS"
################################################################

# Objective: converting GPS coordinates from DMS to DD format
#
# Note that the LASS DB has been changed to DD format since 2016/2/3 11:42am
def dms2dd(dms):
    try:
    	dms = float(dms)
    	d = math.floor(dms)
    	m = ((dms - d) / 60) * 100 * 100
    	s = (m - math.floor(m)) * 100
    	dd = d + math.floor(m) * 0.01 + s * 0.0001
    except:
	print "DMS2DD: Unexpected error:", sys.exc_info()[0]
	dd = 0;
	raise
    return str(dd)

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("MQTT Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe(MQTT_TOPIC)

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload))
    items = re.split('\|',msg.payload)
    lat = "000.000"
    lon = "000.000"
    db_msg = "{"
    flag = 0
    app = 0
    for item in items:
        if item == '':
            continue 
        pairs = re.split('=',item)
	if len(pairs)==1:
            continue
        flag = 1
        if (pairs[0] == "time"):
            LASS_TIME = pairs[1]
        elif (pairs[0] == "date"):
            LASS_DATE = pairs[1]
        elif (pairs[0] == "device_id"):
	    app = 1
            LASS_DEVICE_ID = pairs[1]
	    if pairs[1] in SiteName:
		LASS_SITE_NAME = SiteName[pairs[1]]
	    else:
		LASS_SITE_NAME = pairs[1]
        elif (pairs[0] == "ver_format"):
            if (float(pairs[1])<2.0):
                print("[Error] data format is outdated!")
                return

	if (pairs[0] == "gps_lat" or pairs[0] == "gps-lat"):
	    lat = pairs[1]
	elif (pairs[0] == "gps_lon" or pairs[0] == "gps-lon"):
	    lon = pairs[1]
	else:
            if (num_re_pattern.match(pairs[1])):
                db_msg = db_msg + "\"" + pairs[0] + "\":" + pairs[1] + ",\n"
            else:
                db_msg = db_msg + "\"" + pairs[0] + "\":\"" + pairs[1] + "\",\n"

    db_msg = db_msg + "\"SiteName\":\"" + LASS_SITE_NAME + "\",\n"

    if (flag==0):
        return

#    if (app==1):
#	    lat = dms2dd(lat)
#	    lon = dms2dd(lon)

    if (USE_MongoDB==1):
        mongodb_msg = db_msg + "\"loc\":{\"type\":\"Point\",\"coordinates\":["+ lat + "," + lon + "]}}"
        #print(mongodb_msg)
	try:
        	mongodb_msg = json.loads(mongodb_msg)
	except ValueError:
		print("Exception ValueError: " + db_msg)
	except TypeError:
		print("Exception TypeError: " + db_msg)
	except:
		print "Unexpected error:", sys.exc_info()[0]
		raise
		
	

        mongodb_posts = mongodb_db.posts
	mongodb_latest = mongodb_db.latest
        try:
            db_result = mongodb_posts.insert_one(mongodb_msg)
            print(db_result)
        except pymongo.errors.ServerSelectionTimeoutError:
            print("[ERROR] MongoDB insertion fails for the message: " + msg.payload)

        try:
	    if (app==1):
		mongodb_latest.delete_many({"device_id":LASS_DEVICE_ID})
	    elif (app==2):
		mongodb_latest.delete_many({"SiteName":LASS_DEVICE_ID})
            db_result = mongodb_latest.insert_one(mongodb_msg)
            print(db_result)
        except pymongo.errors.ServerSelectionTimeoutError:
            print("[ERROR] MongoDB insertion fails for the message: " + msg.payload)
	

    if (USE_CouchbaseDB==1):
        couchbase_msg = db_msg + "\"loc\":["+ lat + "," + lon + "]}"
        couchbase_msg = json.loads(couchbase_msg)
        couchbase_key = LASS_DEVICE_ID + "-" + LASS_DATE + "-" + LASS_TIME
        db_result = couchbase_db.set(couchbase_key, couchbase_msg)
        print(db_result)

SiteName = {}
SiteName_File = open(AirBox_SiteName,'r')
for line in SiteName_File:
    items = re.split('\s',line)
    SiteName[items[0]] = items[1]

print SiteName

if (USE_MongoDB==1):
    mongodb_client = pymongo.MongoClient(MongoDB_SERVER, MongoDB_PORT, serverSelectionTimeoutMS=0)
    mongodb_db = mongodb_client[MongoDB_DB]

if (USE_CouchbaseDB==1):
    try:
        couchbase_db = Bucket(Couchbase_SERVER)
    except:
        print("[ERROR] Cannot connect to Couchbase server. Skip the following Couchbase insertions.")
        USE_CouchbaseDB = 0

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
