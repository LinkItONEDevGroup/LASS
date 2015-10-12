#Description

This program will do the followings:
  1. to work as a MQTT subscriber of LASS
  2. to convert the MQTT messages into BSON format and insert into MongoDB
  3. to convert the MQTT messages into BSON format and insert into Couchbase
  
**MongoDB**: https://www.mongodb.org  <br>
**Couchbase**: http://www.couchbase.com

#Parameters 

You may need to configure the following parameters in order to have the program run on your platform.

  * **MongoDB_SERVER**: the host address of your own MongoDB server
  * **MongoDB_PORT**: the port number of your own MongoDB server
  * **Couchbase_SERVER**: te host address of your own Couchbase server
  * **Couchbase_PORT**: the port number of your own Couchbase server

#Input Format:
	LASS version 0.7.1+ (LASS data format version 2.0)

#Requirements:
  * **Paho**: The Paho Python Client provides a client class with support for both MQTT v3.1 and v3.1.1 on Python 2.7 or 3.x. It also provides some helper functions to make publishing one off messages to an MQTT server very straightforward.
	  URL: https://eclipse.org/paho/clients/python/

  * **PyMongo**: PyMongo is a Python distribution containing tools for working with MongoDB, and is the recommended way to work with MongoDB from Python. 
    URL: https://api.mongodb.org/python/current/
    
  * **re**: This module provides regular expression matching operations in Python.
    URL: https://docs.python.org/3/library/re.html
    
  * **json**: JSON encoder and decoder in Python.
    URL: https://docs.python.org/3/library/json.html
         
