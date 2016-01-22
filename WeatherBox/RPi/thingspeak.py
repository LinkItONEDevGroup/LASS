#!/usr/bin/env python
# encoding: utf-8

# Log data and upload to ThingSpeak

from time import sleep
import urllib2
import serial
import json
import csv
import time
import socket

THING_SPEAK_API_KEY = ''

CSV_PATH = '/home/pi/weather.csv'
SAVE_INTERVAL = 3  # 3 * 5 = 15s

read_count = 0


def isHTTPOK(host, port=80):
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(1)
        s.connect((host, port))
        s.send("GET / HTTP/1.0\r\n\r\n")
        s.recv(512)
    except Exception, e:
        # print e
        return False

    return True


def main():
    global read_count

    ser = serial.Serial('/dev/ttyUSB0', 9600)

    baseURL = 'https://api.thingspeak.com/update?api_key=%s' % THING_SPEAK_API_KEY

    while True:
        RH = 0
        T = 0
        PM25 = 0
        LUX = 0.0
        UVIndex = 0.0
        lines = ''

        if ser.inWaiting():
            # print ser.readline().replace('\r', '').replace('\n', '')

            try:
                lines = ser.readline()
                # print lines
                modules = json.loads(lines)
                # print json.dumps(modules, indent=4, sort_keys=True)

                value_array = []
                for module in modules:
                    print module

                    for sensor in module['sensors']:
                        value_array.append(sensor['value'])

                    if module['module'] == 'DHT22':
                        for sensor in module['sensors']:
                            if sensor['name'] == 'Humidity':
                                RH = sensor['value']
                            if sensor['name'] == 'Temperature':
                                T = sensor['value']
                    elif module['module'] == 'G3':
                        for sensor in module['sensors']:
                            if sensor['name'] == 'PM 2.5 (std. atmosphere)':
                                PM25 = sensor['value']
                    elif module['module'] == 'GY-30':
                        for sensor in module['sensors']:
                            if sensor['name'] == 'Lux':
                                LUX = sensor['value']
                    elif module['module'] == 'Si1145':
                        for sensor in module['sensors']:
                            if sensor['name'] == 'UV Index':
                                UVIndex = sensor['value']

                if read_count + 1 == SAVE_INTERVAL:
                    value_array.append(time.strftime("%Y-%m-%d %H:%M:%S"))

                    with open(CSV_PATH, 'a') as csvfile:
                        spamwriter = csv.writer(csvfile)
                        spamwriter.writerow(value_array)

                    if isHTTPOK('api.thingspeak.com'):
                        f = urllib2.urlopen(baseURL + 
                                            "&field1=%s&field2=%s&field3=%s&field4=%s&field5=%s" % (RH, T, PM25, LUX, UVIndex))
                        print f.read()
                        f.close()

                    read_count = 0
                else:
                    read_count = read_count + 1

                sleep(1)
            except ValueError, e:
                # print e
                if lines:
                    print lines

# call main
if __name__ == '__main__':
    main()
