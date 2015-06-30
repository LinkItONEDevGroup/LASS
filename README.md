# LASS
Project Name: 

	Location Aware Sensor System(LASS) by Linkit ONE

Vision: 
	
	We hope location aware sensor data can be easy to provide by anyone, share, and become useful to everyone.

How it works:
	
	Devices (LinkItONE with sensors) sense any data you need ----> 
	Send to gpssensor.ddns.net by MQTT protocol ----> 
	Server (mosquitto server) publish to the subscriber ---->
	Data user subscribe data to get raw data, data with real time chart, export to google map
		Data user have customization capacity to use the data what ever they like

Features:

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

Project status:
	
	Just start. It's expected lots of work need to be done.
		
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

Others:
	
	A lot more informaition in the project WIKI, check it out!
	
Remind:

	We try our best to test the system but we may make mistakes. Used at your own risk.
	
Created 26/06/2015
By Wuulong, https://github.com/LinkItONEDevGroup/LASS.git
