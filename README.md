# LASS
Project Name: Location Aware Sensor System(LASS) by Linkit ONE

Vision: We hope location aware sensor data can be easy to provide by anyone, share, and become useful to everyone.

How it works:
	LinkItONE(with sensors) ----> Send to gpssensor.ddns.net by MQTT ----> Server publish to the subscriber 

Features:

	V0.01
		Devices send sensors value with GPS related information to server
		The server publish the information to subscribers.
		The device log data to flash when network fail at startup stage.
		The log data will be send when network OK at startup stage.
	
Project status:
	Just start. It's expected not stable and ready.
		
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

Created 26/06/2015
By Wuulong
https://github.com/LinkItONEDevGroup/LASS.git
