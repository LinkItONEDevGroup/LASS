# LASS
Project Name: 

	Location Aware Sensor System(LASS) by Linkit ONE

Vision: 
	
	We hope location aware sensor devices can be easy to provide/build by anyone.
	The sensors data can be manage globally.
	Any sensors data can be display, get, analyze by everyone.
	These sensors data become useful to everyone.
	Then we can monitor our environment globally.
	

Features:
	
	
	Sensor devices can be easy to buy parts and build by Maker
	Open source architecture to support the community
	All sensors result include gps location information
	The sensors data send to internet, and can be manage by any PCs
	Architecture design to support sensor customization
	Visual sensors data is easy through the tool we provided.
	Support sensors data visualize on the google map.
	Support export function, data can be easy analyze through perfessional statistic software R
	Mobile phone UI supported by integrated with Blynk
	Default support more and more sensors.

[Presentation slides](https://github.com/LinkItONEDevGroup/LASS/blob/master/Doc/LASS-AllandFuture.pdf)

Contents in [wiki](https://github.com/LinkItONEDevGroup/LASS/wiki):

	0.2 We need your help
	0.3 Device Status
	0.4 Supported sensor status
	1.0 How to get started
	1.1 How to setup the sensor device
	1.2 How to analyse sensor data?
	1.3 How to customize your sensors
	1.4 How to customize your alarms
	2.1 User Data Format Specification
	2.3 Shell requirement
	3.1 Data presentation CLI manual
	4.1 Make a wish
	4.2 Request donation of default supported sensors

Contents in [hackpad](https://lass.hackpad.com/LASS-README-DtZ5T6DXLbu) : workspace for contributors and users

	Contributors Area ( Only contributors have write permission )
	LASS - Contributors List
	LASS - Events
	LASS - Sensors Integration status
	LASS - Data specification
	LASS - Sensors data specification
	LASS - FAQ
	
	Users Area ( Anyone have write permission )
	LASS - Sensor customization service
	LASS - Referenced projects
	LASS - Use case sharing
	LASS - Need help
	LASS - User suggestion
	LASS - Make a wish
	LASS - Scratch pad area

Top level architecture:
![LASS-TopLevelArch](https://github.com/LinkItONEDevGroup/LASS/blob/master/Doc/LASS-TopLevelArch.png)

	Devices (LinkItONE with sensors) sense any data you need ----> 
	Send to gpssensor.ddns.net by MQTT protocol ----> 
	Server (mosquitto server) publish to the subscriber ---->
	Data user subscribe data to get raw data, data with real time chart, export to google map
		Data user have customization capacity to use the data what ever they like
		

Device architecture:	
![LASS-DeviceArch](https://github.com/LinkItONEDevGroup/LASS/blob/master/Doc/LASS-DeviceArch.png)

Change Logs:

	V0.1 
		Devices send sensors value with GPS related information to server
		The server publish the information to subscribers.
		The device log data to flash when network fail at startup stage.
		The log data will be send when network OK at startup stage.
	
	V0.2
		Front End support: 
		
		Plot sensor data in realtime.
		Support device_id filter.
		CLI(Command line interface) support
		User setting can be adjust through CLI
		Support export to KML
		Use google map map to show it. 	
	
	V0.3
		Device:
		
		Solar charger supported and tested.
		User have more configable options
		Auto power saving mode support
		Wifi will get re-connected once it become available
		More organized code for sensor customization
		Architeture support more sensors at the same time.
		Data include date time from GPS information
		Logged data will send out once wifi get connected
		Monitor battery status to switch power mode
		MQTT organize by application scenario
		
		Front end support:
		
		Auto log support
		Data save/load supported
		Data support filter by date time
		Data date time by gps information support
	
	V0.4
		Device:
		
		Start to support dust sensor, UV sensor ( basic calibrated ) 
		Extend MQTT library buffer setting ( PubSubClient need update )
		
		Frond end support;
		Auto save data, Auto monitor
		GPS KML export value more reasonable.
		
		
	V0.4.5
		Example:
		
		Make one example to support [Blynk] (http://www.blynk.cc/), check Blynk directory.
			The example not integrate with LASS.
		
	V0.5
		Device:
		
		LASS support Blynk as mobile phone GUI to read all sensors data.

	V0.5.1 - V0.5.2 
		Frond end:
		
			Fake data generation for data analyze purpose.
			CVS export supported
			R example for data import and google plot 
	V0.6
		Device:
		
			Alarm supported include 3 alarm mode (self detect, central, partner )
			
	V0.6.1
		Frontend:
			GPS altitude information supported in CSV, KML (google map, google earth tested)


Project status:
	
	It's working although we still have lots of idea to do. 
		
Usage procedure:
	
	ViewData: 
		mosquitto_sub -v -h gpssensor.ddns.net -t Sensors/#
		You may need to install the tool from here: http://mosquitto.org/download/
		
	LinkItONE: (you can skip this step if you only want to view the current data)
		Install the libraries ( all in Device_LinkItOne/Libraries )
		Modify user configuration in the code ( Search keyword "REPLACE" )
		Download firmware to LinkItONE
		User Interface:
			the console output use 115200N81
			System LED: 
				On   - Wifi/GPS ready, online publishing
				Off  - Wifi or GPS not ready, offline logging
				Blink- Sensing
        
	Original:
	    The idea come from here: http://iot-hackseries.s3-website-us-west-2.amazonaws.com/linkitone-setup.html

	
Remind:

	We try our best to test the system but we may make mistakes. Used at your own risk.
	
Created 26/06/2015
By Wuulong, https://github.com/LinkItONEDevGroup/LASS.git
