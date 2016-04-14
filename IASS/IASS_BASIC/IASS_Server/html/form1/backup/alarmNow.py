#! /usr/bin/python

import time
import RPi.GPIO as GPIO

PIN_ALARM = 11 

GPIO.setmode(GPIO.BOARD)

GPIO.setup(PIN_ALARM, GPIO.OUT) # Setup GPIO pin

GPIO.output(PIN_ALARM, True)  #Turn on
time.sleep (5)               #Wait
GPIO.output(PIN_ALARM, False) #Turn off

GPIO.cleanup() #Useful to clear the board
