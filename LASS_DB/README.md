#Description

This program will do the followings:
  1. to work as a MQTT subscriber of LASS
  2. to convert the MQTT messages into BSON format and insert into MongoDB
  3. to convert the MQTT messages into BSON format and insert into Couchbase
  
**MongoDB**: https://www.mongodb.org  <br>
**Couchbase**: http://www.couchbase.com

#Data Format
	LASS data format version 2.0 (which is the default data format of LASS version 0.7.1+)
	
#Parameters 

You may need to configure the following parameters in order to have the program run on your platform.

  * **MongoDB_SERVER**: the host address of your own MongoDB server
  * **MongoDB_PORT**: the port number of your own MongoDB server
  * **Couchbase_SERVER**: te host address and the data bucket name of your own Couchbase DB

#Requirements:
  * **Paho**: The Paho Python Client provides a client class with support for both MQTT v3.1 and v3.1.1 on Python 2.7 or 3.x. It also provides some helper functions to make publishing one off messages to an MQTT server very straightforward. <br>
	  URL: https://eclipse.org/paho/clients/python/

  * **PyMongo**: PyMongo is a Python distribution containing tools for working with MongoDB, and is the recommended way to work with MongoDB from Python.  <br>
    URL: https://api.mongodb.org/python/current/

  * **Couchbase**: The Python client for Couchbase. <br>
    URL: https://pypi.python.org/pypi/couchbase
    
  * **re**: This module provides regular expression matching operations in Python. <br>
    URL: https://docs.python.org/3/library/re.html
    
  * **json**: JSON encoder and decoder in Python. <br>
    URL: https://docs.python.org/3/library/json.html

#Installation
  * Download MQTT software from http://mosquitto.org
    * untar the package
    * install MQTT (on Mac OS X)
      * cmake .
      * make
      * sudo make install
    * Test MQTT
      * run server: mosquitto &
      * subscript a topic: mosquitto_sub -t MY/TOPIC
      * publish a message (on the other terminal): mosquito_pub -t MY/TOPIC -m 12345
  * Download MongoDB from https://www.mongodb.org
    * for Mac OS 
      * make new directory for the database: sudo mkdir /data
      * make new directory for the database: sudo mkdir /db
      * change the ownership of the directory: chown -R username /data/db
      * run MongoDB:  ./mongod
    * for Ubuntu 14.04
      * sudo apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv 7F0CEB10
      * echo "deb http://repo.mongodb.org/apt/ubuntu trusty/mongodb-org/3.0 multiverse" | sudo tee /etc/apt/sources.list.d/mongodb-org-3.0.list
      * sudo apt-get update
      * sudo apt-get install -y mongodb-org
      * sudo service mongod start
  * Download Couchbase from http://www.couchbase.com
  * Install Python Couchbase module 
    * install libevent from http://libevent.org
      * for Mac OS 
        * download the tar package
        * untar
        * ./configure
        * make
        * sudo make install
      * for Ubuntu 14.04
        * sudo apt-get install libevent-dev
    * install libcouchbase from https://github.com/couchbase/libcouchbase
      * for Mac OS 
        * git clone git://github.com/couchbase/libcouchbase.git
        * cd libcouchbase && mkdir build && cd build
        * ../cmake/configure
        * make
        * sudo make install
      * for Ubuntu 14.04
        * sudo wget -O/etc/apt/sources.list.d/couchbase.list http://packages.couchbase.com/ubuntu/couchbase-ubuntu1404.list
        * wget -O- http://packages.couchbase.com/ubuntu/couchbase.key | sudo apt-key add -
        * sudo apt-get update
        * sudo apt-get install libcouchbase2-libevent libcouchbase-dev
    * install python couchbase module (https://pypi.python.org/pypi/couchbase)
      * Module document: http://pythonhosted.org/couchbase/#
      * sudo pip install git+git://github.com/couchbase/couchbase-python-client
 
         
