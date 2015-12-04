#!/usr/bin/env python
#
MongoDBServer = "localhost"
MongoDBPort = 27017
MongoDBDatabase = "LASS"
MongoDBCollection = "posts"

import pymongo
from pymongo import MongoClient
import time


client = MongoClient(MongoDBServer, MongoDBPort)
db = client[MongoDBDatabase]
collection = db[MongoDBCollection]

# to find the first item in the database
collection.find_one()

# to find the first item in the database that is contributed by device FT1_001
collection.find_one({"device_id":"FT1_001"})

# to find the number of instances contributed by the device FT1_001
collection.find({"device_id":"FT1_001"}).count()

# to find the number of instances that are sent by FT1_001 and with a PM2.5 measurement larger than 100
collection.find({"device_id":"FT1_001", "s_d0":{"$gt":100}}).count()

# to find all distince devices that have ever sensed the PM2.5 concentration larger than 100
collection.find({"s_d0": {"$gt": 100}}).distinct("device_id")

# to find all distinct devices, in ascending order, that were ever active on 2015/11/21
sorted(collection.find({"date": "2015-11-21"}).distinct("device_id"))

# to find all distinct devices that were ever active in the period from 2015/11/01 to 2015/11/21
collection.find({"$and": [{"date": {"$gt": "2015-11-01"}},{"date": {"$lt": "2015-11-21"}}]}).distinct("device_id")

# to find the last 100 messages contributed by the device FT1_001
DeviceID = "FT1_001"
items = collection.find({"device_id":DeviceID, "date":{"$lte":time.strftime("%Y-%m-%d")}}).sort([("date",pymongo.DESCENDING),("time",pymongo.DESCENDING)]).limit(100)

for item in items:
  print(str(item["date"]) + "T" + str(item["time"]) + "Z\t" + str(item["s_d0"]))

